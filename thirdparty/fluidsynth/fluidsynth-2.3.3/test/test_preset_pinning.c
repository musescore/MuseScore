#include "test.h"
#include "fluidsynth.h"
#include "sfloader/fluid_sfont.h"
#include "sfloader/fluid_defsfont.h"
#include "sfloader/fluid_samplecache.h"
#include "utils/fluid_sys.h"
#include "utils/fluid_list.h"

static int count_loaded_samples(fluid_synth_t *synth, int sfont_id);


/* Test the preset pinning and unpinning feature */
int main(void)
{
    int id;
    fluid_synth_t *synth;
    fluid_sfont_t *sfont;
    fluid_defsfont_t *defsfont;

    /* Setup */
    fluid_settings_t *settings = new_fluid_settings();


    /* Test that pinning and unpinning has no effect and throws no errors
     * when dynamic-sample-loading is disabled */
    fluid_settings_setint(settings, "synth.dynamic-sample-loading", 0);
    synth = new_fluid_synth(settings);
    id = fluid_synth_sfload(synth, TEST_SOUNDFONT, 0);
    TEST_ASSERT(count_loaded_samples(synth, id) == 123);
    TEST_ASSERT(fluid_samplecache_count_entries() == 1);

    /* Attempt to pin and unpin an existing preset should succeed (but have no effect) */
    TEST_ASSERT(fluid_synth_pin_preset(synth, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 123);

    TEST_ASSERT(fluid_synth_unpin_preset(synth, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 123);

    /* Attempt to pin and unpin a non-existent preset should fail */
    TEST_ASSERT(fluid_synth_pin_preset(synth, id, 42, 42) == FLUID_FAILED);
    TEST_ASSERT(count_loaded_samples(synth, id) == 123);

    TEST_ASSERT(fluid_synth_unpin_preset(synth, id, 42, 42) == FLUID_FAILED);
    TEST_ASSERT(count_loaded_samples(synth, id) == 123);

    delete_fluid_synth(synth);


    /* Test pinning and unpinning with dyamic-sample loading enabled */
    fluid_settings_setint(settings, "synth.dynamic-sample-loading", 1);
    synth = new_fluid_synth(settings);
    id = fluid_synth_sfload(synth, TEST_SOUNDFONT, 0);

    TEST_ASSERT(count_loaded_samples(synth, id) == 0);
    TEST_ASSERT(fluid_samplecache_count_entries() == 0);


    /* For the following tests, preset 42 (Lead Synth 2) consists of 4 samples,
     * preset 40 (Aluminum Plate) consists of 1 sample */

    /* Simple pin and unpin */
    TEST_ASSERT(fluid_synth_pin_preset(synth, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 4);
    TEST_ASSERT(fluid_samplecache_count_entries() == 4);

    TEST_ASSERT(fluid_synth_unpin_preset(synth, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 0);
    TEST_ASSERT(fluid_samplecache_count_entries() == 0);


    /* Pinning and unpinning twice should have exactly the same effect */
    TEST_ASSERT(fluid_synth_pin_preset(synth, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 4);
    TEST_ASSERT(fluid_samplecache_count_entries() == 4);

    TEST_ASSERT(fluid_synth_pin_preset(synth, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 4);
    TEST_ASSERT(fluid_samplecache_count_entries() == 4);

    TEST_ASSERT(fluid_synth_unpin_preset(synth, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 0);
    TEST_ASSERT(fluid_samplecache_count_entries() == 0);

    TEST_ASSERT(fluid_synth_unpin_preset(synth, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 0);
    TEST_ASSERT(fluid_samplecache_count_entries() == 0);


    /* Pin a single preset, select both presets, unselect both, ensure the pinned
     * is still in memory and gone after unpinning */
    TEST_ASSERT(fluid_synth_pin_preset(synth, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 4);
    TEST_ASSERT(fluid_samplecache_count_entries() == 4);

    TEST_ASSERT(fluid_synth_program_select(synth, 0, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 4);
    TEST_ASSERT(fluid_samplecache_count_entries() == 4);

    TEST_ASSERT(fluid_synth_program_select(synth, 1, id, 0, 40) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 5);
    TEST_ASSERT(fluid_samplecache_count_entries() == 5);

    TEST_ASSERT(fluid_synth_unset_program(synth, 0) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 5);
    TEST_ASSERT(fluid_samplecache_count_entries() == 5);

    TEST_ASSERT(fluid_synth_unset_program(synth, 1) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 4);
    TEST_ASSERT(fluid_samplecache_count_entries() == 4);

    TEST_ASSERT(fluid_synth_unpin_preset(synth, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 0);
    TEST_ASSERT(fluid_samplecache_count_entries() == 0);


    /* Unpinning an unpinned and selected preset should not remove it from memory */
    TEST_ASSERT(fluid_synth_program_select(synth, 0, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 4);
    TEST_ASSERT(fluid_samplecache_count_entries() == 4);

    TEST_ASSERT(fluid_synth_unpin_preset(synth, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 4);
    TEST_ASSERT(fluid_samplecache_count_entries() == 4);

    TEST_ASSERT(fluid_synth_unset_program(synth, 0) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 0);
    TEST_ASSERT(fluid_samplecache_count_entries() == 0);


    /* Test unload of soundfont with pinned sample automatically unpins
     * the sample and removes all samples from cache */
    TEST_ASSERT(fluid_synth_pin_preset(synth, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 4);
    TEST_ASSERT(fluid_samplecache_count_entries() == 4);

    TEST_ASSERT(fluid_synth_sfunload(synth, id, 0) == FLUID_OK);
    TEST_ASSERT(fluid_samplecache_count_entries() == 0);


    /* Ensure that failures during load of preset results in an
     * error returned by pinning function */
    id = fluid_synth_sfload(synth, TEST_SOUNDFONT, 0);
    // hack the soundfont filename so the next load won't find the file anymore
    sfont = fluid_synth_get_sfont_by_id(synth, id);
    defsfont = fluid_sfont_get_data(sfont);
    defsfont->filename[0]++;

    TEST_ASSERT(fluid_synth_pin_preset(synth, id, 0, 42) == FLUID_FAILED);

    defsfont->filename[0]--;
    TEST_ASSERT(fluid_synth_sfunload(synth, id, 0) == FLUID_OK);


    /* Test that deleting the synth with a pinned preset also leaves no
     * samples in cache */
    id = fluid_synth_sfload(synth, TEST_SOUNDFONT, 0);

    TEST_ASSERT(fluid_synth_pin_preset(synth, id, 0, 42) == FLUID_OK);
    TEST_ASSERT(count_loaded_samples(synth, id) == 4);
    TEST_ASSERT(fluid_samplecache_count_entries() == 4);

    delete_fluid_synth(synth);

    TEST_ASSERT(fluid_samplecache_count_entries() == 0);


    /* Tear down */
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}


static int count_loaded_samples(fluid_synth_t *synth, int sfont_id)
{
    fluid_list_t *list;
    int count = 0;

    fluid_sfont_t *sfont = fluid_synth_get_sfont_by_id(synth, sfont_id);
    fluid_defsfont_t *defsfont = fluid_sfont_get_data(sfont);

    for(list = defsfont->sample; list; list = fluid_list_next(list))
    {
        fluid_sample_t *sample = fluid_list_get(list);

        if(sample->data != NULL)
        {
            count++;
        }
    }

    FLUID_LOG(FLUID_INFO, "Loaded samples on sfont %d: %d\n", sfont_id, count);
    return count;
}
