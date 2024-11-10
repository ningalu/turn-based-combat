#ifndef TBC_UTIL_COMMS_PLAYERCLIENTCOMMSADAPTER_HPP
#define TBC_UTIL_COMMS_PLAYERCLIENTCOMMSADAPTER_HPP

#include <vector>

#include "tbc/util/comms/PlayerClientProxy.hpp"

namespace ngl::tbc::comms {
template <typename TBattle>
class PlayerClientCommsAdapter {
  using TCommand           = typename TBattle::Command;
  using TCommandResult     = typename TBattle::CommandResult;
  using TCommandPayload    = typename TCommand::Payload;
  using TCommandPayloadSet = typename TCommand::PayloadTypeSet;
  using TPlayerComms       = typename TBattle::TPlayerComms;

public:
  PlayerClientCommsAdapter(std::size_t players) : proxies{players} {}
  [[nodiscard]] std::vector<TPlayerComms> MakePlayerComms() {
    std::vector<TPlayerComms> out;
    for (auto &proxy : proxies) {
      auto player = TPlayerComms{
        "Proxy Player", // TODO: fix this naming sometime
        [&proxy](const TCommandPayloadSet &valid, const TBattle &battle) {
          return proxy.CommsRequestCallback(valid, battle);
        }
      };
      player.SetResponseHandler(
        [&proxy](const TCommandResult &result) {
          proxy.CommsResponseCallback(result);
        }

      );
      out.push_back(player);
    }
    return out;
  }

  std::vector<PlayerClientProxy<TBattle>> proxies;
};
} // namespace ngl::tbc::comms

#endif