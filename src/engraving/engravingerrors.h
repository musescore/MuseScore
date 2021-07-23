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

inline Err scoreFileErrorToErr(Ms::Score::FileError err)
{
    switch (err) {
    case Ms::Score::FileError::FILE_NO_ERROR:       return Err::NoError;
    case Ms::Score::FileError::FILE_ERROR:          return Err::FileUnknownError;
    case Ms::Score::FileError::FILE_NOT_FOUND:      return Err::FileNotFound;
    case Ms::Score::FileError::FILE_OPEN_ERROR:     return Err::FileOpenError;
    case Ms::Score::FileError::FILE_BAD_FORMAT:     return Err::FileBadFormat;
    case Ms::Score::FileError::FILE_UNKNOWN_TYPE:   return Err::FileUnknownType;
    case Ms::Score::FileError::FILE_NO_ROOTFILE:    return Err::FileBadFormat;
    case Ms::Score::FileError::FILE_TOO_OLD:        return Err::FileTooOld;
    case Ms::Score::FileError::FILE_TOO_NEW:        return Err::FileTooNew;
    case Ms::Score::FileError::FILE_OLD_300_FORMAT: return Err::FileOld300Format;
    case Ms::Score::FileError::FILE_CORRUPTED:      return Err::FileCorrupted;
    case Ms::Score::FileError::FILE_CRITICALLY_CORRUPTED: return Err::FileCriticalCorrupted;
    case Ms::Score::FileError::FILE_USER_ABORT:      return Err::UserAbort;
    case Ms::Score::FileError::FILE_IGNORE_ERROR:    return Err::IgnoreError;
    }
    return Err::FileUnknownError;
}
}

#endif // MU_ENGRAVING_ENGRAVINGERRORS_H
