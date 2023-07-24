
#include "test.h"
#include "fluidsynth.h"
#include "sfloader/fluid_sfont.h"
#include "sfloader/fluid_defsfont.h"
#include "sfloader/fluid_sffile.h"
#include "utils/fluid_sys.h"


#define SET_BUF2(START, SIZE)        \
    do                               \
    {                                \
        file_buf = START;            \
        file_end = (START) + (SIZE); \
    } while (0)
#define SET_BUF(BUF) SET_BUF2(BUF, FLUID_N_ELEMENTS(BUF))
#define UNSET_BUF        \
    do                   \
    {                    \
        file_buf = NULL; \
        file_end = NULL; \
    } while (0)


typedef struct
{
    // pointer to the start of the file_buf
    const unsigned char *start;
    // actual size of the buffer
    unsigned int size;
    // expected end address of the buffer
    const unsigned char *end;
} buf_t;

static const unsigned char *file_buf = NULL;
static const unsigned char *file_end = NULL;
static int test_reader(void *buf, fluid_long_long_t count, void *h)
{
    if (file_buf + count > file_end)
    {
        return FLUID_FAILED;
    }
    FLUID_MEMCPY(buf, file_buf, count);
    file_buf += count;
    return FLUID_OK;
}

static int test_seek(void *handle, fluid_long_long_t offset, int origin)
{
    if (origin == SEEK_CUR)
    {
        file_buf += offset;
        if (file_buf > file_end)
        {
            return FLUID_FAILED;
        }
        return FLUID_OK;
    }

    // shouldn't happen?
    TEST_ASSERT(0);
}

static const fluid_file_callbacks_t fcb =
{
    NULL, &test_reader, &test_seek, NULL, NULL
};

static SFZone* new_test_zone(fluid_list_t** parent_list, int gen_count)
{
    int i;
    SFZone *zone = FLUID_NEW(SFZone);
    TEST_ASSERT(zone != NULL);
    FLUID_MEMSET(zone, 0, sizeof(*zone));
    
    for (i = 0; i < gen_count; i++)
    {
        zone->gen = fluid_list_prepend(zone->gen, NULL);
    }
    
    if(parent_list != NULL)
    {
        *parent_list = fluid_list_append(*parent_list, zone);
    }
    
    return zone;
}

// test the good case first: one zone, with two generators and one terminal generator
static void good_test_1zone_2gen_1termgen(int (*load_func)(SFData *sf, int size), SFData* sf, SFZone *zone)
{
    const SFGen *gen;
    static const unsigned char buf[] =
    {
        GEN_KEYRANGE, 0, 60, 127, GEN_VELRANGE, 0, 60, 127, 0, 0, 0, 0
    };
    SET_BUF(buf);
    TEST_ASSERT(load_func(sf, FLUID_N_ELEMENTS(buf)));

    gen = fluid_list_get(fluid_list_nth(zone->gen, 0));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == GEN_KEYRANGE);
    TEST_ASSERT(gen->amount.range.lo == 60);
    TEST_ASSERT(gen->amount.range.hi == 127);

    gen = fluid_list_get(fluid_list_nth(zone->gen, 1));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == GEN_VELRANGE);
    TEST_ASSERT(gen->amount.range.lo == 60);
    TEST_ASSERT(gen->amount.range.hi == 127);

    TEST_ASSERT(file_buf == buf + sizeof(buf));
    UNSET_BUF;
}

