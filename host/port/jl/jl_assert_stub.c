/* host/port/jl/jl_assert_stub.c
 *
 * Weak stub for __assert_func used by standard assert() in newlib/pi32.
 * The protobuf-c submodule calls assert(), which on the JL pi32 toolchain
 * resolves to __assert_func. Provide a minimal implementation so linking
 * succeeds without pulling in the full hosted newlib assert machinery.
 *
 * This is intentionally weak so that a platform-specific implementation can
 * override it if desired.
 */

/* Do NOT include <stdio.h> on JL: include_lib/system/fs/fs.h already defines
 * FILE/fread/fwrite/fseek and conflicts with the C library stdio.h. */
extern int printf(const char *fmt, ...);

void __attribute__((weak)) __assert_func(const char *file, int line,
                                          const char *func,
                                          const char *failed_expr)
{
    printf("ASSERT failed at %s:%d (%s): %s\r\n",
           file ? file : "?",
           line,
           func ? func : "?",
           failed_expr ? failed_expr : "?");
    while (1) {
        /* Halt; could also trigger a watchdog reset if preferred. */
    }
}
