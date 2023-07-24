
#include "test.h"
#include "utils/fluid_conv.h"
#include "utils/fluid_sys.h"

// this test makes sure FLUID_SNPRINTF uses a proper C99 compliant implementation

int float_eq(fluid_real_t x, fluid_real_t y)
{
    static const float EPS = 1e-5;
    FLUID_LOG(FLUID_INFO, "Comparing %.9f and %.9f", x, y);
    return fabs(x-y) < EPS;
}

int main(void)
{
    // 440 * 2^((x-6900)/1200) where x is the cent value given to ct2hz()


    TEST_ASSERT(float_eq(fluid_ct2hz_real(38099), 2.9510849101059895e10));

    TEST_ASSERT(float_eq(fluid_ct2hz_real(13500), 19912.12696));

    TEST_ASSERT(float_eq(fluid_ct2hz_real(12900), 14080));
    TEST_ASSERT(float_eq(fluid_ct2hz_real(12899), 14071.86942));

    TEST_ASSERT(float_eq(fluid_ct2hz_real(12700), 12543.85395));
    TEST_ASSERT(float_eq(fluid_ct2hz_real(6900), 440));
    TEST_ASSERT(float_eq(fluid_ct2hz_real(5700), 220));
    TEST_ASSERT(float_eq(fluid_ct2hz_real(4500), 110));

    TEST_ASSERT(float_eq(fluid_ct2hz_real(901), 13.75794461));
    TEST_ASSERT(float_eq(fluid_ct2hz_real(900), 13.75));
    TEST_ASSERT(float_eq(fluid_ct2hz_real(899), 13.74205998));

    TEST_ASSERT(float_eq(fluid_ct2hz_real(1), 8.180522806));
    TEST_ASSERT(float_eq(fluid_ct2hz_real(0), 8.175798916)); // often referred to as Absolute zero in the SF2 spec

    return EXIT_SUCCESS;
}
