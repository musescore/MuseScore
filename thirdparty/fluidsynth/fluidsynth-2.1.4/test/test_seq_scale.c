#include "test.h"
#include "fluidsynth.h"
#include "utils/fluid_sys.h"
#include "synth/fluid_event.h"

static fluid_sequencer_t *sequencer;
static short synthSeqID, mySeqID;

static void sendnoteon(int chan, short key, short velocity, unsigned int time)
{
    fluid_event_t *evt = new_fluid_event();
    fluid_event_set_source(evt, -1);
    fluid_event_set_dest(evt, synthSeqID);
    fluid_event_noteon(evt, chan, key, velocity);
    TEST_SUCCESS(fluid_sequencer_send_at(sequencer, evt, time, 0));
    delete_fluid_event(evt);
}

static void sendpc(int chan, short val, unsigned int time)
{
    fluid_event_t *evt = new_fluid_event();
    fluid_event_set_source(evt, -1);
    fluid_event_set_dest(evt, synthSeqID);
    fluid_event_program_change(evt, chan, val);
    TEST_SUCCESS(fluid_sequencer_send_at(sequencer, evt, time, 0));
    delete_fluid_event(evt);
}

static void schedule_next_callback(unsigned int time)
{
    fluid_event_t *evt = new_fluid_event();
    fluid_event_set_source(evt, -1);
    fluid_event_set_dest(evt, mySeqID);
    fluid_event_timer(evt, NULL);
    TEST_SUCCESS(fluid_sequencer_send_at(sequencer, evt, time, 0));
    delete_fluid_event(evt);
}

static const unsigned int expected_callback_times[] =
{
    0, 0, 120, 120, 240, 240, 360, 360, 480, 1440,
    1560, 1560, 1680, 1680, 1800, 1800, 1920, 2700,
    2820, 2820, 2940, 2940, 3060, 3060, 3180, 4275,
    4395, 4395, 4515, 4515, 4635, 4635, 4755,
    799, 919, 919, 1039, 1039, 1159, 1159, 1279,
    15999
};

static void fake_synth_callback(unsigned int time, fluid_event_t *event, fluid_sequencer_t *seq, void *data)
{
    static int callback_idx = 0;

    TEST_ASSERT(time == expected_callback_times[callback_idx++]);
    FLUID_LOG(FLUID_INFO, "synth callback time: %u %u", time,  fluid_event_get_time(event));
}

static void seq_callback(unsigned int time, fluid_event_t *event, fluid_sequencer_t *seq, void *data)
{
    static int phase = 0;

    switch(phase)
    {
    case 0:
        sendpc(0, 47, 0);
        fluid_sequencer_set_time_scale(seq, 320);
        sendnoteon(0, 47, 60, 0);
        sendnoteon(0, 47, 0, 120);
        sendnoteon(0, 47, 90, 120);
        sendnoteon(0, 47, 0, 240);
        sendnoteon(0, 47, 110, 240);
        sendnoteon(0, 47, 0, 360);
        sendnoteon(0, 47, 127, 360);
        sendnoteon(0, 47, 0, 480);
        schedule_next_callback(480 + 240);
        break;

    case 1:
        fluid_sequencer_set_time_scale(seq, 1280 / 2);
        sendnoteon(0, 47, 60, 0);
        sendnoteon(0, 47, 0, 120);
        sendnoteon(0, 47, 80, 120);
        sendnoteon(0, 47, 0, 240);
        sendnoteon(0, 47, 90, 240);
        sendnoteon(0, 47, 0, 360);
        sendnoteon(0, 47, 100, 360);
        sendnoteon(0, 47, 0, 480);
        schedule_next_callback(480 + 240);
        break;

    case 2:
        fluid_sequencer_set_time_scale(seq, 800);
        sendnoteon(0, 47, 60, 0);
        sendnoteon(0, 47, 0, 120);
        sendnoteon(0, 47, 80, 120);
        sendnoteon(0, 47, 0, 240);
        sendnoteon(0, 47, 90, 240);
        sendnoteon(0, 47, 0, 360);
        sendnoteon(0, 47, 100, 360);
        sendnoteon(0, 47, 0, 480);
        schedule_next_callback(480 + 240);
        break;

    case 3:
        fluid_sequencer_set_time_scale(seq, 1000);
        sendnoteon(0, 47, 60, 0);
        sendnoteon(0, 47, 0, 120);
        sendnoteon(0, 47, 80, 120);
        sendnoteon(0, 47, 0, 240);
        sendnoteon(0, 47, 90, 240);
        sendnoteon(0, 47, 0, 360);
        sendnoteon(0, 47, 100, 360);
        sendnoteon(0, 47, 0, 480);
        schedule_next_callback(480 + 240);
        break;

    case 4:
        fluid_sequencer_set_time_scale(seq, 320 / 2);
        sendnoteon(0, 47, 60, 0);
        sendnoteon(0, 47, 0, 120);
        sendnoteon(0, 47, 80, 120);
        sendnoteon(0, 47, 0, 240);
        sendnoteon(0, 47, 90, 240);
        sendnoteon(0, 47, 0, 360);
        sendnoteon(0, 47, 100, 360);
        sendnoteon(0, 47, 0, 480);
        break;

    case 5:
        // this is the unregistering event, ignore
        break;

    default:
        TEST_ASSERT(0);
    }

    phase++;
}

// for debug, uncomment below to hear the notes
// #define SOUND

int main(void)
{
    int i;
#ifdef SOUND
    fluid_settings_t *settings = new_fluid_settings();
    fluid_settings_setstr(settings, "audio.driver", "alsa");
    fluid_synth_t *synth = new_fluid_synth(settings);
    TEST_SUCCESS(fluid_synth_sfload(synth, TEST_SOUNDFONT, 1));
#endif

    sequencer = new_fluid_sequencer2(0);
    TEST_ASSERT(sequencer != NULL);

#ifdef SOUND
    synthSeqID = fluid_sequencer_register_fluidsynth(sequencer, synth);
#else
    // register fake synth as first destination
    synthSeqID = fluid_sequencer_register_client(sequencer, "fake synth", fake_synth_callback, NULL);
#endif
    TEST_SUCCESS(synthSeqID);
    // register myself as scheduling destination
    mySeqID = fluid_sequencer_register_client(sequencer, "me", seq_callback, NULL);
    TEST_SUCCESS(mySeqID);

    // initialize our absolute date
    schedule_next_callback(0);

#ifdef SOUND
    fluid_audio_driver_t *adriver = new_fluid_audio_driver(settings, synth);
    fluid_msleep(100000);
    delete_fluid_audio_driver(adriver);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
#else

    for(i = 0; i < 100000; i++)
    {
        fluid_sequencer_process(sequencer, i);
    }

#endif

    delete_fluid_sequencer(sequencer);
    return EXIT_SUCCESS;
}
