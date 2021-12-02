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
#ifndef MU_ENGRAVING_SCOREREADER_H
#define MU_ENGRAVING_SCOREREADER_H

#include "../engravingerrors.h"
#include "xml.h"
#include "../infrastructure/io/mscreader.h"
#include "readcontext.h"
#include "../libmscore/masterscore.h"

namespace mu::engraving {
class ScoreReader
{
public:
    ScoreReader() = default;

    Err loadMscz(Ms::MasterScore* score, const mu::engraving::MscReader& mscReader, bool ignoreVersionError);

private:

    friend class Ms::MasterScore;

    Err read(Ms::MasterScore* score, Ms::XmlReader&, ReadContext& ctx, compat::ReadStyleHook* styleHook = nullptr);
    Err doRead(Ms::MasterScore* score, Ms::XmlReader& e, ReadContext& ctx);
};
}

#endif // MU_ENGRAVING_SCOREREADER_H
