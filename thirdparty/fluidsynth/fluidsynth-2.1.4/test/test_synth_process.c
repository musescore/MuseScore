
#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_synth.h"
#include <string.h>

// static const int CHANNELS=16;
enum { SAMPLES=1024 };

static int smpl;

int render_one_mock(fluid_synth_t *synth, int blocks)
{

    fluid_real_t *left_in, *fx_left_in;
    fluid_real_t *right_in, *fx_right_in;

    int i, j;

    int naudchan = fluid_synth_count_audio_channels(synth);

    fluid_rvoice_mixer_get_bufs(synth->eventhandler->mixer, &left_in, &right_in);
    fluid_rvoice_mixer_get_fx_bufs(synth->eventhandler->mixer, &fx_left_in, &fx_right_in);

    for(i = 0; i < naudchan; i++)
    {
        for(j = 0; j < blocks * FLUID_BUFSIZE; j++)
        {
            int idx = i * FLUID_MIXER_MAX_BUFFERS_DEFAULT * FLUID_BUFSIZE + j;

            right_in[idx] = left_in[idx] = (float)smpl++;
        }
    }

    return blocks;
}

int process_and_check(fluid_synth_t* synth, int number_of_samples, int offset)
{
    int i;
    float left[SAMPLES], right[SAMPLES];
    float *dry[1 * 2];
    dry[0] = left;
    dry[1] = right;
    FLUID_MEMSET(left, 0, sizeof(left));
    FLUID_MEMSET(right, 0, sizeof(right));

    TEST_SUCCESS(fluid_synth_process_LOCAL(synth, number_of_samples, 0, NULL, 2, dry, render_one_mock));

    for(i=0; i<number_of_samples; i++)
    {
        TEST_ASSERT(left[i]==offset);
        TEST_ASSERT(right[i]==offset);
        offset++;
    }

    return offset;
}


int write_and_check(fluid_synth_t* synth, int number_of_samples, int offset)
{
    int i;
    float buf[2*SAMPLES];
    FLUID_MEMSET(buf, 0, sizeof(buf));

    // one buffer, interleaved writing
    TEST_SUCCESS(fluid_synth_write_float_LOCAL(synth, number_of_samples, buf, 0, 2, buf, 1, 2, render_one_mock));

    for(i=0; i< 2*number_of_samples; i+=2)
    {
        TEST_ASSERT(buf[i+0] == offset);
        TEST_ASSERT(buf[i+1] == offset);
        offset++;
    }

    FLUID_MEMSET(buf, 0, sizeof(buf));

    // "two" buffers, planar writing
    TEST_SUCCESS(fluid_synth_write_float_LOCAL(synth, number_of_samples, buf, 0, 1, buf, SAMPLES, 1, render_one_mock));

    for(i=0; i< number_of_samples; i++)
    {
        TEST_ASSERT(buf[i+0] == offset);
        TEST_ASSERT(buf[i+SAMPLES] == offset);
        offset++;
    }

    return offset;
}


// this test should make sure that sample rate changed are handled correctly
int main(void)
{
    int off=0;
    fluid_synth_t *synth;
    fluid_settings_t *settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);

    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    off = process_and_check(synth, 100, off);
    off = process_and_check(synth, 200, off);
    off = process_and_check(synth, 300, off);
    // next statement should not result in a call to render_one_mock() and therefore not increase the static "smpl" var
    off = process_and_check(synth, 0, off);
    off = process_and_check(synth, 1000, off);
    off = process_and_check(synth, SAMPLES, off);
    off = process_and_check(synth, 900, off);
    off = process_and_check(synth, 800, off);
    off = process_and_check(synth, FLUID_BUFSIZE, off);

    // currently it is not possible to call different rendering functions subsequently
    off = smpl;

    off = write_and_check(synth, 100, off);
    off = write_and_check(synth, 200, off);
    off = write_and_check(synth, 300, off);
    off = write_and_check(synth, 0, off);
    off = write_and_check(synth, 1000, off);
    off = write_and_check(synth, SAMPLES, off);
    off = write_and_check(synth, 900, off);
    off = write_and_check(synth, 800, off);
    off = write_and_check(synth, FLUID_BUFSIZE, off);

    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
