#ifndef TBC_PLAYERCOMMS_HPP
#define TBC_PLAYERCOMMS_HPP

#include <functional>
#include <future>
#include <string>

namespace ngl::tbc {
template <typename TCommand, typename TCommandResult>
class PlayerComms {
public:
  using TPayload               = typename TCommand::Payload;
  using TPayloadTypeSet        = typename TCommand::PayloadTypeSet;
  using TRequestCommandHandler = std::function<std::vector<TPayload>(const TPayloadTypeSet &)>;

  PlayerComms(const std::string &name, TRequestCommandHandler static_handler)
      : name_{name},
        static_handler_{static_handler} {}

  [[nodiscard]] std::future<std::vector<TPayload>> RequestCommands(const TPayloadTypeSet &types) { return std::async(std::launch::async, static_handler_, types); }
  void SetCommandHandler(TRequestCommandHandler handler) { static_handler_ = handler; }

protected:
  std::string name_;
  TRequestCommandHandler static_handler_;
};
} // namespace ngl::tbc

#endif