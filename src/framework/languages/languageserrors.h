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
#ifndef MUSE_LANGUAGES_LANGUAGESERRORS_H
#define MUSE_LANGUAGES_LANGUAGESERRORS_H

#include "types/ret.h"
#include "translation.h"

namespace muse::languages {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::LanguagesFirst),

    AlreadyUpToDate,
    ErrorParseConfig,
    ErrorDownloadLanguage,
    ErrorWriteLanguage,
    ErrorLanguageNotFound,
    ErrorRemoveLanguageDirectory,
    ErrorAnotherOperationStarted,
};

inline Ret make_ret(Err e)
{
    int retCode = static_cast<int>(e);

    switch (e) {
    case Err::Undefined: return Ret(retCode);
    case Err::NoError: return Ret(retCode);
    case Err::UnknownError: return Ret(retCode);
    case Err::AlreadyUpToDate: return Ret(retCode, muse::trc("languages", "Up to date"));
    case Err::ErrorParseConfig: return Ret(retCode, muse::trc("languages", "Error while parsing response from server"));
    case Err::ErrorDownloadLanguage: return Ret(retCode, muse::trc("languages", "Error while downloading language"));
    case Err::ErrorWriteLanguage: return Ret(retCode, muse::trc("languages", "Error while writing language files"));
    case Err::ErrorLanguageNotFound: return Ret(retCode, muse::trc("languages", "Language not found"));
    case Err::ErrorRemoveLanguageDirectory: return Ret(retCode, muse::trc("languages", "Error while removing language directory"));
    case Err::ErrorAnotherOperationStarted: return Ret(retCode,
                                                       muse::trc("languages",
                                                                 "Another operation on this language has already been started"));
    }

    return retCode;
}
}

#endif // MUSE_LANGUAGES_LANGUAGESERRORS_H
