/** @addtogroup Utilities
 * @{
 */

/** @file process.hh
 *
 * @brief Portable extensions to process handling.
 *
 * Provides POSIX and Windows abstractions for process creation and more.
 *
 * @copyright See COPYING.md in the project tree for further information.
 */

#ifndef OSTD_PROCESS_HH
#define OSTD_PROCESS_HH

#include <stdexcept>
#include <type_traits>
#include <utility>
#include <string>
#include <vector>

#include <ostd/platform.hh>
#include <ostd/string.hh>
#include <ostd/range.hh>
#include <ostd/io.hh>

namespace ostd {

/** @addtogroup Utilities
 * @{
 */

/** @brief Thrown on errors in ostd::split_args(). */
struct OSTD_EXPORT word_error: std::runtime_error {
    using std::runtime_error::runtime_error;
    /* empty, for vtable placement */
    virtual ~word_error();
};

namespace detail {
    OSTD_EXPORT void split_args_impl(
        string_range const &str, void (*func)(string_range, void *), void *data
    );
}

/** @brief Splits command line argument string into individual arguments.
 *
 * The split is done in a platform specific manner, using `wordexp` on POSIX
 * systems and `CommandLineToArgvW` on Windows. On Windows, the input string
 * is assumed to be UTF-8 and the output strings are always UTF-8, on POSIX
 * the wordexp implementation is followed (so it's locale specific).
 *
 * The `out` argument can be an output range that takes a single argument
 * at a time. The value type is an ostd::string_range. If it's not an output
 * range, it has to be a function that takes the ostd::string_range. However,
 * the string range that is used internally during the conversions is just
 * temporary and freed at the end of this function, so it's important that
 * it's copied in the range or the lambda.
 *
 * The ostd::word_error exception is used to handle failures of this
 * function itself. It may also throw other exceptions though, particularly
 * those thrown by `out` when putting the strings into it and also
 * std::bad_alloc on allocation failures.
 *
 * @returns The forwarded `out`.
 *
 * @throws ostd::word_error on failure or anything thrown by `out`.
 * @throws std::bad_alloc on alloation failures.
 */
template<typename Sink>
Sink &&split_args(Sink &&out, string_range str) {
    detail::split_args_impl(str, [](string_range val, void *outp) {
        if constexpr(is_output_range<std::decay_t<Sink>>) {
            static_cast<std::decay_t<Sink> *>(outp)->put(val);
        } else {
            (*static_cast<std::decay_t<Sink> *>(outp))(val);
        }
    }, &out);
    return std::forward<Sink>(out);
}

/** @brief Thrown on errors in ostd::subprocess. */
struct OSTD_EXPORT subprocess_error: std::runtime_error {
    using std::runtime_error::runtime_error;
    /* empty, for vtable placement */
    virtual ~subprocess_error();
};

/** @brief The mode used for standard streams in ostd::subprocess.
 *
 * This way you can turn stdin, stdout or stderr of any subprocess into
 * a stadnard ostd::file_stream or keep them as they are. You can also
 * redirect stderr into stdout, if stdout itself is redirected then
 * stderr will point to the newly redirected stdout.
 *
 * Only use the `STDOUT` value for stderr stream.
 */
enum class subprocess_stream {
    DEFAULT = 0, ///< Do not perform any redirection.
    PIPE,        ///< Capture the stream as an ostd::file_stream.
    STDOUT       ///< Writes to stderr will be written to stdout.
};

/** @brief Implements portable subprocess handling.
 *
 * This is a universal API with which you can freely manipulate standard
 * streams of the child process (therefore manipulate its I/O) as well as
 * get the return code of the child process. It portably covers the role
 * of `popen` (but it is also more powerful, as it can be bidirectional)
 * as well as the `exec` family of functions.
 */
struct OSTD_EXPORT subprocess {
private:
    struct nat {};

