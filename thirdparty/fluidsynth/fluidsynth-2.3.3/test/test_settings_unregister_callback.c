
#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include <string.h>

static fluid_list_t* realtime_int_settings = NULL;
static fluid_list_t* realtime_str_settings = NULL;
static fluid_list_t* realtime_num_settings = NULL;

void iter_func (void *data, const char *name, int type)
{
    if(fluid_settings_is_realtime(data, name))
    {
        switch(type)
        {
            case FLUID_INT_TYPE:
                realtime_int_settings = fluid_list_prepend(realtime_int_settings, FLUID_STRDUP(name));
                break;
            case FLUID_STR_TYPE:
                realtime_str_settings = fluid_list_prepend(realtime_str_settings, FLUID_STRDUP(name));
                break;
            case FLUID_NUM_TYPE:
                realtime_num_settings = fluid_list_prepend(realtime_num_settings, FLUID_STRDUP(name));
                break;
            case FLUID_SET_TYPE:
                break;
            default:
                TEST_ASSERT(FALSE);
        }
    }
}

// this test should make sure that sample rate changed are handled correctly
int main(void)
{
    fluid_list_t* list;
    fluid_player_t* player;
    fluid_synth_t *synth;
    fluid_settings_t *settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    player = new_fluid_player(synth);
    TEST_ASSERT(player != NULL);
    
    // see which of the objects above has registered a realtime setting
    fluid_settings_foreach(settings, settings, iter_func);
    
    // delete the objects
    delete_fluid_player(player);
    delete_fluid_synth(synth);
    
    // and now, start making changes to those realtime settings
    // Anything below fluidsynth 2.1.5 will crash
    
    for(list = realtime_int_settings; list; list = fluid_list_next(list))
    {
        int min, max;
        char* name = fluid_list_get(list);
        TEST_SUCCESS(fluid_settings_getint_range(settings, name, &min, &max));
        TEST_SUCCESS(fluid_settings_setint(settings, name, min));
        FLUID_FREE(name);
    }

    delete_fluid_list(realtime_int_settings);
    
    for(list = realtime_num_settings; list; list = fluid_list_next(list))
    {
        double min, max;
        char* name = fluid_list_get(list);
        TEST_SUCCESS(fluid_settings_getnum_range(settings, name, &min, &max));
        TEST_SUCCESS(fluid_settings_setnum(settings, name, min));
        FLUID_FREE(name);
    }

    delete_fluid_list(realtime_num_settings);
    
    
    for(list = realtime_str_settings; list; list = fluid_list_next(list))
    {
        char* name = fluid_list_get(list);
        TEST_SUCCESS(fluid_settings_setstr(settings, name, "ABCDEFG"));
        FLUID_FREE(name);
    }

    delete_fluid_list(realtime_str_settings);

    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
