
#include "test.h"
#include "fluidsynth.h" // use local fluidsynth header
#include "fluid_event.h"

#include <limits.h>

static short order = 0;
void callback_stable_sort(unsigned int time, fluid_event_t *event, fluid_sequencer_t *seq, void *data)
{
    static const int expected_type_order[] =
    { FLUID_SEQ_NOTEOFF, FLUID_SEQ_NOTEON, FLUID_SEQ_NOTEOFF, FLUID_SEQ_NOTEON, FLUID_SEQ_SYSTEMRESET, FLUID_SEQ_UNREGISTERING };

    TEST_ASSERT(fluid_event_get_type(event) == expected_type_order[order++]);
}

void test_order_same_tick(fluid_sequencer_t *seq, fluid_event_t *evt)
{
    // silently creates a fluid_seqbind_t
    int i, seqid = fluid_sequencer_register_client(seq, "test order at same tick", callback_stable_sort, NULL);
    TEST_SUCCESS(seqid);
    TEST_ASSERT(fluid_sequencer_count_clients(seq) == 1);

    fluid_event_set_source(evt, -1);
    fluid_event_set_dest(evt, seqid);

    for(i = 1; i <= 2; i++)
    {
        fluid_event_noteoff(evt, 0, 64);
        TEST_SUCCESS(fluid_sequencer_send_at(seq, evt, i, 1));
        fluid_event_noteon(evt, 0, 64, 127);
        TEST_SUCCESS(fluid_sequencer_send_at(seq, evt, i, 1));
    }

    fluid_event_system_reset(evt);
    TEST_SUCCESS(fluid_sequencer_send_at(seq, evt, 2, 1));
    fluid_event_unregistering(evt);
    TEST_SUCCESS(fluid_sequencer_send_at(seq, evt, 2, 1));

    fluid_sequencer_process(seq, 1);
    TEST_ASSERT(order == 2);

    fluid_sequencer_process(seq, 2);
    TEST_ASSERT(order == 6);

    fluid_sequencer_unregister_client(seq, seqid);
    TEST_ASSERT(fluid_sequencer_count_clients(seq) == 0);
}

static unsigned int prev_time, done = FALSE;
void callback_correct_order(unsigned int time, fluid_event_t *event, fluid_sequencer_t *seq, void *data)
{
    if(done)
    {
        TEST_ASSERT(fluid_event_get_type(event) == FLUID_SEQ_UNREGISTERING);
    }
    else
    {
        TEST_ASSERT(fluid_event_get_type(event) == FLUID_SEQ_CONTROLCHANGE);
    }
    TEST_ASSERT(prev_time <= fluid_event_get_time(event) && fluid_event_get_time(event) <= prev_time + 1);
    prev_time = fluid_event_get_time(event);
}

void test_correct_order(fluid_sequencer_t *seq, fluid_event_t *evt)
{
    // silently creates a fluid_seqbind_t
    unsigned int i, offset;
    int seqid = fluid_sequencer_register_client(seq, "correct order test", callback_correct_order, NULL);
    TEST_SUCCESS(seqid);

    fluid_event_set_source(evt, -1);
    fluid_event_set_dest(evt, seqid);
    fluid_event_control_change(evt, 0, 1, 127);

    for(i = 0; i < 10000; i++)
    {
        TEST_SUCCESS(fluid_sequencer_send_at(seq, evt, 10000 - i, 0));
    }

    for(; i <= 10000 + 20000; i++)
    {
        TEST_SUCCESS(fluid_sequencer_send_at(seq, evt, i, 0));
    }

    for(; i < 80000; i++)
    {
        TEST_SUCCESS(fluid_sequencer_send_at(seq, evt, 80000 - (i - 10000 - 20000), 0));
    }

    for(; i < 200000; i++)
    {
        TEST_SUCCESS(fluid_sequencer_send_at(seq, evt, i, 0));
    }

    offset = prev_time = fluid_sequencer_get_tick(seq);
    fluid_sequencer_process(seq, i + offset);
    TEST_ASSERT(prev_time == (i - 1) + offset);

    done = TRUE;
    fluid_sequencer_unregister_client(seq, seqid);
}

// simple test to ensure that manually unregistering and deleting the internal fluid_seqbind_t works without crashing
int main(void)
{
    fluid_event_t *evt;
    fluid_sequencer_t *seq = new_fluid_sequencer2(0 /*i.e. use sample timer*/);
    TEST_ASSERT(seq != NULL);
    TEST_ASSERT(fluid_sequencer_get_use_system_timer(seq) == 0);
    evt = new_fluid_event();
    TEST_ASSERT(evt != NULL);

    test_order_same_tick(seq, evt);
    test_correct_order(seq, evt);

    // client should be removed, deleting the seq should not free the struct again
    delete_fluid_event(evt);
    delete_fluid_sequencer(seq);

    return EXIT_SUCCESS;
}
