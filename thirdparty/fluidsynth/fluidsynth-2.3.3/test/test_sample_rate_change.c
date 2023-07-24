
#include "test.h"
#include "fluidsynth.h"
#include "synth/fluid_synth.h"
#include "synth/fluid_voice.h"
#include "rvoice/fluid_rvoice.h"
#include "utils/fluid_sys.h"

static void verify_sample_rate(fluid_synth_t *synth, int expected_srate)
{
    int i;

    TEST_ASSERT(synth->sample_rate == expected_srate);

    for(i = 0; i < synth->polyphony; i++)
    {
        TEST_ASSERT(synth->voice[i]->output_rate == expected_srate);
        TEST_ASSERT(synth->voice[i]->rvoice->dsp.output_rate == expected_srate);
        TEST_ASSERT(synth->voice[i]->overflow_rvoice->dsp.output_rate == expected_srate);
    }

    // TODO check fx, rvoice_mixer et. al.?
}

// this test should make sure that sample rate changed are handled correctly
int main(void)
{
    int polyphony;
    double sample_rate;

    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth = new_fluid_synth(settings);

    TEST_ASSERT(settings != NULL);
    TEST_ASSERT(synth != NULL);

    TEST_SUCCESS(fluid_settings_getint(settings, "synth.polyphony", &polyphony));
    TEST_ASSERT(polyphony == synth->polyphony);

    TEST_SUCCESS(fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate));

    verify_sample_rate(synth, sample_rate);


    TEST_SUCCESS(fluid_synth_sfload(synth, TEST_SOUNDFONT, 1));
    // play some notes to start some voices
    TEST_SUCCESS(fluid_synth_noteon(synth, 0, 60, 127));
    TEST_SUCCESS(fluid_synth_noteon(synth, 2, 71, 127));
    TEST_SUCCESS(fluid_synth_noteon(synth, 3, 82, 127));

    verify_sample_rate(synth, sample_rate);

    // set srate to minimum allowed
    sample_rate = 8000;
    fluid_synth_set_sample_rate(synth, sample_rate);

    // manually dispatch rvoice events to get samples rates updated (this simulates the render thread)
    fluid_synth_process_event_queue(synth);

    verify_sample_rate(synth, sample_rate);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
