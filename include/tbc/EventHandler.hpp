#ifndef TBC_EVENTHANDLER_HPP
#define TBC_EVENTHANDLER_HPP

#include "tbc/Event.hpp"

namespace ngl::tbc {
template <typename TEvents>
class EventHandler {
public:
  TEvents::Handlers callbacks;
};
} // namespace ngl::tbc

#endif