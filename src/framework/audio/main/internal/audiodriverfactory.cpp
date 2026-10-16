/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "audiodriverfactory.h"

#include "muse_framework_config.h"

#ifdef MUSE_MODULE_AUDIO_JACK
#include "audio/driver/platform/jack/jackaudiodriver.h"
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
#include <QtEnvironmentVariables>
#include "audio/driver/platform/lin/alsaaudiodriver.h"
#ifdef MUSE_MODULE_AUDIO_PIPEWIRE
#include "audio/driver/platform/lin/pwaudiodriver.h"
#endif
#endif

#ifdef Q_OS_WIN
#include "audio/driver/platform/win/wasapiaudiodriver.h"
#ifdef MUSE_MODULE_AUDIO_ASIO
#include "audio/driver/platform/win/asio/asioaudiodriver.h"
#endif
#endif

#ifdef Q_OS_MACOS
#include "audio/driver/platform/osx/osxaudiodriver.h"
#endif

#ifdef Q_OS_WASM
#include "audio/driver/platform/web/webaudiodriver.h"
#endif

#include "log.h"

using namespace muse;
using namespace muse::audio;

std::vector<std::string> AudioDriverFactory::availableAudioDrivers() const
{
    std::vector<std::string> names;
#ifdef MUSE_MODULE_AUDIO_JACK
    names.push_back("JACK");
    return names;
#endif

#ifdef Q_OS_WIN
    names.push_back("WASAPI");
#ifdef MUSE_MODULE_AUDIO_ASIO
    names.push_back("ASIO");
#endif

    return names;
#endif // Q_OS_WIN

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    names.push_back("ALSA");
    if (qEnvironmentVariableIsSet("MUSESCORE_FORCE_ALSA")) {
        return names;
    }

#ifdef MUSE_MODULE_AUDIO_PIPEWIRE
    names.push_back("PipeWire");
#endif

    return names;
#endif // Q_OS_LINUX

#ifdef Q_OS_MACOS
    names.push_back("CoreAudio");
    return names;
#endif

#ifdef Q_OS_WASM
    names.push_back("WASM");
    return names;
#endif
}

IAudioDriverPtr AudioDriverFactory::createDriver(const std::string& name) const
{
#ifdef MUSE_MODULE_AUDIO_JACK
    LOGW() << "required: " << name << ", but is set MUSE_MODULE_AUDIO_JACK";
    return std::shared_ptr<IAudioDriver>(new JackAudioDriver());
#endif

#ifdef Q_OS_WIN
    if (name == "ASIO") {
#ifdef MUSE_MODULE_AUDIO_ASIO
        return std::shared_ptr<IAudioDriver>(new AsioAudioDriver());
#else
        LOGW() << "ASIO is required but is not available, WASAPI will be used";
#endif
    }

    // required WASAPI or fallback
    return std::shared_ptr<IAudioDriver>(new WasapiAudioDriver());
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    if (qEnvironmentVariableIsSet("MUSESCORE_FORCE_ALSA")) {
        if (name != "ALSA") {
            LOGW() << "required: " << name << ", but is set MUSESCORE_FORCE_ALSA";
        }
        return std::make_shared<AlsaAudioDriver>();
    }

    if (name == "PipeWire") {
#ifdef MUSE_MODULE_AUDIO_PIPEWIRE
        auto driver = std::make_shared<PwAudioDriver>();
        if (driver->connectedToPwServer()) {
            return driver;
        } else {
            LOGE() << "PipeWire driver failed connected to server";
        }
#else
        LOGW() << "PipeWire is required but is not available, ALSA will be used";
#endif
    }

    // required ALSA or fallback
    return std::make_shared<AlsaAudioDriver>();
#endif

#ifdef Q_OS_MACOS
    UNUSED(name);
    return std::shared_ptr<IAudioDriver>(new OSXAudioDriver());
#endif

#ifdef Q_OS_WASM
    UNUSED(name);
    return std::shared_ptr<IAudioDriver>(new WebAudioDriver());
#endif
}
