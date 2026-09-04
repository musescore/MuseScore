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

static std::string convertLogId(int queueId, ConvertType type)
{
    return std::to_string(queueId) + " (type: " + convertTypeToString(type) + ")";
}

static std::string convertLogId(const QString& convertedFileName, int queueId, ConvertType type)
{
    return "\"" + convertedFileName.toStdString() + "\" (conversion " + convertLogId(queueId, type) + ")";
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

Ret ConvertFileToScoreService::startConvert(const ConvertInput& input, const QString& convertedFileName)
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

        const int queueId = res.val.toMap()["id"].toInt();
        watch(queueId, type, convertedFileName);
    });

    return make_ok();
}

async::Channel<Ret, io::path_t> ConvertFileToScoreService::convertFinished() const
{
    return m_convertFinished;
}

async::Channel<ConvertType, int> ConvertFileToScoreService::reviewRequested() const
{
    return m_reviewRequested;
}

void ConvertFileToScoreService::submitReview(ConvertType type, int queueId, ReviewRating rating, const QString& comment)
{
    IF_ASSERT_FAILED(rating == ReviewRating::Bad || comment.isEmpty()) {
        return;
    }

    museScoreComService()->convert()->submitReview(type, queueId, rating, comment)
    .onResolve(this, [type, queueId](const RetVal<ConvertResult>& submitRes) {
        if (!submitRes.ret) {
            LOGE() << "Could not submit the review for conversion " << convertLogId(queueId, type) << ": " << submitRes.ret.toString();
        }
    });
}

void ConvertFileToScoreService::submitReviewComment(ConvertType type, int queueId, const QString& comment)
{
    museScoreComService()->convert()->submitReviewComment(type, queueId, comment)
    .onResolve(this, [type, queueId](const RetVal<ConvertResult>& submitRes) {
        if (!submitRes.ret) {
            LOGE() << "Could not submit the comment for conversion " << convertLogId(queueId, type) << ": " << submitRes.ret.toString();
        }
    });
}

void ConvertFileToScoreService::watch(int queueId, ConvertType type, const QString& convertedFileName)
{
    m_watchedItems.insert_or_assign(queueId, WatchedItem { type, convertedFileName, DownloadStatus::NotStarted });
    saveWatchedItems();

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
            if (isRetryableError(result.ret) && ++m_pollFailureCount < MAX_CONSECUTIVE_POLL_FAILURES) {
                //! NOTE: the first retry is likely just a stale pooled connection the server closed
                //! (e.g. HTTP/2 GOAWAY) - don't back off yet, retry at the normal interval
                if (m_pollFailureCount > 1) {
                    m_pollIntervalMs = std::min(m_pollIntervalMs * 2, MAX_RETRY_INTERVAL_MS);
                    m_timer.setInterval(m_pollIntervalMs);
                }
                LOGW() << "Could not check the conversion status, retrying in " << m_pollIntervalMs / 1000
                       << "s (attempt " << m_pollFailureCount << "/" << MAX_CONSECUTIVE_POLL_FAILURES
                       << "): " << result.ret.toString();
                return;
            }

            giveUpPolling(result.ret);
            return;
        }

        m_pollFailureCount = 0;
        m_pollIntervalMs = MIN_RETRY_INTERVAL_MS;
        m_timer.setInterval(MIN_RETRY_INTERVAL_MS);

        for (auto it = m_watchedItems.begin(); it != m_watchedItems.end();) {
            const int queueId = it->first;
            const ConvertType type = it->second.type;

            auto found = std::find_if(result.val.begin(), result.val.end(), [queueId](const ConvertQueueItem& item) {
                return item.id == queueId;
            });

            if (found != result.val.end()) {
                onStatusChanged(*found);

                //! NOTE: Done doesn't erase the item yet - it stays watched (and persisted)
                //! until its download actually resolves, so a crash mid-download can be resumed
                if (found->status == ConvertStatus::Failed) {
                    it = m_watchedItems.erase(it);
                } else {
                    ++it;
                }
                continue;
            }

            //! NOTE: a finished entity drops out of the queue entirely rather than
            //! reporting a final "done" status, per the OMR/A2S API contract
            ConvertQueueItem doneItem;
            doneItem.id = queueId;
            doneItem.type = type;
            doneItem.status = ConvertStatus::Done;
            onStatusChanged(doneItem);

            ++it;
        }

        saveWatchedItems();
    });
}

