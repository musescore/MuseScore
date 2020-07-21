//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "audioenginemodule.h"

#include <QQmlEngine>

#include "invoker.h"

#include "modularity/ioc.h"
#include "internal/audioengine.h"
#include "internal/audioplayer.h"

#include "internal/worker/queuedrpcstreamchannel.h"
#include "internal/worker/audiothreadstreamworker.h"

#include "devtools/audioenginedevtools.h"

#ifdef Q_OS_LINUX
#include "platform/lin/linuxaudiodriver.h"
#endif

#ifdef Q_OS_WIN
#include "platform/win/winaudiodriver.h"
#endif

using namespace mu::audio::engine;

static std::shared_ptr<AudioEngine> _audioEngine = std::make_shared<AudioEngine>();
static std::shared_ptr<QueuedRpcStreamChannel> _audioChannel = std::make_shared<QueuedRpcStreamChannel>();
static std::shared_ptr<AudioThreadStreamWorker> _audioWorker = std::make_shared<AudioThreadStreamWorker>(_audioChannel);
static std::shared_ptr<mu::framework::Invoker> _audioChannelInvoker;

std::string AudioEngineModule::moduleName() const
{
    return "audio_engine";
}

void AudioEngineModule::registerExports()
{
    framework::ioc()->registerExport<IAudioEngine>(moduleName(), _audioEngine);
    framework::ioc()->registerExport<IAudioPlayer>(moduleName(), new AudioPlayer());
    framework::ioc()->registerExport<IRpcAudioStreamChannel>(moduleName(), _audioChannel);

#ifdef Q_OS_LINUX
    framework::ioc()->registerExport<IAudioDriver>(moduleName(), new LinuxAudioDriver());
#endif

#ifdef Q_OS_WIN
    framework::ioc()->registerExport<IAudioDriver>(moduleName(), new WinAudioDriver());
#endif
}

void AudioEngineModule::registerUiTypes()
{
    qmlRegisterType<AudioEngineDevTools>("MuseScore.Audio", 1, 0, "AudioEngineDevTools");
}

void AudioEngineModule::onInit()
{
    _audioEngine->init();

    _audioChannelInvoker = std::make_shared<mu::framework::Invoker>();

    _audioChannelInvoker->onInvoked([]() {
        _audioChannel->process();
    });

    _audioChannel->workerQueueChanged().onNotify(this, []() {
        _audioChannelInvoker->invoke();
    });

    _audioWorker->run();
}

void AudioEngineModule::onDeinit()
{
    _audioWorker->stop();
    _audioEngine->deinit();
}
