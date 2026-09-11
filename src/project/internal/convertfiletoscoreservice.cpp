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
#include "convertfiletoscoreservice.h"

#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <unordered_map>

#include <QUrl>

#include "project/types/filecategory.h"
#include "project/projecterrors.h"

#include "network/networkerrors.h"
#include "cloud/clouderrors.h"
#include "multiwindows/resourcelockguard.h"

#include "global/serialization/json.h"
#include "global/types/bytearray.h"
#include "global/io/path.h"
#include "global/io/ioretcodes.h"
#include "global/log.h"

using namespace mu::project;
using namespace muse;
using namespace muse::cloud;

static bool isRetryableError(const Ret& ret)
{
    switch (static_cast<muse::network::Err>(ret.code())) {
    case muse::network::Err::Timeout:
    case muse::network::Err::NetworkError:
        return true;
    default: break;
    }

    switch (static_cast<muse::cloud::Err>(ret.code())) {
    case muse::cloud::Err::NetworkError:
    case muse::cloud::Err::Status429_RateLimitExceeded:
    case muse::cloud::Err::Status500_InternalServerError:
    case muse::cloud::Err::UnknownStatusCode:
        return true;
    default: break;
    }

    return false;
}

static FileCategory resolveFileCategory(const io::path_t& path, const ConvertConfig& config)
{
    const QString ext = QString::fromStdString(muse::io::suffix(path));
    if (ext == "pdf") {
        return FileCategory::Pdf;
    }

    if (config.omr.images.allowedExtensions.contains(ext)) {
        return FileCategory::Image;
    }

    if (config.audio2score.file.allowedExtensions.contains(ext)) {
        return FileCategory::Audio;
    }

    //! NOTE: config is a client-side sanity check only; if it hasn't been fully fetched yet, fall back
    //! to a best-effort guess rather than blocking the conversion
    if (!config.omr.images.allowedExtensions.isEmpty() && !config.audio2score.file.allowedExtensions.isEmpty()) {
        return FileCategory::Unknown;
    }

    return fileCategoryFromSuffix(ext.toStdString());
}

static std::string errorCodeToString(ConvertErrorCode code)
{
    switch (code) {
    case ConvertErrorCode::Unknown: return "Something went wrong";
    case ConvertErrorCode::UnsupportedFormat: return "This file format is not supported";
    case ConvertErrorCode::FileTooLarge: return "The file is too large";
    case ConvertErrorCode::TooManyFiles: return "Too many files were provided";
    case ConvertErrorCode::FileOrLinkRequired: return "A file or a link is required";
    case ConvertErrorCode::InvalidLink: return "The provided link is invalid";
    case ConvertErrorCode::RateLimited: return "Too many conversion requests, please try again later";
    case ConvertErrorCode::MsczNotReady: return "The score is not ready yet";
    case ConvertErrorCode::NoNeedReview: return "This score does not require a review";
    case ConvertErrorCode::ReviewRequired: return "A review must be submitted first";
    case ConvertErrorCode::CommentRequired: return "A comment is required";
    case ConvertErrorCode::VariousFileIssues: return "Invalid format or file type";
    case ConvertErrorCode::TooComplex: return "The file is too large or there is a problem with access to this file";
    case ConvertErrorCode::DontRecognizeNotes: return "Invalid file, could not recognize notes";
    case ConvertErrorCode::GeneralFailure: return "Something went wrong";
    case ConvertErrorCode::BadParams: return "Invalid conversion parameters";
    }
    return std::string();
}

static const std::string WATCHED_CONVERTS_RESOURCE_NAME("WATCHED_CONVERTS");

static std::string convertLogId(ConvertType type, int itemId)
{
    return std::to_string(itemId) + " (type: " + convertTypeToString(type) + ")";
}

static std::string convertLogId(const muse::String& convertedScoreName, ConvertType type, int itemId)
{
    return "\"" + convertedScoreName.toStdString() + "\" (conversion " + convertLogId(type, itemId) + ")";
}

