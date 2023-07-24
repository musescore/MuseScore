
#include "test.h"
#include "fluidsynth.h" // use local fluidsynth header

static void test_unregister_with_event(fluid_synth_t *synth)
{
    fluid_sequencer_t *seq = new_fluid_sequencer2(0 /*i.e. use sample timer*/);

    // silently creates a fluid_seqbind_t
    int seqid = fluid_sequencer_register_fluidsynth(seq, synth);

    fluid_event_t *evt = new_fluid_event();
    fluid_event_set_source(evt, -1);
    fluid_event_set_dest(evt, seqid);

    // manually free the fluid_seqbind_t
    fluid_event_unregistering(evt);
    fluid_sequencer_send_now(seq, evt);

    // client should be removed, deleting the seq should not free the struct again
    delete_fluid_event(evt);
    delete_fluid_sequencer(seq);
}

static void test_unregister_with_client_unregister(fluid_synth_t *synth)
{
    fluid_sequencer_t *seq = new_fluid_sequencer2(0 /*i.e. use sample timer*/);

    // silently creates a fluid_seqbind_t
    int seqid = fluid_sequencer_register_fluidsynth(seq, synth);
    fluid_sequencer_unregister_client(seq, seqid);

    // client should be removed, deleting the seq should not free the struct again
    delete_fluid_sequencer(seq);
}

static void test_unregister_without_unregister(fluid_synth_t *synth)
{
    fluid_sequencer_t *seq = new_fluid_sequencer2(0 /*i.e. use sample timer*/);

    // silently creates a fluid_seqbind_t
    fluid_sequencer_register_fluidsynth(seq, synth);

    // client should be removed
    delete_fluid_sequencer(seq);
}

// test to ensure that unregistering and deleting the internal fluid_seqbind_t works without double-free
//
// valgrind should be used to ensure that there are no memory leaks
int main(void)
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth = new_fluid_synth(settings);

    test_unregister_with_event(synth);
    test_unregister_with_client_unregister(synth);
    test_unregister_without_unregister(synth);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
