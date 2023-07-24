
#include "test.h"
#include "fluidsynth.h"
#include "utils/fluid_sys.h"


// this tests utf-8 file handling by loading the test .sf2 file
// manually and through the soundfont-related APIs
int main(void)
{
    int id;
    fluid_settings_t *settings;
    fluid_synth_t *synth;
    fluid_player_t *player;

    FILE *file;
    file = FLUID_FOPEN(TEST_SOUNDFONT_UTF8_1, "rb");
    TEST_ASSERT(file != NULL);
    TEST_ASSERT(FLUID_FCLOSE(file) == 0);

    file = FLUID_FOPEN(TEST_SOUNDFONT_UTF8_2, "rb");
    TEST_ASSERT(file != NULL);
    TEST_ASSERT(FLUID_FCLOSE(file) == 0);

    file = FLUID_FOPEN(TEST_MIDI_UTF8, "rb");
    TEST_ASSERT(file != NULL);
    TEST_ASSERT(FLUID_FCLOSE(file) == 0);

    settings = new_fluid_settings();
    synth = new_fluid_synth(settings);

    TEST_ASSERT(settings != NULL);
    TEST_ASSERT(synth != NULL);

    // no sfont loaded
    TEST_ASSERT(fluid_synth_sfcount(synth) == 0);

    TEST_ASSERT(fluid_is_soundfont(TEST_SOUNDFONT_UTF8_1) == TRUE);
    TEST_SUCCESS(id = fluid_synth_sfload(synth, TEST_SOUNDFONT_UTF8_1, 1));

    TEST_ASSERT(fluid_is_soundfont(TEST_SOUNDFONT_UTF8_2) == TRUE);
    TEST_SUCCESS(id = fluid_synth_sfload(synth, TEST_SOUNDFONT_UTF8_2, 1));

    player = new_fluid_player(synth);
    TEST_ASSERT(player != NULL);

    TEST_ASSERT(fluid_is_midifile(TEST_MIDI_UTF8) == TRUE);
    TEST_SUCCESS(fluid_player_add(player, TEST_MIDI_UTF8));

    delete_fluid_player(player);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
