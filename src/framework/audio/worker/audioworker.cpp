/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "audioworker.h"

#include "global/runtime.h"
#include "global/threadutils.h"
#include "global/modularity/ioc.h"
#include "global/async/processevents.h"

#ifdef Q_OS_WIN
#include "global/platform/win/waitabletimer.h"
#endif

#include "audio/common/audiosanitizer.h"
#include "audio/common/rpc/platform/general/generalrpcchannel.h"
#include "audio/common/audioutils.h"

#include "internal/audioworkerconfiguration.h"
#include "internal/audiobuffer.h"
#include "internal/audioengine.h"
#include "internal/workerplayback.h"
#include "internal/workerchannelcontroller.h"

#include "internal/fx/fxresolver.h"
#include "internal/fx/musefxresolver.h"

#include "internal/synthesizers/fluidsynth/fluidresolver.h"
#include "internal/synthesizers/synthresolver.h"
#include "internal/synthesizers/soundfontrepository.h"

#include "log.h"

using namespace muse;
using namespace muse::modularity;
using namespace muse::audio;
using namespace muse::audio::worker;
using namespace muse::audio::fx;
using namespace muse::audio::synth;

static uint64_t toWinTime(const msecs_t msecs)
{
    return msecs * 10000;
}

std::thread::id AudioWorker::ID;

AudioWorker::AudioWorker(std::shared_ptr<rpc::GeneralRpcChannel> rpcChannel)
    : m_rpcChannel(rpcChannel)
{
}

AudioWorker::~AudioWorker()
{
    if (m_running) {
        stop();
    }
}

static std::string moduleName()
{
    return "audio_worker";
}

static muse::modularity::ModulesIoC* ioc()
{
    return muse::modularity::globalIoc();
}

void AudioWorker::init()
{
    //! NOTE Services registration
    //! At the moment, it is better to create services
    //! and register them in the IOC in the main thread,
    //! because the IOC thread is not safe.
    //! Therefore, we will need to make own IOC instance for the worker.

    m_configuration = std::make_shared<AudioWorkerConfiguration>();
    m_audioEngine = std::make_shared<AudioEngine>();
    m_audioBuffer = std::make_shared<AudioBuffer>();
    m_workerPlayback = std::make_shared<WorkerPlayback>();
    m_workerChannelController  = std::make_shared<WorkerChannelController>();
    m_fxResolver = std::make_shared<FxResolver>();
    m_synthResolver = std::make_shared<SynthResolver>();
    m_soundFontRepository = std::make_shared<SoundFontRepository>();

    ioc()->registerExport<IAudioWorkerConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IAudioEngine>(moduleName(), m_audioEngine);
    ioc()->registerExport<IWorkerPlayback>(moduleName(), m_workerPlayback);
    ioc()->registerExport<IFxResolver>(moduleName(), m_fxResolver);
    ioc()->registerExport<ISynthResolver>(moduleName(), m_synthResolver);
    ioc()->registerExport<ISoundFontRepository>(moduleName(), m_soundFontRepository);
}

void AudioWorker::run(const ActiveSpec& spec)
{
    m_running = true;
    m_thread = std::make_unique<std::thread>([this, spec]() {
        th_main(spec);
    });

    if (!muse::setThreadPriority(*m_thread, ThreadPriority::High)) {
        LOGE() << "Unable to change audio thread priority";
    }
}

void AudioWorker::setInterval(const msecs_t interval)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_intervalMsecs = interval;
    m_intervalInWinTime = toWinTime(interval);
}

void AudioWorker::stop()
{
    m_running = false;
    if (m_thread) {
        m_thread->join();
    }
}

bool AudioWorker::isRunning() const
{
    return m_running;
}

static msecs_t audioWorkerInterval(const samples_t samples, const sample_rate_t sampleRate)
{
    msecs_t interval = float(samples) / 4.f / float(sampleRate) * 1000.f;
    interval = std::max(interval, msecs_t(1));

    // Found experementaly on a slow laptop (2 core) running on battery power
    interval = std::min(interval, msecs_t(10));

    return interval;
}

void AudioWorker::th_main(const ActiveSpec& spec)
{
    runtime::setThreadName("audio_worker");
    AudioSanitizer::setupWorkerThread();
    ONLY_AUDIO_WORKER_THREAD;

    AudioWorker::ID = std::this_thread::get_id();

    //! NOTE It should be as early as possible
    m_rpcChannel->initOnWorker();

    m_fxResolver->registerResolver(AudioFxType::MuseFx, std::make_shared<MuseFxResolver>());
    m_synthResolver->registerResolver(AudioSourceType::Fluid, std::make_shared<FluidResolver>());

    m_audioBuffer->init(m_configuration->audioChannelsCount());

    worker::AudioEngine::RenderConstraints consts;
    consts.minSamplesToReserveWhenIdle = minSamplesToReserve(RenderMode::IdleMode);
    consts.minSamplesToReserveInRealtime = minSamplesToReserve(RenderMode::RealTimeMode);
    consts.desiredAudioThreadNumber = m_configuration->desiredAudioThreadNumber();
    consts.minTrackCountForMultithreading = m_configuration->minTrackCountForMultithreading();

    // Setup audio engine
    m_audioEngine->init(m_audioBuffer, consts);
    m_audioEngine->setAudioChannelsCount(m_configuration->audioChannelsCount());
    m_audioEngine->setSampleRate(spec.sampleRate);
    m_audioEngine->setReadBufferSize(spec.bufferSize);

    m_audioEngine->setOnReadBufferChanged([this](const samples_t samples, const sample_rate_t rate) {
        msecs_t interval = audioWorkerInterval(samples, rate);
        setInterval(interval);
    });

    m_synthResolver->init(m_configuration->defaultAudioInputParams());

    m_soundFontRepository->init();
    m_workerPlayback->init();
    m_workerChannelController->init(m_workerPlayback);

    m_intervalMsecs = audioWorkerInterval(spec.bufferSize, spec.sampleRate);
    m_intervalInWinTime = toWinTime(m_intervalMsecs);

#ifdef Q_OS_WIN
    WaitableTimer timer;
    bool timerValid = timer.init();
    if (timerValid) {
        LOGI() << "Waitable timer successfully created, interval: " << m_intervalMsecs << " ms";
    }
#endif

    while (m_running) {
        async::processEvents();
        m_rpcChannel->process();
        m_audioBuffer->forward();

#ifdef Q_OS_WIN
        if (!timerValid || !timer.setAndWait(m_intervalInWinTime)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_intervalMsecs));
        }
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(m_intervalMsecs));
#endif
    }

    m_audioEngine->deinit();
    m_workerChannelController->deinit();
    m_workerPlayback->deinit();
}
