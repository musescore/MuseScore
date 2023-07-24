
#include "test.h"
#include "utils/fluid_sys.h"


// test for fluid_align_ptr()
int main(void)
{
    unsigned int align;
    uintptr_t ptr, aligned_ptr;

    for(align = 32; align <= 4 * 1024u; align <<= 1)
    {
        for(ptr = 0; ptr <= (align << 10); ptr++)
        {
            char *tmp = fluid_align_ptr((char *)ptr, align);
            aligned_ptr = (uintptr_t)tmp;

            // pointer must be aligned properly
            TEST_ASSERT(aligned_ptr % align == 0);

            // aligned pointer must not be smaller than ptr
            TEST_ASSERT(aligned_ptr >= ptr);

            // aligned pointer must not be bigger than alignment
            TEST_ASSERT(aligned_ptr < ptr + align);
        }
    }

    return EXIT_SUCCESS;
}
