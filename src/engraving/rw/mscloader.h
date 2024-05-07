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
#ifndef MU_ENGRAVING_MSCLOADER_H
#define MU_ENGRAVING_MSCLOADER_H

#include "global/types/ret.h"

#include "../infrastructure/mscreader.h"
#include "../types/types.h"

namespace mu::engraving::compat {
class ReadStyleHook;
}

namespace mu::engraving::rw {
struct ReadInOutData;
}

namespace mu::engraving {
class MasterScore;
class XmlReader;
class MscLoader
{
public:
    MscLoader() = default;

    muse::Ret loadMscz(MasterScore* score, const MscReader& mscReader, SettingsCompat& settingsCompat, bool ignoreVersionError,
                       rw::ReadInOutData* out = nullptr);

private:
    friend class MasterScore;
    muse::Ret readMasterScore(MasterScore* score, XmlReader&, bool ignoreVersionError, rw::ReadInOutData* out = nullptr,
                              compat::ReadStyleHook* styleHook = nullptr);
};
}

#endif // MU_ENGRAVING_MSCLOADER_H
