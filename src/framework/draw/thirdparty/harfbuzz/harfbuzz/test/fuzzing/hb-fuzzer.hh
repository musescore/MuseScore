#include <hb.h>
#include <stddef.h>

extern "C" int LLVMFuzzerTestOneInput (const uint8_t *data, size_t size);

#if defined(__GNUC__) && (__GNUC__ >= 4) || (__clang__)
#define HB_UNUSED	__attribute__((unused))
#else
#define HB_UNUSED
#endif

#ifdef HB_IS_IN_FUZZER

/* See src/failing-alloc.c */
extern "C" int alloc_state;

#else

/* Just a dummy global variable */
static int HB_UNUSED alloc_state = 0;

#endif

static inline int
_fuzzing_alloc_state (const uint8_t *data, size_t size)
{
  /* https://github.com/harfbuzz/harfbuzz/pull/2764#issuecomment-1172589849 */

  /* In 50% of the runs, don't fail the allocator. */
  if (size && data[size - 1] < 0x80)
    return 0;

  return size;
}
