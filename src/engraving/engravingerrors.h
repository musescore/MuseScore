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

#include "ret.h"
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
    QString text;

    switch (err) {
    case Err::FileUnknownError:
        text = qtrc("engraving", "Unknown error");
        break;
    case Err::FileNotFound:
        text = qtrc("engraving", "File \"%1\" not found")
               .arg(filePath.toQString());
        break;
    case Err::FileOpenError:
        text = qtrc("engraving", "File open error");
        break;
    case Err::FileBadFormat:
        text = qtrc("engraving", "Bad format");
        break;
    case Err::FileUnknownType:
        text = qtrc("engraving", "Unknown filetype");
        break;
    case Err::FileTooOld:
        text = qtrc("engraving", "It was last saved with a version older than 2.0.0.\n"
                                 "You can convert this score by opening and then\n"
                                 "saving with MuseScore version 2.x.\n"
                                 "Visit the %1MuseScore download page%2 to obtain such a 2.x version.")
               .arg("<a href=\"https://musescore.org/download#older-versions\">", "</a>");
        break;
    case Err::FileTooNew:
        text = qtrc("engraving", "This score was saved using a newer version of MuseScore.\n "
                                 "Visit the %1MuseScore website%2 to obtain the latest version.")
               .arg("<a href=\"https://musescore.org\">", "</a>");
        break;
    case Err::FileOld300Format:
        text = qtrc("engraving", "It was last saved with a developer version of 3.0.");
        break;
    case Err::FileCorrupted:
        text = qtrc("engraving", "File \"%1\" corrupted.")
               .arg(filePath.toQString());
        break;
    case Err::FileCriticalCorrupted:
        text = qtrc("engraving", "File \"%1\" is critically corrupted and cannot be processed.")
               .arg(filePath.toQString());
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

inline Err scoreFileErrorToErr(mu::engraving::Score::FileError err)
{
    switch (err) {
    case mu::engraving::Score::FileError::FILE_NO_ERROR:       return Err::NoError;
    case mu::engraving::Score::FileError::FILE_ERROR:          return Err::FileUnknownError;
    case mu::engraving::Score::FileError::FILE_NOT_FOUND:      return Err::FileNotFound;
    case mu::engraving::Score::FileError::FILE_OPEN_ERROR:     return Err::FileOpenError;
    case mu::engraving::Score::FileError::FILE_BAD_FORMAT:     return Err::FileBadFormat;
    case mu::engraving::Score::FileError::FILE_UNKNOWN_TYPE:   return Err::FileUnknownType;
    case mu::engraving::Score::FileError::FILE_NO_ROOTFILE:    return Err::FileBadFormat;
    case mu::engraving::Score::FileError::FILE_TOO_OLD:        return Err::FileTooOld;
    case mu::engraving::Score::FileError::FILE_TOO_NEW:        return Err::FileTooNew;
    case mu::engraving::Score::FileError::FILE_OLD_300_FORMAT: return Err::FileOld300Format;
    case mu::engraving::Score::FileError::FILE_CORRUPTED:      return Err::FileCorrupted;
    case mu::engraving::Score::FileError::FILE_CRITICALLY_CORRUPTED: return Err::FileCriticalCorrupted;
    case mu::engraving::Score::FileError::FILE_USER_ABORT:      return Err::UserAbort;
    case mu::engraving::Score::FileError::FILE_IGNORE_ERROR:    return Err::IgnoreError;
    }
    return Err::FileUnknownError;
}

inline Ret scoreFileErrorToRet(mu::engraving::Score::FileError err, const io::path_t& filePath)
{
    return make_ret(scoreFileErrorToErr(err), filePath);
}
}

#endif // MU_ENGRAVING_ENGRAVINGERRORS_H
