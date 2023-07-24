
#include "test.h"
#include "fluidsynth.h" // use local fluidsynth header
#include "fluid_seq_queue.h"

// simple test to ensure that manually unregistering and deleting the internal fluid_seqbind_t works without crashing
int main(void)
{
    fluid_event_t* evt1 = new_fluid_event();
    fluid_event_t* evt2 = new_fluid_event();

    fluid_event_set_time(evt1, 1);
    fluid_event_set_time(evt2, 1);

    // Note that event_compare() returns !leftIsBeforeRight

    TEST_ASSERT( !event_compare_for_test(evt1, evt1));
    TEST_ASSERT( !event_compare_for_test(evt2, evt2));

    fluid_event_bank_select(evt1, 0, 0);
    fluid_event_program_change(evt2, 0, 0);

    TEST_ASSERT( !event_compare_for_test(evt1, evt2));
    TEST_ASSERT(!!event_compare_for_test(evt2, evt1));

    fluid_event_note(evt1, 0, 0, 0, 1);

    TEST_ASSERT(!!event_compare_for_test(evt1, evt2));
    TEST_ASSERT( !event_compare_for_test(evt2, evt1));

    fluid_event_noteon(evt1, 0, 0, 60);
    fluid_event_noteoff(evt2, 0, 0);

    TEST_ASSERT(!!event_compare_for_test(evt1, evt2));
    TEST_ASSERT( !event_compare_for_test(evt2, evt1));

    // make sure noteons with vel=0 are handled like noteoffs
    fluid_event_noteon(evt1, 0, 0, 60);
    fluid_event_noteon(evt2, 0, 0, 0);

    TEST_ASSERT(!!event_compare_for_test(evt1, evt2));
    TEST_ASSERT( !event_compare_for_test(evt2, evt1));

    // two noteoffs
    fluid_event_noteon(evt1, 0, 0, 0);
    fluid_event_noteoff(evt2, 0, 0);

    TEST_ASSERT( !event_compare_for_test(evt1, evt2));
    TEST_ASSERT( !event_compare_for_test(evt2, evt1));

    fluid_event_unregistering(evt1);
    fluid_event_system_reset(evt2);

    TEST_ASSERT(!!event_compare_for_test(evt1, evt2));
    TEST_ASSERT( !event_compare_for_test(evt2, evt1));

    fluid_event_unregistering(evt1);
    fluid_event_pan(evt2, 0, 0);

    TEST_ASSERT( !event_compare_for_test(evt1, evt2));
    TEST_ASSERT(!!event_compare_for_test(evt2, evt1));

    fluid_event_modulation(evt1, 0, 0);
    fluid_event_pan(evt2, 0, 0);

    TEST_ASSERT( !event_compare_for_test(evt1, evt2));
    TEST_ASSERT( !event_compare_for_test(evt2, evt1));

    delete_fluid_event(evt1);
    delete_fluid_event(evt2);

    return EXIT_SUCCESS;
}