void ConvertFileToScoreService::init()
{
    TRACEFUNC;

    m_timer.setInterval(MIN_RETRY_INTERVAL_MS); // poll once a minute

    QObject::connect(&m_timer, &QTimer::timeout, [this]() { poll(); });

    //! NOTE: fallback is used if fetchConfig() fails
    m_config.omr.pdf.maxFileSizeBytes = 78643200;
    m_config.omr.pdf.maxFiles = 1;
    m_config.omr.pdf.maxPages = 50;
    m_config.omr.images.allowedExtensions = { "jpeg", "jpg", "png" };
    m_config.omr.images.maxFileSizeBytes = 78643200;
    m_config.omr.images.maxFiles = 15;
    m_config.audio2score.file.allowedExtensions = { "mp3" };
    m_config.audio2score.file.maxFileSizeBytes = 52428800;
    m_config.audio2score.file.maxFiles = 1;
    m_config.audio2score.link.maxLength = 2048;
    m_config.audio2score.link.allowedSources = LinkSource::YouTube | LinkSource::AudioCom;

    //! NOTE: prefetch and cache convert config
    museScoreComService()->convert()->fetchConfig().onResolve(this, [this](const RetVal<ConvertConfig>& config) {
        if (!config.ret) {
            LOGE() << "Could not prefetch convert config: " << config.ret.toString();
        } else {
            m_config = config.val;
        }
    });

    multiwindowsProvider()->resourceChanged().onReceive(this, [this](const std::string& resourceName) {
        if (resourceName == WATCHED_CONVERTS_RESOURCE_NAME && !m_isSaving) {
            loadWatchedScores();

            if (!m_watchedScores.empty() && !m_timer.isActive()) {
                m_timer.start();
            }

            m_watchedScoresChanged.notify();
        }
    });
}

void ConvertFileToScoreService::resumeConvert()
{
    loadWatchedScores();

    if (m_watchedScores.empty()) {
        return;
    }

    LOGI() << "Resuming watching " << m_watchedScores.size() << " pending conversion(s)";

    m_timer.start();
    m_watchedScoresChanged.notify();

    for (const WatchedScore& watched : m_watchedScores) {
        if (watched.conversion.status == ConvertStatus::AwaitingReview && watched.scoreId) {
            m_reviewRequested.send(*watched.scoreId);
        }
    }

    poll();
}

const ConvertConfig& ConvertFileToScoreService::config() const
{
    return m_config;
}

bool ConvertFileToScoreService::isFileSupported(const io::path_t& path) const
{
    return resolveFileCategory(path, m_config) != FileCategory::Unknown;
}

