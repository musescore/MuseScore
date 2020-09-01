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
#include "audiomodule.h"

#include <QQmlEngine>

#include "invoker.h"

#include "modularity/ioc.h"
#include "internal/audioengine.h"
#include "internal/audioplayer.h"

#include "internal/worker/queuedrpcstreamchannel.h"
#include "internal/worker/audiothreadstreamworker.h"
#include "internal/rpcmidisource.h"

#include "ui/iuiengine.h"
#include "devtools/audioenginedevtools.h"

#ifdef Q_OS_LINUX
#include "internal/platform/lin/linuxaudiodriver.h"
#endif

#ifdef Q_OS_WIN
#include "internal/platform/win/winmmdriver.h"
#endif

#ifdef Q_OS_MACOS
#include "internal/platform/osx/osxaudiodriver.h"
#endif

using namespace mu::audio;
using namespace mu::audio::worker;

static std::shared_ptr<AudioEngine> s_audioEngine = std::make_shared<AudioEngine>();
static std::shared_ptr<QueuedRpcStreamChannel> s_rpcChannel = std::make_shared<QueuedRpcStreamChannel>();
static std::shared_ptr<AudioThreadStreamWorker> s_worker = std::make_shared<AudioThreadStreamWorker>(s_rpcChannel);
static std::shared_ptr<mu::framework::Invoker> s_rpcChannelInvoker;

std::string AudioModule::moduleName() const
{
    return "audio_engine";
}

void AudioModule::registerExports()
{
    framework::ioc()->registerExport<IAudioEngine>(moduleName(), s_audioEngine);
    framework::ioc()->registerExport<IAudioPlayer>(moduleName(), new AudioPlayer());
    framework::ioc()->registerExport<IRpcAudioStreamChannel>(moduleName(), s_rpcChannel);
    framework::ioc()->registerExport<IMidiSource>(moduleName(), std::make_shared<RpcMidiSource>());

#ifdef Q_OS_LINUX
    framework::ioc()->registerExport<IAudioDriver>(moduleName(), new LinuxAudioDriver());
#endif

#ifdef Q_OS_WIN
    framework::ioc()->registerExport<IAudioDriver>(moduleName(), new WinmmDriver());
#endif

#ifdef Q_OS_MACOS
    framework::ioc()->registerExport<IAudioDriver>(moduleName(), new OSXAudioDriver());
#endif
}

void AudioModule::registerUiTypes()
{
    qmlRegisterType<AudioEngineDevTools>("MuseScore.Audio", 1, 0, "AudioEngineDevTools");

    //! NOTE No Qml, as it will be, need to uncomment
    //framework::ioc()->resolve<framework::IUiEngine>(moduleName())->addSourceImportPath(mu4_audio_QML_IMPORT);
}

void AudioModule::onInit()
{
    s_audioEngine->init();

    s_rpcChannelInvoker = std::make_shared<mu::framework::Invoker>();

    s_rpcChannel->onWorkerQueueChanged([]() {
        //! NOTE Called from worker thread
        s_rpcChannelInvoker->invoke([]() {
            //! NOTE Called from main thread
            s_rpcChannel->process();
        });
    });

    s_worker->run();
}

void AudioModule::onDeinit()
{
    s_worker->stop();
    s_audioEngine->deinit();
}
