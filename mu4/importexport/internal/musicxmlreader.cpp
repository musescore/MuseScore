//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "musicxmlreader.h"

#include "io/path.h"
#include "libmscore/score.h"
#include "notation/notationerrors.h"

namespace Ms {
extern Score::FileError importMusicXml(MasterScore*, const QString&);
extern Score::FileError importCompressedMusicXml(MasterScore*, const QString&);
}

using namespace mu::importexport;

mu::Ret MusicXmlReader::read(Ms::MasterScore* score, const io::path& path)
{
    Ms::Score::FileError err = Ms::Score::FileError::FILE_UNKNOWN_TYPE;
    std::string syffix = mu::io::syffix(path);
    if (syffix == "xml" || syffix == "musicxml") {
        err = Ms::importMusicXml(score, path.toQString());
    } else if (syffix == "mxl") {
        err = Ms::importCompressedMusicXml(score, path.toQString());
    }
    return mu::notation::scoreFileErrorToRet(err);
}