RetVal<ConvertFilesValidation> ConvertFileToScoreService::validateFiles(const io::paths_t& paths) const
{
    if (paths.empty()) {
        return RetVal<ConvertFilesValidation>::make_ret(make_ret(Err::ConvertValidationFailed));
    }

    std::optional<FileCategory> firstCategory;
    qint64 totalSizeBytes = 0;

    for (const io::path_t& path : paths) {
        const FileCategory category = resolveFileCategory(path, m_config);
        if (category == FileCategory::Unknown) {
            return RetVal<ConvertFilesValidation>::make_ret(make_ret(Err::ConvertUnsupportedFormat));
        }

        if (!firstCategory) {
            firstCategory = category;
        } else if (category != firstCategory) {
            return RetVal<ConvertFilesValidation>::make_ret(make_ret(Err::ConvertMixedFileTypes));
        }

        const RetVal<uint64_t> fileSizeResult = fileSystem()->fileSize(path);
        if (!fileSizeResult.ret) {
            return RetVal<ConvertFilesValidation>::make_ret(fileSizeResult.ret);
        }

        const qint64 fileSizeBytes = static_cast<qint64>(fileSizeResult.val);

        if (category == FileCategory::Audio
            && m_config.audio2score.file.maxFileSizeBytes > 0 && fileSizeBytes > m_config.audio2score.file.maxFileSizeBytes) {
            return RetVal<ConvertFilesValidation>::make_ret(make_ret(Err::ConvertAudioFileTooLarge));
        }

        totalSizeBytes += fileSizeBytes;
    }

    if (firstCategory == FileCategory::Pdf
        && m_config.omr.pdf.maxFiles > 0 && int(paths.size()) > m_config.omr.pdf.maxFiles) {
        return RetVal<ConvertFilesValidation>::make_ret(make_ret(Err::ConvertMultiplePdfFiles));
    }

    if (firstCategory == FileCategory::Audio) {
        if (m_config.audio2score.file.maxFiles > 0 && int(paths.size()) > m_config.audio2score.file.maxFiles) {
            return RetVal<ConvertFilesValidation>::make_ret(make_ret(Err::ConvertTooManyAudioFiles));
        }

        return RetVal<ConvertFilesValidation>::make_ok(ConvertFilesValidation { ConvertType::Audio2Score, FileCategory::Audio });
    }

    if (m_config.omr.images.maxFiles > 0 && paths.size() > 1 && int(paths.size()) > m_config.omr.images.maxFiles) {
        return RetVal<ConvertFilesValidation>::make_ret(make_ret(Err::ConvertTooManyImages));
    }

    //! NOTE: maxFileSizeBytes is a combined budget across all selected images
    //! (or the single file's own size, for a PDF)
    const qint64 maxFileSizeBytes = firstCategory == FileCategory::Image
                                    ? m_config.omr.images.maxFileSizeBytes
                                    : m_config.omr.pdf.maxFileSizeBytes;
    if (maxFileSizeBytes > 0 && totalSizeBytes > maxFileSizeBytes) {
        if (firstCategory == FileCategory::Image) {
            return RetVal<ConvertFilesValidation>::make_ret(make_ret(Err::ConvertCombinedImageTooLarge));
        }
        return RetVal<ConvertFilesValidation>::make_ret(make_ret(Err::ConvertFileTooLarge));
    }

    return RetVal<ConvertFilesValidation>::make_ok(ConvertFilesValidation { ConvertType::Omr, *firstCategory });
}

Ret ConvertFileToScoreService::validateLink(const QUrl& link) const
{
    const LinkSources sources = m_config.audio2score.link.allowedSources
                                ? m_config.audio2score.link.allowedSources
                                : LinkSource::YouTube | LinkSource::AudioCom;

    if (!link.isValid()) {
        return make_ret(Err::ConvertUnsupportedLink, link.errorString().toStdString());
    }

    const QString host = link.host().toLower();

    if (sources.testFlag(LinkSource::YouTube)
        && (host == "youtube.com" || host.endsWith(".youtube.com") || host == "youtu.be")) {
        return make_ok();
    }

    if (sources.testFlag(LinkSource::AudioCom)
        && (host == "audio.com" || host.endsWith(".audio.com"))) {
        return make_ok();
    }

    return make_ret(Err::ConvertUnsupportedLink);
}

Ret ConvertFileToScoreService::startConvert(const ConvertInput& input, const muse::String& convertedScoreName)
{
    const io::paths_t paths = convertPathsOf(input);
    const QUrl link = convertLinkOf(input);

    IF_ASSERT_FAILED(!paths.empty() || link.isValid()) {
        return make_ret(Err::ConvertValidationFailed);
    }

    IF_ASSERT_FAILED(io::isAllowedFileName(io::path_t(convertedScoreName))) {
        return make_ret(Err::ConvertValidationFailed);
    }

    ConvertFileDataList files;
    files.reserve(paths.size());
    for (const io::path_t& path : paths) {
        RetVal<ByteArray> fileData = fileSystem()->readFile(path);
        if (!fileData.ret) {
            return fileData.ret;
        }

        files.push_back(ConvertFileData { std::move(fileData.val), io::filename(path) });
    }

    const ConvertType type = convertTypeOf(input);
    auto data = std::make_shared<const ConvertUploadData>(ConvertUploadData { type, std::move(files), link,
                                                                              convertedScoreName.toQString() });
    ProgressPtr progress = museScoreComService()->convert()->upload(data);

    progress->progressChanged().onReceive(this, [convertedScoreName](int64_t current, int64_t total, const std::string&) {
        LOGI() << "Uploading for convert \"" << convertedScoreName << "\": " << current << "/" << total;
    });

    progress->finished().onReceive(this, [this, type, convertedScoreName](const ProgressResult& res) {
        if (!res.ret) {
            LOGE() << "Could not upload files for \"" << convertedScoreName << "\" (type: "
                   << convertTypeToString(type) << "): " << res.ret.toString();
            Ret ret = res.ret;
            ret.setData(CONVERT_FAILED_FILE_NAME_KEY, convertedScoreName);
            finishConvert(ret);
            return;
        }

        const int itemId = res.val.toMap()["id"].toInt();
        watch(type, itemId, convertedScoreName);
    });

    return make_ok();
}

