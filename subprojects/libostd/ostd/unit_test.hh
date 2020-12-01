/** @defgroup Testing
 *
 * @brief Reusable unit test infrastructure.
 *
 * This module defines header-only infra for unit tests that can be used to
 * integrate tests directly where the implementation is. All you really need
 * to do then is enable a macro, include the file you're testing and compile
 * into an executable.
 *
 * @{
 */

/** @file unit_test.hh
 *
 * @brief The unit test infrastructure implementation.
 *
 * Include this at the very beginning in the files you want to define unit
 * tests in. It has no dependencies within libostd.
 *
 * @copyright See COPYING.md in the project tree for further information.
 */

#ifndef OSTD_UNIT_TEST_HH
#define OSTD_UNIT_TEST_HH

#ifdef OSTD_BUILD_TESTS
#include <cstdio>
#include <cstddef>
#include <utility>
#include <vector>
#include <string>
#include <functional>
#endif

namespace ostd {
namespace test {

/** @addtogroup Testing
 * @{
 */

#if defined(OSTD_BUILD_TESTS) || defined(OSTD_GENERATING_DOC)

#define OSTD_TEST_MODULE_STRINGIFY(x) #x
#define OSTD_TEST_MODULE_STR(x) OSTD_TEST_MODULE_STRINGIFY(x)
#define OSTD_TEST_MODULE_CURRENT OSTD_TEST_MODULE_STR(OSTD_BUILD_TESTS)

namespace detail {
    static inline std::vector<void (*)()> test_cases;

    inline bool add_test(std::string testn, void (*func)()) {
        if (testn == OSTD_TEST_MODULE_CURRENT) {
            test_cases.push_back(func);
        }
        return true;
    }

    struct test_error {};
}

#define OSTD_TEST_FUNC_CONCAT(p, m, l) p##_##m##_##line
#define OSTD_TEST_FUNC_NAME(p, m, l) OSTD_TEST_FUNC_CONCAT(p, m, l)

/** @brief Defines a unit test.
 *
 * The body of the test follows the expansion of this macro like a
 * normal function body.
 *
 * The test is only enabled if the module name matches the value of the
 * `OSTD_BUILD_TESTS` macro, defined before inclusion of this header. The
 * macro also has to be defined for this to be available at all. The module
 * name is defined using the `OSTD_TEST_MODULE` macro, which should be defined
 * after including any headers and undefined at the end of the file.
 */
#define OSTD_UNIT_TEST \
inline void OSTD_TEST_FUNC_NAME(test_func, OSTD_TEST_MODULE, __LINE__)(); \
static inline bool OSTD_TEST_FUNC_NAME(test_case, OSTD_TEST_MODULE, __LINE__) = \
    ostd::test::detail::add_test( \
        OSTD_TEST_MODULE_STR(OSTD_TEST_MODULE), \
        &OSTD_TEST_FUNC_NAME(test_func, OSTD_TEST_MODULE, __LINE__) \
    ); \
inline void OSTD_TEST_FUNC_NAME(test_func, OSTD_TEST_MODULE, __LINE__)()

/** @brief Makes the test fail if the given value is true.
 *
 * Fail in this case means that the test will exit there. Other tests
 * within the same module will still run, and in the end the user will
 * be able to see how many tests of the module succeeded and how many
 * failed.
 *
 * @see ostd::test::fail(), ostd::test::fail_if_not()
 */
inline void fail_if(bool b) {
    if (b) {
        throw detail::test_error{};
    }
}

/** @brief Like ostd::test::fail_if(), but inverse.
 *
 * The test will fail if the given value is false.
 */
inline void fail_if_not(bool b) {
    if (!b) {
        throw detail::test_error{};
    }
}

/** @brief Makes the test fail.
 *
 * The current test will exit.
 *
 * @see ostd::test::fail_if()
 */
[[noreturn]] inline void fail() {
    throw detail::test_error{};
}

/** @brief Runs all enabled test cases.
 *
 * @returns An std::pair containing the number of tests that succeeded
 *          as the first value and failed as the second value.
 */
inline std::pair<std::size_t, std::size_t> run() {
    std::size_t succ = 0, fail = 0;
    for (auto &f: detail::test_cases) {
        try {
            f();
        } catch (detail::test_error) {
            ++fail;
            continue;
        } catch (...) {
            std::printf("warning: uncaught exception");
            ++fail;
            continue;
        }
        ++succ;
    }
    return std::make_pair(succ, fail);
}

#endif /* OSTD_BUILD_TESTS */

/** @} */

} /* namespace test */
} /* namespace ostd */

#endif

/** @} */