void ConvertFileToScoreService::giveUpPolling(const Ret& ret)
{
    LOGE() << "Could not check the conversion status, stopping polling for now, "
           << m_watchedItems.size() << " pending conversion(s) remain watched: " << ret.toString();

    m_timer.stop();
    m_pollFailureCount = 0;
    m_pollIntervalMs = MIN_RETRY_INTERVAL_MS;
    finishConvert(ret);
}

void ConvertFileToScoreService::loadWatchedItems()
{
    TRACEFUNC;

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
    for (size_t i = 0; i < array.size(); ++i) {
        const JsonObject obj = array.at(i).toObject();
        const int queueId = obj.value("id").toInt();
        const ConvertType type = static_cast<ConvertType>(obj.value("type").toInt());
        const QString convertedFileName = QString::fromStdString(obj.value("convertedFileName").toStdString());

        m_watchedItems.insert_or_assign(queueId, WatchedItem { type, convertedFileName, DownloadStatus::NotStarted });
    }

    if (!m_watchedItems.empty()) {
        m_timer.start();
        poll();
    }
}

void ConvertFileToScoreService::saveWatchedItems()
{
    TRACEFUNC;

    JsonArray array;
    for (const auto& pair : m_watchedItems) {
        JsonObject obj;
        obj["id"] = pair.first;
        obj["type"] = static_cast<int>(pair.second.type);

        if (!pair.second.convertedFileName.isEmpty()) {
            obj["convertedFileName"] = pair.second.convertedFileName.toStdString();
        }

        array << obj;
    }

    JsonDocument json(array);
    Ret ret = fileSystem()->writeFile(configuration()->pendingConvertsJsonPath(), json.toJson());
    if (!ret) {
        LOGE() << "Could not save the pending conversions list: " << ret.toString();
    }
}

QString ConvertFileToScoreService::convertedFileNameFor(int queueId) const
{
    auto it = m_watchedItems.find(queueId);
    return it != m_watchedItems.end() ? it->second.convertedFileName : QString();
}

void ConvertFileToScoreService::onStatusChanged(const ConvertQueueItem& item)
{
    if (!shouldHandle(item.id, item.status)) {
        return;
    }

    LOGI() << "Conversion status changed: " << item;

    switch (item.status) {
    case ConvertStatus::Processing:
    case ConvertStatus::Unknown:
        break;
    case ConvertStatus::AwaitingReview:
        //! NOTE: the MSCZ is already available at this point; the review rating doesn't gate the download
        downloadIfNotAlready(item.type, item.id);
        m_reviewRequested.send(item.type, item.id);
        break;
    case ConvertStatus::Done:
        downloadIfNotAlready(item.type, item.id);
        break;
    case ConvertStatus::Failed: {
        const QString convertedFileName = convertedFileNameFor(item.id);

        Ret ret = make_ret(Err::ConvertProcessingFailed);
        ret.setText("Conversion failed for \"" + convertedFileName.toStdString() + "\": " + errorCodeToString(item.errorCode));
        ret.setData(CONVERT_FAILED_FILE_NAME_KEY, convertedFileName);

        LOGE() << ret.toString();

        finishConvert(ret);
        break;
    }
    }
}

bool ConvertFileToScoreService::shouldHandle(int queueId, ConvertStatus status)
{
    auto it = m_watchedItems.find(queueId);
    if (it == m_watchedItems.end()) {
        return true;
    }

    if (it->second.lastHandledStatus == status) {
        return false;
    }

    it->second.lastHandledStatus = status;
    return true;
}

void ConvertFileToScoreService::downloadIfNotAlready(ConvertType type, int queueId)
{
    auto it = m_watchedItems.find(queueId);
    if (it != m_watchedItems.end()) {
        if (it->second.downloadStatus != DownloadStatus::NotStarted) {
            return;
        }
        it->second.downloadStatus = DownloadStatus::Downloading;
    }

    fetchScoreUrlAndDownload(type, queueId);
}

