
#include "test.h"
#include "fluidsynth.h" // use local fluidsynth header
#include "utils/fluid_sys.h"


// this test aims to make sure that sample data used by multiple synths is not freed
// once unloaded by its parent synth
int main(void)
{
    enum { FRAMES = 1024 };
    float buf[FRAMES * 2];

    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth1 = new_fluid_synth(settings),
                   *synth2 = new_fluid_synth(settings);

    TEST_ASSERT(settings != NULL);
    TEST_ASSERT(synth1 != NULL);
    TEST_ASSERT(synth2 != NULL);

    // load a sfont to synth1
    TEST_SUCCESS(fluid_synth_sfload(synth1, TEST_SOUNDFONT, 1));

    // load the same font to synth2 and start a note
    TEST_SUCCESS(fluid_synth_sfload(synth2, TEST_SOUNDFONT, 1));
    TEST_SUCCESS(fluid_synth_noteon(synth2, 0, 60, 127));

    // render some audio
    TEST_SUCCESS(fluid_synth_write_float(synth2, FRAMES, buf, 0, 2, buf, 1, 2));

    // delete the synth that owns the soundfont
    delete_fluid_synth(synth1);

    // render again with the unloaded sfont and hope no segfault happens
    TEST_SUCCESS(fluid_synth_write_float(synth2, FRAMES, buf, 0, 2, buf, 1, 2));

    delete_fluid_synth(synth2);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
