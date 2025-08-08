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
#ifndef MUSE_AUDIO_AUDIOMODULE_H
#define MUSE_AUDIO_AUDIOMODULE_H

#include <memory>
#include <QTimer>

#include "modularity/imodulesetup.h"
#include "global/async/asyncable.h"

#include "../iaudiodriver.h"

namespace muse::audio::worker  {
class AudioWorkerModule;
}

namespace muse::audio::rpc  {
class GeneralRpcChannel;
}

namespace muse::audio {
class AudioConfiguration;
class AudioThread;
class AudioOutputDeviceController;
class Playback;
class SoundFontController;
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
    void onInit(const IApplication::RunMode& mode) override;
    void onDelayedInit() override;
    void onDeinit() override;
    void onDestroy() override;

private:
    void setupAudioDriver(const IApplication::RunMode& mode);
    void setupAudioWorker(const IAudioDriver::Spec& activeSpec);

    std::shared_ptr<AudioConfiguration> m_configuration;
    std::shared_ptr<AudioThread> m_audioWorker;
    std::shared_ptr<AudioOutputDeviceController> m_audioOutputController;
    std::shared_ptr<Playback> m_mainPlayback;
    std::shared_ptr<SoundFontController> m_soundFontController;

    std::shared_ptr<worker::AudioWorkerModule> m_workerModule;

    QTimer m_rpcTimer;
    std::shared_ptr<rpc::GeneralRpcChannel> m_rpcChannel;

    std::shared_ptr<IAudioDriver> m_audioDriver;
};
}

#endif // MUSE_AUDIO_AUDIOMODULE_H
