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

#include <optional>
#include <variant>
#include <vector>

#include <QUrl>

#include "filecategory.h"

#include "cloud/musescorecom/converttypes.h"
#include "cloud/cloudtypes.h"

#include "global/io/path.h"
#include "global/types/string.h"
#include "global/types/secs.h"
#include "global/types/ret.h"

namespace mu::project {
using ConvertConfig = muse::cloud::ConvertConfig;
using ConvertType = muse::cloud::ConvertType;
using ConvertStatus = muse::cloud::ConvertStatus;
using ReviewRating = muse::cloud::ReviewRating;
using LinkSource = muse::cloud::LinkSource;
using LinkSources = muse::cloud::LinkSources;
using ScoreInfo = muse::cloud::ScoreInfo;

struct OmrConvertInput {
    muse::io::paths_t paths;
};

struct Audio2ScoreConvertInput {
    std::variant<muse::io::paths_t, QUrl> data; // paths or link
};

using ConvertInput = std::variant<OmrConvertInput, Audio2ScoreConvertInput>;

inline ConvertType convertTypeOf(const ConvertInput& input)
{
    return std::holds_alternative<OmrConvertInput>(input) ? ConvertType::Omr : ConvertType::Audio2Score;
}

inline muse::io::paths_t convertPathsOf(const ConvertInput& input)
{
    if (const OmrConvertInput* omr = std::get_if<OmrConvertInput>(&input)) {
        return omr->paths;
    }

    const muse::io::paths_t* paths = std::get_if<muse::io::paths_t>(&std::get<Audio2ScoreConvertInput>(input).data);
    return paths ? *paths : muse::io::paths_t();
}

inline QUrl convertLinkOf(const ConvertInput& input)
{
    const Audio2ScoreConvertInput* a2s = std::get_if<Audio2ScoreConvertInput>(&input);
    if (!a2s) {
        return QUrl();
    }

    const QUrl* link = std::get_if<QUrl>(&a2s->data);
    return link ? *link : QUrl();
}

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

struct WatchedScore {
    int convertId = 0;
    ConvertType convertType = ConvertType::Omr;
    ConvertStatus convertStatus = ConvertStatus::Unknown;
    std::optional<int> scoreId; //! set once the score is ready and reported (Done/AwaitingReview)
    bool startedLocally = false; //! true if started in MuseScore
    muse::String name;

    bool operator==(const WatchedScore& other) const
    {
        return convertId == other.convertId
               && convertType == other.convertType
               && convertStatus == other.convertStatus
               && scoreId == other.scoreId
               && startedLocally == other.startedLocally
               && name == other.name;
    }
};
using WatchedScoreList = std::vector<WatchedScore>;
}
