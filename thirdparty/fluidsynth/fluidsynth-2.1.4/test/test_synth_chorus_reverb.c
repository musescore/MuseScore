
#include "test.h"
#include "fluidsynth.h"

// this test should make sure that sample rate changed are handled correctly
int main(void)
{
    fluid_synth_t *synth;
    fluid_settings_t *settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.room-size", 0.1));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.damp", 0.2));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.width", 0.3));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.level", 0.4));

    TEST_SUCCESS(fluid_settings_setint(settings, "synth.chorus.nr", 99));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.level", 0.5));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.speed", 0.6));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.depth", 0.7));

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    // check that the synth is initialized with the correct values
    TEST_ASSERT(fluid_synth_get_reverb_roomsize(synth) == 0.1);
    TEST_ASSERT(fluid_synth_get_reverb_damp(synth) == 0.2);
    TEST_ASSERT(fluid_synth_get_reverb_width(synth) == 0.3);
    TEST_ASSERT(fluid_synth_get_reverb_level(synth) == 0.4);

    TEST_ASSERT(fluid_synth_get_chorus_nr(synth) == 99);
    TEST_ASSERT(fluid_synth_get_chorus_level(synth) == 0.5);
    TEST_ASSERT(fluid_synth_get_chorus_speed(synth) == 0.6);
    TEST_ASSERT(fluid_synth_get_chorus_depth(synth) == 0.7);

    // update the realtime settings afterward
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.room-size", 0.11));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.damp", 0.22));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.width", 0.33));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.level", 0.44));

    TEST_SUCCESS(fluid_settings_setint(settings, "synth.chorus.nr", 11));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.level", 0.55));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.speed", 0.66));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.depth", 0.77));

    // check that the realtime settings correctly update the values in the synth
    TEST_ASSERT(fluid_synth_get_reverb_roomsize(synth) == 0.11);
    TEST_ASSERT(fluid_synth_get_reverb_damp(synth) == 0.22);
    TEST_ASSERT(fluid_synth_get_reverb_width(synth) == 0.33);
    TEST_ASSERT(fluid_synth_get_reverb_level(synth) == 0.44);

    TEST_ASSERT(fluid_synth_get_chorus_nr(synth) == 11);
    TEST_ASSERT(fluid_synth_get_chorus_level(synth) == 0.55);
    TEST_ASSERT(fluid_synth_get_chorus_speed(synth) == 0.66);
    TEST_ASSERT(fluid_synth_get_chorus_depth(synth) == 0.77);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
