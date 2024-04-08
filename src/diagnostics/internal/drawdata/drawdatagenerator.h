/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_DIAGNOSTICS_DRAWDATAGENERATOR_H
#define MU_DIAGNOSTICS_DRAWDATAGENERATOR_H

#include "global/types/ret.h"
#include "global/io/path.h"
#include "draw/types/drawdata.h"
#include "../../diagnosticstypes.h"

#include "modularity/ioc.h"
#include "engraving/rendering/iscorerenderer.h"

namespace mu::engraving {
class MasterScore;
}

namespace mu::diagnostics {
class DrawDataGenerator
{
    INJECT(engraving::rendering::IScoreRenderer, scoreRenderer)
public:
    DrawDataGenerator() = default;

    muse::Ret processDir(const muse::io::path_t& scoreDir, const muse::io::path_t& outDir, const GenOpt& opt = GenOpt());
    muse::Ret processFile(const muse::io::path_t& scoreFile, const muse::io::path_t& outFile, const GenOpt& opt = GenOpt());

    muse::draw::DrawDataPtr genDrawData(const muse::io::path_t& scorePath, const GenOpt& opt = GenOpt()) const;
    muse::draw::Pixmap genImage(const muse::io::path_t& scorePath) const;

private:
    bool loadScore(engraving::MasterScore* score, const muse::io::path_t& path) const;
    void applyOptions(engraving::MasterScore* score, const GenOpt& opt) const;
};
}

#endif // MU_DIAGNOSTICS_DRAWDATAGENERATOR_H
