#ifndef NGL_UTIL_H
#define NGL_UTIL_H

#include <string>
#include <string_view>
#include <vector>

// TODO: consolidate misc util stuff into a separate project and FetchContent it? this is a pain in the ass
namespace ngl::tbc {

[[noreturn]] inline void unreachable() {
  // Uses compiler specific extensions if possible.
  // Even if no extension is used, undefined behavior is still raised by
  // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
  __assume(false);
#else // GCC, Clang
  __builtin_unreachable();
#endif
}

// https://stackoverflow.com/questions/48012539/idiomatically-split-a-string-view
[[nodiscard]] static inline std::vector<std::string_view> split(std::string_view to_split, char delimiter) {
  std::vector<std::string_view> result;

  int indexCommaToLeftOfColumn  = 0;
  int indexCommaToRightOfColumn = -1;

  for (int i = 0; i < static_cast<int>(to_split.size()); i++) {
    if (to_split[static_cast<std::size_t>(i)] == delimiter) {
      indexCommaToLeftOfColumn  = indexCommaToRightOfColumn;
      indexCommaToRightOfColumn = i;
      int index                 = indexCommaToLeftOfColumn + 1;
      int length                = indexCommaToRightOfColumn - index;

      // Bounds checking can be omitted as logically, this code can never be invoked
      // Try it: put a breakpoint here and run the unit tests.
      /*if (index + length >= static_cast<int>(str.size()))
      {
          length--;
      }
      if (length < 0)
      {
          length = 0;
      }*/

      std::string_view column(to_split.data() + index, static_cast<std::size_t>(length));
      result.push_back(column);
    }
  }
  const std::string_view finalColumn(to_split.data() + indexCommaToRightOfColumn + 1, to_split.size() - static_cast<std::size_t>(indexCommaToRightOfColumn) - 1);
  result.push_back(finalColumn);
  return result;
}

} // namespace ngl::tbc

#endif