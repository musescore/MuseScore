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
#include "modularity/ioc.h"
#include "internal/audioengine.h"
#include "internal/audioconfiguration.h"
#include "ui/iuiengine.h"
#include "devtools/audioenginedevtools.h"

using namespace mu::audio;

static std::shared_ptr<AudioConfiguration> s_audioConfiguration = std::make_shared<AudioConfiguration>();
static std::shared_ptr<AudioEngine> s_audioEngine = std::make_shared<AudioEngine>();
#ifdef Q_OS_LINUX
#include "internal/platform/lin/linuxaudiodriver.h"
static std::shared_ptr<IAudioDriver> s_audioDriver = std::shared_ptr<IAudioDriver>(new LinuxAudioDriver());
#endif

#ifdef Q_OS_WIN
#include "internal/platform/win/wincoreaudiodriver.h"
static std::shared_ptr<IAudioDriver> s_audioDriver = std::shared_ptr<IAudioDriver>(new CoreAudioDriver());
#endif

#ifdef Q_OS_MACOS
#include "internal/platform/osx/osxaudiodriver.h"
static std::shared_ptr<IAudioDriver> s_audioDriver = std::shared_ptr<IAudioDriver>(new OSXAudioDriver());
#endif

#ifdef Q_OS_WASM
#include "internal/platform/web/webaudiodriver.h"
static std::shared_ptr<IAudioDriver> s_audioDriver = std::shared_ptr<IAudioDriver>(new WebAudioDriver());
#endif

std::string AudioModule::moduleName() const
{
    return "audio_engine";
}

void AudioModule::registerExports()
{
    framework::ioc()->registerExport<IAudioEngine>(moduleName(), s_audioEngine);
    framework::ioc()->registerExport<ISequencer>(moduleName(), s_audioEngine->sequencer());
}

void AudioModule::registerUiTypes()
{
    qmlRegisterType<AudioEngineDevTools>("MuseScore.Audio", 1, 0, "AudioEngineDevTools");

    //! NOTE No Qml, as it will be, need to uncomment
    //framework::ioc()->resolve<framework::IUiEngine>(moduleName())->addSourceImportPath(mu4_audio_QML_IMPORT);
}

void AudioModule::onInit(const framework::IApplication::RunMode& mode)
{
    s_audioConfiguration->init();
    s_audioEngine->init(s_audioDriver, s_audioConfiguration->driverBufferSize());
}

void AudioModule::onDeinit()
{
    s_audioEngine->deinit();
}
