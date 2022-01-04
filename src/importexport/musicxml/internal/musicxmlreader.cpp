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
#include "notation/notationerrors.h"

namespace Ms {
extern Score::FileError importMusicXml(MasterScore*, const QString&);
extern Score::FileError importCompressedMusicXml(MasterScore*, const QString&);
}

using namespace mu::iex::musicxml;

mu::Ret MusicXmlReader::read(Ms::MasterScore* score, const io::path& path, const Options&)
{
    Ms::Score::FileError err = Ms::Score::FileError::FILE_UNKNOWN_TYPE;
    std::string syffix = mu::io::suffix(path);
    if (syffix == "xml" || syffix == "musicxml") {
        err = Ms::importMusicXml(score, path.toQString());
    } else if (syffix == "mxl") {
        err = Ms::importCompressedMusicXml(score, path.toQString());
    }
    return mu::notation::scoreFileErrorToRet(err, path);
}
