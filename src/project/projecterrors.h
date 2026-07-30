/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#ifndef MU_PROJECT_PROJECTERRORS_H
#define MU_PROJECT_PROJECTERRORS_H

#include "types/ret.h"

namespace mu::project {
enum class Err {
    Undefined       = int(muse::Ret::Code::Undefined),
    NoError         = int(muse::Ret::Code::Ok),
    UnknownError    = int(muse::Ret::Code::ProjectFirst),

    NoProjectError,
    NoPartsError,
    CorruptionError,
    CorruptionUponOpenningError,
    CorruptionUponSavingError,

    FileOpenError,
    FileCreateError,
    InvalidCloudScoreId,

    UnsupportedUrl,
    MalformedOpenScoreUrl,

    ImportValidationFailed,
    ImportProcessingFailed,
    DownloadLinkExpired,
};

//! NOTE: key for the list of files stored in Ret::data
static const std::string IMPORT_FAILED_FILES_KEY("failedFiles");

inline muse::Ret make_ret(Err e)
{
    return muse::Ret(static_cast<int>(e));
}

inline muse::Ret make_ret(Err e, const std::string& text)
{
    return muse::Ret(static_cast<int>(e), text);
}
}

#endif // MU_PROJECT_PROJECTERRORS_H
