
#include "test.h"
#include "fluidsynth.h"
#include "fluid_adriver.h"

// The jack driver may need the synth instance to adjust the sample-rate in case it mismatches with
// the sample-rate of the jack driver. However, new_fluid_audio_driver2() does not receive a synth pointer.
// Thus looking up the synth instance must be done via the settings object.
int main(void)
{
#if JACK_SUPPORT
    fluid_synth_t *obtained_synth;
    fluid_synth_t *expected_synth;
    fluid_settings_t *settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    expected_synth = new_fluid_synth(settings);
    TEST_ASSERT(expected_synth != NULL);

    TEST_SUCCESS(fluid_jack_obtain_synth(settings, &obtained_synth));
    TEST_ASSERT(obtained_synth == expected_synth);

    delete_fluid_synth(obtained_synth);
    delete_fluid_settings(settings);

    obtained_synth = expected_synth = NULL;

    settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);
    TEST_ASSERT(fluid_jack_obtain_synth(settings, &obtained_synth) == FLUID_FAILED);
    delete_fluid_settings(settings);
#endif
    return EXIT_SUCCESS;
}
