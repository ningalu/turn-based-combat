#ifndef TBC_PLAYERCOMMS_HPP
#define TBC_PLAYERCOMMS_HPP

#include <functional>
#include <future>
#include <string>

namespace ngl::tbc {
template <typename TPayload>
class PlayerComms {
public:
  PlayerComms(const std::string &name, std::function<std::vector<TPayload>()> static_handler)
      : name_{name}, static_handler_{static_handler} {}

  [[nodiscard]] std::future<std::vector<TPayload>> GetCommands() { return std::async(std::launch::async, static_handler_); }
  void SetCommandHandler(std::function<std::vector<TPayload>()> handler) { static_handler_ = handler; }

protected:
  std::string name_;
  std::function<std::vector<TPayload>()> static_handler_;
};
} // namespace ngl::tbc

#endif