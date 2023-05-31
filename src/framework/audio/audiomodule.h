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
#ifndef MU_AUDIO_AUDIOMODULE_H
#define MU_AUDIO_AUDIOMODULE_H

#include <memory>
#include "modularity/imodulesetup.h"
#include "async/asyncable.h"

#include "iaudiodriver.h"

namespace mu::audio::fx {
class FxResolver;
}

namespace mu::audio::synth  {
class SynthResolver;
}

namespace mu::audio {
class AudioConfiguration;
class AudioThread;
class AudioBuffer;
class AudioOutputDeviceController;
class Playback;
class SoundFontRepository;
class KnownAudioPluginsRegister;
class RegisterAudioPluginsScenario;
class AudioModule : public modularity::IModuleSetup, public async::Asyncable
{
public:
    AudioModule();

    std::string moduleName() const override;

    void registerExports() override;
    void registerResources() override;
    void registerUiTypes() override;
    void resolveImports() override;
    void onInit(const framework::IApplication::RunMode& mode) override;
    void onDelayedInit() override;
    void onDeinit() override;
    void onDestroy() override;

private:
    void setupAudioDriver(const framework::IApplication::RunMode& mode);
    void setupAudioWorker(const IAudioDriver::Spec& activeSpec);

    std::shared_ptr<AudioConfiguration> m_configuration;
    std::shared_ptr<AudioThread> m_audioWorker;
    std::shared_ptr<AudioBuffer> m_audioBuffer;
    std::shared_ptr<AudioOutputDeviceController> m_audioOutputController;

    std::shared_ptr<fx::FxResolver> m_fxResolver;
    std::shared_ptr<synth::SynthResolver> m_synthResolver;

    std::shared_ptr<Playback> m_playbackFacade;

    std::shared_ptr<SoundFontRepository> m_soundFontRepository;

    std::shared_ptr<KnownAudioPluginsRegister> m_knownAudioPluginsRegister;
    std::shared_ptr<RegisterAudioPluginsScenario> m_registerAudioPluginsScenario;

    #ifdef Q_OS_LINUX
    std::shared_ptr<IAudioDriver> m_audioDriver;
    #endif
    #ifdef Q_OS_FREEBSD
    std::shared_ptr<IAudioDriver> m_audioDriver;
    #endif

    #ifdef Q_OS_WIN
    //std::shared_ptr<IAudioDriver> m_audioDriver;
    //std::shared_ptr<IAudioDriver> m_audioDriver;
    std::shared_ptr<IAudioDriver> m_audioDriver;
    #endif

    #ifdef Q_OS_MACOS
    std::shared_ptr<IAudioDriver> m_audioDriver;
    #endif

    #ifdef Q_OS_WASM
    std::shared_ptr<IAudioDriver> m_audioDriver;
    #endif
};
}

#endif // MU_AUDIO_AUDIOMODULE_H
