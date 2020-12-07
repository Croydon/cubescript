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

    gcs.new_command("echo", "C", [](auto &, auto args, auto &res) {
        res.set_str(args[0].get_str());
    });

    // TODO: FIXME: Consider moving this to core?
    gcs.new_command("exec", "s", [](auto &cs, auto args, auto &res) {
        auto file = args[0].get_strr();
        std::string filename{file.iter_begin(), file.iter_end()};
        bool ret = cs.run_file(filename, res);
        if (!ret)
        {
            throw cscript::cs_error(
                cs, "could not run file \"%s\"", file);
        }
    });

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

TEST(MATH, multiplication)
{
    run_test("eq", "* 40 2", "80");
    run_test("ne", "* 30 9", "100");
}

TEST(MATH, division)
{
    run_test("eq", "div 40 2", "20");
    run_test("ne", "div 90 3", "40");
}

TEST(MATH, nested)
{
    run_test("eq", "- 82 (+ 40 2)", "40");
    run_test("ne", "- 82 (+ 40 -200)", "40");
}


TEST(ECHO, literals_ascii)
{
    run_test("eq", "echo hello world", "hello world");
    run_test("eq", "echo echo", "echo");
    run_test("eq", "echo quit", "quit");
    run_test("eq", "echo 42", "42");
    run_test("eq", "echo /echo", "/echo");
    run_test("eq", "echo", "");
    run_test("ne", "echo + 40 2", "42");
}

TEST(ECHO, literals_unicode)
{
    run_test("eq", "echo äöü", "äöü");
}

TEST(ECHO, expression_result)
{
    run_test("eq", "echo (+ 40 2)", "42");
}


TEST(VARIABLES, assign)
{
    run_test("eq", "foo = \"bar\"", "");
    run_test("ne", "foo = \"bar\"", "bar");
}

TEST(VARIABLES, assign_and_echo)
{
    run_test("eq", "foo = \"bar\"; echo $foo", "bar");
    run_test("ne", "foo = \"bar\"; echo foo", "bar");
}


TEST(LOOPS, basic)
{
    run_test(
        "eq",
        "res = 0; loop i 10 [ res = (+ $res $i) ]; echo $res",
        "45"
    );
}

TEST(LOOPS, nested)
{
    run_test(
        "eq",
        "res = 0; loop x 2 [ loop i 10 [ res = (+ $res $i) ] ]; echo $res",
        "90"
    );

    run_test(
        "eq",
        "res = 0; loop y 2 [ loop x 2 [ loop i 10 [ res = (+ $res $i) ] ] ]; echo $res",
        "180"
    );
}


TEST(EXEC, basic)
{
    run_test(
        "eq",
        "exec files/basic.cfg",
        "This is a file run"
    );
}
