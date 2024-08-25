#ifndef TBC_PLAYERCOMMS_HPP
#define TBC_PLAYERCOMMS_HPP

#include <functional>
#include <future>
#include <string>

namespace ngl::tbc {
template <typename TCommand>
class PlayerComms {
public:
  PlayerComms(const std::string &name, std::function<std::vector<TCommand>()> static_handler, std::function<TCommand(std::size_t)> dynamic_handler)
      : name_{name}, static_handler_{static_handler}, dynamic_handler_{dynamic_handler} {}

  [[nodiscard]] std::future<std::vector<TCommand>> GetStaticCommand() { return std::async(std::launch::async, static_handler_); }
  void SetStaticCommandHandler(std::function<std::vector<TCommand>()> handler) { static_handler_ = handler; }

  [[nodiscard]] std::future<TCommand> GetDynamicCommand(std::size_t slot) { return std::async(std::launch::async, dynamic_handler_, slot); }
  void SetDynamicCommandHandler(std::function<TCommand(std::size_t)> handler) { dynamic_handler_ = handler; }

protected:
  std::string name_;
  std::function<std::vector<TCommand>()> static_handler_;
  std::function<TCommand(std::size_t slot)> dynamic_handler_;
};
} // namespace ngl::tbc

#endif