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

#pragma once

#include "../../isoundfontcontroller.h"

#include "global/async/asyncable.h"

#include "global/modularity/ioc.h"
#include "global/iinteractive.h"
#include "global/io/ifilesystem.h"
#include "../../iaudioconfiguration.h"
#include "audio/common/rpc/irpcchannel.h"

namespace muse::audio {
class GeneralSoundFontController : public ISoundFontController, public async::Asyncable, public muse::Injectable
{
    GlobalInject<IAudioConfiguration> configuration;
    GlobalInject<io::IFileSystem> fileSystem;
    Inject<IInteractive> interactive = { this };
    Inject<rpc::IRpcChannel> channel = { this };

public:
    GeneralSoundFontController(const muse::modularity::ContextPtr& iocCtx);

    void loadSoundFonts() override;

    void addSoundFont(const synth::SoundFontUri& uri) override;

private:

    Ret doAddSoundFont(const io::path_t& src, const io::path_t& dst);

    RetVal<io::path_t> resolveInstallationPath(const io::path_t& path) const;

    void doLoadSoundFonts();
    void loadSoundFonts(const std::vector<io::path_t>& paths);
};
}