    std::aligned_storage_t<2 * sizeof(void *), alignof(void *)> p_data;
    void *p_current = nullptr;

public:
    /** @brief The standard input stream when redirected.
     *
     * If no redirection is done (see ostd::subprocess::use_in) then
     * this stream will not be opened.
     *
     * @see ostd::subprocess::out, ostd::subprocess::err
     */
    file_stream in = file_stream{};

    /** @brief The standard output stream when redirected.
     *
     * If no redirection is done (see ostd::subprocess::use_out) then
     * this stream will not be opened.
     *
     * @see ostd::subprocess::in, ostd::subprocess::err
     */
    file_stream out = file_stream{};

    /** @brief The standard error stream when redirected.
     *
     * If no redirection is done (see ostd::subprocess::use_err) then
     * this stream will not be opened.
     *
     * @see ostd::subprocess::in, ostd::subprocess::out
     */
    file_stream err = file_stream{};

    /** @brief The standard input redirection mode.
     *
     * The value is one of ostd::subprocess_stream. Set this before opening
     * the subprocess. If it's set to `PIPE`, you will be able to write
     * into the standard input of the child process using the `in` member,
     * which is a standard ostd::file_stream. Never set it to `STDOUT`
     * as that will make process opening throw an error. By default
     * no redirection is done.
     *
     * @see ostd::subprocess::in, ostd::subprocess::use_out,
     *      ostd::subprocess::use_err
     */
    subprocess_stream use_in = subprocess_stream::DEFAULT;

    /** @brief The standard output redirection mode.
     *
     * The value is one of ostd::subprocess_stream. Set this before opening
     * the subprocess. If it's set to `PIPE`, you will be able to read
     * from the standard output of the child process using the `out` member,
     * which is a standard ostd::file_stream. Setting this to `STDOUT` has
     * the same effect as `DEFAULT`.
     *
     * @see ostd::subprocess::out, ostd::subprocess::use_in,
     *      ostd::subprocess::use_err
     */
    subprocess_stream use_out = subprocess_stream::DEFAULT;

    /** @brief The standard error redirection mode.
     *
     * The value is one of ostd::subprocess_stream. Set this before opening
     * the subprocess. If it's set to `PIPE`, you will be able to read
     * from the standard error of the child process using the `err` member,
     * which is a standard ostd::file_stream. Setting this to `STDOUT`
     * redirects the child process standard error into its stanard output,
     * no matter what the redirection mode of the standard output is, so
     * they will be effectively the same stream - if you redirect the
     * standard output you will also read standard error using `err`.
     *
     * @see ostd::subprocess::err, ostd::subprocess::use_in,
     *      ostd::subprocess::use_out
     */
    subprocess_stream use_err = subprocess_stream::DEFAULT;

    /** @brief Initializes the structure with the given stream redirections. */
    subprocess(
        subprocess_stream  in_use = subprocess_stream::DEFAULT,
        subprocess_stream out_use = subprocess_stream::DEFAULT,
        subprocess_stream err_use = subprocess_stream::DEFAULT
    ) noexcept:
        use_in(in_use), use_out(out_use), use_err(err_use)
    {}

    /** @brief Initializes the structure and opens a subprocess.
     *
     * This is similar to calling either open_command() or open_path()
     * after constructing the object depending on `use_path`. It may
     * throw the same exceptions as the respective two methods.
     */
    template<typename InputRange1, typename InputRange2>
    subprocess(
        string_range cmd, InputRange1 &&args, InputRange2 &&envs,
        bool use_path = true,
        subprocess_stream  in_use = subprocess_stream::DEFAULT,
        subprocess_stream out_use = subprocess_stream::DEFAULT,
        subprocess_stream err_use = subprocess_stream::DEFAULT
    ):
        use_in(in_use), use_out(out_use), use_err(err_use)
    {
        open_full(
            cmd, std::forward<InputRange1>(args),
            std::forward<InputRange2>(envs), use_path
        );
    }

