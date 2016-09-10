#include <limits.h>

/* c++ versions for overloaded math */
#include <stdlib.h>
#include <math.h>

#include "cubescript/cubescript.hh"

namespace cscript {

static constexpr CsFloat PI = 3.14159265358979f;
static constexpr CsFloat RAD = PI / 180.0f;

template<typename T>
struct CsMathVal;

template<>
struct CsMathVal<CsInt> {
    static CsInt get(CsValue &tv) {
        return tv.get_int();
    }
    static void set(CsValue &res, CsInt val) {
        res.set_int(val);
    }
};

template<>
struct CsMathVal<CsFloat> {
    static CsFloat get(CsValue &tv) {
        return tv.get_float();
    }
    static void set(CsValue &res, CsFloat val) {
        res.set_float(val);
    }
};

template<typename T>
struct CsMathNoop {
    T operator()(T arg) {
        return arg;
    }
};

template<typename T, typename F1, typename F2>
static inline void cs_mathop(
    CsValueRange args, CsValue &res, T initval,
    F1 binop, F2 unop
) {
    T val;
    if (args.size() >= 2) {
        val = binop(CsMathVal<T>::get(args[0]), CsMathVal<T>::get(args[1]));
        for (ostd::Size i = 2; i < args.size(); ++i) {
            val = binop(val, CsMathVal<T>::get(args[i]));
        }
    } else {
        val = unop(!args.empty() ? CsMathVal<T>::get(args[0]) : initval);
    }
    CsMathVal<T>::set(res, val);
}

template<typename T, typename F>
static inline void cs_cmpop(CsValueRange args, CsValue &res, F cmp) {
    bool val;
    if (args.size() >= 2) {
        val = cmp(CsMathVal<T>::get(args[0]), CsMathVal<T>::get(args[1]));
        for (ostd::Size i = 2; (i < args.size()) && val; ++i) {
            val = cmp(val, CsMathVal<T>::get(args[i]));
        }
    } else {
        val = cmp(!args.empty() ? CsMathVal<T>::get(args[0]) : T(0), T(0));
    }
    res.set_int(CsInt(val));
}

/* overloads for various float types */

inline auto cs_sin(float f) { return sinf(f); }
inline auto cs_sin(double f) { return sin(f); }
inline auto cs_sin(long double f) { return sinl(f); }

inline auto cs_cos(float f) { return cosf(f); }
inline auto cs_cos(double f) { return cos(f); }
inline auto cs_cos(long double f) { return cosl(f); }

inline auto cs_tan(float f) { return tanf(f); }
inline auto cs_tan(double f) { return tan(f); }
inline auto cs_tan(long double f) { return tanl(f); }

inline auto cs_asin(float f) { return asinf(f); }
inline auto cs_asin(double f) { return asin(f); }
inline auto cs_asin(long double f) { return asinl(f); }

inline auto cs_acos(float f) { return acosf(f); }
inline auto cs_acos(double f) { return acos(f); }
inline auto cs_acos(long double f) { return acosl(f); }

inline auto cs_atan(float f) { return atanf(f); }
inline auto cs_atan(double f) { return atan(f); }
inline auto cs_atan(long double f) { return atanl(f); }

inline auto cs_atan2(float f, float f2) { return atan2f(f, f2); }
inline auto cs_atan2(double f, double f2) { return atan2(f, f2); }
inline auto cs_atan2(long double f, long double f2) { return atan2l(f, f2); }

inline auto cs_sqrt(float f) { return sqrtf(f); }
inline auto cs_sqrt(double f) { return sqrt(f); }
inline auto cs_sqrt(long double f) { return sqrtl(f); }

inline auto cs_log(float f) { return logf(f); }
inline auto cs_log(double f) { return log(f); }
inline auto cs_log(long double f) { return logl(f); }

inline auto cs_log10(float f) { return log10f(f); }
inline auto cs_log10(double f) { return log10(f); }
inline auto cs_log10(long double f) { return log10l(f); }

inline auto cs_exp(float f) { return expf(f); }
inline auto cs_exp(double f) { return exp(f); }
inline auto cs_exp(long double f) { return expl(f); }

inline int cs_abs(int i) { return abs(i); }
inline long cs_abs(long i) { return labs(i); }
inline long long cs_abs(long long i) { return llabs(i); }
inline auto cs_abs(float f) { return fabsf(f); }
inline auto cs_abs(double f) { return fabs(f); }
inline auto cs_abs(long double f) { return fabsl(f); }

inline auto cs_floor(float f) { return floorf(f); }
inline auto cs_floor(double f) { return floor(f); }
inline auto cs_floor(long double f) { return floorl(f); }

inline auto cs_ceil(float f) { return ceilf(f); }
inline auto cs_ceil(double f) { return ceil(f); }
inline auto cs_ceil(long double f) { return ceill(f); }

inline auto cs_fmod(float f, float f2) { return fmodf(f, f2); }
inline auto cs_fmod(double f, double f2) { return fmod(f, f2); }
inline auto cs_fmod(long double f, long double f2) { return fmodl(f, f2); }

void cs_init_lib_math(CsState &cs) {
    cs.new_command("sin", "f", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_sin(args[0].get_float() * RAD));
    });
    cs.new_command("cos", "f", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_cos(args[0].get_float() * RAD));
    });
    cs.new_command("tan", "f", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_tan(args[0].get_float() * RAD));
    });

    cs.new_command("asin", "f", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_asin(args[0].get_float()) / RAD);
    });
    cs.new_command("acos", "f", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_acos(args[0].get_float()) / RAD);
    });
    cs.new_command("atan", "f", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_atan(args[0].get_float()) / RAD);
    });
    cs.new_command("atan2", "ff", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_atan2(args[0].get_float(), args[1].get_float()) / RAD);
    });

    cs.new_command("sqrt", "f", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_sqrt(args[0].get_float()));
    });
    cs.new_command("loge", "f", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_log(args[0].get_float()));
    });
    cs.new_command("log2", "f", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_log(args[0].get_float()) / M_LN2);
    });
    cs.new_command("log10", "f", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_log10(args[0].get_float()));
    });

    cs.new_command("exp", "f", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_exp(args[0].get_float()));
    });

    cs.new_command("min", "i1V", [](CsValueRange args, CsValue &res) {
        CsInt v = (!args.empty() ? args[0].get_int() : 0);
        for (ostd::Size i = 1; i < args.size(); ++i) {
            v = ostd::min(v, args[i].get_int());
        }
        res.set_int(v);
    });
    cs.new_command("max", "i1V", [](CsValueRange args, CsValue &res) {
        CsInt v = (!args.empty() ? args[0].get_int() : 0);
        for (ostd::Size i = 1; i < args.size(); ++i) {
            v = ostd::max(v, args[i].get_int());
        }
        res.set_int(v);
    });
    cs.new_command("minf", "f1V", [](CsValueRange args, CsValue &res) {
        CsFloat v = (!args.empty() ? args[0].get_float() : 0);
        for (ostd::Size i = 1; i < args.size(); ++i) {
            v = ostd::min(v, args[i].get_float());
        }
        res.set_float(v);
    });
    cs.new_command("maxf", "f1V", [](CsValueRange args, CsValue &res) {
        CsFloat v = (!args.empty() ? args[0].get_float() : 0);
        for (ostd::Size i = 1; i < args.size(); ++i) {
            v = ostd::max(v, args[i].get_float());
        }
        res.set_float(v);
    });

    cs.new_command("abs", "i", [](CsValueRange args, CsValue &res) {
        res.set_int(cs_abs(args[0].get_int()));
    });
    cs.new_command("absf", "f", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_abs(args[0].get_float()));
    });

    cs.new_command("floor", "f", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_floor(args[0].get_float()));
    });
    cs.new_command("ceil", "f", [](CsValueRange args, CsValue &res) {
        res.set_float(cs_ceil(args[0].get_float()));
    });

    cs.new_command("round", "ff", [](CsValueRange args, CsValue &res) {
        CsFloat step = args[1].get_float();
        CsFloat r = args[0].get_float();
        if (step > 0) {
            r += step * ((r < 0) ? -0.5 : 0.5);
            r -= cs_fmod(r, step);
        } else {
            r = (r < 0) ? cs_ceil(r - 0.5) : cs_floor(r + 0.5);
        }
        res.set_float(r);
    });

    cs.new_command("+", "i1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsInt>(args, res, 0, ostd::Add<CsInt>(), CsMathNoop<CsInt>());
    });
    cs.new_command("*", "i1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsInt>(
            args, res, 1, ostd::Multiply<CsInt>(), CsMathNoop<CsInt>()
        );
    });
    cs.new_command("-", "i1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsInt>(
            args, res, 0, ostd::Subtract<CsInt>(), ostd::Negate<CsInt>()
        );
    });

    cs.new_command("^", "i1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsInt>(
            args, res, 0, ostd::BitXor<CsInt>(), [](CsInt val) { return ~val; }
        );
    });
    cs.new_command("~", "i1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsInt>(
            args, res, 0, ostd::BitXor<CsInt>(), [](CsInt val) { return ~val; }
        );
    });
    cs.new_command("&", "i1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsInt>(
            args, res, 0, ostd::BitAnd<CsInt>(), CsMathNoop<CsInt>()
        );
    });
    cs.new_command("|", "i1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsInt>(
            args, res, 0, ostd::BitOr<CsInt>(), CsMathNoop<CsInt>()
        );
    });

    /* special combined cases */
    cs.new_command("^~", "i1V", [](CsValueRange args, CsValue &res) {
        CsInt val;
        if (args.size() >= 2) {
            val = args[0].get_int() ^ ~args[1].get_int();
            for (ostd::Size i = 2; i < args.size(); ++i) {
                val ^= ~args[i].get_int();
            }
        } else {
            val = !args.empty() ? args[0].get_int() : 0;
        }
        res.set_int(val);
    });
    cs.new_command("&~", "i1V", [](CsValueRange args, CsValue &res) {
        CsInt val;
        if (args.size() >= 2) {
            val = args[0].get_int() & ~args[1].get_int();
            for (ostd::Size i = 2; i < args.size(); ++i) {
                val &= ~args[i].get_int();
            }
        } else {
            val = !args.empty() ? args[0].get_int() : 0;
        }
        res.set_int(val);
    });
    cs.new_command("|~", "i1V", [](CsValueRange args, CsValue &res) {
        CsInt val;
        if (args.size() >= 2) {
            val = args[0].get_int() | ~args[1].get_int();
            for (ostd::Size i = 2; i < args.size(); ++i) {
                val |= ~args[i].get_int();
            }
        } else {
            val = !args.empty() ? args[0].get_int() : 0;
        }
        res.set_int(val);
    });

    cs.new_command("<<", "i1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsInt>(
            args, res, 0, [](CsInt val1, CsInt val2) {
                return (val2 < CsInt(sizeof(CsInt) * CHAR_BIT))
                    ? (val1 << ostd::max(val2, CsInt(0)))
                    : 0;
            }, CsMathNoop<CsInt>()
        );
    });
    cs.new_command(">>", "i1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsInt>(
            args, res, 0, [](CsInt val1, CsInt val2) {
                return val1 >> ostd::clamp(
                    val2, CsInt(0), CsInt(sizeof(CsInt) * CHAR_BIT)
                );
            }, CsMathNoop<CsInt>()
        );
    });

    cs.new_command("+f", "f1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsFloat>(
            args, res, 0, ostd::Add<CsFloat>(), CsMathNoop<CsFloat>()
        );
    });
    cs.new_command("*f", "f1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsFloat>(
            args, res, 1, ostd::Multiply<CsFloat>(), CsMathNoop<CsFloat>()
        );
    });
    cs.new_command("-f", "f1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsFloat>(
            args, res, 0, ostd::Subtract<CsFloat>(), ostd::Negate<CsFloat>()
        );
    });

    cs.new_command("div", "i1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsInt>(
            args, res, 0, [](CsInt val1, CsInt val2) {
                if (val2) {
                    return val1 / val2;
                }
                return CsInt(0);
            }, CsMathNoop<CsInt>()
        );
    });
    cs.new_command("mod", "i1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsInt>(
            args, res, 0, [](CsInt val1, CsInt val2) {
                if (val2) {
                    return val1 % val2;
                }
                return CsInt(0);
            }, CsMathNoop<CsInt>()
        );
    });
    cs.new_command("divf", "f1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsFloat>(
            args, res, 0, [](CsFloat val1, CsFloat val2) {
                if (val2) {
                    return val1 / val2;
                }
                return CsFloat(0);
            }, CsMathNoop<CsFloat>()
        );
    });
    cs.new_command("modf", "f1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsFloat>(
            args, res, 0, [](CsFloat val1, CsFloat val2) {
                if (val2) {
                    return CsFloat(fmod(val1, val2));
                }
                return CsFloat(0);
            }, CsMathNoop<CsFloat>()
        );
    });

    cs.new_command("pow", "f1V", [](CsValueRange args, CsValue &res) {
        cs_mathop<CsFloat>(
            args, res, 0, [](CsFloat val1, CsFloat val2) {
                return CsFloat(pow(val1, val2));
            }, CsMathNoop<CsFloat>()
        );
    });

    cs.new_command("=", "i1V", [](CsValueRange args, CsValue &res) {
        cs_cmpop<CsInt>(args, res, ostd::Equal<CsInt>());
    });
    cs.new_command("!=", "i1V", [](CsValueRange args, CsValue &res) {
        cs_cmpop<CsInt>(args, res, ostd::NotEqual<CsInt>());
    });
    cs.new_command("<", "i1V", [](CsValueRange args, CsValue &res) {
        cs_cmpop<CsInt>(args, res, ostd::Less<CsInt>());
    });
    cs.new_command(">", "i1V", [](CsValueRange args, CsValue &res) {
        cs_cmpop<CsInt>(args, res, ostd::Greater<CsInt>());
    });
    cs.new_command("<=", "i1V", [](CsValueRange args, CsValue &res) {
        cs_cmpop<CsInt>(args, res, ostd::LessEqual<CsInt>());
    });
    cs.new_command(">=", "i1V", [](CsValueRange args, CsValue &res) {
        cs_cmpop<CsInt>(args, res, ostd::GreaterEqual<CsInt>());
    });

    cs.new_command("=f", "f1V", [](CsValueRange args, CsValue &res) {
        cs_cmpop<CsFloat>(args, res, ostd::Equal<CsFloat>());
    });
    cs.new_command("!=f", "f1V", [](CsValueRange args, CsValue &res) {
        cs_cmpop<CsFloat>(args, res, ostd::NotEqual<CsFloat>());
    });
    cs.new_command("<f", "f1V", [](CsValueRange args, CsValue &res) {
        cs_cmpop<CsFloat>(args, res, ostd::Less<CsFloat>());
    });
    cs.new_command(">f", "f1V", [](CsValueRange args, CsValue &res) {
        cs_cmpop<CsFloat>(args, res, ostd::Greater<CsFloat>());
    });
    cs.new_command("<=f", "f1V", [](CsValueRange args, CsValue &res) {
        cs_cmpop<CsFloat>(args, res, ostd::LessEqual<CsFloat>());
    });
    cs.new_command(">=f", "f1V", [](CsValueRange args, CsValue &res) {
        cs_cmpop<CsFloat>(args, res, ostd::GreaterEqual<CsFloat>());
    });
}

} /* namespace cscript */