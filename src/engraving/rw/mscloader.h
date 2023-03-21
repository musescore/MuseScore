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

#include "global/types/ret.h"

#include "infrastructure/mscreader.h"
#include "engraving/types/types.h"

namespace mu::engraving::compat {
class ReadStyleHook;
}

namespace mu::engraving {
class MasterScore;
struct ReadInOutData;
class XmlReader;
class MscLoader
{
public:
    MscLoader() = default;

    Ret loadMscz(MasterScore* score, const MscReader& mscReader, SettingsCompat& settingsCompat, bool ignoreVersionError);

private:

    friend class MasterScore;

    Ret read(MasterScore* score, XmlReader&, bool ignoreVersionError, ReadInOutData* out = nullptr,
             compat::ReadStyleHook* styleHook = nullptr);
};
}

#endif // MU_ENGRAVING_SCOREREADER_H
