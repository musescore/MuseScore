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

#include "modularity/ioc.h"
#include "internal/audioengine.h"

#include "devtools/audioenginedevtools.h"

#ifdef Q_OS_LINUX
#include "platform/lin/linuxaudiodriver.h"
#endif

using namespace mu::audio::engine;

std::shared_ptr<AudioEngine> audioEngine = std::make_shared<AudioEngine>();

std::string AudioEngineModule::moduleName() const
{
    return "audio_engine";
}

void AudioEngineModule::registerExports()
{
    framework::ioc()->registerExport<IAudioEngine>(moduleName(), audioEngine);

#ifdef Q_OS_LINUX
    framework::ioc()->registerExport<IAudioDriver>(moduleName(), new LinuxAudioDriver());
#endif
}

void AudioEngineModule::registerUiTypes()
{
    qmlRegisterType<AudioEngineDevTools>("MuseScore.Audio", 1, 0, "AudioEngineDevTools");
}

void AudioEngineModule::onInit()
{
    audioEngine->init();
}

void AudioEngineModule::onDeinit()
{
    audioEngine->deinit();
}
