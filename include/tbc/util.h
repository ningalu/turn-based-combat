#ifndef NGL_UTIL_H
#define NGL_UTIL_H

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
} // namespace ngl::tbc

#endif