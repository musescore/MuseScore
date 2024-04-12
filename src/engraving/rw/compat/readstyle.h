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

#ifndef MU_ENGRAVING_READSTYLE_H
#define MU_ENGRAVING_READSTYLE_H

#include "types/bytearray.h"
#include "types/string.h"

namespace mu::engraving {
class Score;
class XmlReader;
class MStyle;
}

namespace mu::engraving::compat {
class ReadStyleHook
{
public:
    ReadStyleHook(Score* score, const muse::ByteArray& scoreData, const muse::String& completeBaseName);

    void setupDefaultStyle();

    void readStyleTag(XmlReader& e);

    static int styleDefaultByMscVersion(const int mscVer);
    static void setupDefaultStyle(Score* score);
    static void readStyleTag(Score* score, XmlReader& e);
    static bool readStyleProperties(MStyle* style, XmlReader& e);

private:
    Score* m_score = nullptr;
    const muse::ByteArray& m_scoreData;
    const muse::String& m_completeBaseName;
};
}

#endif // MU_ENGRAVING_READSTYLE_H
