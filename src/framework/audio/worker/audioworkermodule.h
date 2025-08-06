/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#pragma once

#include "modularity/imodulesetup.h"

namespace muse::audio::fx {
class FxResolver;
}

namespace muse::audio::synth  {
class SynthResolver;
class SoundFontRepository;
}

namespace muse::audio::worker {
class AudioEngine;
class AudioBuffer;
class WorkerPlayback;
class WorkerChannelController;

class AudioWorkerModule : public modularity::IModuleSetup
{
public:

    std::string moduleName() const override;

    void registerExports() override;
    void resolveImports() override;
    void onPreInit(const IApplication::RunMode& mode) override;
    void onInit(const IApplication::RunMode& mode) override;
    void onDestroy() override;

    // Temporarily for compatibility
    std::shared_ptr<AudioEngine> audioEngine() const { return m_audioEngine; }
    std::shared_ptr<AudioBuffer> audioBuffer() const { return m_audioBuffer; }
    std::shared_ptr<synth::SynthResolver> synthResolver() const { return m_synthResolver; }

private:
    std::shared_ptr<AudioEngine> m_audioEngine;
    std::shared_ptr<AudioBuffer> m_audioBuffer;
    std::shared_ptr<WorkerPlayback> m_workerPlayback;
    std::shared_ptr<WorkerChannelController> m_workerChannelController;
    std::shared_ptr<fx::FxResolver> m_fxResolver;
    std::shared_ptr<synth::SynthResolver> m_synthResolver;
    std::shared_ptr<synth::SoundFontRepository> m_soundFontRepository;
};
}
