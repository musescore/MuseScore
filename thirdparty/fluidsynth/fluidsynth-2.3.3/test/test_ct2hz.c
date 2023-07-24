
#include "test.h"
#include "utils/fluid_conv.h"
#include "utils/fluid_conv_tables.h"
#include "utils/fluid_sys.h"

// this test makes sure FLUID_SNPRINTF uses a proper C99 compliant implementation

int float_eq(double x, double y)
{
    static const double EPS = 1e-3;
    FLUID_LOG(FLUID_INFO, "Comparing %.9f and %.9f", x, y);
    return fabs(x-y) < EPS;
}

int main(void)
{
    int i;
    // 440 * 2^((x-6900)/1200) where x is the cent value given to ct2hz()

    TEST_ASSERT(float_eq(fluid_ct2hz_real(13500), 19912.12695821317828712777723687254894626098));

    TEST_ASSERT(float_eq(fluid_ct2hz_real(12900), 14080));
    TEST_ASSERT(float_eq(fluid_ct2hz_real(12899), 14071.86942151064095341800489737387241797607));

    TEST_ASSERT(float_eq(fluid_ct2hz_real(12700), 12543.85395141597741074238497471441611245995));
    TEST_ASSERT(float_eq(fluid_ct2hz_real(6900), 440));
    TEST_ASSERT(float_eq(fluid_ct2hz_real(5700), 220));
    TEST_ASSERT(float_eq(fluid_ct2hz_real(4500), 110));

    TEST_ASSERT(float_eq(fluid_ct2hz_real(901), 13.7579446057151293153308979171569743434390204));
    TEST_ASSERT(float_eq(fluid_ct2hz_real(900), 13.75));
    TEST_ASSERT(float_eq(fluid_ct2hz_real(899), 13.7420599819439853060722704075916722831797578));

    TEST_ASSERT(float_eq(fluid_ct2hz_real(1), 8.1805228064648688650522010380302841769481091116));
    TEST_ASSERT(float_eq(fluid_ct2hz_real(0), 8.1757989156437073336828122976032719176391831357)); // often referred to as Absolute zero in the SF2 spec

    // Test the entire possible range: from lowest permitted value of MODLFOFREQ up to filter fc limit
    for(i = -16000; i < 13500; i++)
    {
        TEST_ASSERT(float_eq(fluid_ct2hz_real(i), fluid_act2hz(i)));
    }
    return EXIT_SUCCESS;
}