void ConvertFileToScoreService::fetchScoreUrlAndDownload(ConvertType type, int queueId)
{
    museScoreComService()->convert()->fetchMsczUrl(type, queueId)
    .onResolve(this, [this, type, queueId](const RetVal<SignedMsczUrl>& urlInfo) {
        const QString convertedFileName = convertedFileNameFor(queueId);

        if (!urlInfo.ret) {
            if (isRetryableError(urlInfo.ret)) {
                LOGW() << "Could not fetch the converted score " << convertLogId(convertedFileName, queueId, type)
                       << ", will retry on next poll: " << urlInfo.ret.toString();
                clearDownloading(queueId);
                return;
            }

            Ret ret = urlInfo.ret;
            ret.setText("Could not fetch the converted score: " + ret.text());
            failConvert(ret, type, queueId, convertedFileName);
            return;
        }

        if (urlInfo.val.expiresInSeconds <= 0) {
            Ret ret = make_ret(Err::DownloadLinkExpired, std::string("The download link has already expired"));
            ret.setData(CONVERT_FAILED_FILE_NAME_KEY, convertedFileName);
            LOGW() << "Could not download the converted score " << convertLogId(convertedFileName, queueId, type)
                   << ": " << ret.toString();
            m_watchedItems.erase(queueId);
            saveWatchedItems();
            finishConvert(ret);
            return;
        }

        downloadScoreAndFinish(type, queueId, urlInfo.val);
    });
}

void ConvertFileToScoreService::downloadScoreAndFinish(ConvertType type, int queueId, const SignedMsczUrl& urlInfo)
{
    const io::path_t dir = configuration()->convertedScoresPath();
    Ret ret = fileSystem()->makePath(dir);
    if (!ret) {
        ret.setText("Could not create the directory for the converted score: " + ret.text());
        failConvert(ret, type, queueId, convertedFileNameFor(queueId));
        return;
    }

    auto scoreData = std::make_shared<QBuffer>();
    ProgressPtr progress = museScoreComService()->convert()->downloadConvertedScore(urlInfo, scoreData);

    progress->finished().onReceive(this, [this, type, queueId, dir, scoreData](const ProgressResult& res) {
        const QString convertedFileName = convertedFileNameFor(queueId);

        if (!res.ret) {
            if (isRetryableError(res.ret)) {
                LOGW() << "Could not download the converted score " << convertLogId(convertedFileName, queueId, type)
                       << ", will retry on next poll: " << res.ret.toString();
                clearDownloading(queueId);
                return;
            }

            Ret ret = res.ret;
            ret.setText("Could not download the converted score: " + ret.text());
            failConvert(ret, type, queueId, convertedFileName);
            return;
        }

        const io::path_t baseName = io::escapeFileName(io::path_t(convertedFileName));
        const std::string addition = configuration()->uniqueFileNameAddition(baseName, dir, "mscz");
        const io::path_t path = dir.appendingComponent(baseName + addition).appendingSuffix("mscz");
        const QByteArray data = scoreData->data();
        const ByteArray byteArray = ByteArray::fromQByteArrayNoCopy(data);

        Ret ret = fileSystem()->writeFile(path, byteArray);
        if (!ret) {
            ret.setText("Could not save the converted score: " + ret.text());
            failConvert(ret, type, queueId, convertedFileName);
            return;
        }

        markDownloaded(queueId);
        finishConvert(make_ok(), path);
    });
}

void ConvertFileToScoreService::markDownloaded(int queueId)
{
    //! NOTE: the download is done, stop watching
    m_watchedItems.erase(queueId);
    saveWatchedItems();
}

void ConvertFileToScoreService::clearDownloading(int queueId)
{
    auto it = m_watchedItems.find(queueId);
    if (it != m_watchedItems.end() && it->second.downloadStatus == DownloadStatus::Downloading) {
        it->second.downloadStatus = DownloadStatus::NotStarted;
        it->second.lastHandledStatus = ConvertStatus::Unknown;
    }
}

void ConvertFileToScoreService::finishConvert(const Ret& ret, const io::path_t& path)
{
    m_convertFinished.send(ret, path);
}

void ConvertFileToScoreService::failConvert(Ret ret, ConvertType type, int queueId, const QString& convertedFileName)
{
    LOGE() << ret.toString() << " " << convertLogId(convertedFileName, queueId, type);
    ret.setData(CONVERT_FAILED_FILE_NAME_KEY, convertedFileName);
    clearDownloading(queueId);
    finishConvert(ret);
}
