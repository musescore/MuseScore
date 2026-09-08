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
#include <optional>

#include <QBuffer>
#include <QUrl>

#include "project/types/filecategory.h"
#include "project/projecterrors.h"

#include "network/networkerrors.h"
#include "cloud/clouderrors.h"

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
    }
    return std::string();
}

static std::string convertLogId(ConvertType type, int itemId)
{
    return std::to_string(itemId) + " (type: " + convertTypeToString(type) + ")";
}

static std::string convertLogId(const muse::String& convertedFileName, ConvertType type, int itemId)
{
    return "\"" + convertedFileName.toStdString() + "\" (conversion " + convertLogId(type, itemId) + ")";
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
}

void ConvertFileToScoreService::resumeConvert()
{
    loadWatchedItems();

    if (m_watchedItems.empty()) {
        return;
    }

    LOGI() << "Resuming " << m_watchedItems.size() << " pending conversion(s)";

    m_timer.start();
    m_fileNamesBeingConvertedChanged.notify();

    for (const WatchedItem& item : m_watchedItems) {
        if (item.convertStatus == ConvertStatus::AwaitingReview && !item.downloadedScorePath.empty()) {
            m_reviewRequested.send(item.type, item.id, item.downloadedScorePath);
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

Ret ConvertFileToScoreService::startConvert(const ConvertInput& input, const muse::String& convertedFileName)
{
    IF_ASSERT_FAILED(!convertPathsOf(input).empty() || !convertLinkOf(input).isEmpty()) {
        return make_ret(Err::ConvertValidationFailed);
    }

    IF_ASSERT_FAILED(io::isAllowedFileName(io::path_t(convertedFileName))) {
        return make_ret(Err::ConvertValidationFailed);
    }

    const ConvertType type = convertTypeOf(input);
    ProgressPtr progress = museScoreComService()->convert()->upload(input);

    progress->progressChanged().onReceive(this, [convertedFileName](int64_t current, int64_t total, const std::string&) {
        LOGI() << "Uploading for convert \"" << convertedFileName << "\": " << current << "/" << total;
    });

    progress->finished().onReceive(this, [this, type, convertedFileName](const ProgressResult& res) {
        if (!res.ret) {
            LOGE() << "Could not upload files for \"" << convertedFileName << "\" (type: "
                   << convertTypeToString(type) << "): " << res.ret.toString();
            Ret ret = res.ret;
            ret.setData(CONVERT_FAILED_FILE_NAME_KEY, convertedFileName);
            finishConvert(ret);
            return;
        }

        const int itemId = res.val.toMap()["id"].toInt();
        watch(type, itemId, convertedFileName);
    });

    return make_ok();
}

async::Channel<Ret, io::path_t> ConvertFileToScoreService::convertFinished() const
{
    return m_convertFinished;
}

bool ConvertFileToScoreService::isPending(const WatchedItem& item)
{
    //! NOTE: an AwaitingReview item that's already downloaded is just waiting on the user
    //! to submit a review - not "being converted" anymore
    return item.convertStatus != ConvertStatus::AwaitingReview || item.downloadedScorePath.empty();
}

muse::StringList ConvertFileToScoreService::fileNamesBeingConverted() const
{
    muse::StringList result;
    result.reserve(m_watchedItems.size());

    for (const WatchedItem& item : m_watchedItems) {
        if (isPending(item)) {
            result.push_back(item.convertedFileName);
        }
    }

    return result;
}

async::Notification ConvertFileToScoreService::fileNamesBeingConvertedChanged() const
{
    return m_fileNamesBeingConvertedChanged;
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

async::Channel<ConvertType, int, io::path_t> ConvertFileToScoreService::reviewRequested() const
{
    return m_reviewRequested;
}

void ConvertFileToScoreService::submitReview(ConvertType type, int itemId, ReviewRating rating, const QString& comment)
{
    IF_ASSERT_FAILED(rating == ReviewRating::Bad || comment.isEmpty()) {
        return;
    }

    museScoreComService()->convert()->submitReview(type, itemId, rating, comment)
    .onResolve(this, [type, itemId](const RetVal<ConvertResult>& submitRes) {
        if (!submitRes.ret) {
            LOGE() << "Could not submit the review for conversion " << convertLogId(type, itemId) << ": " << submitRes.ret.toString();
        }
    });
}

void ConvertFileToScoreService::submitReviewComment(ConvertType type, int itemId, const QString& comment)
{
    museScoreComService()->convert()->submitReviewComment(type, itemId, comment)
    .onResolve(this, [type, itemId](const RetVal<ConvertResult>& submitRes) {
        if (!submitRes.ret) {
            LOGE() << "Could not submit the comment for conversion " << convertLogId(type, itemId) << ": " << submitRes.ret.toString();
        }
    });
}

void ConvertFileToScoreService::watch(ConvertType type, int itemId, const muse::String& convertedFileName)
{
    LOGI() << "Watching conversion " << convertLogId(convertedFileName, type, itemId);

    m_watchedItems.push_back(WatchedItem { itemId, type, convertedFileName });
    saveWatchedItems();
    m_fileNamesBeingConvertedChanged.notify();

    if (!m_timer.isActive()) {
        m_timer.start();
    }

    poll();
}

void ConvertFileToScoreService::poll()
{
    if (m_watchedItems.empty()) {
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
        updateWatchedItems(result.val);
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
           << m_watchedItems.size() << " pending conversion(s) remain watched: " << ret.toString();

    const int count = m_pollFailureCount;

    m_timer.stop();
    resetPollState();

    m_pollingFailed.send(PollingFailure { ret, count, MAX_POLL_RETRY_ATTEMPTS, secs_t(0), true });
}

void ConvertFileToScoreService::updateWatchedItems(const ConvertQueueList& queue)
{
    const std::vector<WatchedItem> previousWatchedItems = m_watchedItems;

    for (auto it = m_watchedItems.begin(); it != m_watchedItems.end();) {
        WatchedItem& item = *it;

        auto found = std::find_if(queue.begin(), queue.end(), [&item](const ConvertQueueItem& queueItem) {
            return queueItem.id == item.id && queueItem.type == item.type;
        });

        //! NOTE: normally a Done item still reports its status while queued, but it may
        //! be dropped from the queue automatically at some point afterwards - treat that as Done too
        const ConvertStatus status = found != queue.end() ? found->status : ConvertStatus::Done;
        const ConvertErrorCode errorCode = found != queue.end() ? found->errorCode : ConvertErrorCode::Unknown;

        handleItem(item, status, errorCode);

        if (status == ConvertStatus::Failed
            || (status == ConvertStatus::Done && !item.downloadedScorePath.empty())) {
            it = m_watchedItems.erase(it);
        } else {
            ++it;
        }
    }

    if (m_watchedItems != previousWatchedItems) {
        saveWatchedItems();
        m_fileNamesBeingConvertedChanged.notify();
    }
}

void ConvertFileToScoreService::loadWatchedItems()
{
    TRACEFUNC;

    m_watchedItems.clear();

    RetVal<ByteArray> data = fileSystem()->readFile(configuration()->pendingConvertsJsonPath());
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
    m_watchedItems.reserve(array.size());

    for (size_t i = 0; i < array.size(); ++i) {
        const JsonObject obj = array.at(i).toObject();
        const int itemId = obj.value("id").toInt();
        const ConvertType type = static_cast<ConvertType>(obj.value("type").toInt());
        const muse::String convertedFileName = muse::String::fromStdString(obj.value("convertedFileName").toStdString());
        const ConvertStatus convertStatus = static_cast<ConvertStatus>(obj.value("convertStatus").toInt());
        const io::path_t downloadedScorePath = obj.value("downloadedScorePath").toStdString();

        m_watchedItems.push_back(WatchedItem { itemId, type, convertedFileName, convertStatus, false, downloadedScorePath });
    }
}

void ConvertFileToScoreService::saveWatchedItems()
{
    TRACEFUNC;

    JsonArray array;
    for (const WatchedItem& item : m_watchedItems) {
        JsonObject obj;
        obj["id"] = item.id;
        obj["type"] = static_cast<int>(item.type);
        obj["convertStatus"] = static_cast<int>(item.convertStatus);

        if (!item.convertedFileName.isEmpty()) {
            obj["convertedFileName"] = item.convertedFileName.toStdString();
        }

        if (!item.downloadedScorePath.empty()) {
            obj["downloadedScorePath"] = item.downloadedScorePath.toStdString();
        }

        array << obj;
    }

    JsonDocument json(array);
    Ret ret = fileSystem()->writeFile(configuration()->pendingConvertsJsonPath(), json.toJson());
    if (!ret) {
        LOGE() << "Could not save the pending conversions list: " << ret.toString();
    }
}

std::vector<ConvertFileToScoreService::WatchedItem>::iterator ConvertFileToScoreService::findWatchedItem(ConvertType type, int itemId)
{
    return std::find_if(m_watchedItems.begin(), m_watchedItems.end(), [type, itemId](const WatchedItem& item) {
        return item.type == type && item.id == itemId;
    });
}

void ConvertFileToScoreService::eraseWatchedItem(ConvertType type, int itemId)
{
    auto it = findWatchedItem(type, itemId);
    if (it != m_watchedItems.end()) {
        m_watchedItems.erase(it);
    }
}

void ConvertFileToScoreService::handleItem(WatchedItem& item, ConvertStatus status, ConvertErrorCode errorCode)
{
    const bool statusChanged = item.convertStatus != status;
    if (statusChanged) {
        LOGI() << "Conversion status changed: " << convertLogId(item.convertedFileName, item.type, item.id)
               << " -> " << convertStatusToString(status);
    }
    item.convertStatus = status;

    switch (status) {
    case ConvertStatus::Processing:
    case ConvertStatus::Unknown:
        break;
    case ConvertStatus::AwaitingReview:
        //! NOTE: the MSCZ is already available at this point; the review rating doesn't gate the download
        downloadIfNotAlready(item);
        break;
    case ConvertStatus::Done:
        downloadIfNotAlready(item);
        break;
    case ConvertStatus::Failed: {
        if (!statusChanged) {
            return;
        }

        Ret ret = make_ret(Err::ConvertProcessingFailed);
        ret.setText("Conversion failed for \"" + item.convertedFileName.toStdString() + "\": " + errorCodeToString(errorCode));
        ret.setData(CONVERT_FAILED_FILE_NAME_KEY, item.convertedFileName);

        LOGE() << ret.toString();

        finishConvert(ret);
        break;
    }
    }
}

void ConvertFileToScoreService::downloadIfNotAlready(WatchedItem& item)
{
    if (item.isDownloading || !item.downloadedScorePath.empty()) {
        return;
    }

    item.isDownloading = true;
    fetchScoreUrlAndDownload(item.type, item.id, item.convertedFileName);
}

void ConvertFileToScoreService::fetchScoreUrlAndDownload(ConvertType type, int itemId, const muse::String& convertedFileName)
{
    museScoreComService()->convert()->fetchMsczUrl(type, itemId)
    .onResolve(this, [this, type, itemId, convertedFileName](const RetVal<SignedMsczUrl>& urlInfo) {
        if (!urlInfo.ret) {
            if (isRetryableError(urlInfo.ret)) {
                LOGW() << "Could not fetch the converted score " << convertLogId(convertedFileName, type, itemId)
                       << ", will retry on next poll: " << urlInfo.ret.toString();
                clearDownloading(type, itemId);
                return;
            }

            Ret ret = urlInfo.ret;
            ret.setText("Could not fetch the converted score: " + ret.text());
            failConvert(ret, type, itemId, convertedFileName);
            return;
        }

        if (urlInfo.val.expiresInSeconds <= 0) {
            Ret ret = make_ret(Err::DownloadLinkExpired, std::string("The download link has already expired"));
            ret.setData(CONVERT_FAILED_FILE_NAME_KEY, convertedFileName);
            LOGW() << "Could not download the converted score " << convertLogId(convertedFileName, type, itemId)
                   << ": " << ret.toString();
            eraseWatchedItem(type, itemId);
            saveWatchedItems();
            m_fileNamesBeingConvertedChanged.notify();
            finishConvert(ret);
            return;
        }

        downloadScoreAndFinish(type, itemId, convertedFileName, urlInfo.val);
    });
}

void ConvertFileToScoreService::downloadScoreAndFinish(ConvertType type, int itemId, const muse::String& convertedFileName,
                                                       const SignedMsczUrl& urlInfo)
{
    auto scoreData = std::make_shared<QBuffer>();
    ProgressPtr progress = museScoreComService()->convert()->downloadConvertedScore(urlInfo, scoreData);

    progress->finished().onReceive(this, [this, type, itemId, convertedFileName, scoreData](const ProgressResult& res) {
        if (!res.ret) {
            if (isRetryableError(res.ret)) {
                LOGW() << "Could not download the converted score " << convertLogId(convertedFileName, type, itemId)
                       << ", will retry on next poll: " << res.ret.toString();
                clearDownloading(type, itemId);
                return;
            }

            Ret ret = res.ret;
            ret.setText("Could not download the converted score: " + ret.text());
            failConvert(ret, type, itemId, convertedFileName);
            return;
        }

        if (findWatchedItem(type, itemId) == m_watchedItems.end()) {
            //! NOTE: the item was already removed (e.g. reported as Failed) while this download
            //! was in progress - discard the result rather than reporting a contradictory outcome
            LOGW() << "Conversion " << convertLogId(convertedFileName, type, itemId)
                   << " was already removed while its download was in progress, discarding the result";
            return;
        }

        writeConvertedScore(convertedFileName, scoreData, [this, type, itemId, convertedFileName](const RetVal<io::path_t>& writeResult) {
            onWriteFinished(type, itemId, convertedFileName, writeResult);
        });
    });
}

void ConvertFileToScoreService::writeConvertedScore(const muse::String& convertedFileName, const std::shared_ptr<QBuffer>& scoreData,
                                                    std::function<void(const RetVal<io::path_t>&)> onFinished)
{
    const io::path_t dir = configuration()->convertedScoresPath();

    makePathWithRetry(dir, 0, [this, dir, convertedFileName, scoreData, onFinished](const Ret& ret) {
        if (!ret) {
            onFinished(RetVal<io::path_t>::make_ret(ret));
            return;
        }

        const io::path_t baseName = io::escapeFileName(io::path_t(convertedFileName));
        const std::string addition = configuration()->uniqueFileNameAddition(baseName, dir, "mscz");
        const io::path_t path = dir.appendingComponent(baseName + addition).appendingSuffix("mscz");

        writeFileWithRetry(path, scoreData, 0, [onFinished, path](const Ret& ret) {
            onFinished(ret ? RetVal<io::path_t>::make_ok(path) : RetVal<io::path_t>::make_ret(ret));
        });
    });
}

void ConvertFileToScoreService::onWriteFinished(ConvertType type, int itemId, const muse::String& convertedFileName,
                                                const RetVal<io::path_t>& writeResult)
{
    if (!writeResult.ret) {
        failConvert(writeResult.ret, type, itemId, convertedFileName);
        return;
    }

    //! NOTE: re-lookup rather than reusing an iterator from before the (possibly retried) write -
    //! the vector may have changed while the write was in progress
    auto watched = findWatchedItem(type, itemId);
    if (watched == m_watchedItems.end()) {
        LOGW() << "Conversion " << convertLogId(convertedFileName, type, itemId)
               << " was already removed while its write was in progress, discarding the result";
        return;
    }

    watched->isDownloading = false;
    watched->downloadedScorePath = writeResult.val;
    const bool requestReview = watched->convertStatus == ConvertStatus::AwaitingReview;

    //! NOTE: keep watching an AwaitingReview item even after it's downloaded, so the
    //! review can be re-requested on resume if the app closes before it's submitted
    if (watched->convertStatus == ConvertStatus::Done) {
        m_watchedItems.erase(watched);
    }

    saveWatchedItems();

    m_fileNamesBeingConvertedChanged.notify();
    finishConvert(make_ok(), writeResult.val);

    if (requestReview) {
        m_reviewRequested.send(type, itemId, writeResult.val);
    }
}

void ConvertFileToScoreService::makePathWithRetry(const io::path_t& dir, int attempt, std::function<void(const Ret&)> onFinished)
{
    Ret ret = fileSystem()->makePath(dir);
    if (ret) {
        onFinished(ret);
        return;
    }

    if (attempt + 1 == MAX_FS_RETRY_ATTEMPTS) {
        LOGE() << "Could not create the directory for converted scores \"" << dir << "\", giving up: " << ret.toString();
        onFinished(ret);
        return;
    }

    LOGW() << "Could not create the directory for converted scores \"" << dir
           << "\", retrying (attempt " << (attempt + 1) << "/" << MAX_FS_RETRY_ATTEMPTS << "): " << ret.toString();

    QTimer::singleShot(FS_RETRY_INTERVAL_MS, this, [this, dir, attempt, onFinished]() {
        makePathWithRetry(dir, attempt + 1, onFinished);
    });
}

void ConvertFileToScoreService::writeFileWithRetry(const io::path_t& path, const std::shared_ptr<QBuffer>& scoreData, int attempt,
                                                   std::function<void(const Ret&)> onFinished)
{
    //! NOTE: a no-copy view - scoreData is kept alive via capture for as long as retries are needed
    const ByteArray byteArray = ByteArray::fromQByteArrayNoCopy(scoreData->data());

    Ret ret = fileSystem()->writeFile(path, byteArray);
    if (ret) {
        onFinished(ret);
        return;
    }

    if (attempt + 1 == MAX_FS_RETRY_ATTEMPTS) {
        LOGE() << "Could not save the converted score \"" << path << "\", giving up: " << ret.toString();
        onFinished(ret);
        return;
    }

    LOGW() << "Could not save the converted score \"" << path
           << "\", retrying (attempt " << (attempt + 1) << "/" << MAX_FS_RETRY_ATTEMPTS << "): " << ret.toString();

    QTimer::singleShot(FS_RETRY_INTERVAL_MS, this, [this, path, scoreData, attempt, onFinished]() {
        writeFileWithRetry(path, scoreData, attempt + 1, onFinished);
    });
}

void ConvertFileToScoreService::clearDownloading(ConvertType type, int itemId)
{
    auto it = findWatchedItem(type, itemId);
    if (it != m_watchedItems.end()) {
        it->isDownloading = false;
    }
}

void ConvertFileToScoreService::finishConvert(const Ret& ret, const io::path_t& path)
{
    m_convertFinished.send(ret, path);
}

void ConvertFileToScoreService::failConvert(Ret ret, ConvertType type, int itemId, const muse::String& convertedFileName)
{
    LOGE() << ret.toString() << " " << convertLogId(convertedFileName, type, itemId);
    ret.setData(CONVERT_FAILED_FILE_NAME_KEY, convertedFileName);
    clearDownloading(type, itemId);
    finishConvert(ret);
}
