
#include "test.h"
#include "utils/fluid_sys.h"

// this test makes sure FLUID_SNPRINTF uses a proper C99 compliant implementation

int main(void)
{
    char buf[2 + 1];

    int ret = FLUID_SNPRINTF(buf, sizeof(buf), "99");
    TEST_ASSERT(ret == 2);

    TEST_ASSERT(buf[2] == '\0');

    ret = FLUID_SNPRINTF(buf, sizeof(buf), "999");
    TEST_ASSERT(ret == 3);

    // output truncated, buffer must be NULL terminated!
    TEST_ASSERT(buf[2] == '\0');

    return EXIT_SUCCESS;
}
