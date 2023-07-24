#include "test.h"
#include "fluidsynth.h"
#include "sfloader/fluid_sfont.h"
#include "sfloader/fluid_defsfont.h"
#include "utils/fluid_sys.h"
#include "utils/fluid_list.h"

// load our sf2 and sf3 test soundfonts, with and without dynamic sample loading
int main(void)
{
    int id[2], sfcount, dyn_sample, i;

    /* setup */
    fluid_settings_t *settings = new_fluid_settings();

    for(dyn_sample = 0; dyn_sample <= 1; dyn_sample++)
    {
        fluid_synth_t *synth;
        fluid_settings_setint(settings, "synth.dynamic-sample-loading", dyn_sample);
        synth = new_fluid_synth(settings);
        id[0] = fluid_synth_sfload(synth, TEST_SOUNDFONT, 0);
        id[1] = FLUID_FAILED;//fluid_synth_sfload(synth, TEST_SOUNDFONT_SF3, 0);
        sfcount = fluid_synth_sfcount(synth);

        TEST_ASSERT(id[0] != FLUID_FAILED);

#if 0
        TEST_ASSERT(id[1] != FLUID_FAILED);
        TEST_ASSERT(sfcount == 2);
#else
        TEST_ASSERT(id[1] == FLUID_FAILED);
        TEST_ASSERT(sfcount == 1);
#endif

        for(i = 0; i < sfcount; i++)
        {
            fluid_preset_t *preset;
            fluid_list_t *list;
            fluid_preset_t *prev_preset = NULL;
            fluid_sample_t *prev_sample = NULL;
            int count = 0;
            fluid_sfont_t *sfont = fluid_synth_get_sfont_by_id(synth, id[i]);
            fluid_defsfont_t *defsfont = fluid_sfont_get_data(sfont);

            /* Make sure we have the right number of presets */
            fluid_sfont_iteration_start(sfont);

            while((preset = fluid_sfont_iteration_next(sfont)) != NULL)
            {
                count++;

                if(preset->notify != NULL)
                {
                    preset->notify(preset, FLUID_PRESET_SELECTED, 0);
                }

                /* make sure we actually got a different preset */
                TEST_ASSERT(preset != prev_preset);
                prev_preset = preset;
            }

            /* VintageDreams has 136 presets */
            TEST_ASSERT(count == 136);

            /* Make sure we have the right number of samples */
            count = 0;

            for(list = defsfont->sample; list; list = fluid_list_next(list))
            {
                fluid_sample_t *sample = fluid_list_get(list);

                if(sample->data != NULL)
                {
                    count++;
                }

                TEST_ASSERT(sample->amplitude_that_reaches_noise_floor_is_valid);

                /* Make sure we actually got a different sample */
                TEST_ASSERT(sample != prev_sample);
                prev_sample = sample;
            }

            /* VintageDreams has 123 valid samples (one is a ROM sample and ignored) */
            TEST_ASSERT(count == 123);
        }

        /* teardown */
        delete_fluid_synth(synth);
    }

    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
