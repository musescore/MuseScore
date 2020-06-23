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
#ifndef MU_DOMAIN_NOTATIONERRORS_H
#define MU_DOMAIN_NOTATIONERRORS_H

#include "ret.h"
#include "libmscore/score.h"

namespace mu {
namespace domain {
namespace notation {
// 100 - 499
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::NotationFirst),
    UserAbort,
    IgnoreError,

    // files
    FileUnknownError    = 110,
    FileNotFound        = 111,
    FileOpenError       = 112,
    FileBadFormat       = 113,
    FileUnknownType     = 114,
    FileNoRootfile      = 115,
    FileTooOld          = 116,
    FileTooNew          = 117,
    FileOld300Format    = 118,
    FileCorrupted       = 119,
    FileCriticalCorrupted = 120,
};

inline mu::Ret make_ret(Err e)
{
    return Ret(static_cast<int>(e));
}

inline Ret scoreFileErrorToRet(Ms::Score::FileError e)
{
    switch (e) {
    case Ms::Score::FileError::FILE_NO_ERROR:       return make_ret(Err::NoError);
    case Ms::Score::FileError::FILE_ERROR:          return make_ret(Err::FileUnknownError);
    case Ms::Score::FileError::FILE_NOT_FOUND:      return make_ret(Err::FileNotFound);
    case Ms::Score::FileError::FILE_OPEN_ERROR:     return make_ret(Err::FileOpenError);
    case Ms::Score::FileError::FILE_BAD_FORMAT:     return make_ret(Err::FileBadFormat);
    case Ms::Score::FileError::FILE_UNKNOWN_TYPE:   return make_ret(Err::FileUnknownType);
    case Ms::Score::FileError::FILE_NO_ROOTFILE:    return make_ret(Err::FileNoRootfile);
    case Ms::Score::FileError::FILE_TOO_OLD:        return make_ret(Err::FileTooOld);
    case Ms::Score::FileError::FILE_TOO_NEW:        return make_ret(Err::FileTooNew);
    case Ms::Score::FileError::FILE_OLD_300_FORMAT: return make_ret(Err::FileOld300Format);
    case Ms::Score::FileError::FILE_CORRUPTED:      return make_ret(Err::FileCorrupted);
    case Ms::Score::FileError::FILE_CRITICALLY_CORRUPTED: return make_ret(Err::FileCriticalCorrupted);
    case Ms::Score::FileError::FILE_USER_ABORT:      return make_ret(Err::UserAbort);
    case Ms::Score::FileError::FILE_IGNORE_ERROR:    return make_ret(Err::IgnoreError);
    }
    return Ret();
}
}
}
}

#endif // MU_DOMAIN_NOTATIONERRORS_H
