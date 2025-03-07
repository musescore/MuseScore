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

inline muse::Ret make_ret(Err err, const muse::String& text)
{
    return muse::Ret(static_cast<int>(err), text.toStdString());
}

inline muse::Ret make_ret(Err err, const muse::io::path_t& filePath = {})
{
    muse::String text;

    switch (err) {
    case Err::NoError:
        return muse::make_ok();
    case Err::FileUnknownError:
        text = muse::mtrc("engraving", "Unknown error");
        break;
    case Err::FileNotFound:
        text = muse::mtrc("engraving", "File “%1” not found").arg(filePath.toString());
        break;
    case Err::FileOpenError:
        text = muse::mtrc("engraving", "File open error");
        break;
    case Err::FileBadFormat:
        text = muse::mtrc("engraving", "Bad format");
        break;
    case Err::FileUnknownType:
        text = muse::mtrc("engraving", "Unknown filetype");
        break;
    case Err::FileTooOld:
        text = muse::mtrc("engraving", "This file was last saved in a version older than 2.0.0. "
                                       "You can convert this score by opening and then "
                                       "saving in MuseScore version 2.x. "
                                       "Visit the <a href=\"%1\">MuseScore download page</a> to obtain such a 2.x version.")
               .arg(u"https://musescore.org/download#older-versions");
        break;
    case Err::FileTooNew:
        text = muse::mtrc("engraving", "This file was saved using a newer version of MuseScore Studio. "
                                       "Please visit <a href=\"%1\">MuseScore.org</a> to obtain the latest version.")
               .arg(u"https://musescore.org");
        break;
    case Err::FileOld300Format:
        text = muse::mtrc("engraving", "This file was last saved in a development version of 3.0.");
        break;
    case Err::FileCorrupted:
        text = muse::mtrc("engraving", "File “%1” is corrupted.").arg(filePath.toString());
        break;
    case Err::FileCriticallyCorrupted:
        text = muse::mtrc("engraving", "File “%1” is critically corrupted and cannot be processed.").arg(filePath.toString());
        break;
    case Err::Undefined:
    case Err::UnknownError:
    case Err::IgnoreError:
    case Err::UserAbort:
        break;
    }

    return muse::Ret(static_cast<int>(err), text.toStdString());
}
}

#endif // MU_ENGRAVING_ENGRAVINGERRORS_H
