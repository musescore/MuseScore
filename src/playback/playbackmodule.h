/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_PLAYBACK_PLAYBACKMODULE_H
#define MU_PLAYBACK_PLAYBACKMODULE_H

#include <memory>

#include "modularity/imodulesetup.h"

namespace mu::playback {
class PlaybackConfiguration;
class PlaybackController;
class PlaybackUiActions;
class SoundProfilesRepository;
class PlaybackModule : public muse::modularity::IModuleSetup
{
public:

    std::string moduleName() const override;
    void registerExports() override;
    void resolveImports() override;
    void registerResources() override;
    void registerUiTypes() override;
    void onInit(const muse::IApplication::RunMode& mode) override;
    void onAllInited(const muse::IApplication::RunMode& mode) override;

private:
    std::shared_ptr<PlaybackConfiguration> m_configuration;
    std::shared_ptr<PlaybackController> m_playbackController;
    std::shared_ptr<PlaybackUiActions> m_playbackUiActions;
    std::shared_ptr<SoundProfilesRepository> m_soundProfileRepo;
};
}

#endif // MU_PLAYBACK_PLAYBACKMODULE_H
