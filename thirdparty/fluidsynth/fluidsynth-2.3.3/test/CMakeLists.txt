
ENABLE_TESTING()
include ( FluidUnitTest )

# first define the test target, used by the macros below
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIG>  --output-on-failure)


## add unit tests here ##
ADD_FLUID_TEST(test_synth_reset_cc)
ADD_FLUID_TEST(test_sample_cache)
ADD_FLUID_TEST(test_sfont_loading)
ADD_FLUID_TEST(test_sample_rate_change)
ADD_FLUID_TEST(test_preset_sample_loading)
ADD_FLUID_TEST(test_preset_pinning)
ADD_FLUID_TEST(test_bug_635)
ADD_FLUID_TEST(test_settings_unregister_callback)
ADD_FLUID_TEST(test_pointer_alignment)
ADD_FLUID_TEST(test_seqbind_unregister)
ADD_FLUID_TEST(test_synth_chorus_reverb)
ADD_FLUID_TEST(test_snprintf)
ADD_FLUID_TEST(test_synth_process)
ADD_FLUID_TEST(test_ct2hz)
ADD_FLUID_TEST(test_sample_validate)
ADD_FLUID_TEST(test_sfont_unloading)
ADD_FLUID_TEST(test_sfont_zone)
ADD_FLUID_TEST(test_seq_event_queue_sort)
ADD_FLUID_TEST(test_seq_scale)
ADD_FLUID_TEST(test_seq_evt_order)
ADD_FLUID_TEST(test_seq_event_queue_remove)
ADD_FLUID_TEST(test_jack_obtaining_synth)
ADD_FLUID_TEST(test_utf8_open)

ADD_FLUID_TEST_UTIL(dump_sfont)

ADD_FLUID_SF_DUMP_TEST(VintageDreamsWaves-v2.sf2)

if ( LIBSNDFILE_HASVORBIS )
    ADD_FLUID_TEST(test_sf3_sfont_loading)
    ADD_FLUID_SF_DUMP_TEST(VintageDreamsWaves-v2.sf3)
endif ( LIBSNDFILE_HASVORBIS )
