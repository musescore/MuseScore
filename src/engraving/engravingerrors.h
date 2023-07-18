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

#ifndef MU_ENGRAVING_ENGRAVINGERRORS_H
#define MU_ENGRAVING_ENGRAVINGERRORS_H

#include "io/path.h"
#include "types/ret.h"

#include "translation.h"

namespace mu::engraving {
enum class Err {
    Undefined       = -1,
    NoError         = 0,
    UnknownError    = 2000,

    // file
    FileUnknownError = 2001,
    FileNotFound = 2002,
    FileOpenError = 2003,
    FileBadFormat = 2004,
    FileUnknownType = 2005,
    FileTooOld = 2006,
    FileTooNew = 2007,
    FileOld300Format = 2008,
    FileCorrupted = 2009,
    FileCriticallyCorrupted = 2010,

    UserAbort = 2011,
    IgnoreError = 2012
};

inline Ret make_ret(Err err, const io::path_t& filePath = "")
{
    String text;

    switch (err) {
    case Err::NoError:
        return make_ok();
    case Err::FileUnknownError:
        text = mtrc("engraving", "Unknown error");
        break;
    case Err::FileNotFound:
        text = mtrc("engraving", "File \"%1\" not found").arg(filePath.toString());
        break;
    case Err::FileOpenError:
        text = mtrc("engraving", "File open error");
        break;
    case Err::FileBadFormat:
        text = mtrc("engraving", "Bad format");
        break;
    case Err::FileUnknownType:
        text = mtrc("engraving", "Unknown filetype");
        break;
    case Err::FileTooOld:
        text = mtrc("engraving", "This file was last saved in a version older than 2.0.0. "
                                 "You can convert this score by opening and then "
                                 "saving in MuseScore version 2.x. "
                                 "Visit the <a href=\"%1\">MuseScore download page</a> to obtain such a 2.x version.")
               .arg(u"https://musescore.org/download#older-versions");
        break;
    case Err::FileTooNew:
        text = mtrc("engraving", "This file was saved using a newer version of MuseScore. "
                                 "Please visit <a href=\"https://musescore.org\">musescore.org</a> to obtain the latest version.");
        break;
    case Err::FileOld300Format:
        text = mtrc("engraving", "This file was last saved in a development version of 3.0.");
        break;
    case Err::FileCorrupted:
        text = mtrc("engraving", "File \"%1\" is corrupted.").arg(filePath.toString());
        break;
    case Err::FileCriticallyCorrupted:
        text = mtrc("engraving", "File \"%1\" is critically corrupted and cannot be processed.").arg(filePath.toString());
        break;
    case Err::Undefined:
    case Err::UnknownError:
    case Err::IgnoreError:
    case Err::UserAbort:
        break;
    }

    return mu::Ret(static_cast<int>(err), text.toStdString());
}
}

#endif // MU_ENGRAVING_ENGRAVINGERRORS_H
