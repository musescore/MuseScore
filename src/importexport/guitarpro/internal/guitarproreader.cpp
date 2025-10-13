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
#include "guitarproreader.h"

#include "io/file.h"

#include "engraving/dom/excerpt.h"
#include "engraving/dom/masterscore.h"
#include "engraving/engravingerrors.h"

namespace mu::iex::guitarpro {
extern mu::engraving::Err importGTP(mu::engraving::MasterScore*, muse::io::IODevice* io, const muse::modularity::ContextPtr& iocCtx,
                                    bool experimental = false);

muse::Ret GuitarProReader::read(mu::engraving::MasterScore* score, const muse::io::path_t& path, const Options&)
{
    muse::io::File file(path);
    mu::engraving::Err err = importGTP(score, &file, iocContext(), guitarProConfiguration()->experimental());

    score->cmdResetToDefaultLayout();
    switch (guitarProConfiguration()->deviceForSvgExport()) {
    case 0:
    {
        muse::io::File styleFile(":/engraving/styles/svg-import-style-phone.mss");
        score->loadStyle(styleFile);
        break;
    }
    case 1:
    {
        muse::io::File styleFile(":/engraving/styles/svg-import-style-tablet.mss");
        score->loadStyle(styleFile);
        break;
    }
    case 2:
    {
        muse::io::File styleFile(":/engraving/styles/svg-import-style-desktop.mss");
        score->loadStyle(styleFile);
        break;
    }
    }

    return mu::engraving::make_ret(err, path);
}
} // namespace mu::iex::guitarpro
