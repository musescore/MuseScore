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
#include "musicxmlreader.h"

#include "io/path.h"
#include "libmscore/masterscore.h"
#include "engraving/engravingerrors.h"

namespace mu::engraving {
extern Score::FileError importMusicXml(MasterScore*, const QString&);
extern Score::FileError importCompressedMusicXml(MasterScore*, const QString&);
}

using namespace mu::iex::musicxml;

mu::Ret MusicXmlReader::read(mu::engraving::MasterScore* score, const io::path_t& path, const Options&)
{
    mu::engraving::Score::FileError err = mu::engraving::Score::FileError::FILE_UNKNOWN_TYPE;
    std::string suffix = mu::io::suffix(path);
    if (suffix == "xml" || suffix == "musicxml") {
        err = mu::engraving::importMusicXml(score, path.toQString());
    } else if (suffix == "mxl") {
        err = mu::engraving::importCompressedMusicXml(score, path.toQString());
    }
    return mu::engraving::scoreFileErrorToRet(err, path);
}
