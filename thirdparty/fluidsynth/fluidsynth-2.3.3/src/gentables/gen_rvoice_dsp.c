
#include "rvoice/fluid_rvoice_dsp_tables.h"
#include "make_tables.h"

/* Linear interpolation table (2 coefficients centered on 1st) */
static double interp_coeff_linear[FLUID_INTERP_MAX][2];

/* 4th order (cubic) interpolation table (4 coefficients centered on 2nd) */
static double interp_coeff[FLUID_INTERP_MAX][4];

/* 7th order interpolation (7 coefficients centered on 3rd) */
static double sinc_table7[FLUID_INTERP_MAX][SINC_INTERP_ORDER];

static double cb_interp_coeff_linear(int y, int x) { return interp_coeff_linear[y][x]; }
static double cb_interp_coeff       (int y, int x) { return interp_coeff[y][x]; }
static double cb_sinc_table7        (int y, int x) { return sinc_table7[y][x]; }

/* Initializes interpolation tables */
void fluid_rvoice_dsp_config(void)
{
    int i, i2;
    double x, v;
    double i_shifted;

    /* Initialize the coefficients for the interpolation. The math comes
     * from a mail, posted by Olli Niemitalo to the music-dsp mailing
     * list (I found it in the music-dsp archives
     * http://www.smartelectronix.com/musicdsp/).  */

    for(i = 0; i < FLUID_INTERP_MAX; i++)
    {
        x = (double) i / (double) FLUID_INTERP_MAX;

        interp_coeff[i][0] = (x * (-0.5 + x * (1 - 0.5 * x)));
        interp_coeff[i][1] = (1.0 + x * x * (1.5 * x - 2.5));
        interp_coeff[i][2] = (x * (0.5 + x * (2.0 - 1.5 * x)));
        interp_coeff[i][3] = (0.5 * x * x * (x - 1.0));

        interp_coeff_linear[i][0] = (1.0 - x);
        interp_coeff_linear[i][1] = x;
    }

    /* i: Offset in terms of whole samples */
    for(i = 0; i < SINC_INTERP_ORDER; i++)
    {
        /* i2: Offset in terms of fractional samples ('subsamples') */
        for(i2 = 0; i2 < FLUID_INTERP_MAX; i2++)
        {
            /* center on middle of table */
            i_shifted = (double)i - ((double)SINC_INTERP_ORDER / 2.0)
                        + (double)i2 / (double)FLUID_INTERP_MAX;

            /* sinc(0) cannot be calculated straightforward (limit needed for 0/0) */
            if(fabs(i_shifted) > 0.000001)
            {
                double arg = M_PI * i_shifted;
                v = sin(arg) / (arg);
                /* Hanning window */
                v *= 0.5 * (1.0 + cos(2.0 * arg / (double)SINC_INTERP_ORDER));
            }
            else
            {
                v = 1.0;
            }

            sinc_table7[FLUID_INTERP_MAX - i2 - 1][i] = v;
        }
    }
}


void gen_rvoice_table_dsp (FILE *fp)
{
    /* Calculate the values */
    fluid_rvoice_dsp_config();

    /* Emit the matrices */
    emit_matrix(fp, "interp_coeff_linear", cb_interp_coeff_linear, FLUID_INTERP_MAX, 2);
    emit_matrix(fp, "interp_coeff",        cb_interp_coeff,        FLUID_INTERP_MAX, 4);
    emit_matrix(fp, "sinc_table7",         cb_sinc_table7,         FLUID_INTERP_MAX, 7);
}
