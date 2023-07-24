
#include "test.h"
#include "fluidsynth.h"
#include "sfloader/fluid_sfont.h"
#include "utils/fluid_sys.h"


// this tests ensures that samples with invalid SfSampleType flag combinations are rejected
int main(void)
{
    fluid_sample_t* sample = new_fluid_sample();
    sample->start = 0;
    sample->end = 1;

    /// valid flags
    {
        sample->sampletype = FLUID_SAMPLETYPE_OGG_VORBIS;
        TEST_SUCCESS(fluid_sample_validate(sample, 2));

        sample->sampletype = FLUID_SAMPLETYPE_LINKED;
        TEST_SUCCESS(fluid_sample_validate(sample, 2));

        sample->sampletype = FLUID_SAMPLETYPE_MONO;
        TEST_SUCCESS(fluid_sample_validate(sample, 2));

        sample->sampletype = FLUID_SAMPLETYPE_RIGHT;
        TEST_SUCCESS(fluid_sample_validate(sample, 2));

        sample->sampletype = FLUID_SAMPLETYPE_LEFT;
        TEST_SUCCESS(fluid_sample_validate(sample, 2));
    }

    /// valid, but unsupported linked sample flags
    {
        sample->sampletype = FLUID_SAMPLETYPE_LINKED;
        TEST_SUCCESS(fluid_sample_validate(sample, 2));

        sample->sampletype = FLUID_SAMPLETYPE_LINKED | FLUID_SAMPLETYPE_OGG_VORBIS;
        TEST_SUCCESS(fluid_sample_validate(sample, 2));

        sample->sampletype = FLUID_SAMPLETYPE_LINKED | FLUID_SAMPLETYPE_MONO;
        TEST_SUCCESS(fluid_sample_validate(sample, 2));

        sample->sampletype = FLUID_SAMPLETYPE_LINKED | FLUID_SAMPLETYPE_RIGHT;
        TEST_SUCCESS(fluid_sample_validate(sample, 2));

        sample->sampletype = FLUID_SAMPLETYPE_LINKED | FLUID_SAMPLETYPE_LEFT;
        TEST_SUCCESS(fluid_sample_validate(sample, 2));
    }

    /// valid, but rejected ROM sample flags
    {
        sample->sampletype = FLUID_SAMPLETYPE_ROM;
        TEST_ASSERT(fluid_sample_validate(sample, 2) == FLUID_FAILED);

        sample->sampletype = FLUID_SAMPLETYPE_ROM | FLUID_SAMPLETYPE_OGG_VORBIS;
        TEST_ASSERT(fluid_sample_validate(sample, 2) == FLUID_FAILED);

        sample->sampletype = FLUID_SAMPLETYPE_ROM | FLUID_SAMPLETYPE_LINKED;
        TEST_ASSERT(fluid_sample_validate(sample, 2) == FLUID_FAILED);

        sample->sampletype = FLUID_SAMPLETYPE_ROM | FLUID_SAMPLETYPE_MONO;
        TEST_ASSERT(fluid_sample_validate(sample, 2) == FLUID_FAILED);

        sample->sampletype = FLUID_SAMPLETYPE_ROM | FLUID_SAMPLETYPE_RIGHT;
        TEST_ASSERT(fluid_sample_validate(sample, 2) == FLUID_FAILED);

        sample->sampletype = FLUID_SAMPLETYPE_ROM | FLUID_SAMPLETYPE_LEFT;
        TEST_ASSERT(fluid_sample_validate(sample, 2) == FLUID_FAILED);
    }

    /// invalid flag combinations
    {
        // no flags set? technically illegal, but handle it as mono
        sample->sampletype = 0;
        TEST_SUCCESS(fluid_sample_validate(sample, 2));

        sample->sampletype = FLUID_SAMPLETYPE_MONO | FLUID_SAMPLETYPE_RIGHT;
        TEST_SUCCESS(fluid_sample_validate(sample, 2));

        sample->sampletype |= FLUID_SAMPLETYPE_LEFT;
        TEST_SUCCESS(fluid_sample_validate(sample, 2));

        sample->sampletype |= FLUID_SAMPLETYPE_LINKED;
        TEST_SUCCESS(fluid_sample_validate(sample, 2));
    }

    // unknown flags must be rejected (this seems to be implicitly required by SF3)
    {
        sample->sampletype = 0x20;
        TEST_ASSERT(fluid_sample_validate(sample, 2) == FLUID_FAILED);

        sample->sampletype |= 0x40;
        TEST_ASSERT(fluid_sample_validate(sample, 2) == FLUID_FAILED);

        sample->sampletype = 0x80;
        TEST_ASSERT(fluid_sample_validate(sample, 2) == FLUID_FAILED);

        sample->sampletype <<= 1;
        TEST_ASSERT(fluid_sample_validate(sample, 2) == FLUID_FAILED);
    }

    delete_fluid_sample(sample);

    return EXIT_SUCCESS;
}