async::Channel<Ret, ScoreInfo> ConvertFileToScoreService::convertFinished() const
{
    return m_convertFinished;
}

ValNt<WatchedScoreList> ConvertFileToScoreService::watchedScores() const
{
    ValNt<WatchedScoreList> result;
    result.val = m_watchedScores;
    result.notification = m_watchedScoresChanged;

    return result;
}

async::Channel<PollingFailure> ConvertFileToScoreService::pollingFailed() const
{
    return m_pollingFailed;
}

void ConvertFileToScoreService::retryPolling()
{
    resetPollState();
    m_timer.start();
    poll();
}

async::Channel<int> ConvertFileToScoreService::reviewRequested() const
{
    return m_reviewRequested;
}

void ConvertFileToScoreService::submitReview(int scoreId, ReviewRating rating, const QString& comment)
{
    IF_ASSERT_FAILED(rating == ReviewRating::Bad || comment.isEmpty()) {
        return;
    }

    const WatchedScore* watched = findWatchedScoreByScoreId(scoreId);
    IF_ASSERT_FAILED(watched) {
        return;
    }

    const ConvertType type = watched->conversion.type;
    const int convertId = watched->conversion.id;

    museScoreComService()->convert()->submitReview(type, convertId, rating, comment)
    .onResolve(this, [type, convertId](const RetVal<ConvertResult>& submitRes) {
        if (!submitRes.ret) {
            LOGE() << "Could not submit the review for conversion " << convertLogId(type, convertId) << ": " << submitRes.ret.toString();
        }
    });
}

void ConvertFileToScoreService::submitReviewComment(int scoreId, const QString& comment)
{
    const WatchedScore* watched = findWatchedScoreByScoreId(scoreId);
    IF_ASSERT_FAILED(watched) {
        return;
    }

    const ConvertType type = watched->conversion.type;
    const int convertId = watched->conversion.id;

    museScoreComService()->convert()->submitReviewComment(type, convertId, comment)
    .onResolve(this, [type, convertId](const RetVal<ConvertResult>& submitRes) {
        if (!submitRes.ret) {
            LOGE() << "Could not submit the comment for conversion " << convertLogId(type, convertId) << ": " << submitRes.ret.toString();
        }
    });
}

void ConvertFileToScoreService::deleteConversion(ConvertType type, int convertId)
{
    museScoreComService()->convert()->deleteConversion(type, convertId)
    .onResolve(this, [this, type, convertId](const Ret& ret) {
        if (!ret) {
            LOGE() << "Could not delete conversion " << convertLogId(type, convertId) << ": " << ret.toString();
            return;
        }

        const auto it = std::find_if(m_watchedScores.begin(), m_watchedScores.end(), [type, convertId](const WatchedScore& watched) {
            return watched.conversion.type == type && watched.conversion.id == convertId;
        });

        if (it == m_watchedScores.end()) {
            return;
        }

        m_watchedScores.erase(it);
        saveWatchedScores();
        m_watchedScoresChanged.notify();
    });
}

