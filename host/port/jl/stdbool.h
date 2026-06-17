/* host/port/jl/stdbool.h
 * Shadow for the system <stdbool.h> to avoid conflict with JL SDK's own
 * 'bool' typedef in include_lib/driver/cpu/br28/asm/cpu.h.
 *
 * ESP-Hosted core headers include <stdbool.h>. On JL, that header would
 * #define bool _Bool before JL's typedef is seen, causing:
 *   typedef unsigned char u8, bool, BOOL;
 * to expand into an invalid typedef of _Bool.
 *
 * By placing host/port/jl/ first in the include path, this shadow is picked
 * instead of the toolchain stdbool.h. It bootstraps JL typedefs first, then
 * provides the C99 stdbool macros that ESP-Hosted expects.
 */

#ifndef JL_STDBOOL_SHADOW_H
#define JL_STDBOOL_SHADOW_H

/* Pull in JL typedefs first so its 'bool' typedef wins. */
#include "generic/typedef.h"

/* Provide C99-compatible macros if the JL header has not already done so. */
#ifndef true
#define true  1
#endif
#ifndef false
#define false 0
#endif
#ifndef __bool_true_false_are_defined
#define __bool_true_false_are_defined 1
#endif

#endif /* JL_STDBOOL_SHADOW_H */
