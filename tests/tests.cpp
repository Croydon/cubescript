#include <gtest/gtest.h>

#include <signal.h>

#include <optional>

#include <ostd/platform.hh>
#include <ostd/io.hh>
#include <ostd/string.hh>

#include <cubescript/cubescript.hh>

using namespace cscript;

// TODO: FIXME: Why can't this function be removed without causing errors?
void print_usage(ostd::string_range progname, bool err)
{
    auto &s = err ? ostd::cerr : ostd::cout;
    s.writeln("test");
    s.flush();
}


void run_test(std::string compare, std::string input, std::string expected_result)
{
    cs_state gcs;
    cs_value ret;

    gcs.init_libs();

    gcs.run(input, ret);

    if(compare == "eq")
    {
        ASSERT_EQ(ret.get_str(), expected_result);
    }
    else if(compare == "ne")
    {
        ASSERT_NE(ret.get_str(), expected_result);
    }
    else 
    {
        ASSERT_EQ("Unknown compare mode", "false");
    }
}

TEST(MATH, plus)
{
    run_test("eq", "+ 40 2", "42");
    run_test("ne", "+ 30 9", "75");
}

TEST(MATH, minus)
{
    run_test("eq", "- 40 2", "38");
    run_test("ne", "- 30 9", "86");
}
