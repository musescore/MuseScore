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
#include "libmscore/masterscore.h"
#include "engraving/engravingerrors.h"

namespace mu {
inline Ret make_ret(engraving::Err e)
{
    return Ret(static_cast<int>(e));
}
}

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

    NoteOrRestIsNotSelected,
    NoteOrFiguredBassIsNotSelected,
    MeasureIsNotSelected,

    // playback
    UnableToPlaybackElement = 1040,

    // selection
    EmptySelection = 1050,
};

inline Ret make_ret(Err err, const io::path& filePath = "")
{
    int code = static_cast<int>(err);
    QString fileName = io::filename(filePath).toQString();
    QString text;

    switch (err) {
    case Err::FileUnknownError:
        text = qtrc("notation", "Unknown error");
        break;
    case Err::FileNotFound:
        text = qtrc("notation", "File \"%1\" not found")
               .arg(fileName);
        break;
    case Err::FileOpenError:
        text = qtrc("notation", "File open error");
        break;
    case Err::FileBadFormat:
        text = qtrc("notation", "Bad format");
        break;
    case Err::FileUnknownType:
        text = qtrc("notation", "Unknown filetype");
        break;
    case Err::FileNoRootFile:
        text = qtrc("notation", "Not found root file");
        break;
    case Err::FileTooOld:
        text = qtrc("notation", "It was last saved with a version older than 2.0.0.\n"
                                "You can convert this score by opening and then\n"
                                "saving with MuseScore version 2.x.\n"
                                "Visit the %1MuseScore download page%2 to obtain such a 2.x version.")
               .arg("<a href=\"https://musescore.org/download#older-versions\">", "</a>");
        break;
    case Err::FileTooNew:
        text = qtrc("notation", "This score was saved using a newer version of MuseScore.\n "
                                "Visit the %1MuseScore website%2 to obtain the latest version.")
               .arg("<a href=\"https://musescore.org\">", "</a>");
        break;
    case Err::FileOld300Format:
        text = qtrc("notation", "It was last saved with a developer version of 3.0.");
        break;
    case Err::FileCorrupted:
        text = qtrc("notation", "File \"%1\" corrupted.")
               .arg(fileName);
        break;
    case Err::FileCriticalCorrupted:
        text = qtrc("notation", "File \"%1\" is critically corrupted and cannot be processed.")
               .arg(fileName);
        break;
    case Err::NoScore:
        text = qtrc("notation", "No score");
        break;
    case Err::NoteOrRestIsNotSelected:
        text = qtrc("notation", "No note or rest selected: Please select a note or rest and retry");
        break;
    case Err::NoteOrFiguredBassIsNotSelected:
        text = qtrc("notation", "No note or figured bass selected: Please select a note or figured bass and retry");
        break;
    case Err::MeasureIsNotSelected:
        text = qtrc("notation", "No measure selected: Please select a measure and retry");
        break;
    case Err::UnableToPlaybackElement:
        text = qtrc("notation", "Unable to playback element");
        break;
    case Err::EmptySelection:
        text = qtrc("notation", "The selection is empty");
        break;
    case Err::Undefined:
    case Err::NoError:
    case Err::UnknownError:
    case Err::UserAbort:
    case Err::IgnoreError:
        break;
    }

    return Ret(code, text.toStdString());
}

inline Ret scoreFileErrorToRet(Ms::Score::FileError err, const io::path& filePath)
{
    auto makeRet = [=](Err err) {
        return make_ret(err, filePath);
    };

    switch (err) {
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