// bad case: too few generators in buffer, triggering a chunk size mismatch
static void bad_test_too_short_gen_buffer(int (*load_func)(SFData *sf, int size), SFData *sf, SFZone *zone)
{
    const unsigned char final_gen = (load_func == &load_pgen) ? GEN_INSTRUMENT : GEN_SAMPLEID;
    SFGen *gen;
    unsigned int i;
    static const unsigned char buf1[] = { GEN_KEYRANGE, 0, 0 };
    static const unsigned char buf2[] = { GEN_KEYRANGE, 0 };
    static const unsigned char buf3[] = { GEN_KEYRANGE };
    static const unsigned char buf8[] = { GEN_VELRANGE, 0, 0 };
    static const unsigned char buf9[] = { GEN_VELRANGE, 0 };
    static const unsigned char buf10[] = { GEN_VELRANGE };
    static const unsigned char buf4[] = { GEN_VELRANGE, 0, 0, 127, GEN_COARSETUNE, 0, 4 };
    static const unsigned char buf5[] = { GEN_VELRANGE, 0, 0, 127, GEN_COARSETUNE, 0 };
    static const unsigned char buf6[] = { GEN_VELRANGE, 0, 0, 127, GEN_COARSETUNE };
    const unsigned char buf11[] = { GEN_VELRANGE, 0, 0, 127, final_gen, 0, 4 };
    const unsigned char buf12[] = { GEN_VELRANGE, 0, 0, 127, final_gen, 0 };
    const unsigned char buf13[] = { GEN_VELRANGE, 0, 0, 127, final_gen };
    static const unsigned char buf7[] = { GEN_KEYRANGE, 0, 60, 127, GEN_OVERRIDEROOTKEY };

    static const buf_t buf_with_one_gen[] =
    {
        { buf1, sizeof(buf1), buf1 + sizeof(buf1) },
        { buf2, sizeof(buf2),buf2 + sizeof(buf2) },
        { buf3, sizeof(buf3), buf3 },
        { buf8, sizeof(buf8), buf8 + sizeof(buf8) },
        { buf9, sizeof(buf9), buf9 + sizeof(buf9) },
        { buf10, sizeof(buf10), buf10 }
    };

    const buf_t buf_with_two_gen[] =
    {
        { buf4, sizeof(buf4), buf4 + sizeof(buf4) -1 },
        { buf5, sizeof(buf5), buf5 + sizeof(buf5) },
        { buf6, sizeof(buf6), buf6 + sizeof(buf6) - 1 },
        { buf11, sizeof(buf11), buf11 + sizeof(buf11) - 1 },
        { buf12, sizeof(buf12), buf12 + sizeof(buf12) },
        { buf13, sizeof(buf13), buf13 + sizeof(buf13) -1}
    };

    for (i = 0; i < FLUID_N_ELEMENTS(buf_with_one_gen); i++)
    {
        SET_BUF2(buf_with_one_gen[i].start, buf_with_one_gen[i].size);
        TEST_ASSERT(load_func(sf, 8 /* pretend that our input buffer is big enough, to make it fail in the fcbs later */) == FALSE);
        gen = fluid_list_get(fluid_list_nth(zone->gen, 0));
        TEST_ASSERT(gen == NULL);
        TEST_ASSERT(file_buf == buf_with_one_gen[i].end);
        UNSET_BUF;
    }

    for (i = 0; i < FLUID_N_ELEMENTS(buf_with_two_gen); i++)
    {
        SET_BUF2(buf_with_two_gen[i].start, buf_with_two_gen[i].size);
        TEST_ASSERT(load_func(sf, 8) == FALSE);
        gen = fluid_list_get(fluid_list_nth(zone->gen, 0));
        TEST_ASSERT(gen != NULL);
        FLUID_FREE(gen);
        zone->gen->data = NULL;
        gen = fluid_list_get(fluid_list_nth(zone->gen, 1));
        TEST_ASSERT(gen == NULL);
        TEST_ASSERT(file_buf == buf_with_two_gen[i].end);
        UNSET_BUF;
    }

    SET_BUF(buf7);
    TEST_ASSERT(load_func(sf, FLUID_N_ELEMENTS(buf7)) == FALSE);
    gen = fluid_list_get(fluid_list_nth(zone->gen, 0));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == GEN_KEYRANGE);
    TEST_ASSERT(gen->amount.range.lo == 60);
    TEST_ASSERT(gen->amount.range.hi == 127);

    TEST_ASSERT(file_buf == buf7 + sizeof(buf7) - 1);
    UNSET_BUF;
}

// bad case: one zone, with two similar generators
static void bad_test_duplicate_gen(int (*load_func)(SFData *sf, int size), SFData *sf, SFZone *zone)
{
    const SFGen *gen;
    static const unsigned char buf[] = { GEN_COARSETUNE, 0, 5, 0, GEN_COARSETUNE, 0, 10, 0 };

    SET_BUF(buf);
    TEST_ASSERT(load_func(sf, FLUID_N_ELEMENTS(buf)));

    gen = fluid_list_get(fluid_list_nth(zone->gen, 0));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == GEN_COARSETUNE);
    TEST_ASSERT(gen->amount.sword == 10);

    gen = fluid_list_get(fluid_list_nth(zone->gen, 1));
    TEST_ASSERT(gen == NULL);

    TEST_ASSERT(file_buf == buf + sizeof(buf));
    UNSET_BUF;
}

