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
#ifndef MU_LANGUAGES_LANGUAGESERRORS_H
#define MU_LANGUAGES_LANGUAGESERRORS_H

#include "ret.h"
#include "translation.h"

namespace mu {
namespace languages {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::LanguagesFirst),

    ErrorParseConfig,
    ErrorDownloadLanguage,
    ErrorLanguageNotFound,
    ErrorRemoveLanguageDirectory,

    UnpackDestinationReadOnly,
    UnpackNoFreeSpace,
    UnpackErrorRemovePreviousVersion,
    UnpackError
};

inline Ret make_ret(Err e)
{
    int retCode = static_cast<int>(e);

    switch (e) {
    case Err::Undefined: return Ret(retCode);
    case Err::NoError: return Ret(retCode);
    case Err::UnknownError: return Ret(retCode);
    case Err::ErrorParseConfig: return Ret(retCode, trc("languages", "Error parsing response from server"));
    case Err::ErrorDownloadLanguage: return Ret(retCode, trc("languages", "Error download language"));
    case Err::ErrorLanguageNotFound: return Ret(retCode, trc("languages", "Language not found"));
    case Err::ErrorRemoveLanguageDirectory: return Ret(retCode, trc("languages", "Error remove language directory"));
    case Err::UnpackDestinationReadOnly: return Ret(retCode, trc("languages", "Cannot import language on read-only storage"));
    case Err::UnpackNoFreeSpace: return Ret(retCode, trc("languages", "Cannot import language on full storage"));
    case Err::UnpackErrorRemovePreviousVersion: return Ret(retCode, trc("languages", "Error removing previous version"));
    case Err::UnpackError: return Ret(retCode, trc("languages", "Error unpacking language"));
    }

    return retCode;
}
}
}

#endif // MU_LANGUAGES_LANGUAGESERRORS_H
