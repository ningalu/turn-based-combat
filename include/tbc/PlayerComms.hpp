#ifndef TBC_PLAYERCOMMS_HPP
#define TBC_PLAYERCOMMS_HPP

#include <functional>
#include <future>
#include <string>

namespace ngl::tbc {
template <typename TCommand, typename TCommandResult>
class PlayerComms {
public:
  using TPayload                = typename TCommand::Payload;
  using TPayloadTypeSet         = typename TCommand::PayloadTypeSet;
  using TRequestCommandHandler  = std::function<std::vector<TPayload>(const TPayloadTypeSet &)>;
  using TCommandResponseHandler = std::function<void(const TCommandResult &)>;

  PlayerComms(const std::string &name, TRequestCommandHandler request_handler)
      : name_{name},
        request_handler_{request_handler} {}

  void SetCommandHandler(TRequestCommandHandler handler) { request_handler_ = handler; }
  [[nodiscard]] std::future<std::vector<TPayload>> RequestCommands(const TPayloadTypeSet &types) { return std::async(std::launch::async, request_handler_, types); }

  void SetResponseHandler(TCommandResponseHandler handler) { response_handler_ = handler; }
  void RequestCommandsResponse(const TCommandResult &res) const {
    if (response_handler_) {
      response_handler_(res);
    }
  }

protected:
  std::string name_;
  TRequestCommandHandler request_handler_;
  TCommandResponseHandler response_handler_;
};
} // namespace ngl::tbc

#endif