// bad case: with one zone, generators in wrong order
static void bad_test_gen_wrong_order(int (*load_func)(SFData *sf, int size), SFData *sf, SFZone *zone)
{
    const SFGen *gen;
    static const unsigned char buf[] =
    {
        GEN_VELRANGE, 0, 60, 127,
        GEN_KEYRANGE, 0, 60, 127,
        GEN_INSTRUMENT, 0, 0xDD, 0xDD
    };
    SET_BUF(buf);
    TEST_ASSERT(load_func(sf, FLUID_N_ELEMENTS(buf)));

    gen = fluid_list_get(fluid_list_nth(zone->gen, 0));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == GEN_VELRANGE);
    TEST_ASSERT(gen->amount.range.lo == 60);
    TEST_ASSERT(gen->amount.range.hi == 127);

    gen = fluid_list_get(fluid_list_nth(zone->gen, 1));
    if (load_func == &load_igen)
    {
        TEST_ASSERT(gen == NULL);
    }
    else
    {
        TEST_ASSERT(gen != NULL);
        TEST_ASSERT(gen->id == GEN_INSTRUMENT);
        TEST_ASSERT(gen->amount.uword == 0xDDDDu);
    }
   
    gen = fluid_list_get(fluid_list_nth(zone->gen, 2));
    TEST_ASSERT(gen == NULL);

    TEST_ASSERT(file_buf == buf + sizeof(buf));
    UNSET_BUF;
}

// This test-case is derived from the invalid SoundFont provided in #808
static void bad_test_issue_808(int (*load_func)(SFData *sf, int size), SFData *sf, SFZone *zone1)
{
    const SFGen *gen;
    static const unsigned char buf[] =
    {
        // zone 1
        GEN_REVERBSEND, 0, 50, 0,
        GEN_VOLENVRELEASE, 0, 0xCE, 0xF9,
        // zone 2
        GEN_KEYRANGE, 0, 0, 35,
        GEN_OVERRIDEROOTKEY, 0, 43, 0,
        GEN_STARTADDRCOARSEOFS, 0, 0, 0,
        GEN_SAMPLEMODE, 0, 1, 0,
        GEN_STARTADDROFS, 0, 0, 0
    };

    SET_BUF(buf);
    TEST_ASSERT(load_func(sf, FLUID_N_ELEMENTS(buf)));

    gen = fluid_list_get(fluid_list_nth(zone1->gen, 0));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == GEN_REVERBSEND);
    TEST_ASSERT(gen->amount.uword == 50);

    gen = fluid_list_get(fluid_list_nth(zone1->gen, 1));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == GEN_VOLENVRELEASE);
    TEST_ASSERT(gen->amount.uword == 0xF9CE);

    gen = fluid_list_get(fluid_list_nth(zone1->gen, 2));
    TEST_ASSERT(gen == NULL);

    TEST_ASSERT(file_buf == buf + sizeof(buf));
    UNSET_BUF;
}


// This test-case is derived from #1250
static void default_key_range_test_issue_1250(int (*load_func)(SFData *sf, int size), SFData *sf, SFZone *zone1, SFZone *zone2)
{
    const SFGen *gen;
    static const unsigned char buf[] =
    {
        // zone 1
        GEN_KEYRANGE, 0, 105, 114,
        GEN_OVERRIDEROOTKEY, 0, 110, 0,
        // zone 2
        GEN_PAN, 0, 50, 0,
        GEN_SAMPLEMODE, 0, 1, 0,
        GEN_SAMPLEID, 0, 0, 0,
    };

    SET_BUF(buf);
    TEST_ASSERT(load_func(sf, FLUID_N_ELEMENTS(buf)));

    gen = fluid_list_get(fluid_list_nth(zone1->gen, 0));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == GEN_KEYRANGE);
    TEST_ASSERT(gen->amount.range.lo == 105);
    TEST_ASSERT(gen->amount.range.hi == 114);

    gen = fluid_list_get(fluid_list_nth(zone1->gen, 1));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == GEN_OVERRIDEROOTKEY);
    TEST_ASSERT(gen->amount.uword == 110);

    gen = fluid_list_get(fluid_list_nth(zone1->gen, 2));
    TEST_ASSERT(gen == NULL);

    gen = fluid_list_get(fluid_list_nth(zone2->gen, 0));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == GEN_PAN);
    TEST_ASSERT(gen->amount.uword == 50);

    gen = fluid_list_get(fluid_list_nth(zone2->gen, 1));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == GEN_SAMPLEMODE);
    TEST_ASSERT(gen->amount.uword == 1);

    gen = fluid_list_get(fluid_list_nth(zone2->gen, 2));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == GEN_SAMPLEID);
    TEST_ASSERT(gen->amount.uword == 0);

    gen = fluid_list_get(fluid_list_nth(zone2->gen, 3));
    TEST_ASSERT(gen == NULL);

    TEST_ASSERT(file_buf == buf + sizeof(buf));
    UNSET_BUF;
}

