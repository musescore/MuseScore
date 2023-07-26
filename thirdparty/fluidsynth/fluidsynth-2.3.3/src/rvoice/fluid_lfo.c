#include "fluid_lfo.h"

DECLARE_FLUID_RVOICE_FUNCTION(fluid_lfo_set_incr)
{
    fluid_lfo_t *lfo = obj;
    fluid_real_t increment = param[0].real;

    lfo->increment = increment;
}

DECLARE_FLUID_RVOICE_FUNCTION(fluid_lfo_set_delay)
{
    fluid_lfo_t *lfo = obj;
    unsigned int delay = param[0].i;

    lfo->delay = delay;
}
