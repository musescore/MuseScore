/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include "cloud/musescorecom/converttypes.h"
#include "cloud/cloudtypes.h"

#include "filecategory.h"

#include "types/ret.h"
#include "types/secs.h"

namespace mu::project {
using ConvertConfig = muse::cloud::ConvertConfig;
using ConvertType = muse::cloud::ConvertType;
using ConvertInput = muse::cloud::ConvertInput;
using ReviewRating = muse::cloud::ReviewRating;
using LinkSource = muse::cloud::LinkSource;
using LinkSources = muse::cloud::LinkSources;
using ScoreInfo = muse::cloud::ScoreInfo;

struct ConvertFilesValidation {
    ConvertType type = ConvertType::Omr;
    FileCategory category = FileCategory::Unknown;
};

struct PollingFailure {
    muse::Ret ret;
    int attempt = 0;
    int maxAttempts = 0;
    muse::secs_t nextInterval = 0.;
    bool gaveUp = false;
};
}
