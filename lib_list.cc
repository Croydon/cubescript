#include "cubescript.hh"
#include "lib_list.hh"

namespace cscript {

char *cs_dup_ostr(ostd::ConstCharRange s);
int cs_parse_int(ostd::ConstCharRange s);
float cs_parse_float(ostd::ConstCharRange s);

struct NullValue: TaggedValue {
    NullValue() { set_null(); }
} const null_value;

static inline void cs_set_iter(Ident &id, char *val, IdentStack &stack) {
    if (id.stack == &stack) {
        if (id.get_valtype() == VAL_STR) {
            delete[] id.val.s;
        } else {
            id.valtype = VAL_STR;
        }
        id.clean_code();
        id.val.s = val;
        id.val.len = strlen(val);
        return;
    }
    TaggedValue v;
    v.set_mstr(val);
    id.push_arg(v, stack);
}

static void cs_loop_list_conc(CsState &cs, Ident *id, ostd::ConstCharRange list,
                              Bytecode const *body, bool space) {
    if (id->type != ID_ALIAS)
        return;
    IdentStack stack;
    ostd::Vector<char> r;
    int n = 0;
    for (ListParser p(list); p.parse(); ++n) {
        char *val = p.element().disown();
        cs_set_iter(*id, val, stack);
        if (n && space)
            r.push(' ');
        TaggedValue v;
        cs.run_ret(body, v);
        ostd::String vstr = ostd::move(v.get_str());
        r.push_n(vstr.data(), vstr.size());
        v.cleanup();
    }
    if (n >= 0)
        id->pop_arg();
    r.push('\0');
    ostd::Size len = r.size();
    cs.result->set_mstr(ostd::CharRange(r.disown(), len - 1));
}

int cs_list_includes(ostd::ConstCharRange list, ostd::ConstCharRange needle) {
    int offset = 0;
    for (ListParser p(list); p.parse();) {
        if (p.item == needle)
            return offset;
        ++offset;
    }
    return -1;
}

static void cs_init_lib_list_sort(CsState &cs);

void cs_init_lib_list(CsState &cs) {
    cs.add_command("listlen", "s", [&cs](TvalRange args) {
        cs.result->set_int(int(util::list_length(args[0].get_strr())));
    });

    cs.add_command("at", "si1V", [&cs](TvalRange args) {
        if (args.empty())
            return;
        ostd::String str = ostd::move(args[0].get_str());
        ListParser p(str);
        p.item = str;
        for (ostd::Size i = 1; i < args.size(); ++i) {
            p.input = str;
            int pos = args[i].get_int();
            for (; pos > 0; --pos)
                if (!p.parse()) break;
            if (pos > 0 || !p.parse())
                p.item = p.quote = ostd::ConstCharRange();
        }
        auto elem = p.element();
        auto er = p.element().iter();
        elem.disown();
        cs.result->set_mstr(er);
    });

    cs.add_command("sublist", "siiN", [&cs](TvalRange args) {
        int skip    = args[1].get_int(),
            count   = args[2].get_int(),
            numargs = args[2].get_int();

        int offset = ostd::max(skip, 0),
            len = (numargs >= 3) ? ostd::max(count, 0) : -1;

        ListParser p(args[0].get_strr());
        for (int i = 0; i < offset; ++i)
            if (!p.parse()) break;
        if (len < 0) {
            if (offset > 0)
                p.skip();
            cs.result->set_str(p.input);
            return;
        }

        char const *list = p.input.data();
        p.quote = ostd::ConstCharRange();
        if (len > 0 && p.parse())
            while (--len > 0 && p.parse());
        char const *qend = !p.quote.empty() ? &p.quote[p.quote.size()] : list;
        cs.result->set_str(ostd::ConstCharRange(list, qend - list));
    });

    cs.add_command("listfind", "rse", [&cs](TvalRange args) {
        Ident *id = args[0].get_ident();
        auto body = args[2].get_code();
        if (id->type != ID_ALIAS) {
            cs.result->set_int(-1);
            return;
        }
        IdentStack stack;
        int n = -1;
        for (ListParser p(args[1].get_strr()); p.parse();) {
            ++n;
            cs_set_iter(*id, cs_dup_ostr(p.item), stack);
            if (cs.run_bool(body)) {
                cs.result->set_int(n);
                goto found;
            }
        }
        cs.result->set_int(-1);
found:
        if (n >= 0)
            id->pop_arg();
    });

    cs.add_command("listassoc", "rse", [&cs](TvalRange args) {
        Ident *id = args[0].get_ident();
        auto body = args[2].get_code();
        if (id->type != ID_ALIAS)
            return;
        IdentStack stack;
        int n = -1;
        for (ListParser p(args[1].get_strr()); p.parse();) {
            ++n;
            cs_set_iter(*id, cs_dup_ostr(p.item), stack);
            if (cs.run_bool(body)) {
                if (p.parse()) {
                    auto elem = p.element();
                    auto er = elem.iter();
                    elem.disown();
                    cs.result->set_mstr(er);
                }
                break;
            }
            if (!p.parse())
                break;
        }
        if (n >= 0)
            id->pop_arg();
    });

#define CS_CMD_LIST_FIND(name, fmt, gmeth, cmp) \
    cs.add_command(name, "s" fmt "i", [&cs](TvalRange args) { \
        int n = 0, skip = args[2].get_int(); \
        auto val = args[1].gmeth(); \
        for (ListParser p(args[0].get_strr()); p.parse(); ++n) { \
            if (cmp) { \
                cs.result->set_int(n); \
                return; \
            } \
            for (int i = 0; i < skip; ++i) { \
                if (!p.parse()) \
                    goto notfound; \
                ++n; \
            } \
        } \
    notfound: \
        cs.result->set_int(-1); \
    });

    CS_CMD_LIST_FIND("listfind=", "i", get_int, cs_parse_int(p.item) == val);
    CS_CMD_LIST_FIND("listfind=f", "f", get_float, cs_parse_float(p.item) == val);
    CS_CMD_LIST_FIND("listfind=s", "s", get_strr, p.item == val);

#undef CS_CMD_LIST_FIND

#define CS_CMD_LIST_ASSOC(name, fmt, gmeth, cmp) \
    cs.add_command(name, "s" fmt, [&cs](TvalRange args) { \
        auto val = args[1].gmeth(); \
        for (ListParser p(args[0].get_strr()); p.parse();) { \
            if (cmp) { \
                if (p.parse()) { \
                    auto elem = p.element(); \
                    auto er = elem.iter(); \
                    elem.disown(); \
                    cs.result->set_mstr(er); \
                } \
                return; \
            } \
            if (!p.parse()) \
                break; \
        } \
    });

    CS_CMD_LIST_ASSOC("listassoc=", "i", get_int, cs_parse_int(p.item) == val);
    CS_CMD_LIST_ASSOC("listassoc=f", "f", get_float, cs_parse_float(p.item) == val);
    CS_CMD_LIST_ASSOC("listassoc=s", "s", get_strr, p.item == val);

#undef CS_CMD_LIST_ASSOC

    cs.add_command("looplist", "rse", [&cs](TvalRange args) {
        Ident *id = args[0].get_ident();
        auto body = args[2].get_code();
        if (id->type != ID_ALIAS)
            return;
        IdentStack stack;
        int n = 0;
        for (ListParser p(args[1].get_strr()); p.parse(); ++n) {
            cs_set_iter(*id, p.element().disown(), stack);
            cs.run_int(body);
        }
        if (n >= 0)
            id->pop_arg();
    });

    cs.add_command("looplist2", "rrse", [&cs](TvalRange args) {
        Ident *id = args[0].get_ident(), *id2 = args[1].get_ident();
        auto body = args[3].get_code();
        if (id->type != ID_ALIAS || id2->type != ID_ALIAS)
            return;
        IdentStack stack, stack2;
        int n = 0;
        for (ListParser p(args[2].get_strr()); p.parse(); n += 2) {
            cs_set_iter(*id, p.element().disown(), stack);
            cs_set_iter(*id2, p.parse() ? p.element().disown()
                                        : cs_dup_ostr(""), stack2);
            cs.run_int(body);
        }
        if (n >= 0) {
            id->pop_arg();
            id2->pop_arg();
        }
    });

    cs.add_command("looplist3", "rrrse", [&cs](TvalRange args) {
        Ident *id = args[0].get_ident();
        Ident *id2 = args[1].get_ident();
        Ident *id3 = args[2].get_ident();
        auto body = args[4].get_code();
        if (id->type != ID_ALIAS)
            return;
        if (id2->type != ID_ALIAS || id3->type != ID_ALIAS)
            return;
        IdentStack stack, stack2, stack3;
        int n = 0;
        for (ListParser p(args[3].get_strr()); p.parse(); n += 3) {
            cs_set_iter(*id, p.element().disown(), stack);
            cs_set_iter(*id2, p.parse() ? p.element().disown()
                                        : cs_dup_ostr(""), stack2);
            cs_set_iter(*id3, p.parse() ? p.element().disown()
                                        : cs_dup_ostr(""), stack3);
            cs.run_int(body);
        }
        if (n >= 0) {
            id->pop_arg();
            id2->pop_arg();
            id3->pop_arg();
        }
    });

    cs.add_command("looplistconcat", "rse", [&cs](TvalRange args) {
        cs_loop_list_conc(
            cs, args[0].get_ident(), args[1].get_strr(),
            args[2].get_code(), true
        );
    });

    cs.add_command("looplistconcatword", "rse", [&cs](TvalRange args) {
        cs_loop_list_conc(
            cs, args[0].get_ident(), args[1].get_strr(),
            args[2].get_code(), false
        );
    });

    cs.add_command("listfilter", "rse", [&cs](TvalRange args) {
        Ident *id = args[0].get_ident();
        auto body = args[2].get_code();
        if (id->type != ID_ALIAS)
            return;
        IdentStack stack;
        ostd::Vector<char> r;
        int n = 0;
        for (ListParser p(args[1].get_strr()); p.parse(); ++n) {
            char *val = cs_dup_ostr(p.item);
            cs_set_iter(*id, val, stack);
            if (cs.run_bool(body)) {
                if (r.size()) r.push(' ');
                r.push_n(p.quote.data(), p.quote.size());
            }
        }
        if (n >= 0)
            id->pop_arg();
        r.push('\0');
        ostd::Size len = r.size() - 1;
        cs.result->set_mstr(ostd::CharRange(r.disown(), len));
    });

    cs.add_command("listcount", "rse", [&cs](TvalRange args) {
        Ident *id = args[0].get_ident();
        auto body = args[2].get_code();
        if (id->type != ID_ALIAS)
            return;
        IdentStack stack;
        int n = 0, r = 0;
        for (ListParser p(args[1].get_strr()); p.parse(); ++n) {
            char *val = cs_dup_ostr(p.item);
            cs_set_iter(*id, val, stack);
            if (cs.run_bool(body))
                r++;
        }
        if (n >= 0)
            id->pop_arg();
        cs.result->set_int(r);
    });

    cs.add_command("prettylist", "ss", [&cs](TvalRange args) {
        ostd::Vector<char> buf;
        ostd::ConstCharRange s = args[0].get_strr();
        ostd::ConstCharRange conj = args[1].get_strr();
        ostd::Size len = util::list_length(s);
        ostd::Size n = 0;
        for (ListParser p(s); p.parse(); ++n) {
            if (!p.quote.empty() && (p.quote.front() == '"')) {
                buf.reserve(buf.size() + p.item.size());
                auto writer = ostd::CharRange(&buf[buf.size()],
                    buf.capacity() - buf.size());
                ostd::Size adv = util::unescape_string(writer, p.item);
                writer.put('\0');
                buf.advance(adv);
            } else {
                buf.push_n(p.item.data(), p.item.size());
            }
            if ((n + 1) < len) {
                if ((len > 2) || conj.empty())
                    buf.push(',');
                if ((n + 2 == len) && !conj.empty()) {
                    buf.push(' ');
                    buf.push_n(conj.data(), conj.size());
                }
                buf.push(' ');
            }
        }
        buf.push('\0');
        ostd::Size slen = buf.size() - 1;
        cs.result->set_mstr(ostd::CharRange(buf.disown(), slen));
    });

    cs.add_command("indexof", "ss", [&cs](TvalRange args) {
        cs.result->set_int(
            cs_list_includes(args[0].get_strr(), args[1].get_strr())
        );
    });

#define CS_CMD_LIST_MERGE(name, init, iter, filter, dir) \
    cs.add_command(name, "ss", [&cs](TvalRange args) { \
        ostd::ConstCharRange list = args[0].get_strr(); \
        ostd::ConstCharRange elems = args[1].get_strr(); \
        ostd::Vector<char> buf; \
        init; \
        for (ListParser p(iter); p.parse();) { \
            if (cs_list_includes(filter, p.item) dir 0) { \
                if (!buf.empty()) \
                    buf.push(' '); \
                buf.push_n(p.quote.data(), p.quote.size()); \
            } \
        } \
        buf.push('\0'); \
        ostd::Size len = buf.size() - 1; \
        cs.result->set_mstr(ostd::CharRange(buf.disown(), len)); \
    });

    CS_CMD_LIST_MERGE("listdel", {}, list, elems, <);
    CS_CMD_LIST_MERGE("listintersect", {}, list, elems, >=);
    CS_CMD_LIST_MERGE("listunion", buf.push_n(list.data(), list.size()), elems,
        list, <);

#undef CS_CMD_LIST_MERGE

    cs.add_command("listsplice", "ssii", [&cs](TvalRange args) {
        int offset = ostd::max(args[2].get_int(), 0);
        int len    = ostd::max(args[3].get_int(), 0);
        ostd::ConstCharRange s = args[0].get_strr();
        ostd::ConstCharRange vals = args[1].get_strr();
        char const *list = s.data();
        ListParser p(s);
        for (int i = 0; i < offset; ++i)
            if (!p.parse())
                break;
        char const *qend = !p.quote.empty() ? &p.quote[p.quote.size()] : list;
        ostd::Vector<char> buf;
        if (qend > list)
            buf.push_n(list, qend - list);
        if (!vals.empty()) {
            if (!buf.empty())
                buf.push(' ');
            buf.push_n(vals.data(), vals.size());
        }
        for (int i = 0; i < len; ++i)
            if (!p.parse())
                break;
        p.skip();
        if (!p.input.empty()) switch (p.input.front()) {
        case ')':
        case ']':
            break;
        default:
            if (!buf.empty())
                buf.push(' ');
            buf.push_n(p.input.data(), p.input.size());
            break;
        }
        buf.push('\0');
        ostd::Size slen = buf.size() - 1;
        cs.result->set_mstr(ostd::CharRange(buf.disown(), slen));
    });

    cs_init_lib_list_sort(cs);
}

struct ListSortItem {
    char const *str;
    ostd::ConstCharRange quote;
};

struct ListSortFun {
    CsState &cs;
    Ident *x, *y;
    Bytecode *body;

