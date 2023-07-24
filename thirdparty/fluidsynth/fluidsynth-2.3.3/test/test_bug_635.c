#include "test.h"
#include "fluidsynth.h"
#include "sfloader/fluid_sfont.h"
#include "sfloader/fluid_defsfont.h"
#include "utils/fluid_sys.h"
#include "utils/fluid_list.h"

// load our sf2 and sf3 test soundfonts, with and without dynamic sample loading
int main(void)
{
    int id[2], sfcount, i;

    fluid_synth_t *synth;
    fluid_settings_t *settings = new_fluid_settings();

    fluid_settings_setint(settings, "synth.dynamic-sample-loading", 1);
    synth = new_fluid_synth(settings);
    id[0] = fluid_synth_sfload(synth, TEST_SOUNDFONT, 0);
    sfcount = fluid_synth_sfcount(synth);

    TEST_ASSERT(id[0] != FLUID_FAILED);
    TEST_ASSERT(sfcount == 1);

    for(i = 0; i < sfcount; i++)
    {
        fluid_preset_t *preset;
        fluid_list_t *list;

        fluid_sfont_t *sfont = fluid_synth_get_sfont_by_id(synth, id[i]);
        fluid_defsfont_t *defsfont = fluid_sfont_get_data(sfont);

        /* Make sure we have the right number of presets */
        fluid_sfont_iteration_start(sfont);

        while((preset = fluid_sfont_iteration_next(sfont)) != NULL)
        {
            // Try to start a voice with a preset that has not been selected on any channel (bug #635)

            // ignore return value check on fluid_synth_start
            fluid_synth_start(synth, 0, preset, 0, 1, 60, 127);

            // fluidsynth < 2.1.3 would crash here
            TEST_SUCCESS(fluid_synth_process(synth, 1024, 0, NULL, 0, NULL));

            fluid_synth_stop(synth, 0);
        }

        for(list = defsfont->sample; list; list = fluid_list_next(list))
        {
            fluid_sample_t *sample = fluid_list_get(list);
            fluid_voice_t *v;

            v = fluid_synth_alloc_voice(synth, sample, 0, 60, 127);

            fluid_synth_start_voice(synth, v);

            // fluidsynth < 2.1.3 would crash here
            TEST_SUCCESS(fluid_synth_process(synth, 1024, 0, NULL, 0, NULL));

            // make sure the voice was NULL, no need for fluid_synth_stop() here
            TEST_ASSERT(v == NULL);
        }
    }

    /* teardown */
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
