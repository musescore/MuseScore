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
#ifndef MU_ENGRAVING_MSCIO_H
#define MU_ENGRAVING_MSCIO_H

#include <string>

#include "io/path.h"

namespace mu::engraving {
//! NOTE The main format is MuseScore, is a zip archive with a specific structure
static const std::string MSCZ = "mscz";

//! NOTE Before MuseScore 4, MuseScore could save data in one xml file (excluding binary) with `.mscx` extension.
//! Starting from MuseScore 4, only the score domain model store to the `.mscx` file,
//! and other data, such as styles, chordlist, synthesizer settings, etc. are stored in separate files.
//! We can choose to save a `.mscx` file, in this case a folder with the same structure
//! as a zip container will be created (i.e. an unpacked zip container)
//! We can select `.mscx` to open, in this case it is now expected that it is in a folder
//! in which there are also other data, structured as in a zip container.
static const std::string MSCX = "mscx";

//! NOTE For testing purposes and some workflows of work with score, there is a need to store all data in one text file.
//! Therefore, a new file type has been introduced for this.
//! This is a XML file that stores all files from the ZIP container by concatenation (excluding binaries)
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

inline MscIoMode mscIoModeBySuffix(const std::string& suffix)
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

inline muse::io::path_t containerPath(const muse::io::path_t& path)
{
    if (muse::io::suffix(path) == MSCX) {
        return muse::io::absoluteDirpath(path);
    }

    return path;
}

inline muse::io::path_t mainFilePath(const muse::io::path_t& path)
{
    if (isMuseScoreFile(muse::io::suffix(path))) {
        return path;
    }

    return path.appendingComponent(muse::io::filename(path)).appendingSuffix(MSCX);
}

inline muse::io::path_t mainFileName(const muse::io::path_t& path)
{
    return muse::io::filename(path, !isMuseScoreFile(muse::io::suffix(path))).appendingSuffix(MSCX);
}
}

#endif // MU_ENGRAVING_MSCIO_H