void ConvertFileToScoreService::loadWatchedScores()
{
    TRACEFUNC;

    m_watchedScores.clear();

    RetVal<ByteArray> data;
    {
        muse::mi::ReadResourceLockGuard resource_guard(multiwindowsProvider(), WATCHED_CONVERTS_RESOURCE_NAME);
        data = fileSystem()->readFile(configuration()->watchedConvertsJsonPath());
    }

    if (!data.ret || data.val.empty()) {
        if (!data.ret && data.ret.code() != static_cast<int>(io::Err::FSNotExist)) {
            LOGE() << "Could not read the pending conversions file: " << data.ret;
        }
        return;
    }

    std::string err;
    const JsonDocument json = JsonDocument::fromJson(data.val, &err);
    if (!err.empty() || !json.isArray()) {
        if (!err.empty()) {
            LOGE() << "Could not parse the pending conversions file: " << err;
        }
        return;
    }

    const JsonArray array = json.rootArray();
    m_watchedScores.reserve(array.size());

    for (size_t i = 0; i < array.size(); ++i) {
        const JsonObject obj = array.at(i).toObject();
        const int typeInt = obj.value("type").toInt();
        if (typeInt < 0 || typeInt > static_cast<int>(ConvertType::Last)) {
            LOGW() << "Skipping conversion with unknown type: " << typeInt;
            continue;
        }

        WatchedScore& watched = m_watchedScores.emplace_back();
        watched.conversion.id = obj.value("id").toInt();
        watched.conversion.type = static_cast<ConvertType>(typeInt);
        watched.conversion.status = static_cast<ConvertStatus>(obj.value("status").toInt());
        watched.scoreId = obj.contains("scoreId") ? std::optional<int>(obj.value("scoreId").toInt()) : std::nullopt;
        watched.startedLocally = obj.value("startedLocally").toBool();
        watched.name = muse::String::fromStdString(obj.value("convertedScoreName").toStdString());
    }
}

void ConvertFileToScoreService::saveWatchedScores()
{
    TRACEFUNC;

    JsonArray array;
    for (const WatchedScore& watched : m_watchedScores) {
        JsonObject obj;
        obj["id"] = watched.conversion.id;
        obj["type"] = static_cast<int>(watched.conversion.type);
        obj["status"] = static_cast<int>(watched.conversion.status);
        obj["startedLocally"] = watched.startedLocally;

        if (!watched.name.isEmpty()) {
            obj["convertedScoreName"] = watched.name.toStdString();
        }

        if (watched.scoreId) {
            obj["scoreId"] = *watched.scoreId;
        }

        array << obj;
    }

    JsonDocument json(array);

    m_isSaving = true;
    {
        muse::mi::WriteResourceLockGuard resource_guard(multiwindowsProvider(), WATCHED_CONVERTS_RESOURCE_NAME);
        Ret ret = fileSystem()->writeFile(configuration()->watchedConvertsJsonPath(), json.toJson());
        if (!ret) {
            LOGE() << "Could not save the pending conversions list: " << ret.toString();
        }
    }
    m_isSaving = false;
}

void ConvertFileToScoreService::watch(ConvertType type, int itemId, const muse::String& convertedScoreName)
{
    LOGI() << "Start watching conversion " << convertLogId(convertedScoreName, type, itemId);

    const auto it = std::find_if(m_watchedScores.begin(), m_watchedScores.end(), [type, itemId](const WatchedScore& watched) {
        return watched.conversion.type == type && watched.conversion.id == itemId;
    });
    WatchedScore& watched = it != m_watchedScores.end() ? *it : m_watchedScores.emplace_back();
    watched.conversion.id = itemId;
    watched.conversion.type = type;
    watched.conversion.status = ConvertStatus::Processing;
    watched.startedLocally = true;
    watched.name = convertedScoreName;

    saveWatchedScores();
    m_watchedScoresChanged.notify();

    if (!m_timer.isActive()) {
        m_timer.start();
    }

    poll();
}

void ConvertFileToScoreService::poll()
{
    if (m_watchedScores.empty()) {
        m_timer.stop();
        return;
    }

    if (m_pollInProgress) {
        return;
    }

    m_pollInProgress = true;

    museScoreComService()->convert()->fetchQueue().onResolve(this, [this](const RetVal<ConvertQueueList>& result) {
        m_pollInProgress = false;

        if (!result.ret) {
            handlePollFailure(result.ret);
            return;
        }

        resetPollState();
        updateWatchedScores(result.val);
    });
}

