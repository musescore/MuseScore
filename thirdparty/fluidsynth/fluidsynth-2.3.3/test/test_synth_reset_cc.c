
#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include "fluid_midi.h"
#include "fluid_chan.h"
#include <string.h>

// render enough samples to go past the release phase of the voice
enum { SAMPLES=100*1024 };

static void test_sustain(fluid_synth_t* synth, int chan)
{
    // depress sustain pedal
    TEST_SUCCESS(fluid_synth_cc(synth, chan, SUSTAIN_SWITCH, 127));
    TEST_SUCCESS(fluid_synth_process(synth, SAMPLES, 0, NULL, 0, NULL));

    // hit a note, after some (render-)time has passed
    TEST_SUCCESS(fluid_synth_noteon(synth, chan, 60, 127));

    // trigger the dsp loop to force rvoice creation
    TEST_SUCCESS(fluid_synth_process(synth, SAMPLES, 0, NULL, 0, NULL));

    // one voice must be playing by now
    TEST_ASSERT(fluid_synth_get_active_voice_count(synth) == 2);

    // send noteoff
    TEST_SUCCESS(fluid_synth_noteoff(synth, chan, 60));

    // voice stays on
    TEST_SUCCESS(fluid_synth_process(synth, SAMPLES, 0, NULL, 0, NULL));
    TEST_ASSERT(fluid_synth_get_active_voice_count(synth) == 2);

    // reset controllers
    TEST_SUCCESS(fluid_synth_cc(synth, chan, ALL_CTRL_OFF, 0));

    // voice should be off now
    TEST_SUCCESS(fluid_synth_process(synth, SAMPLES, 0, NULL, 0, NULL));
    TEST_ASSERT(fluid_synth_get_active_voice_count(synth) == 0);
}

static void test_sostenuto(fluid_synth_t *synth, int chan)
{
    // play a note
    TEST_SUCCESS(fluid_synth_noteon(synth, chan, 60, 127));
    TEST_SUCCESS(fluid_synth_process(synth, SAMPLES, 0, NULL, 0, NULL));

    // depress sostenuto pedal
    TEST_ASSERT(fluid_synth_get_active_voice_count(synth) == 2);
    TEST_SUCCESS(fluid_synth_cc(synth, chan, SOSTENUTO_SWITCH, 127));

    // send noteoff right afterwards
    TEST_SUCCESS(fluid_synth_noteoff(synth, chan, 60));

    // voice stays on after rendering
    TEST_SUCCESS(fluid_synth_process(synth, SAMPLES, 0, NULL, 0, NULL));
    TEST_ASSERT(fluid_synth_get_active_voice_count(synth) == 2);

    // reset controllers
    TEST_SUCCESS(fluid_synth_cc(synth, chan, ALL_CTRL_OFF, 0));

    // voice should be off now
    TEST_SUCCESS(fluid_synth_process(synth, SAMPLES, 0, NULL, 0, NULL));
    TEST_ASSERT(fluid_synth_get_active_voice_count(synth) == 0);
}

static void test_portamento_fromkey(fluid_synth_t* synth, int chan)
{
    int ptc;

    // Portamento is disabled
    TEST_SUCCESS(fluid_synth_cc(synth, chan, PORTAMENTO_CTRL, 127));
    TEST_SUCCESS(fluid_synth_get_cc(synth, chan, PORTAMENTO_CTRL, &ptc));
    TEST_ASSERT(ptc == 127);
    
    TEST_SUCCESS(fluid_synth_cc(synth, chan, ALL_CTRL_OFF, 0));
    
    // Because PTC is used for modulating, it should be reset to zero
    TEST_SUCCESS(fluid_synth_get_cc(synth, chan, PORTAMENTO_CTRL, &ptc));
    TEST_ASSERT(!fluid_channel_is_valid_note(ptc));
    
    // Enable Portamento
    TEST_SUCCESS(fluid_synth_cc(synth, chan, PORTAMENTO_SWITCH, 64));
    TEST_SUCCESS(fluid_synth_cc(synth, chan, PORTAMENTO_TIME_MSB, 10));
    TEST_SUCCESS(fluid_synth_get_cc(synth, chan, PORTAMENTO_SWITCH, &ptc));
    TEST_ASSERT(ptc == 64);
    TEST_SUCCESS(fluid_synth_get_cc(synth, chan, PORTAMENTO_TIME_MSB, &ptc));
    TEST_ASSERT(ptc == 10);
    
    TEST_SUCCESS(fluid_synth_cc(synth, chan, ALL_CTRL_OFF, 0));
    TEST_SUCCESS(fluid_synth_get_cc(synth, chan, PORTAMENTO_CTRL, &ptc));
    TEST_ASSERT(!fluid_channel_is_valid_note(ptc));
    TEST_SUCCESS(fluid_synth_get_cc(synth, chan, PORTAMENTO_SWITCH, &ptc));
    TEST_ASSERT(ptc == 0);
    TEST_SUCCESS(fluid_synth_get_cc(synth, chan, PORTAMENTO_TIME_MSB, &ptc));
    TEST_ASSERT(ptc == 0);
    
    // Portamento is disabled
    TEST_SUCCESS(fluid_synth_cc(synth, chan, PORTAMENTO_CTRL, 127));
    TEST_SUCCESS(fluid_synth_get_cc(synth, chan, PORTAMENTO_CTRL, &ptc));
    TEST_ASSERT(ptc == 127);
    
    TEST_SUCCESS(fluid_synth_cc(synth, chan, ALL_CTRL_OFF, 0));
    
    // Because PTC is used for modulating, it should be reset to zero
    TEST_SUCCESS(fluid_synth_get_cc(synth, chan, PORTAMENTO_CTRL, &ptc));
    TEST_ASSERT(!fluid_channel_is_valid_note(ptc));
}

// this test should make sure that sample rate changed are handled correctly
int main(void)
{
    int chan;
    fluid_synth_t *synth;
    fluid_settings_t *settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);
    TEST_SUCCESS(fluid_synth_sfload(synth, TEST_SOUNDFONT, 1));

    for (chan = 0; chan < fluid_synth_count_midi_channels(synth); chan++)
    {
        const fluid_channel_t* channel = synth->channel[chan];
        if(channel->channel_type == CHANNEL_TYPE_DRUM)
        {
            // drum channels won't spawn voices
            continue;
        }

        test_portamento_fromkey(synth, chan);
        fluid_synth_system_reset(synth);
        
        test_sustain(synth, chan);
        fluid_synth_system_reset(synth);
        
        test_sostenuto(synth, chan);
        fluid_synth_system_reset(synth);
    }
    
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
