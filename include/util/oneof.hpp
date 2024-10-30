#ifndef NGL_ONEOF_HPP
#define NGL_ONEOF_HPP

#include <stdexcept>
#include <variant>

#include "util/tmp.hpp"

namespace ngl {

class non_comprehensive_match : public std::exception {
  using std::exception::exception;
};

template <typename... TCallable>
struct visit_impl : TCallable... {
  using TCallable::operator()...;
};

template <class... Ts>
visit_impl(Ts...) -> visit_impl<Ts...>;

template <typename TVariant, typename... TCallable>
auto match(const TVariant &v, const TCallable &...f) {
  return std::visit(visit_impl{f...}, v);
}

template <typename TVariant, typename... TCallable>
auto match(TVariant &v, const TCallable &...f) {
  return std::visit(
    visit_impl{
      f...,
      [](auto &&) {
        throw non_comprehensive_match{"unmatched"};
      }
    },
    v
  );
}

template <typename... TUnion>
  requires(tmp::is_unique_v<TUnion...>)
class oneof : std::variant<TUnion...> {

  using Self = std::variant<TUnion...>;

public:
  using Self::Self;
  using Self::operator=;

  template <typename... TMatch>
  auto match(TMatch... f) {
    return std::visit(
      visit_impl{
        f...,
        [](auto &&) {
          throw non_comprehensive_match{"unmatched"};
        }
      },
      static_cast<Self>(*this)
    );
  }

  template <typename TGet>
    requires(tmp::is_present_v<TGet, TUnion...>)
  [[nodiscard]] TGet as() {
    return std::get<TGet>(*this);
  }
};
} // namespace ngl

#endif