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
#pragma once

#include "project/inotationreader.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "io/ifilesystem.h"

#include "engraving/engravingerrors.h"

namespace mu::iex::mei {
class MeiReader : public project::INotationReader
{
    INJECT(muse::IInteractive, interactive)
    INJECT(muse::io::IFileSystem, fileSystem)

public:
    muse::Ret read(mu::engraving::MasterScore* score, const muse::io::path_t& path, const Options& options = Options()) override;
    mu::engraving::Err import(mu::engraving::MasterScore* score, const muse::io::path_t& path, const Options& options = Options());

private:
    bool askToLoadDespiteWarnings(const muse::String& text, const muse::String& detailedText);
};
}
