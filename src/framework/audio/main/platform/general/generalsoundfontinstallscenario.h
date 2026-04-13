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

#pragma once

#include "../../isoundfontinstallscenario.h"

#include "global/async/asyncable.h"

#include "global/modularity/ioc.h"
#include "global/io/ifilesystem.h"
#include "interactive/iinteractive.h"
#include "../../iaudioconfiguration.h"
#include "../../isoundfontcontroller.h"

namespace muse::audio {
class GeneralSoundFontInstallScenario : public ISoundFontInstallScenario, public async::Asyncable, public muse::Contextable
{
    GlobalInject<IAudioConfiguration> configuration;
    GlobalInject<io::IFileSystem> fileSystem;
    GlobalInject<ISoundFontController> soundFontController;
    ContextInject<IInteractive> interactive = { this };

public:
    GeneralSoundFontInstallScenario(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    void installSoundFont(const synth::SoundFontUri& uri) override;

private:

    RetVal<io::path_t> resolveInstallationPath(const io::path_t& path) const;
    Ret doAddSoundFont(const io::path_t& src, const io::path_t& dst);
};
}
