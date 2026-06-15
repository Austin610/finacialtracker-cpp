// Minimal doctest-compatible single-header test framework
// Public API matches doctest v2: TEST_CASE, CHECK, CHECK_THROWS_AS, doctest::Approx
#pragma once
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace doctest {
struct Approx {
  double val; double eps;
  explicit Approx(double v, double e = 1e-9) : val(v), eps(e) {}
};
inline bool operator==(double a, const Approx &b) { return std::abs(a-b.val) <= b.eps; }
inline bool operator==(const Approx &a, double b) { return b == a; }
} // namespace doctest

namespace doctest_impl {
struct TestCase { const char *name; std::function<void()> fn; };
inline std::vector<TestCase> &registry() { static std::vector<TestCase> r; return r; }
struct Reg { Reg(const char *n, std::function<void()> f) { registry().push_back({n,f}); } };
inline int  &fails()       { static int  n = 0; return n; }
inline const char *&curTest() { static const char *s = ""; return s; }
inline void check(bool ok, const char *expr, const char *file, int line) {
  if (ok) return;
  ++fails();
  std::cerr << file << ":" << line << ": FAILED [" << curTest() << "]\n"
            << "  CHECK(" << expr << ")\n";
}
} // namespace doctest_impl

// Each TEST_CASE gets a unique struct type whose constructor registers the test.
// This avoids needing __COUNTER__ to appear twice with the same value.
#define TEST_CASE(name)                                              \
  static void DOCTEST_FN(__LINE__)();                               \
  namespace { struct DOCTEST_ST(__LINE__) {                         \
    DOCTEST_ST(__LINE__)() {                                         \
      ::doctest_impl::registry().push_back({name, DOCTEST_FN(__LINE__)}); \
    }                                                                \
  } DOCTEST_OBJ(__LINE__); }                                        \
  static void DOCTEST_FN(__LINE__)()

#define DOCTEST_FN(l)  DOCTEST_CAT(doctest_fn_, l)
#define DOCTEST_ST(l)  DOCTEST_CAT(doctest_st_, l)
#define DOCTEST_OBJ(l) DOCTEST_CAT(doctest_obj_, l)
#define DOCTEST_CAT(a,b) DOCTEST_CAT2(a,b)
#define DOCTEST_CAT2(a,b) a##b

#define CHECK(expr) \
  ::doctest_impl::check(!!(expr), #expr, __FILE__, __LINE__)

#define CHECK_THROWS_AS(expr, etype)                                    \
  do {                                                                  \
    bool _threw = false;                                                \
    try { (void)(expr); }                                               \
    catch (const etype &) { _threw = true; }                           \
    catch (...) {}                                                      \
    ::doctest_impl::check(_threw,                                       \
      "CHECK_THROWS_AS(" #expr ", " #etype ")", __FILE__, __LINE__);   \
  } while(0)

#ifdef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
int main() {
  int total = 0, failed = 0;
  for (auto &tc : doctest_impl::registry()) {
    doctest_impl::curTest() = tc.name;
    int before = doctest_impl::fails();
    try { tc.fn(); }
    catch (const std::exception &e) {
      ++doctest_impl::fails();
      std::cerr << "  EXCEPTION [" << tc.name << "]: " << e.what() << "\n";
    }
    catch (...) {
      ++doctest_impl::fails();
      std::cerr << "  UNKNOWN EXCEPTION [" << tc.name << "]\n";
    }
    bool ok = (doctest_impl::fails() == before);
    std::cout << (ok ? "[PASSED] " : "[FAILED] ") << tc.name << "\n";
    if (!ok) ++failed;
    ++total;
  }
  std::cout << "\n" << total << " tests, " << failed << " failed, "
            << (total-failed) << " passed\n";
  return failed > 0 ? 1 : 0;
}
#endif