void ConvertFileToScoreService::resetPollState()
{
    m_pollFailureCount = 0;
    m_pollIntervalMs = MIN_RETRY_INTERVAL_MS;
    m_timer.setInterval(MIN_RETRY_INTERVAL_MS);
}

void ConvertFileToScoreService::handlePollFailure(const Ret& ret)
{
    if (isRetryableError(ret) && ++m_pollFailureCount < MAX_POLL_RETRY_ATTEMPTS) {
        //! NOTE: the first retry is likely just a stale pooled connection the server closed
        //! (e.g. HTTP/2 GOAWAY) - don't back off yet, retry at the normal interval
        if (m_pollFailureCount > 1) {
            m_pollIntervalMs = std::min(m_pollIntervalMs * 2, MAX_RETRY_INTERVAL_MS);
            m_timer.setInterval(m_pollIntervalMs);
        }
        const secs_t intervalSecs(m_pollIntervalMs / 1000.0);
        LOGW() << "Could not check the conversion status, retrying in " << intervalSecs.raw()
               << "s (attempt " << m_pollFailureCount << "/" << MAX_POLL_RETRY_ATTEMPTS
               << "): " << ret.toString();
        m_pollingFailed.send(PollingFailure { ret, m_pollFailureCount, MAX_POLL_RETRY_ATTEMPTS, intervalSecs, false });
        return;
    }

    giveUpPolling(ret);
}

void ConvertFileToScoreService::giveUpPolling(const Ret& ret)
{
    LOGE() << "Could not check the conversion status, stopping polling for now, "
           << m_watchedScores.size() << " pending conversion(s) remain watched: " << ret.toString();

    const int count = m_pollFailureCount;

    m_timer.stop();
    resetPollState();

    m_pollingFailed.send(PollingFailure { ret, count, MAX_POLL_RETRY_ATTEMPTS, secs_t(0), true });
}

void ConvertFileToScoreService::updateWatchedScores(const ConvertQueueList& queue)
{
    TRACEFUNC;

    constexpr size_t CONVERT_TYPE_COUNT = static_cast<size_t>(ConvertType::Last) + 1;
    std::array<std::unordered_map<int /*itemId*/, size_t /*index*/>, CONVERT_TYPE_COUNT> oldByTypeAndId;
    for (size_t i = 0; i < m_watchedScores.size(); ++i) {
        const WatchedScore& watched = m_watchedScores[i];
        oldByTypeAndId[static_cast<size_t>(watched.conversion.type)][watched.conversion.id] = i;
    }

    std::vector<bool> seen(m_watchedScores.size(), false);

    //! NOTE: the queue is the source of truth - rebuild m_watchedScores from it every time,
    //! since it may also contain conversions started outside MuseScore
    std::vector<WatchedScore> newWatchedScores;
    newWatchedScores.reserve(queue.size());

    for (const ConvertQueueItem& queueItem : queue) {
        const std::unordered_map<int, size_t>& oldIndexById = oldByTypeAndId[static_cast<size_t>(queueItem.type)];
        const auto it = oldIndexById.find(queueItem.id);

        if (it != oldIndexById.end()) {
            //! NOTE: already watched - update it (name, status, scoreId)
            seen[it->second] = true;
            WatchedScore watched = m_watchedScores.at(it->second);
            if (!queueItem.filename.isEmpty()) {
                watched.name = queueItem.filename;
            }
            handleItem(watched, queueItem.status, queueItem.errorCode, queueItem.scoreId);

            if (watched.conversion.status != ConvertStatus::Done) {
                newWatchedScores.push_back(watched);
            }
            continue;
        }

        if (queueItem.status != ConvertStatus::Processing && queueItem.status != ConvertStatus::AwaitingReview) {
            //! NOTE: not previously watched, and already terminal - nothing to watch for anymore
            continue;
        }

        LOGI() << "New external conversion: " << convertLogId(queueItem.type, queueItem.id);

        WatchedScore watched;
        watched.conversion.id = queueItem.id;
        watched.conversion.type = queueItem.type;
        watched.name = queueItem.filename;

        handleItem(watched, queueItem.status, queueItem.errorCode, queueItem.scoreId);

        if (watched.conversion.status != ConvertStatus::Done) {
            newWatchedScores.push_back(watched);
        }
    }

    for (size_t i = 0; i < m_watchedScores.size(); ++i) {
        if (seen.at(i)) {
            continue;
        }

        WatchedScore dropped = m_watchedScores.at(i);
        ConvertStatus status = ConvertStatus::Unknown;
        std::optional<int> scoreId;

        if (dropped.scoreId) {
            //! NOTE: a Done/AwaitingReview item still reports its status while queued, but it may be
            //! dropped from the queue automatically at some point afterwards, once already reported ready
            status = ConvertStatus::Done;
            scoreId = dropped.scoreId;
        } else {
            //! NOTE: dropped from the queue before ever reporting a scoreId - there's no way to
            //! identify the resulting score anymore, so it can't be recovered as a success
            status = ConvertStatus::Failed;
        }

        //! NOTE: always terminal, so never added back to newWatchedScores
        handleItem(dropped, status, ConvertErrorCode::Unknown, scoreId);
    }

    if (m_watchedScores != newWatchedScores) {
        m_watchedScores = std::move(newWatchedScores);
        saveWatchedScores();
        m_watchedScoresChanged.notify();
    }
}

