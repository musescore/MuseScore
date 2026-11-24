/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

#include <string>
#include <vector>

#include "musesounds/imusehublauncher.h"

#include "global/async/asyncable.h"
#include "global/modularity/ioc.h"
#include "global/iinteractive.h"
#include "global/iprocess.h"
#include "musesounds/imusesoundsconfiguration.h"

namespace mu::musesounds {
class MuseHubLauncher : public IMuseHubLauncher, public muse::Contextable, public muse::async::Asyncable
{
public:
    muse::GlobalInject<IMuseSoundsConfiguration> configuration;
    muse::GlobalInject<muse::IProcess> process;
    muse::ContextInject<muse::IInteractive> interactive = { this };

public:
    MuseHubLauncher(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    muse::async::Promise<muse::Ret> open(MuseHubPage page, const std::string& utmContent = std::string()) override;
    muse::async::Promise<muse::Ret> openSoundLibrary(const SoundLibraryInfo& soundLibrary,
                                                     const std::string& utmContent = std::string()) override;

private:
    using Resolve = muse::async::Promise<muse::Ret>::Resolve;

    muse::async::Promise<muse::Ret> tryOpenMuseHub(const MuseHubUrls& urls);
    muse::async::Promise<muse::Ret> tryOpenMuseSoundsManager(const std::vector<std::string>& args, const std::string& webUrl);
    void openMuseHubApp(const muse::UriQuery& museHubUri, const std::string& webUrl, const Resolve& resolve);
    void openWebUrl(const std::string& webUrl, const Resolve& resolve);
};
}
