
#include "test.h"
#include "fluidsynth.h"

// this test should make sure that effects change (reverb, chorus) are handled correctly
int main(void)
{
    fluid_synth_t *synth;
    fluid_settings_t *settings = new_fluid_settings();
    double value;
    int int_value;

    TEST_ASSERT(settings != NULL);
    /* set 2 group of effects */
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.effects-groups", 2));

    /* set values for all reverb group */
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.room-size", 0.1));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.damp", 0.2));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.width", 0.3));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.level", 0.4));

    /* set values for all chorus group */
    TEST_SUCCESS(fluid_settings_setint(settings, "synth.chorus.nr", 99));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.level", 0.5));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.speed", 0.6));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.depth", 0.7));

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    /*
       check that the synth is initialized with the correct values (for all reverb group)
    */
    TEST_ASSERT(fluid_synth_get_reverb_group_roomsize(synth, -1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.1);
    TEST_ASSERT(fluid_synth_get_reverb_group_damp(synth, -1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.2);
    TEST_ASSERT(fluid_synth_get_reverb_group_width(synth, -1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.3);
    TEST_ASSERT(fluid_synth_get_reverb_group_level(synth, -1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.4);

    TEST_ASSERT(fluid_synth_get_chorus_group_nr(synth, -1, &int_value) == FLUID_OK);
    TEST_ASSERT(int_value == 99);
    TEST_ASSERT(fluid_synth_get_chorus_group_level(synth, -1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.5);
    TEST_ASSERT(fluid_synth_get_chorus_group_speed(synth, -1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.6);
    TEST_ASSERT(fluid_synth_get_chorus_group_depth(synth, -1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.7);

    // update the realtime settings for all reverb group and all chorus group afterward
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.room-size", 0.11));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.damp", 0.22));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.width", 0.33));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.reverb.level", 0.44));

    TEST_SUCCESS(fluid_settings_setint(settings, "synth.chorus.nr", 11));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.level", 0.55));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.speed", 0.66));
    TEST_SUCCESS(fluid_settings_setnum(settings, "synth.chorus.depth", 0.77));

    // check that the realtime settings correctly update the values in the synth
    TEST_ASSERT(fluid_synth_get_reverb_group_roomsize(synth, -1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.11);
    TEST_ASSERT(fluid_synth_get_reverb_group_damp(synth, -1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.22);
    TEST_ASSERT(fluid_synth_get_reverb_group_width(synth, -1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.33);
    TEST_ASSERT(fluid_synth_get_reverb_group_level(synth, -1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.44);

    TEST_ASSERT(fluid_synth_get_chorus_group_nr(synth, -1, &int_value) == FLUID_OK);
    TEST_ASSERT(int_value == 11);
    TEST_ASSERT(fluid_synth_get_chorus_group_level(synth, -1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.55);
    TEST_ASSERT(fluid_synth_get_chorus_group_speed(synth, -1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.66);
    TEST_ASSERT(fluid_synth_get_chorus_group_depth(synth, -1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.77);

    /*
       Set/get that the synth is initialized with the correct values for one group only
       calling fx set/get API
    */
    // test reverb invalid parameters range
    // room size valid range: 0.0..1.0
    TEST_ASSERT(fluid_synth_set_reverb_group_roomsize(synth, 0, 1.1) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_set_reverb_group_roomsize(synth, 0, -0.1) == FLUID_FAILED);

    // damp valid range: 0.0..1.0
    TEST_ASSERT(fluid_synth_set_reverb_group_damp(synth, 0, 1.1) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_set_reverb_group_damp(synth, 0, -0.1) == FLUID_FAILED);

    // width valid range: 0.0..100.0
    TEST_ASSERT(fluid_synth_set_reverb_group_width(synth, 0, 100.1) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_set_reverb_group_width(synth, 0, -0.1) == FLUID_FAILED);

    // level valid range: 0.0..1.0
    TEST_ASSERT(fluid_synth_set_reverb_group_level(synth, 0, 1.1) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_set_reverb_group_level(synth, 0, -0.1) == FLUID_FAILED);

    // test chorus invalid parameters range
    // number of chorus block valid range: 0..99
    TEST_ASSERT(fluid_synth_set_chorus_group_nr(synth, 1, 100) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_set_chorus_group_nr(synth, 1, -1) == FLUID_FAILED);

    // level valid range: 0..10
    TEST_ASSERT(fluid_synth_set_chorus_group_level(synth, 0, 10.1) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_set_chorus_group_level(synth, 0, -0.1) == FLUID_FAILED);

    // lfo speed valid range: 0.1..5.0
    TEST_ASSERT(fluid_synth_set_chorus_group_speed(synth, 0, 5.1) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_set_chorus_group_speed(synth, 0, 0.09) == FLUID_FAILED);

    // lfo depth valid range: 0..256
    TEST_ASSERT(fluid_synth_set_chorus_group_depth(synth, 0, 256.1) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_set_chorus_group_depth(synth, 0, -0.1) == FLUID_FAILED);

    // lfo wafeform type valid range: 0..1
    TEST_ASSERT(fluid_synth_set_chorus_group_type(synth, 0, 2) == FLUID_FAILED);
    TEST_ASSERT(fluid_synth_set_chorus_group_type(synth, 0, -1) == FLUID_FAILED);

    // set a value and check if we get the same value to reverb group 0
    TEST_ASSERT(fluid_synth_set_reverb_group_roomsize(synth, 0, 0.1110) == FLUID_OK);
    TEST_ASSERT(fluid_synth_set_reverb_group_damp(synth, 0, 0.2220) == FLUID_OK);
    TEST_ASSERT(fluid_synth_set_reverb_group_width(synth, 0, 0.3330) == FLUID_OK);
    TEST_ASSERT(fluid_synth_set_reverb_group_level(synth, 0, 0.4440) == FLUID_OK);

    TEST_ASSERT(fluid_synth_get_reverb_group_roomsize(synth, 0, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.1110);
    TEST_ASSERT(fluid_synth_get_reverb_group_damp(synth, 0, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.2220);
    TEST_ASSERT(fluid_synth_get_reverb_group_width(synth, 0, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.3330);
    TEST_ASSERT(fluid_synth_get_reverb_group_level(synth, 0, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.4440);

    // set a value and check if we get the same value to reverb group 1
    TEST_ASSERT(fluid_synth_set_reverb_group_roomsize(synth, 1, 0.1111) == FLUID_OK);
    TEST_ASSERT(fluid_synth_set_reverb_group_damp(synth, 1, 0.2221) == FLUID_OK);
    TEST_ASSERT(fluid_synth_set_reverb_group_width(synth, 1, 0.3331) == FLUID_OK);
    TEST_ASSERT(fluid_synth_set_reverb_group_level(synth, 1, 0.4441) == FLUID_OK);

    TEST_ASSERT(fluid_synth_get_reverb_group_roomsize(synth, 1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.1111);
    TEST_ASSERT(fluid_synth_get_reverb_group_damp(synth, 1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.2221);
    TEST_ASSERT(fluid_synth_get_reverb_group_width(synth, 1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.3331);
    TEST_ASSERT(fluid_synth_get_reverb_group_level(synth, 1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.4441);

    // set a value and check if we get the same value to chorus group 0
    TEST_ASSERT(fluid_synth_set_chorus_group_nr(synth, 0, 20) == FLUID_OK);
    TEST_ASSERT(fluid_synth_set_chorus_group_level(synth, 0, 0.5550) == FLUID_OK);
    TEST_ASSERT(fluid_synth_set_chorus_group_speed(synth, 0, 0.6660) == FLUID_OK);
    TEST_ASSERT(fluid_synth_set_chorus_group_depth(synth, 0, 0.7770) == FLUID_OK);
    TEST_ASSERT(fluid_synth_set_chorus_group_type(synth, 0, 0) == FLUID_OK);

    TEST_ASSERT(fluid_synth_get_chorus_group_nr(synth, 0, &int_value) == FLUID_OK);
    TEST_ASSERT(int_value == 20);
    TEST_ASSERT(fluid_synth_get_chorus_group_level(synth, 0, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.5550);
    TEST_ASSERT(fluid_synth_get_chorus_group_speed(synth, 0, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.6660);
    TEST_ASSERT(fluid_synth_get_chorus_group_depth(synth, 0, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.7770);
    TEST_ASSERT(fluid_synth_get_chorus_group_type(synth, 0, &int_value) == FLUID_OK);
    TEST_ASSERT(int_value == 0);

    // set a value and check if we get the same value to chorus group 1
    TEST_ASSERT(fluid_synth_set_chorus_group_nr(synth, 1, 21) == FLUID_OK);
    TEST_ASSERT(fluid_synth_set_chorus_group_level(synth, 1, 0.5551) == FLUID_OK);
    TEST_ASSERT(fluid_synth_set_chorus_group_speed(synth, 1, 0.6661) == FLUID_OK);
    TEST_ASSERT(fluid_synth_set_chorus_group_depth(synth, 1, 0.7771) == FLUID_OK);
    TEST_ASSERT(fluid_synth_set_chorus_group_type(synth, 1, 1) == FLUID_OK);

    TEST_ASSERT(fluid_synth_get_chorus_group_nr(synth, 1, &int_value) == FLUID_OK);
    TEST_ASSERT(int_value == 21);
    TEST_ASSERT(fluid_synth_get_chorus_group_level(synth, 1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.5551);
    TEST_ASSERT(fluid_synth_get_chorus_group_speed(synth, 1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.6661);
    TEST_ASSERT(fluid_synth_get_chorus_group_depth(synth, 1, &value) == FLUID_OK);
    TEST_ASSERT(value == 0.7771);
    TEST_ASSERT(fluid_synth_get_chorus_group_type(synth, 1, &int_value) == FLUID_OK);
    TEST_ASSERT(int_value == 1);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
