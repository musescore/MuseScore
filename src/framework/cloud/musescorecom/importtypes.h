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
#include <set>
#include <string>
#include <vector>

#include <QDateTime>
#include <QString>
#include <QUrl>

class QIODevice;

namespace muse::cloud {
enum class ImportType {
    Omr,
    Audio2Score
};

enum class ImportStatus {
    Processing,
    AwaitingMeta,
    AwaitingReview,
    Done,
    Failed,
    Unknown
};

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

struct Genre {
    int id = 0;
    QString name;

    bool isValid() const { return id > 0 && !name.isEmpty(); }
};

using GenreList = std::vector<Genre>;
using GenreIdSet = std::set<int>;

struct OmrMeta {
    int id = 0;
    QString title;
    QString songName;
    QString artistName;
    int songId = 0;
    int artistId = 0;
    GenreIdSet genreIds;
    bool isOriginComposition = false;

    bool isValid() const { return id > 0 && !title.isEmpty(); }
};

struct SongAutocompleteItem {
    int songId = 0;
    QString songName;
    int artistId = 0;
    QString artistName;
    bool isPublicDomain = false;
    bool isModerated = false;
    int scoresCount = 0;
    GenreList genres;

    bool isValid() const { return songId > 0 && !songName.isEmpty(); }
};

using SongAutocompleteList = std::vector<SongAutocompleteItem>;

//! NOTE: must be in sync with the musescore.com API
enum class OmrReviewRating {
    Bad = 0,
    Good = 1,
};
}
