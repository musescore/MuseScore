
#include "test.h"
#include "fluidsynth.h" // use local fluidsynth header


static int callback_triggered = 0;
void callback_remove_events(unsigned int time, fluid_event_t *event, fluid_sequencer_t *seq, void *data)
{
    callback_triggered = 1;
}

void test_remove_events(fluid_sequencer_t *seq, fluid_event_t *evt)
{
    // silently creates a fluid_seqbind_t
    unsigned int i;
    int seqid = fluid_sequencer_register_client(seq, "remove event test", callback_remove_events, NULL);
    TEST_SUCCESS(seqid);

    fluid_event_set_source(evt, -1);
    fluid_event_set_dest(evt, seqid);
    fluid_event_control_change(evt, 0, 1, 127);

    for(i = 0; i < 10000; i++)
    {
        TEST_SUCCESS(fluid_sequencer_send_at(seq, evt, i, 0));
    }

    fluid_sequencer_remove_events(seq, -1, seqid, -1);
    fluid_sequencer_process(seq, i + fluid_sequencer_get_tick(seq));
    TEST_ASSERT(!callback_triggered);

    for(i = 0; i < 10000; i++)
    {
        fluid_event_control_change(evt, 0, 1, 127);
        TEST_SUCCESS(fluid_sequencer_send_at(seq, evt, i, 0));

        fluid_event_noteon(evt, 0, 64, 127);
        TEST_SUCCESS(fluid_sequencer_send_at(seq, evt, i, 0));
    }

    fluid_sequencer_remove_events(seq, -1, seqid, FLUID_SEQ_CONTROLCHANGE);
    fluid_sequencer_process(seq, i + fluid_sequencer_get_tick(seq));
    TEST_ASSERT(callback_triggered);

    callback_triggered = 0;

    for(i = 0; i < 10000; i++)
    {
        fluid_event_program_change(evt, 0, 0);
        TEST_SUCCESS(fluid_sequencer_send_at(seq, evt, i, 0));

        fluid_event_noteon(evt, 0, 64, 127);
        TEST_SUCCESS(fluid_sequencer_send_at(seq, evt, i, 0));
    }

    fluid_sequencer_remove_events(seq, -1, -1, -1);
    fluid_sequencer_process(seq, i + fluid_sequencer_get_tick(seq));
    TEST_ASSERT(!callback_triggered);

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

    test_remove_events(seq, evt);

    // client should be removed, deleting the seq should not free the struct again
    delete_fluid_event(evt);
    delete_fluid_sequencer(seq);

    return EXIT_SUCCESS;
}
