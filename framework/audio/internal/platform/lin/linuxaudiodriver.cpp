#include "linuxaudiodriver.h"

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "log.h"
#include "runtime.h"

using namespace mu::audio;

namespace  {
struct ALSAData
{
    float* buffer;
    snd_pcm_t* alsaDeviceHandle;
    int samples;
    int channels;
    bool audioProcessingDone;
    pthread_t threadHandle;
    IAudioDriver::Callback callback;
    void* userdata;
};

static ALSAData* _alsaData{ nullptr };

static void* alsaThread(void* aParam)
{
    mu::runtime::setThreadName("audio_driver");
    ALSAData* data = static_cast<ALSAData*>(aParam);

    int ret = snd_pcm_wait(data->alsaDeviceHandle, 1000);
    IF_ASSERT_FAILED(ret > 0) {
        return nullptr;
    }

    while (!data->audioProcessingDone)
    {
        uint8_t* stream = (uint8_t*)data->buffer;
        int len = data->samples * data->channels * sizeof(float);

        data->callback(data->userdata, stream, len);

        snd_pcm_sframes_t pcm = snd_pcm_writei(data->alsaDeviceHandle, data->buffer, data->samples);
        if (pcm != -EPIPE) {
        } else {
            snd_pcm_prepare(data->alsaDeviceHandle);
        }
    }

    LOGI() << "exit";
    return nullptr;
}

static void alsaCleanup()
{
    IF_ASSERT_FAILED(_alsaData) {
        return;
    }

    _alsaData->audioProcessingDone = true;
    if (_alsaData->threadHandle) {
        pthread_join(_alsaData->threadHandle, nullptr);
    }
    snd_pcm_drain(_alsaData->alsaDeviceHandle);
    snd_pcm_close(_alsaData->alsaDeviceHandle);

    if (nullptr != _alsaData->buffer) {
        delete[] _alsaData->buffer;
    }

    delete _alsaData;
    _alsaData = nullptr;
}
}

LinuxAudioDriver::LinuxAudioDriver()
{
}

std::string LinuxAudioDriver::name() const
{
    return "MUAUDIO(ALSA)";
}

bool LinuxAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    _alsaData = new ALSAData;
    memset(_alsaData, 0, sizeof(ALSAData));
    _alsaData->samples = spec.samples;
    _alsaData->channels = spec.channels;
    _alsaData->callback = spec.callback;
    _alsaData->userdata = spec.userdata;

    int rc;
    snd_pcm_t* handle;
    rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        return false;
    }

    _alsaData->alsaDeviceHandle = handle;

    snd_pcm_hw_params_t* params;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);

    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_FLOAT_LE);
    snd_pcm_hw_params_set_channels(handle, params, spec.channels);
    snd_pcm_hw_params_set_buffer_size(handle, params, spec.samples);

    unsigned int aSamplerate = spec.freq;
    unsigned int val = aSamplerate;
    int dir = 0;
    rc = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
    if (rc < 0) {
        return false;
    }

    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        return false;
    }

    snd_pcm_hw_params_get_rate(params, &val, &dir);
    aSamplerate = val;

    _alsaData->buffer = new float[_alsaData->samples * _alsaData->channels];
    //_alsaData->sampleBuffer = new short[_alsaData->samples * _alsaData->channels];

    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = Format::AudioF32;
        activeSpec->freq = aSamplerate;
    }

    _alsaData->threadHandle = 0;
    int ret = pthread_create(&_alsaData->threadHandle, NULL, alsaThread, (void*)_alsaData);

    if (0 != ret) {
        return false;
    }

    return true;
}

void LinuxAudioDriver::close()
{
    alsaCleanup();
}

bool LinuxAudioDriver::isOpened() const
{
    return _alsaData != nullptr;
}