    /** @brief Initializes the structure and opens a subprocess.
     *
     * This is similar to calling either open_command() or open_path()
     * after constructing the object depending on `use_path`. It may
     * throw the same exceptions as the respective two methods.
     */
    template<typename InputRange1>
    subprocess(
        string_range cmd, InputRange1 &&args, bool use_path = true,
        subprocess_stream  in_use = subprocess_stream::DEFAULT,
        subprocess_stream out_use = subprocess_stream::DEFAULT,
        subprocess_stream err_use = subprocess_stream::DEFAULT
    ):
        use_in(in_use), use_out(out_use), use_err(err_use)
    {
        open_full(cmd, std::forward<InputRange1>(args), nullptr, use_path);
    }

    /** @brief Moves the subprocess data. */
    subprocess(subprocess &&i) noexcept:
        in(std::move(i.in)), out(std::move(i.out)), err(std::move(i.err)),
        use_in(i.use_in), use_out(i.use_out), use_err(i.use_err)
    {
        move_data(i);
    }

    /** @brief Move assigns the subprocess data.
     *
     * Effectively equivalent to swap().
     */
    subprocess &operator=(subprocess &&i) noexcept {
        swap(i);
        return *this;
    }

    /** @brief Swaps the data of two subprocess structures. */
    void swap(subprocess &i) noexcept {
        using std::swap;
        swap(use_in, i.use_in);
        swap(use_out, i.use_out);
        swap(use_err, i.use_err);
        swap(in, i.in);
        swap(out, i.out);
        swap(err, i.err);
        swap_data(i);
    }

    /** @brief Destroys the subprocess.
     *
     * If a child process assigned to this structure currently
     * exists, it will wait for it to finish first by calling close().
     */
    ~subprocess() {
        if (!p_current) {
            return;
        }
        try {
            close();
        } catch (subprocess_error const &) {}
        reset();
    }

    /** @brief Waits for a currently running child process to be done.
     *
     * If there isn't any child process assigned to this, it will throw
     * ostd::subprocess_error. It will also throw the same exception if
     * some other error has occured. It will not throw if the command has
     * executed but exited with a non-zero code. This code will be
     * returned instead.
     *
     * @returns The child process return code on success.
     *
     * @throws ostd::subprocess_error on failure of any kind.
     *
     * @see open_path(), open_command(), valid()
     */
    int close();

    /** @brief Opens a subprocess using the given arguments.
     *
     * The `path` is an actual absolute or relative path to an executable
     * file (as in POSIX `execv` or Windows `CreateProcess`) and `args`
     * is a range of string-like types (any works as long as a string can
     * be constructed from it). The first element of `args` is `argv[0]`.
     *
     * If `path` is empty, the first element of `args` is used.
     *
     * The `envs` argument represents a range of string-like types just like
     * `args` and each string represents an environment variable of the
     * subprocess, in format `name=value`. If you do not want to specify
     * custom environment variables, use an overload without the `envs`
     * argument - the child process will inherit its parent's environment.
     *
     * If this fails for any reason, ostd::subprocess_error will be thrown.
     * Having another child process running is considered a failure, so
     * use close() or detach() before opening another.
     *
     * On success, a new subprocess will be created and this will return
     * without waiting for it to finish. Use close() to wait and get the
     * return code or detach() to give up ownership of the child process.
     *
     * @throws ostd::subprocess_error on failure of any kind.
     *
     * @see open_command(), close()
     */
    template<typename InputRange1, typename InputRange2>
    void open_path(string_range path, InputRange1 &&args, InputRange2 &&envs) {
        open_full(
            path, std::forward<InputRange1>(args),
            std::forward<InputRange2>(envs), false
        );
    }

    /** @brief See the main open_path() documentation. */
    template<typename InputRange>
    void open_path(string_range path, InputRange &&args) {
        open_full(path, std::forward<InputRange>(args), nullptr, false);
    }

