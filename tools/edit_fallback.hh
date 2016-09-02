#ifndef CS_REPL_HAS_EDIT
/* use nothing (no line editing support) */

#include <ostd/string.hh>
#include <ostd/maybe.hh>

static void init_lineedit(ostd::ConstCharRange) {
}

static ostd::Maybe<ostd::String> read_line(CsSvar *pr) {
    ostd::write(pr->get_value());
    ostd::String ret;
    /* i really need to implement some sort of get_line for ostd streams */
    for (char c = ostd::in.getchar(); c && (c != '\n'); c = ostd::in.getchar()) {
        ret += c;
    }
    return ostd::move(ret);
}

static void add_history(ostd::ConstCharRange) {
}

#endif