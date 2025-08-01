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

namespace muse::audio::worker {
class AudioEngine;
class WorkerPlayback;
class WorkerChannelController;

class AudioWorkerModule : public modularity::IModuleSetup
{
public:

    std::string moduleName() const override;

    void registerExports() override;
    void resolveImports() override;
    void onInit(const IApplication::RunMode& mode) override;
    void onDestroy() override;

    // Temporarily for compatibility
    std::shared_ptr<AudioEngine> audioEngine() const { return m_audioEngine; }

private:
    std::shared_ptr<AudioEngine> m_audioEngine;
    std::shared_ptr<WorkerPlayback> m_workerPlayback;
    std::shared_ptr<WorkerChannelController> m_workerChannelController;
    std::shared_ptr<fx::FxResolver> m_fxResolver;
};
}