void ConvertFileToScoreService::handleItem(WatchedScore& watched, ConvertStatus status, ConvertErrorCode errorCode,
                                           std::optional<int> scoreId)
{
    const ConvertStatus previousStatus = watched.conversion.status;
    const bool statusChanged = previousStatus != status;
    if (statusChanged) {
        LOGI() << "Conversion status changed: " << convertLogId(watched.name, watched.conversion.type, watched.conversion.id)
               << " -> " << convertStatusToString(status);
    }

    switch (status) {
    case ConvertStatus::Processing:
    case ConvertStatus::Unknown:
        break;
    case ConvertStatus::AwaitingReview:
    case ConvertStatus::Done:
        if (!scoreId || watched.scoreId) {
            break;
        }

        if (watched.startedLocally) {
            const RetVal<ScoreInfo> scoreInfo = museScoreComService()->downloadScoreInfo(*scoreId);
            if (!scoreInfo.ret) {
                LOGW() << "Could not fetch score info for " << convertLogId(watched.name, watched.conversion.type, watched.conversion.id)
                       << ", will retry on next poll: " << scoreInfo.ret.toString();
                return; //! NOTE: watched.conversion.status stays at previousStatus - retried next poll
            }

            finishConvert(make_ok(), scoreInfo.val);
        }

        watched.scoreId = *scoreId;

        if (status == ConvertStatus::AwaitingReview) {
            m_reviewRequested.send(*scoreId);
        }
        break;
    case ConvertStatus::Failed: {
        if (!statusChanged) {
            return;
        }

        Ret ret = make_ret(Err::ConvertProcessingFailed);
        ret.setText("Conversion failed for \"" + watched.name.toStdString() + "\": " + errorCodeToString(errorCode));
        ret.setData(CONVERT_FAILED_FILE_NAME_KEY, watched.name);

        LOGE() << ret.toString();

        if (watched.startedLocally) {
            finishConvert(ret);
        }
        break;
    }
    }

    watched.conversion.status = status;
}

void ConvertFileToScoreService::finishConvert(const Ret& ret, const ScoreInfo& scoreInfo)
{
    m_convertFinished.send(ret, scoreInfo);
}

WatchedScore* ConvertFileToScoreService::findWatchedScoreByScoreId(int scoreId)
{
    auto it = std::find_if(m_watchedScores.begin(), m_watchedScores.end(), [scoreId](const WatchedScore& watched) {
        return watched.scoreId == scoreId;
    });

    return it != m_watchedScores.end() ? &*it : nullptr;
}
