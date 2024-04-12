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

#ifndef MU_ENGRAVING_SCORERW_H
#define MU_ENGRAVING_SCORERW_H

#include <functional>

#include "types/string.h"

#include "engraving/engravingerrors.h"
#include "engraving/dom/masterscore.h"

namespace mu::engraving {
class ScoreRW
{
public:
    ScoreRW() = default;

    static void setRootPath(const String& path);
    static String rootPath();

    using ImportFunc = std::function<Err(MasterScore* score, const muse::io::path_t& path)>;
    using ExportFunc = std::function<Err(Score* score, const muse::io::path_t& path)>;

    static MasterScore* readScore(const String& path, bool isAbsolutePath = false, ImportFunc importFunc = nullptr);
    static bool saveScore(Score* score, const String& name);
    static bool saveScore(Score* score, const String& name, ExportFunc exportFunc);
    static EngravingItem* writeReadElement(EngravingItem* element);
    static bool saveMimeData(muse::ByteArray mimeData, const String& saveName);

private:
    static String m_rootPath;
};
}

#endif // MU_ENGRAVING_SCORERW_H
