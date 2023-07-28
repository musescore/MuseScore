
#include "utils/fluid_conv_tables.h"
#include "make_tables.h"


/* conversion tables */
static double fluid_ct2hz_tab[FLUID_CENTS_HZ_SIZE];
static double fluid_cb2amp_tab[FLUID_CB_AMP_SIZE];
static double fluid_concave_tab[FLUID_VEL_CB_SIZE];
static double fluid_convex_tab[FLUID_VEL_CB_SIZE];
static double fluid_pan_tab[FLUID_PAN_SIZE];

/*
 * void fluid_synth_init
 *
 * Does all the initialization for this module.
 */
static void fluid_conversion_config(void)
{
    int i;
    long double x;

    for(i = 0; i < FLUID_CENTS_HZ_SIZE; i++)
    {
        // 6,875 is just a factor that we already multiply into the lookup table to save
        // that multiplication in fluid_ct2hz_real()
        // 6.875 Hz because 440Hz / 2^6
        fluid_ct2hz_tab[i] = 6.875L * powl(2.0L, i / 1200.0L);
    }

    /* centibels to amplitude conversion
     * Note: SF2.01 section 8.1.3: Initial attenuation range is
     * between 0 and 144 dB. Therefore a negative attenuation is
     * not allowed.
     */
    for(i = 0; i < FLUID_CB_AMP_SIZE; i++)
    {
        fluid_cb2amp_tab[i] = powl(10.0L, i / -200.0L);
    }

    /* initialize the conversion tables (see fluid_mod.c
       fluid_mod_get_value cases 4 and 8) */

    /* concave unipolar positive transform curve */
    fluid_concave_tab[0] = 0.0;
    fluid_concave_tab[FLUID_VEL_CB_SIZE - 1] = 1.0;

    /* convex unipolar positive transform curve */
    fluid_convex_tab[0] = 0;
    fluid_convex_tab[FLUID_VEL_CB_SIZE - 1] = 1.0;

    /* There seems to be an error in the specs. The equations are
       implemented according to the pictures on SF2.01 page 73. */

    for(i = 1; i < FLUID_VEL_CB_SIZE - 1; i++)
    {
        x = (-200.0L * 2 / FLUID_PEAK_ATTENUATION) * logl(i / (FLUID_VEL_CB_SIZE - 1.0L)) / M_LN10;
        fluid_convex_tab[i] = (1.0L - x);
        fluid_concave_tab[(FLUID_VEL_CB_SIZE - 1) - i] =  x;
    }

    /* initialize the pan conversion table */
    x = M_PI / 2.0L / (FLUID_PAN_SIZE - 1.0L);

    for(i = 0; i < FLUID_PAN_SIZE; i++)
    {
        fluid_pan_tab[i] = sinl(i * x);
    }
}


void gen_conv_table(FILE *fp)
{
    /* Calculate the values */
    fluid_conversion_config();

    /* fluid_ct2hz_tab */
    EMIT_ARRAY(fp, fluid_ct2hz_tab);
    EMIT_ARRAY(fp, fluid_cb2amp_tab);
    EMIT_ARRAY(fp, fluid_concave_tab);
    EMIT_ARRAY(fp, fluid_convex_tab);
    EMIT_ARRAY(fp, fluid_pan_tab);
}

