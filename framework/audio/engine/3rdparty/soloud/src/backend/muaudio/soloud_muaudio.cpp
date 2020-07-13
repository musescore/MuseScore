#include <stdlib.h>

#include "soloud.h"

#if !defined(WITH_MUAUDIO)

namespace SoLoud {
result muaudio_init(SoLoud::Soloud* aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer)
{
    return NOT_IMPLEMENTED;
}
}

#else

#include <math.h>
#include "log.h"
#include "modularity/ioc.h"
#include "audio/engine/iaudiodriver.h"

//#define DEBUG_AUDIO_DRIVER

#ifdef DEBUG_AUDIO_DRIVER
#define DRIVER_LOG() LOGD()
#else
#define DRIVER_LOG() LOGNOOP()
#endif

using namespace mu::audio::engine;

namespace  {
std::shared_ptr<IAudioDriver> adriver()
{
    return mu::framework::ioc()->resolve<mu::audio::engine::IAudioDriver>("soloud");
}
}

namespace SoLoud {
static IAudioDriver::Spec gActiveAudioSpec;
static IAudioDriver::DeviceID gAudioDeviceID;
static bool gInited{ false };

void soloud_muaudio_audiomixer(void* userdata, uint8_t* stream, int len)
{
    if (!gInited) {
        for (int i = 0; i < len; ++i) {
            stream[i] = 0;
        }

        return;
    }

    short* buf = (short*)stream;
    SoLoud::Soloud* soloud = (SoLoud::Soloud*)userdata;
    if (gActiveAudioSpec.format == IAudioDriver::Format::AudioF32) {
        int samples = len / (gActiveAudioSpec.channels * sizeof(float));
        soloud->mix((float*)buf, samples);
    } else { // assume s16 if not float
        int samples = len / (gActiveAudioSpec.channels * sizeof(short));
        soloud->mixSigned16(buf, samples);
    }
}

static void soloud_muaudio_deinit(SoLoud::Soloud* aSoloud)
{
    gInited = false;
    UNUSED(aSoloud);
    std::shared_ptr<IAudioDriver> driver = adriver();
    IF_ASSERT_FAILED(driver) {
        LOGE() << "no audio driver \n";
        return;
    }
    driver->close(gAudioDeviceID);
}

result muaudio_init(SoLoud::Soloud* aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer,
                    unsigned int aChannels)
{
    std::shared_ptr<IAudioDriver> driver = adriver();

    IF_ASSERT_FAILED(driver) {
        LOGE() << "no audio driver \n";
        return UNKNOWN_ERROR;
    }

    IAudioDriver::Spec as;
    as.freq = aSamplerate;
    as.format = IAudioDriver::Format::AudioF32;
    as.channels = aChannels;
    as.samples = aBuffer;
    as.callback = soloud_muaudio_audiomixer;
    as.userdata = (void*)aSoloud;

    gAudioDeviceID = driver->open(as, &gActiveAudioSpec);
    if (gAudioDeviceID == 0) {
        LOGE() << "failed open audio driver \n";
        return UNKNOWN_ERROR;
    }

    aSoloud->postinit_internal(gActiveAudioSpec.freq, gActiveAudioSpec.samples, aFlags, gActiveAudioSpec.channels);

    aSoloud->mBackendCleanupFunc = soloud_muaudio_deinit;

    aSoloud->mBackendString = driver->name().c_str();

    gInited = true;

    return 0;
}
}
#endif
