#ifndef TBC_UTIL_COMMS_PLAYERCLIENTPROXY_HPP
#define TBC_UTIL_COMMS_PLAYERCLIENTPROXY_HPP

#include <condition_variable>
#include <mutex>
#include <vector>

namespace ngl::tbc::comms {
template <typename TBattle>
class PlayerClientProxy {
  using TCommand           = typename TBattle::Command;
  using TCommandError      = typename TBattle::CommandResult;
  using TCommandResponse   = typename TBattle::TCommandResponse;
  using TCommandPayload    = typename TCommand::Payload;
  using TCommandPayloadSet = typename TCommand::PayloadTypeSet;

public:
  [[nodiscard]] bool NeedsCommands() {
    std::unique_lock<std::mutex> lock(request_mutex_);
    return valid_payloads_.has_value();
  }

  [[nodiscard]] TCommandResponse ProvideCommands(const std::vector<TCommandPayload> &payloads) {
    {
      std::unique_lock<std::mutex> request_lock(request_mutex_);
      request_ = payloads;
      request_ready_.notify_all();
    }

    std::unique_lock<std::mutex> response_lock(response_mutex_);
    response_ready_.wait(response_lock, [this]() {
      return response_.has_value();
    });
    const auto out = response_.value();
    response_      = std::nullopt;
    return out;
  }

  [[nodiscard]] std::vector<TCommandPayload> CommsRequestCallback(const TCommandPayloadSet &valid_payloads, const TBattle &battle) {
    std::unique_lock<std::mutex> lock(request_mutex_);
    valid_payloads_ = valid_payloads;
    request_ready_.wait(lock, [this]() {
      return request_.has_value();
    });

    const auto out  = request_.value();
    request_        = std::nullopt;
    valid_payloads_ = std::nullopt;
    return out;
  }

  void CommsResponseCallback(const TCommandResponse &result) {
    std::unique_lock<std::mutex> lock(response_mutex_);
    response_ = result;
    response_ready_.notify_all();
  }

protected:
  std::mutex request_mutex_;
  std::condition_variable request_ready_;
  std::optional<std::vector<TCommandPayload>> request_;
  std::optional<TCommandPayloadSet> valid_payloads_;

  std::mutex response_mutex_;
  std::condition_variable response_ready_;
  std::optional<TCommandResponse> response_;
};

} // namespace ngl::tbc::comms

#endif