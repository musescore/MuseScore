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
#ifndef MU_ENGRAVING_MSCIO_H
#define MU_ENGRAVING_MSCIO_H

#include <string>

namespace mu::engraving {
static const std::string MSCZ = "mscz";
static const std::string MSCX = "mscx";
static const std::string MSCS = "mscs";

inline bool isMuseScoreFile(const std::string& suffix)
{
    return suffix == MSCZ || suffix == MSCX || suffix == MSCS;
}

enum class MscIoMode {
    Unknown = 0,
    Zip,
    Dir,
    XmlFile
};

inline MscIoMode mcsIoModeBySuffix(const std::string& suffix)
{
    if (suffix == MSCZ) {
        return MscIoMode::Zip;
    } else if (suffix == MSCX) {
        return MscIoMode::Dir;
    } else if (suffix == MSCS) {
        return MscIoMode::XmlFile;
    }
    return MscIoMode::Unknown;
}
}

#endif // MU_ENGRAVING_MSCIO_H