    bool operator()(ListSortItem const &xval, ListSortItem const &yval) {
        x->clean_code();
        if (x->get_valtype() != VAL_CSTR) {
            x->valtype = VAL_CSTR;
        }
        x->val.cstr = xval.str;
        x->val.len = strlen(xval.str);
        y->clean_code();
        if (y->get_valtype() != VAL_CSTR) {
            y->valtype = VAL_CSTR;
        }
        y->val.cstr = yval.str;
        y->val.len = strlen(yval.str);
        return cs.run_bool(body);
    }
};

static void cs_list_sort(
    CsState &cs, ostd::ConstCharRange list, Ident *x, Ident *y,
    Bytecode *body, Bytecode *unique
) {
    if (x == y || x->type != ID_ALIAS || y->type != ID_ALIAS)
        return;

    ostd::Vector<ListSortItem> items;
    ostd::Size clen = list.size();
    ostd::Size total = 0;

    char *cstr = cs_dup_ostr(list);
    for (ListParser p(list); p.parse();) {
        cstr[&p.item[p.item.size()] - list.data()] = '\0';
        ListSortItem item = { &cstr[p.item.data() - list.data()], p.quote };
        items.push(item);
        total += item.quote.size();
    }

    if (items.empty()) {
        cs.result->set_mstr(cstr);
        return;
    }

    IdentStack xstack, ystack;
    x->push_arg(null_value, xstack);
    y->push_arg(null_value, ystack);

    ostd::Size totaluniq = total;
    ostd::Size nuniq = items.size();
    if (body) {
        ListSortFun f = { cs, x, y, body };
        ostd::sort_cmp(items.iter(), f);
        if (!code_is_empty(unique)) {
            f.body = unique;
            totaluniq = items[0].quote.size();
            nuniq = 1;
            for (ostd::Size i = 1; i < items.size(); i++) {
                ListSortItem &item = items[i];
                if (f(items[i - 1], item))
                    item.quote = nullptr;
                else {
                    totaluniq += item.quote.size();
                    ++nuniq;
                }
            }
        }
    } else {
        ListSortFun f = { cs, x, y, unique };
        totaluniq = items[0].quote.size();
        nuniq = 1;
        for (ostd::Size i = 1; i < items.size(); i++) {
            ListSortItem &item = items[i];
            for (ostd::Size j = 0; j < i; ++j) {
                ListSortItem &prev = items[j];
                if (!prev.quote.empty() && f(item, prev)) {
                    item.quote = nullptr;
                    break;
                }
            }
            if (!item.quote.empty()) {
                totaluniq += item.quote.size();
                ++nuniq;
            }
        }
    }

    x->pop_arg();
    y->pop_arg();

    char *sorted = cstr;
    ostd::Size sortedlen = totaluniq + ostd::max(nuniq - 1, ostd::Size(0));
    if (clen < sortedlen) {
        delete[] cstr;
        sorted = new char[sortedlen + 1];
    }

    ostd::Size offset = 0;
    for (ostd::Size i = 0; i < items.size(); ++i) {
        ListSortItem &item = items[i];
        if (item.quote.empty())
            continue;
        if (i)
            sorted[offset++] = ' ';
        memcpy(&sorted[offset], item.quote.data(), item.quote.size());
        offset += item.quote.size();
    }
    sorted[offset] = '\0';

    cs.result->set_mstr(sorted);
}

static void cs_init_lib_list_sort(CsState &cs) {
    cs.add_command("sortlist", "srree", [&cs](TvalRange args) {
        cs_list_sort(
            cs, args[0].get_strr(), args[1].get_ident(), args[2].get_ident(),
            args[3].get_code(), args[4].get_code()
        );
    });
    cs.add_command("uniquelist", "srre", [&cs](TvalRange args) {
        cs_list_sort(
            cs, args[0].get_strr(), args[1].get_ident(), args[2].get_ident(),
            nullptr, args[3].get_code()
        );
    });
}

} /* namespace cscript */