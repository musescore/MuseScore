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
#ifndef MU_NOTATION_NOTATIONERRORS_H
#define MU_NOTATION_NOTATIONERRORS_H

#include "ret.h"
#include "translation.h"
#include "io/path.h"
#include "libmscore/score.h"

namespace mu::notation {
// 1000 - 1299
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::NotationFirst),
    UserAbort,
    IgnoreError,

    // files
    FileUnknownError    = 1010,
    FileNotFound        = 1011,
    FileOpenError       = 1012,
    FileBadFormat       = 1013,
    FileUnknownType     = 1014,
    FileNoRootFile      = 1015,
    FileTooOld          = 1016,
    FileTooNew          = 1017,
    FileOld300Format    = 1018,
    FileCorrupted       = 1019,
    FileCriticalCorrupted = 1020,

    // notation
    NoScore = 1030,
};

inline Ret make_ret(Err e, const io::path& filePath = "")
{
    int retCode = static_cast<int>(e);
    QString fileName = io::filename(filePath).toQString();

    switch (e) {
    case Err::Undefined: return Ret(retCode);
    case Err::NoError: return Ret(retCode);
    case Err::UnknownError: return Ret(retCode);
    case Err::UserAbort: return Ret(retCode);
    case Err::IgnoreError: return Ret(retCode);
    case Err::FileUnknownError: return Ret(retCode, trc("notation", "Unknown error"));
    case Err::FileNotFound: return Ret(retCode, qtrc("notation", "File \"%1\" not found").arg(fileName).toStdString());
    case Err::FileOpenError: return Ret(retCode, trc("notation", "File open error"));
    case Err::FileBadFormat: return Ret(retCode, trc("notation", "Bad format"));
    case Err::FileUnknownType: return Ret(retCode, trc("notation", "Unknown filetype"));
    case Err::FileNoRootFile: return Ret(retCode, trc("notation", "Not found root file"));
    case Err::FileTooOld: return Ret(retCode, qtrc("notation", "It was last saved with a version older than 2.0.0.\n"
                                                               "You can convert this score by opening and then\n"
                                                               "saving with MuseScore version 2.x.\n"
                                                               "Visit the %1MuseScore download page%2 to obtain such a 2.x version.")
                                     .arg("<a href=\"https://musescore.org/download#older-versions\">")
                                     .arg("</a>").toStdString());
    case Err::FileTooNew: return Ret(retCode, qtrc("notation", "This score was saved using a newer version of MuseScore.\n "
                                                               "Visit the %1MuseScore website%2 to obtain the latest version.")
                                     .arg("<a href=\"https://musescore.org\">")
                                     .arg("</a>").toStdString());
    case Err::FileOld300Format: return Ret(retCode, trc("notation", "It was last saved with a developer version of 3.0."));
    case Err::FileCorrupted: return Ret(retCode, qtrc("notation", "File \"%1\" corrupted.").arg(fileName).toStdString());
    case Err::FileCriticalCorrupted: return Ret(retCode, qtrc("notation",
                                                              "File \"%1\" is critically corrupted and cannot be processed.").arg(fileName).toStdString());
    case Err::NoScore: return Ret(retCode, trc("notation", "No score"));
    }

    return Ret(retCode);
}

inline Ret scoreFileErrorToRet(Ms::Score::FileError e, const io::path& filePath)
{
    auto makeRet = [=](Err err) {
        return make_ret(err, filePath);
    };

    switch (e) {
    case Ms::Score::FileError::FILE_NO_ERROR:       return makeRet(Err::NoError);
    case Ms::Score::FileError::FILE_ERROR:          return makeRet(Err::FileUnknownError);
    case Ms::Score::FileError::FILE_NOT_FOUND:      return makeRet(Err::FileNotFound);
    case Ms::Score::FileError::FILE_OPEN_ERROR:     return makeRet(Err::FileOpenError);
    case Ms::Score::FileError::FILE_BAD_FORMAT:     return makeRet(Err::FileBadFormat);
    case Ms::Score::FileError::FILE_UNKNOWN_TYPE:   return makeRet(Err::FileUnknownType);
    case Ms::Score::FileError::FILE_NO_ROOTFILE:    return makeRet(Err::FileNoRootFile);
    case Ms::Score::FileError::FILE_TOO_OLD:        return makeRet(Err::FileTooOld);
    case Ms::Score::FileError::FILE_TOO_NEW:        return makeRet(Err::FileTooNew);
    case Ms::Score::FileError::FILE_OLD_300_FORMAT: return makeRet(Err::FileOld300Format);
    case Ms::Score::FileError::FILE_CORRUPTED:      return makeRet(Err::FileCorrupted);
    case Ms::Score::FileError::FILE_CRITICALLY_CORRUPTED: return makeRet(Err::FileCriticalCorrupted);
    case Ms::Score::FileError::FILE_USER_ABORT:      return makeRet(Err::UserAbort);
    case Ms::Score::FileError::FILE_IGNORE_ERROR:    return makeRet(Err::IgnoreError);
    }
    return Ret();
}
}

#endif // MU_NOTATION_NOTATIONERRORS_H