    /** @brief Opens a subprocess using the given arguments.
     *
     * The `cmd` is a command name looked up in the `PATH` environment
     * variable when it contains no slash and an ordinary executable
     * path when it contains one (as in POSIX `execvp` or Windows
     * `CreateProcess`) and `args` is a range of string-like types
     * (any works as long as a string can be constructed from it).
     * The first element of `args` is `argv[0]`.
     *
     * If `cmd` is empty, the first element of `args` is used.
     *
     * The `envs` argument represents a range of string-like types just like
     * `args` and each string represents an environment variable of the
     * subprocess, in format `name=value`. If you do not want to specify
     * custom environment variables, use an overload without the `envs`
     * argument - the child process will inherit its parent's environment.
     *
     * If this fails for any reason, ostd::subprocess_error will be thrown.
     * Having another child process running is considered a failure, so
     * use close() or detach() before opening another.
     *
     * On success, a new subprocess will be created and this will return
     * without waiting for it to finish. Use close() to wait and get the
     * return code or detach() to give up ownership of the child process.
     *
     * @throws ostd::subprocess_error on failure of any kind.
     *
     * @see open_path(), close()
     */
    template<typename InputRange1, typename InputRange2>
    void open_command(string_range cmd, InputRange1 &&args, InputRange2 &&envs) {
        open_full(
            cmd, std::forward<InputRange1>(args),
            std::forward<InputRange2>(envs), true
        );
    }

    /** @brief See the main open_command() documentation. */
    template<typename InputRange>
    void open_command(string_range cmd, InputRange &&args) {
        open_full(cmd, std::forward<InputRange>(args), nullptr, true);
    }

    /** @brief Detaches the child process.
     *
     * If there is a running child process, makes it so that it's no
     * longer owned by this subprocess object. The child process will
     * keep running even after the subprocess object has died.
     *
     * @throws ostd::subprocess_error if there is no child process.
     *
     * @see valid()
     */
    void detach() {
        if (!p_current) {
            throw subprocess_error{"no child process"};
        }
        reset();
    }

    /** @brief Checks if there is a running child process.
     *
     * @returns If there is one, true, otherwise false.
     */
    bool valid() const noexcept {
        return !!p_current;
    }

private:
    template<typename InputRange1, typename InputRange2>
    void open_full(
        string_range cmd, InputRange1 args, [[maybe_unused]] InputRange2 env,
        bool use_path
    ) {
        static_assert(
            std::is_constructible_v<
                string_range, range_reference_t<InputRange1>
            >,
            "The arguments must be strings"
        );
        if constexpr(!std::is_null_pointer_v<InputRange2>) {
            static_assert(
                std::is_constructible_v<
                    string_range, range_reference_t<InputRange2>
                >,
                "The environment variables must be strings"
            );
        }

        if (p_current) {
            throw subprocess_error{"another child process already running"};
        }

        auto argf = [](string_range &arg, void *data) {
            InputRange1 &argr = *static_cast<InputRange1 *>(data);
            if (argr.empty()) {
                return false;
            }
            arg = string_range{argr.front()};
            argr.pop_front();
            return true;
        };
        bool (*envf)(string_range &, void *) = nullptr;
        void *argp = &args, *envp = nullptr;

        if constexpr(!std::is_null_pointer_v<InputRange2>) {
            envf = [](string_range envv, void *data) {
                InputRange2 &envr = *static_cast<InputRange2 *>(data);
                if (envr.empty()) {
                    return false;
                }
                envv = string_range{envr.front()};
                envr.pop_front();
                return true;
            };
            envp = &env;
        }
        open_impl(use_path, cmd, argf, argp, envf, envp);
    }

    void open_impl(
        bool use_path, string_range cmd,
        bool (*func)(string_range &, void *), void *data,
        bool (*efunc)(string_range &, void *), void *edatap
    );

    void reset();
    void move_data(subprocess &i);
    void swap_data(subprocess &i);
};

/** @} */

} /* namespace ostd */

#endif

/** @} */
