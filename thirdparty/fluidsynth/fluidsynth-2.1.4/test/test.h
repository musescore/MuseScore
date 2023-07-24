
#pragma once

#include "config.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * We call abort() because on Linux this sends a signal to the process and causes the debugger to break.
 *
 * MSVC++ runtime opens a dialog when abort() is called, saying that abort() has been called and you can
 * click "Retry" to make the debugger break.
 * When executed by CI build however, the dialog causes the unit tests to be stuck forever.
 * Thus suppress the dialog on windows.
 * MinGW however requires explicit linking against MSVCRT >= 8.0 for _set_abort_behavior().
 * It's not worth the hassle to implement this with cmake...
 */
#if defined(NO_GUI) && defined(MINGW32)
#define TEST_ABORT exit(EXIT_FAILURE);
#elif defined(NO_GUI) && defined(WIN32)
#define TEST_ABORT _set_abort_behavior(0, _WRITE_ABORT_MSG); abort()
#else
#define TEST_ABORT abort()
#endif

#define TEST_ASSERT(COND) do { if (!(COND)) { fprintf(stderr, __FILE__ ":%d assertion (%s) failed\n", __LINE__, #COND); TEST_ABORT; } } while (0)

/* macro to test whether a fluidsynth function succeeded or not */
#define TEST_SUCCESS(FLUID_FUNCT) TEST_ASSERT((FLUID_FUNCT) != FLUID_FAILED)
