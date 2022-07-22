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
#ifndef MU_LANGUAGES_LANGUAGESERRORS_H
#define MU_LANGUAGES_LANGUAGESERRORS_H

#include "types/ret.h"
#include "translation.h"

namespace mu::languages {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::LanguagesFirst),

    ErrorParseConfig,
    ErrorDownloadLanguage,
    ErrorLanguageNotFound,
    ErrorRemoveLanguageDirectory,
    ErrorAnotherOperationStarted,

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
    case Err::ErrorParseConfig: return Ret(retCode, trc("languages", "Error while parsing response from server"));
    case Err::ErrorDownloadLanguage: return Ret(retCode, trc("languages", "Error while downloading language"));
    case Err::ErrorLanguageNotFound: return Ret(retCode, trc("languages", "Language not found"));
    case Err::ErrorRemoveLanguageDirectory: return Ret(retCode, trc("languages", "Error while removing language directory"));
    case Err::ErrorAnotherOperationStarted: return Ret(retCode,
                                                       trc("languages", "Another operation on this language has already been started"));
    case Err::UnpackDestinationReadOnly: return Ret(retCode, trc("languages", "Cannot import language on read-only storage"));
    case Err::UnpackNoFreeSpace: return Ret(retCode, trc("languages", "Cannot import language due to lack of free disk space"));
    case Err::UnpackErrorRemovePreviousVersion: return Ret(retCode, trc("languages", "Error while removing previous version of language"));
    case Err::UnpackError: return Ret(retCode, trc("languages", "Error while unpacking language"));
    }

    return retCode;
}
}

#endif // MU_LANGUAGES_LANGUAGESERRORS_H
