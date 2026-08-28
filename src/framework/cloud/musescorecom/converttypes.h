/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include <string>
#include <variant>
#include <vector>

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QUrl>

#include "global/logstream.h"
#include "io/path.h"

namespace muse::cloud {
enum class ConvertType {
    Omr,
    Audio2Score
};

enum class ConvertStatus {
    Processing,
    AwaitingReview,
    Done,
    Failed,
    Unknown
};

inline const char* convertTypeToString(ConvertType type)
{
    switch (type) {
    case ConvertType::Omr: return "Omr";
    case ConvertType::Audio2Score: return "Audio2Score";
    }
    return "Unknown";
}

inline const char* convertStatusToString(ConvertStatus status)
{
    switch (status) {
    case ConvertStatus::Processing: return "Processing";
    case ConvertStatus::AwaitingReview: return "AwaitingReview";
    case ConvertStatus::Done: return "Done";
    case ConvertStatus::Failed: return "Failed";
    case ConvertStatus::Unknown: break;
    }
    return "Unknown";
}

//! NOTE: must be in sync with the musescore.com API's error_code values
enum class ConvertErrorCode {
    Unknown,
    UnsupportedFormat,
    FileTooLarge,
    TooManyFiles,
    RateLimited,
    MsczNotReady,
    MetaLocked,
    NoNeedReview,
    SearchRequired,
    InvalidInput,
    InvalidFileType,
    InvalidFormat,
    FileProcessingError,
    ModelExecutionError,
    ConversionError,
    ResourceNotFound,
    InternalServerError,
};

//! NOTE: key for ConvertErrorCode stored in Ret::data
static const std::string CONVERT_ERROR_CODE_KEY("errorCode");

static const qint64 MAX_CONVERT_FILE_SIZE_BYTES = 1024LL * 1024 * 1024; // 1 GB

struct OmrConfig {
    qint64 maxFileSizeBytes = 0;
    int maxPages = 0;
    int maxImages = 0;
    QStringList allowedExtensions;
};

struct Audio2ScoreConfig {
    qint64 maxFileSizeBytes = 0;
    int maxFiles = 0;
    QStringList allowedExtensions;
    int maxLinkLength = 0;
    QStringList allowedLinkSources;
};

struct ConvertConfig {
    OmrConfig omr;
    Audio2ScoreConfig audio2score;
};

struct OmrConvertInput {
    muse::io::paths_t paths;
};

struct Audio2ScoreConvertInput {
    std::variant<muse::io::paths_t, QString> data; // paths or link
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

inline QString convertLinkOf(const ConvertInput& input)
{
    const Audio2ScoreConvertInput* a2s = std::get_if<Audio2ScoreConvertInput>(&input);
    if (!a2s) {
        return QString();
    }

    const QString* link = std::get_if<QString>(&a2s->data);
    return link ? *link : QString();
}

struct ConvertResult {
    int id = 0;
    ConvertType type = ConvertType::Omr;
    ConvertStatus status = ConvertStatus::Processing;

    bool isValid() const { return id > 0; }
};

struct ConvertQueueItem {
    int id = 0;
    ConvertType type = ConvertType::Omr;
    ConvertStatus status = ConvertStatus::Processing;
    QString filename;
    QString link; //! audio2score only
    int scoreId = 0;
    QDateTime createdAt;
    QDateTime updatedAt;
    ConvertErrorCode errorCode = ConvertErrorCode::Unknown;

    bool isValid() const { return id > 0; }
};

using ConvertQueueList = std::vector<ConvertQueueItem>;

struct SignedMsczUrl {
    int id = 0;
    ConvertType type = ConvertType::Omr;
    QUrl url;
    int expiresInSeconds = 0;

    bool isValid() const { return id > 0 && url.isValid(); }
};

//! NOTE: must be in sync with the musescore.com API
enum class ReviewRating {
    Bad = 0,
    Good = 1,
};
}

inline muse::logger::Stream& operator<<(muse::logger::Stream& s, const muse::cloud::ConvertQueueItem& item)
{
    s << "id: " << item.id
      << ", filename: \"" << item.filename << "\""
      << ", link: \"" << item.link << "\""
      << ", type: " << muse::cloud::convertTypeToString(item.type)
      << ", status: " << muse::cloud::convertStatusToString(item.status)
      << ", scoreId: " << item.scoreId
      << ", createdAt: " << item.createdAt.toString(Qt::ISODate)
      << ", updatedAt: " << item.updatedAt.toString(Qt::ISODate);
    return s;
}
