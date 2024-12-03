#ifndef TBC_PLAYERCOMMS_HPP
#define TBC_PLAYERCOMMS_HPP

#include <functional>
#include <future>
#include <string>

#include "tbc/CommandResponse.hpp"
#include "tbc/Log.h"

namespace ngl::tbc {
template <typename TBattle, typename TCommand, typename TCommandError>
class PlayerComms {
public:
  using TPayload                = typename TCommand::Payload;
  using TPayloadTypeSet         = typename TCommand::PayloadTypeSet;
  using TCommandResponse        = CommandResponse<TCommandError>;
  using TRequestCommandHandler  = std::function<std::vector<TPayload>(const TPayloadTypeSet &, const TBattle &)>;
  using TCommandResponseHandler = std::function<void(const TCommandResponse &)>;

  PlayerComms(std::string name, TRequestCommandHandler request_handler)
      : name_{std::move(name)},
        request_handler_{std::move(request_handler)} {}

  void SetCommandHandler(const TRequestCommandHandler &handler) { request_handler_ = handler; }
  [[nodiscard]] std::future<std::vector<TPayload>> RequestCommands(const TPayloadTypeSet &types, const TBattle &battle) {
    return std::async(std::launch::async, request_handler_, types, battle);
  }

  void SetResponseHandler(const TCommandResponseHandler &handler) { response_handler_ = handler; }
  void RespondToCommands(const TCommandResponse &res) const {
    if (response_handler_) {
      response_handler_(res);
    }
  }

  void SetLogHandler(const Log::Handler &handler) { log_handler_ = handler; }
  void PostLog(const std::string &log) {
    if (log_handler_) {
      log_handler_(log);
    }
  }

protected:
  std::string name_;
  TRequestCommandHandler request_handler_;
  TCommandResponseHandler response_handler_;
  Log::Handler log_handler_;
};
} // namespace ngl::tbc

#endif