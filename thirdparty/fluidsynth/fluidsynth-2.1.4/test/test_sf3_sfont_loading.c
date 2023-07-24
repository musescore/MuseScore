#include "test.h"
#include "fluidsynth.h"
#include "sfloader/fluid_sfont.h"
#include "utils/fluid_sys.h"


int main(void)
{
    int id;
    fluid_sfont_t *sfont;

    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth = new_fluid_synth(settings);

    TEST_ASSERT(settings != NULL);
    TEST_ASSERT(synth != NULL);

    // no sfont loaded
    TEST_ASSERT(fluid_synth_sfcount(synth) == 0);

    FLUID_LOG(FLUID_INFO, "Attempt to open %s", TEST_SOUNDFONT_SF3);

    // load a sfont to synth
    TEST_SUCCESS(id = fluid_synth_sfload(synth, TEST_SOUNDFONT_SF3, 1));

    // one sfont loaded
    TEST_ASSERT(fluid_synth_sfcount(synth) == 1);

    sfont = fluid_synth_get_sfont_by_id(synth, id);
    TEST_ASSERT(sfont != NULL);

    // this is still the same filename as we've put in
    TEST_ASSERT(FLUID_STRCMP(TEST_SOUNDFONT_SF3, fluid_sfont_get_name(sfont)) == 0);
    TEST_ASSERT(fluid_sfont_get_id(sfont) == id);

    // still the same id?
    TEST_ASSERT(fluid_synth_sfreload(synth, id) == id);
    // one sfont loaded
    TEST_ASSERT(fluid_synth_sfcount(synth) == 1);

    sfont = fluid_synth_get_sfont_by_id(synth, id);
    TEST_ASSERT(sfont != NULL);

    // still the same filename?
    TEST_ASSERT(FLUID_STRCMP(TEST_SOUNDFONT_SF3, fluid_sfont_get_name(sfont)) == 0);
    // correct id stored?
    TEST_ASSERT(fluid_sfont_get_id(sfont) == id);

    // remove the sfont without deleting
    TEST_SUCCESS(fluid_synth_remove_sfont(synth, sfont));

    // no sfont loaded
    TEST_ASSERT(fluid_synth_sfcount(synth) == 0);

    // re-add the sfont without deleting
    TEST_SUCCESS(id = fluid_synth_add_sfont(synth, sfont));

    // one sfont loaded
    TEST_ASSERT(fluid_synth_sfcount(synth) == 1);

    // destroy the sfont
    TEST_SUCCESS(fluid_synth_sfunload(synth, id, 0));
    // no sfont loaded
    TEST_ASSERT(fluid_synth_sfcount(synth) == 0);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
