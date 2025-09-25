/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "audiodrivercontroller.h"

#ifdef MUSE_MODULE_AUDIO_JACK
#include "audio/driver/platform/jack/jackaudiodriver.h"
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
#include <QtEnvironmentVariables>
#include "audio/driver/platform/lin/alsaaudiodriver.h"
#ifdef MUSE_PIPEWIRE_AUDIO_DRIVER
#include "audio/driver/platform/lin/pwaudiodriver.h"
#endif
#endif

#ifdef Q_OS_WIN
//#include "audio/driver/platform/win/winmmdriver.h"
//#include "audio/driver/platform/win/wincoreaudiodriver.h"
#include "audio/driver/platform/win/wasapiaudiodriver.h"
#endif

#ifdef Q_OS_MACOS
#include "audio/driver/platform/osx/osxaudiodriver.h"
#endif

#ifdef Q_OS_WASM
#include "audio/driver/platform/web/webaudiodriver.h"
#endif

#include "log.h"

using namespace muse::audio;

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
static std::shared_ptr<IAudioDriver> makeLinuxAudioDriver(const std::string& driverName)
{
#if defined(MUSE_PIPEWIRE_AUDIO_DRIVER)
    if (!qEnvironmentVariableIsSet("MUSESCORE_FORCE_ALSA") && driverName == "PipeWire") {
        auto driver = std::make_shared<PwAudioDriver>();
        if (driver->connectedToPwServer()) {
            LOGI() << "Using audio driver: Pipewire";
            return driver;
        }
    }
#endif // MUSE_PIPEWIRE_AUDIO_DRIVER
    LOGI() << "Using audio driver: ALSA";
    return std::make_shared<AlsaAudioDriver>();
}

#endif // Q_OS_LINUX || Q_OS_FREEBSD

void AudioDriverController::init()
{
#if defined(MUSE_MODULE_AUDIO_JACK)
    m_audioDriver = std::shared_ptr<IAudioDriver>(new JackAudioDriver());
#else

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    std::string name = configuration()->currentAudioApi();
    m_audioDriver = makeLinuxAudioDriver(name);
#endif

#ifdef Q_OS_WIN
    //m_audioDriver = std::shared_ptr<IAudioDriver>(new WinmmDriver());
    //m_audioDriver = std::shared_ptr<IAudioDriver>(new CoreAudioDriver());
    m_audioDriver = std::shared_ptr<IAudioDriver>(new WasapiAudioDriver());
#endif

#ifdef Q_OS_MACOS
    m_audioDriver = std::shared_ptr<IAudioDriver>(new OSXAudioDriver());
#endif

#ifdef Q_OS_WASM
    m_audioDriver = std::shared_ptr<IAudioDriver>(new WebAudioDriver());
#endif

#endif // MUSE_MODULE_AUDIO_JACK
}

std::string AudioDriverController::currentAudioApi() const
{
    return configuration()->currentAudioApi();
}

void AudioDriverController::setCurrentAudioApi(const std::string& name)
{
    configuration()->setCurrentAudioApi(name);
}

muse::async::Notification AudioDriverController::currentAudioApiChanged() const
{
    return configuration()->currentAudioApiChanged();
}

std::vector<std::string> AudioDriverController::availableAudioApiList() const
{
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    std::vector<std::string> names {
        "ALSA Audio",
        "PipeWire",
    };

    return names;
#else
    return {};
#endif
}

IAudioDriverPtr AudioDriverController::audioDriver() const
{
    DO_ASSERT(m_audioDriver);
    return m_audioDriver;
}