// This test-case has a single zone which has additional generators after the final generator, while some of them are incomplete and others still have an extra (maybe incomplete) terminal gen.
static void bad_test_additional_gens_after_final_gen(int (*load_func)(SFData *sf, int size), SFData *sf, SFZone *zone1)
{
    unsigned int i;
    SFGen *gen;
    const unsigned char final_gen = (load_func == &load_pgen) ? GEN_INSTRUMENT : GEN_SAMPLEID;

    const unsigned char buf1[] =
    {
        // zone 1
        GEN_KEYRANGE, 0, 60, 127,
        GEN_UNUSED1, 0, 0xFF, 0xFF,
        final_gen, 0, 0xDD, 0xDD,
        GEN_KEYRANGE, 0, 0, 35,
        GEN_OVERRIDEROOTKEY, 0, 43, 0,
        0, 0, 0, 0 // terminal generator
    };

    const unsigned char buf2[] =
    {
        // zone 1
        GEN_KEYRANGE, 0, 60, 127,
        GEN_UNUSED1, 0, 0xFF, 0xFF,
        final_gen, 0, 0xDD, 0xDD,
        GEN_KEYRANGE, 0, 0, 35,
        GEN_OVERRIDEROOTKEY, 0, 43, 0,
        0, 0, 0 // incomplete terminal generator
    };

    const unsigned char buf3[] =
    {
        // zone 1
        GEN_KEYRANGE, 0, 60, 127,
        GEN_UNUSED1, 0, 0xFF, 0xFF,
        final_gen, 0, 0xDD, 0xDD,
        GEN_KEYRANGE, 0, 0, 35,
        GEN_OVERRIDEROOTKEY, 0, 43
    };

    const unsigned char buf4[] =
    {
        // zone 1
        GEN_KEYRANGE, 0, 60, 127,
        GEN_UNUSED1, 0, 0xFF, 0xFF,
        final_gen, 0, 0xDD, 0xDD,
        GEN_KEYRANGE, 0, 0, 35,
        GEN_OVERRIDEROOTKEY, 0
    };

    const buf_t buf[] =
    {
        { buf1, sizeof(buf1), buf1 + sizeof(buf1) },
        { buf2, sizeof(buf2), buf2 + sizeof(buf2) - 3 },
        { buf3, sizeof(buf3), buf3 + sizeof(buf3) - 3 },
        { buf4, sizeof(buf4), buf4 + sizeof(buf4) - 2 },
    };

    // the first test case should return true, all others false
    int expected_ret_val = TRUE;
    for (i = 0; i < FLUID_N_ELEMENTS(buf); i++)
    {
        SET_BUF2(buf[i].start, buf[i].size);
        TEST_ASSERT(load_func(sf, buf[i].size) == expected_ret_val);
        expected_ret_val = FALSE;

        gen = fluid_list_get(fluid_list_nth(zone1->gen, 0));
        TEST_ASSERT(gen != NULL);
        TEST_ASSERT(gen->id == GEN_KEYRANGE);
        TEST_ASSERT(gen->amount.range.lo == 60);
        TEST_ASSERT(gen->amount.range.hi == 127);

        // delete this generator
        FLUID_FREE(gen);
        zone1->gen->data = NULL;

        gen = fluid_list_get(fluid_list_nth(zone1->gen, 1));
        TEST_ASSERT(gen != NULL);
        if (load_func == &load_igen)
        {
            TEST_ASSERT(gen->id == GEN_SAMPLEID);
        }
        else
        {
            TEST_ASSERT(gen->id == GEN_INSTRUMENT);
        }
        TEST_ASSERT(gen->amount.uword == 0xDDDDu);
        // delete this generator
        FLUID_FREE(gen);
        zone1->gen->data = NULL;

        gen = fluid_list_get(fluid_list_nth(zone1->gen, 2));
        TEST_ASSERT(gen == NULL);

        gen = fluid_list_get(fluid_list_nth(zone1->gen, 3));
        TEST_ASSERT(gen == NULL);

        gen = fluid_list_get(fluid_list_nth(zone1->gen, 4));
        TEST_ASSERT(gen == NULL);

        TEST_ASSERT(file_buf == buf[i].end);
        UNSET_BUF;

        // The test cases above expect zone1 to be pre-populated with 5 generators
        delete_fluid_list(zone1->gen);
        zone1->gen = NULL;
        zone1->gen = fluid_list_prepend(zone1->gen, NULL);
        zone1->gen = fluid_list_prepend(zone1->gen, NULL);
        zone1->gen = fluid_list_prepend(zone1->gen, NULL);
        zone1->gen = fluid_list_prepend(zone1->gen, NULL);
        zone1->gen = fluid_list_prepend(zone1->gen, NULL);
    }
}

