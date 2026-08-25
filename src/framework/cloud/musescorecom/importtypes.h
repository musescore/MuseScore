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
#include <string>
#include <vector>

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QUrl>

#include "global/logstream.h"

class QIODevice;

namespace muse::cloud {
enum class ImportType {
    Omr,
    Audio2Score
};

enum class ImportStatus {
    Processing,
    AwaitingReview,
    Done,
    Failed,
    Unknown
};

inline const char* importTypeToString(ImportType type)
{
    switch (type) {
    case ImportType::Omr: return "Omr";
    case ImportType::Audio2Score: return "Audio2Score";
    }
    return "Unknown";
}

inline const char* importStatusToString(ImportStatus status)
{
    switch (status) {
    case ImportStatus::Processing: return "Processing";
    case ImportStatus::AwaitingReview: return "AwaitingReview";
    case ImportStatus::Done: return "Done";
    case ImportStatus::Failed: return "Failed";
    case ImportStatus::Unknown: break;
    }
    return "Unknown";
}

//! NOTE: must be in sync with the musescore.com API's error_code values
enum class ImportErrorCode {
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

//! NOTE: key for ImportErrorCode stored in Ret::data
static const std::string IMPORT_ERROR_CODE_KEY("errorCode");

static const qint64 MAX_IMPORT_FILE_SIZE_BYTES = 1024LL * 1024 * 1024; // 1 GB

struct OmrImportConfig {
    qint64 maxFileSizeBytes = 0;
    int maxPages = 0;
    int maxImages = 0;
    QStringList allowedExtensions;
};

struct Audio2ScoreImportConfig {
    qint64 maxFileSizeBytes = 0;
    int maxFiles = 0;
    QStringList allowedExtensions;
};

struct ImportConfig {
    OmrImportConfig omr;
    Audio2ScoreImportConfig audio2score;
};

struct ImportFile {
    std::shared_ptr<QIODevice> data;
    QString fileName;

    bool isValid() const { return data != nullptr && !fileName.isEmpty(); }
};

using ImportFileList = std::vector<ImportFile>;

struct ImportResult {
    int id = 0;
    ImportType type = ImportType::Omr;
    ImportStatus status = ImportStatus::Processing;

    bool isValid() const { return id > 0; }
};

struct ImportQueueItem {
    int id = 0;
    ImportType type = ImportType::Omr;
    ImportStatus status = ImportStatus::Processing;
    QString filename;
    int scoreId = 0;
    QDateTime createdAt;
    QDateTime updatedAt;
    ImportErrorCode errorCode = ImportErrorCode::Unknown;

    bool isValid() const { return id > 0; }
};

using ImportQueueList = std::vector<ImportQueueItem>;

struct SignedMsczUrl {
    int id = 0;
    ImportType type = ImportType::Omr;
    QUrl url;
    int expiresInSeconds = 0;

    bool isValid() const { return id > 0 && url.isValid(); }
};

//! NOTE: must be in sync with the musescore.com API
enum class OmrReviewRating {
    Bad = 0,
    Good = 1,
};
}

inline muse::logger::Stream& operator<<(muse::logger::Stream& s, const muse::cloud::ImportQueueItem& item)
{
    s << "id: " << item.id
      << ", filename: \"" << item.filename << "\""
      << ", type: " << muse::cloud::importTypeToString(item.type)
      << ", status: " << muse::cloud::importStatusToString(item.status)
      << ", scoreId: " << item.scoreId
      << ", createdAt: " << item.createdAt.toString(Qt::ISODate)
      << ", updatedAt: " << item.updatedAt.toString(Qt::ISODate);
    return s;
}
