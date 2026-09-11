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

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QUrl>

#include "global/types/bytearray.h"
#include "global/types/flags.h"
#include "global/io/path.h"
#include "global/logstream.h"

namespace muse::cloud {
enum class ConvertType {
    Omr = 0,
    Audio2Score,

    Last = Audio2Score
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
    FileOrLinkRequired,
    InvalidLink,
    RateLimited,
    MsczNotReady,
    NoNeedReview,
    ReviewRequired,
    CommentRequired,
    VariousFileIssues,
    TooComplex,
    DontRecognizeNotes,
    GeneralFailure,
    BadParams,
};

//! NOTE: key for ConvertErrorCode stored in Ret::data
static const std::string CONVERT_ERROR_CODE_KEY("errorCode");

enum class LinkSource {
    NoSources = 0x0,
    YouTube = 0x1,
    AudioCom = 0x2
};
DECLARE_FLAGS(LinkSources, LinkSource)
DECLARE_OPERATORS_FOR_FLAGS(LinkSources)

struct OmrPdfConfig {
    qint64 maxFileSizeBytes = 0;
    int maxFiles = 0;
    int maxPages = 0;
};

struct OmrImagesConfig {
    qint64 maxFileSizeBytes = 0;
    int maxFiles = 0;
    QStringList allowedExtensions;
};

struct OmrConfig {
    OmrPdfConfig pdf;
    OmrImagesConfig images;
};

struct Audio2ScoreFileConfig {
    qint64 maxFileSizeBytes = 0;
    int maxFiles = 0;
    QStringList allowedExtensions;
};

struct Audio2ScoreLinkConfig {
    int maxLength = 0;
    LinkSources allowedSources;
};

struct Audio2ScoreConfig {
    Audio2ScoreFileConfig file;
    Audio2ScoreLinkConfig link;
};

struct ConvertConfig {
    int version = 0;
    OmrConfig omr;
    Audio2ScoreConfig audio2score;
};

struct ConvertFileData {
    muse::ByteArray data;
    muse::io::path_t fileName; // basename, with extension
};
using ConvertFileDataList = std::vector<ConvertFileData>;

struct ConvertUploadData {
    ConvertType type = ConvertType::Omr;
    ConvertFileDataList files;
    QUrl link; // Audio2Score only
    QString filename; // desired name for the converted score, no extension
};
using ConvertUploadDataPtr = std::shared_ptr<const ConvertUploadData>;

struct ConvertResult {
    int id = 0;
    ConvertType type = ConvertType::Omr;
    ConvertStatus status = ConvertStatus::Unknown;
};

struct ConvertQueueItem {
    int id = 0;
    ConvertType type = ConvertType::Omr;
    ConvertStatus status = ConvertStatus::Unknown;
    QString filename;
    QString link; //! audio2score only
    std::optional<int> scoreId; //! set once the score is ready (AwaitingReview/Done)
    QDateTime createdAt;
    QDateTime updatedAt;
    ConvertErrorCode errorCode = ConvertErrorCode::Unknown;
};

using ConvertQueueList = std::vector<ConvertQueueItem>;

//! NOTE: must be in sync with the musescore.com API
enum class ReviewRating {
    Bad = 0,
    Good = 1,
};
}

inline muse::logger::Stream& operator<<(muse::logger::Stream& s, const muse::cloud::ConvertQueueItem& item)
{
    auto dateTimeToString = [](const QDateTime& dateTime) {
        return dateTime.toSecsSinceEpoch() > 0 ? dateTime.toString(Qt::ISODate) : QString("unknown");
    };

    s << "id: " << item.id
      << ", filename: \"" << item.filename << "\""
      << ", link: \"" << item.link << "\""
      << ", type: " << muse::cloud::convertTypeToString(item.type)
      << ", status: " << muse::cloud::convertStatusToString(item.status)
      << ", scoreId: " << (item.scoreId ? QString::number(*item.scoreId) : QString("none"))
      << ", createdAt: " << dateTimeToString(item.createdAt)
      << ", updatedAt: " << dateTimeToString(item.updatedAt);
    return s;
}