int main(void)
{
    // prepare a soundfont that has one preset and one instrument, with up to 2 zones

    SFZone *zone1, *zone2;
    SFData *sf = FLUID_NEW(SFData);
    SFPreset *preset = FLUID_NEW(SFPreset);
    SFInst *inst = FLUID_NEW(SFInst);

    TEST_ASSERT(sf != NULL);
    FLUID_MEMSET(sf, 0, sizeof(*sf));
    TEST_ASSERT(preset != NULL);
    FLUID_MEMSET(preset, 0, sizeof(*preset));
    TEST_ASSERT(inst != NULL);
    FLUID_MEMSET(inst, 0, sizeof(*inst));

    sf->fcbs = &fcb;
    sf->preset = fluid_list_append(sf->preset, preset);
    sf->inst = fluid_list_append(sf->inst, inst);

    // Calls the given test function for 1 zone once for preset and once for inst case.
    #define TEST_CASE_1(TEST_FUNC, GEN_COUNT)            \
    do                                                   \
    {                                                    \
        zone1 = new_test_zone(&preset->zone, GEN_COUNT); \
        TEST_FUNC(&load_pgen, sf, zone1);                \
        delete_zone(zone1);                              \
        delete_fluid_list(preset->zone);                 \
        preset->zone = NULL;                             \
                                                         \
        zone1 = new_test_zone(&inst->zone, GEN_COUNT);   \
        TEST_FUNC(&load_igen, sf, zone1);                \
        delete_zone(zone1);                              \
        delete_fluid_list(inst->zone);                   \
        inst->zone = NULL;                               \
    } while (0)

    TEST_CASE_1(good_test_1zone_2gen_1termgen, 2);
    TEST_CASE_1(good_test_1zone_2gen_1termgen, 3);

    TEST_CASE_1(bad_test_too_short_gen_buffer, 2);

    TEST_CASE_1(bad_test_duplicate_gen, 2);

    TEST_CASE_1(bad_test_gen_wrong_order, 3);

    TEST_CASE_1(bad_test_additional_gens_after_final_gen, 5);

    zone1 = new_test_zone(&preset->zone, 2);
    (void)new_test_zone(&preset->zone, 5);
    bad_test_issue_808(&load_pgen, sf, zone1);
    // zone 2 was dropped
    TEST_ASSERT(preset->zone->next == NULL);
    delete_zone(zone1);
    // zone2 already deleted
    delete_fluid_list(preset->zone);
    preset->zone = NULL;

    zone1 = new_test_zone(&inst->zone, 2);
    (void)new_test_zone(&inst->zone, 5);
    bad_test_issue_808(&load_igen, sf, zone1);
    // zone 2 was dropped
    TEST_ASSERT(inst->zone->next == NULL);
    delete_zone(zone1);
    // zone2 already deleted
    delete_fluid_list(inst->zone);
    inst->zone = NULL;


    zone1 = new_test_zone(&inst->zone, 2);
    zone2 = new_test_zone(&inst->zone, 3);
    
    TEST_ASSERT(inst->zone->next != NULL);
    default_key_range_test_issue_1250(&load_igen, sf, zone1, zone2);
    
    TEST_ASSERT(inst->zone->data == zone1);
    TEST_ASSERT(inst->zone->next->data == zone2);
    delete_zone(zone2);
    delete_zone(zone1);
    delete_fluid_list(inst->zone);
    inst->zone = NULL;


    delete_inst(inst);
    delete_preset(preset);
    delete_fluid_list(sf->inst);
    delete_fluid_list(sf->preset);
    // we cannot call fluid_sffile_close here, because it would destroy the mutex which is not initialized
    FLUID_FREE(sf);
    return EXIT_SUCCESS;
}
