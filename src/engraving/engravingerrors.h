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

#include "libmscore/masterscore.h"

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
    FileCriticalCorrupted = 2010,

    UserAbort = 2011,
    IgnoreError = 2012
};

inline Ret make_ret(Err err, const io::path_t& filePath = "")
{
    String text;

    switch (err) {
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
        //: The text between %1 and %2 will be a clickable link.
        text = mtrc("engraving", "It was last saved with a version older than 2.0.0. "
                                 "You can convert this score by opening and then "
                                 "saving with MuseScore version 2.x. "
                                 "Visit the %1MuseScore download page%2 to obtain such a 2.x version.")
               .arg(u"<a href=\"https://musescore.org/download#older-versions\">", u"</a>");
        break;
    case Err::FileTooNew:
        //: The text between %1 and %2 will be a clickable link.
        text = mtrc("engraving", "This score was saved using a newer version of MuseScore. "
                                 "Visit the %1MuseScore website%2 to obtain the latest version.")
               .arg(u"<a href=\"https://musescore.org\">", u"</a>");
        break;
    case Err::FileOld300Format:
        text = mtrc("engraving", "It was last saved with a development version of 3.0.");
        break;
    case Err::FileCorrupted:
        text = mtrc("engraving", "File \"%1\" is corrupted.").arg(filePath.toString());
        break;
    case Err::FileCriticalCorrupted:
        text = mtrc("engraving", "File \"%1\" is critically corrupted and cannot be processed.").arg(filePath.toString());
        break;
    case Err::Undefined:
    case Err::NoError:
    case Err::UnknownError:
    case Err::IgnoreError:
    case Err::UserAbort:
        break;
    }

    return mu::Ret(static_cast<int>(err), text.toStdString());
}

inline Err scoreFileErrorToErr(Score::FileError err)
{
    switch (err) {
    case Score::FileError::FILE_NO_ERROR:       return Err::NoError;
    case Score::FileError::FILE_ERROR:          return Err::FileUnknownError;
    case Score::FileError::FILE_NOT_FOUND:      return Err::FileNotFound;
    case Score::FileError::FILE_OPEN_ERROR:     return Err::FileOpenError;
    case Score::FileError::FILE_BAD_FORMAT:     return Err::FileBadFormat;
    case Score::FileError::FILE_UNKNOWN_TYPE:   return Err::FileUnknownType;
    case Score::FileError::FILE_NO_ROOTFILE:    return Err::FileBadFormat;
    case Score::FileError::FILE_TOO_OLD:        return Err::FileTooOld;
    case Score::FileError::FILE_TOO_NEW:        return Err::FileTooNew;
    case Score::FileError::FILE_OLD_300_FORMAT: return Err::FileOld300Format;
    case Score::FileError::FILE_CORRUPTED:      return Err::FileCorrupted;
    case Score::FileError::FILE_CRITICALLY_CORRUPTED: return Err::FileCriticalCorrupted;
    case Score::FileError::FILE_USER_ABORT:      return Err::UserAbort;
    case Score::FileError::FILE_IGNORE_ERROR:    return Err::IgnoreError;
    }
    return Err::FileUnknownError;
}

inline Ret scoreFileErrorToRet(Score::FileError err, const io::path_t& filePath)
{
    return make_ret(scoreFileErrorToErr(err), filePath);
}
}

#endif // MU_ENGRAVING_ENGRAVINGERRORS_H
