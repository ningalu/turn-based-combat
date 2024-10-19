#ifndef NGL_UTIL_TMP_HPP
#define NGL_UTIL_TMP_HPP

#include <optional>
#include <tuple>
#include <type_traits>

namespace ngl::tmp {

// is_unique
template <typename...>
struct is_one_of;

template <typename F>
struct is_one_of<F> {
  static constexpr bool value = false;
};

template <typename F, typename S, typename... T>
struct is_one_of<F, S, T...> {
  static constexpr bool value = std::is_same<F, S>::value
                                || is_one_of<F, T...>::value;
};

template <typename...>
struct is_unique;

template <>
struct is_unique<> {
  static constexpr bool value = true;
};

template <typename F, typename... T>
struct is_unique<F, T...> {
  static constexpr bool value = is_unique<T...>::value
                                && !is_one_of<F, T...>::value;
};

template <typename... T>
constexpr bool is_unique_v = is_unique<T...>::value;

namespace {
static_assert(is_unique_v<int, double>);
static_assert(!is_unique_v<int, int>);
} // namespace

template <typename T, typename... TTypes>
struct is_present {
  static constexpr bool value{(std::is_same_v<T, TTypes> || ...)};
};

template <typename T, typename... TTypes>
constexpr bool is_present_v = is_present<T, TTypes...>::value;

namespace {
static_assert(is_present_v<int, int, double>);
static_assert(!is_present_v<int, char, double>);
} // namespace

// typeset
template <typename... TTypes>
  requires(is_unique_v<TTypes...>)
class typeset {

  template <typename TType>
  using key = std::optional<std::decay_t<TType> *>;

public:
  typeset() = default;
  typeset(bool on) {
    if (on) {
      insert<TTypes...>();
    }
  }

  // template <typename TType>
  // typeset() {
  //   static_assert(is_present_v<TType, TTypes...>);
  //   insert<TType>();
  // }

  // template <typename... TInitTypes>
  // typeset() {
  //   static_assert(is_unique_v<TInitTypes...>);
  //   static_assert((is_present_v<TTypes, TTypes...> || ...));
  //   insert(TInitTypes...);
  // }

  template <typename TInsertType>
  void insert() {
    static_assert(is_present_v<TInsertType, TTypes...>);
    std::get<key<TInsertType>>(storage_) = nullptr;
  }

  template <typename... TInsertTypes>
    requires(sizeof...(TInsertTypes) > 1)
  void insert() {
    static_assert(is_unique_v<TInsertTypes...>);
    (insert<TInsertTypes>(), ...);
  }

  template <typename TEraseType>
  void erase() {
    static_assert(is_present_v<TEraseType, TTypes...>);
    std::get<key<TEraseType>>(storage_) = std::nullopt;
  }

  template <typename... TEraseTypes>
    requires(sizeof...(TEraseTypes) > 1)
  void erase() {
    static_assert(is_unique_v<TEraseTypes...>);
    (erase(TEraseTypes), ...);
  }

  template <typename T>
  [[nodiscard]] bool contains() const {
    return std::get<key<T>>(storage_) == nullptr;
  }

protected:
  std::tuple<key<TTypes>...> storage_;
};

} // namespace ngl::tmp
#endif