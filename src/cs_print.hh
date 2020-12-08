#include <iostream>

namespace cscript {
    template <typename A>
    static inline void writeSingleStandard(A const &output)
    {
        std::cout << output;
    }

    template <typename A>
    static inline void writeSingleError(A const &output)
    {
        std::cerr << output;
    }

    template <typename... A>
    static inline void writeStandardln(A const &... output)
    {
        (std::cout << ... << output) << std::endl;
    }

    template<typename ...A>
    static inline void writeErrorln(A const &... output)
    {
        (std::cerr << ... << output) << std::endl;
    }

    template <typename... A>
    static inline void writeStandard(A const &... output)
    {
        (std::cout << ... << output);
    }

    template <typename... A>
    static inline void writeError(A const &... output)
    {
        (std::cerr << ... << output);
    }
}
