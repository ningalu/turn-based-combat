#ifndef NGL_UTIL_TMP_HPP
#define NGL_UTIL_TMP_HPP

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

template <typename T, typename... Ts>
struct is_present {
  static constexpr bool value{(std::is_same_v<T, Ts> || ...)};
};

template <typename T, typename... Ts>
constexpr bool is_present_v = is_present<T, Ts...>::value;

namespace {
static_assert(is_present_v<int, int, double>);
static_assert(!is_present_v<int, char, double>);
} // namespace

// typeset
template <typename... Ts>
  requires(is_unique<Ts...>)
class typeset {

  template <typename T>
  void insert() {
    std::get<T>(storage_) = nullptr;
  }

  template <typename T>
  void erase() {
    std::get<T>(storage_) = std::nullopt;
  }

  template <typename T>
  [[nodiscard]] bool contains() const {
    return std::get<T>(storage_) == nullptr;
  }

public:
protected:
  std::tuple<std::optional<Ts *>...> storage_;
};

} // namespace ngl::tmp
#endif