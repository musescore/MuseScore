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
#include "convertfiletoscorescenario.h"

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>

#include "project/projecterrors.h"
#include "project/types/projecttypes.h"

#include "global/dataformatter.h"
#include "global/serialization/json.h"
#include "global/io/ioretcodes.h"
#include "global/log.h"

using namespace mu::project;
using namespace muse;
using namespace muse::cloud;

static constexpr int MAX_CONSECUTIVE_POLL_FAILURES = 5;

static std::string errorCodeToString(ConvertErrorCode code)
{
    switch (code) {
    case ConvertErrorCode::Unknown: return "Something went wrong";
    case ConvertErrorCode::UnsupportedFormat: return "This file format is not supported";
    case ConvertErrorCode::FileTooLarge: return "The file is too large";
    case ConvertErrorCode::TooManyFiles: return "Too many files were provided";
    case ConvertErrorCode::RateLimited: return "Too many conversion requests, please try again later";
    case ConvertErrorCode::MsczNotReady: return "The score is not ready yet";
    case ConvertErrorCode::MetaLocked: return "The score information can no longer be changed";
    case ConvertErrorCode::NoNeedReview: return "This score does not require a review";
    case ConvertErrorCode::SearchRequired: return "A song search is required";
    case ConvertErrorCode::InvalidInput: return "The provided input is invalid";
    case ConvertErrorCode::InvalidFileType: return "The file type is invalid";
    case ConvertErrorCode::InvalidFormat: return "The file format is invalid";
    case ConvertErrorCode::FileProcessingError: return "The file could not be processed";
    case ConvertErrorCode::ModelExecutionError: return "The conversion model failed to run";
    case ConvertErrorCode::ConversionError: return "The file could not be converted";
    case ConvertErrorCode::ResourceNotFound: return "The requested resource was not found";
    case ConvertErrorCode::InternalServerError: return "A server error occurred";
    }
    return std::string();
}

static std::string convertLogId(int queueId, ConvertType type)
{
    return std::to_string(queueId) + " (type: " + convertTypeToString(type) + ")";
}

static ConvertType convertTypeFromPath(const io::path_t& path, const ConvertConfig& config)
{
    const QString ext = QFileInfo(path.toQString()).suffix();

    if (!config.audio2score.allowedExtensions.isEmpty()) {
        return config.audio2score.allowedExtensions.contains(ext, Qt::CaseInsensitive)
               ? ConvertType::Audio2Score : ConvertType::Omr;
    }

    //! NOTE: config unavailable, fall back to a best-effort guess
    return isAudioFileSuffix(ext.toLower().toStdString()) ? ConvertType::Audio2Score : ConvertType::Omr;
}

void ConvertFileToScoreScenario::init()
{
    TRACEFUNC;

    m_timer.setInterval(1 * 60000); // poll once a minute

    QObject::connect(&m_timer, &QTimer::timeout, [this]() { poll(); });

    //! NOTE: prefetch and cache convert config
    museScoreComService()->convert()->fetchConvertConfig().onResolve(this, [](const RetVal<ConvertConfig>& config) {
        if (!config.ret) {
            LOGW() << "Could not prefetch convert config: " << config.ret.toString();
        }
    });
}

void ConvertFileToScoreScenario::resumeConvert()
{
    //! NOTE: resuming polling can show dialogs (errors, review prompts), so it must wait
    //! until the main window is up rather than running during onInit()
    loadWatchedItems();
}

async::Promise<ConvertSelection> ConvertFileToScoreScenario::selectFilesToConvert()
{
    return async::Promise<ConvertSelection>([this](auto resolve, auto reject) {
        Ret authRet = ensureAuthorization();
        if (!authRet) {
            return reject(authRet.code(), authRet.text());
        }

        interactive()->open("musescore://project/convert/selectfiles")
        .then<ConvertSelection>(this, [](const Val& val, auto innerResolve) {
            QVariantMap map = val.toQVariant().toMap();

            ConvertSelection selection;
            selection.type = static_cast<ConvertType>(map.value("type").toInt());

            const QStringList paths = map.value("paths").toStringList();
            selection.paths.reserve(paths.size());
            for (const QString& path : paths) {
                selection.paths.push_back(io::path_t(path));
            }

            return innerResolve(selection);
        })
        .onResolve(this, [resolve](const ConvertSelection& selection) {
            (void)resolve(selection);
        })
        .onReject(this, [reject](int code, const std::string& msg) {
            (void)reject(code, msg);
        });

        return async::Promise<ConvertSelection>::dummy_result();
    });
}

async::Promise<RetVal<ConvertType> > ConvertFileToScoreScenario::validateFiles(const io::paths_t& paths)
{
    return async::Promise<RetVal<ConvertType> >([this, paths](auto resolve, auto) {
        if (paths.empty()) {
            return resolve(RetVal<ConvertType>::make_ret(make_ret(Err::ConvertValidationFailed)));
        }

        museScoreComService()->convert()->fetchConvertConfig().onResolve(this, [this, paths, resolve](const RetVal<ConvertConfig>& config) {
            if (!config.ret) {
                LOGW() << "Could not fetch convert config: " << config.ret.toString();
            }

            const ConvertType type = convertTypeFromPath(paths.front(), config.val);

            //! NOTE: config is a client-side sanity check only; if it can't be fetched, fall back
            //! to letting the server enforce its own limits rather than blocking the conversion
            if (config.ret && !validateAgainstConfig(type, paths, config.val)) {
                (void)resolve(RetVal<ConvertType>::make_ret(make_ret(Err::ConvertValidationFailed)));
                return;
            }

            (void)resolve(RetVal<ConvertType>::make_ok(type));
        });

        return async::Promise<RetVal<ConvertType> >::dummy_result();
    });
}

bool ConvertFileToScoreScenario::convertFiles(ConvertType type, const io::paths_t& paths)
{
    if (paths.empty()) {
        return false;
    }

    if (!ensureAuthorization()) {
        return false;
    }

    openFilesAndUpload(type, paths);

    return true;
}

async::Channel<Ret, io::path_t> ConvertFileToScoreScenario::convertFinished() const
{
    return m_convertFinished;
}

Ret ConvertFileToScoreScenario::ensureAuthorization()
{
    std::string dialogText = "Log in or create a free account on MuseScore.com to convert a file.";
    return museScoreComService()->authorization()->ensureAuthorization(false, dialogText).ret;
}

bool ConvertFileToScoreScenario::validateAgainstConfig(ConvertType type, const io::paths_t& paths, const ConvertConfig& config)
{
    for (const io::path_t& path : paths) {
        if (convertTypeFromPath(path, config) != type) {
            showFileValidationError("Conversion failed", "All files must be of the same type");
            return false;
        }
    }

    if (type == ConvertType::Audio2Score) {
        if (config.audio2score.maxFiles > 0 && int(paths.size()) > config.audio2score.maxFiles) {
            std::string text = "Only up to " + std::to_string(config.audio2score.maxFiles)
                               + " audio file(s) can be converted at a time";
            showFileValidationError("Conversion failed", text);
            return false;
        }

        for (const io::path_t& path : paths) {
            QFileInfo info(path.toQString());

            if (!config.audio2score.allowedExtensions.isEmpty()
                && !config.audio2score.allowedExtensions.contains(info.suffix(), Qt::CaseInsensitive)) {
                showUnsupportedFormatError(config.audio2score.allowedExtensions);
                return false;
            }

            if (config.audio2score.maxFileSizeBytes > 0 && info.size() > config.audio2score.maxFileSizeBytes) {
                showFileTooLargeError(config.audio2score.maxFileSizeBytes);
                return false;
            }
        }

        return true;
    }

    if (config.omr.maxImages > 0 && paths.size() > 1 && int(paths.size()) > config.omr.maxImages) {
        std::string text = "Only up to " + std::to_string(config.omr.maxImages)
                           + " image(s) can be converted at a time";
        showFileValidationError("Conversion failed", text);
        return false;
    }

    for (const io::path_t& path : paths) {
        QFileInfo info(path.toQString());

        if (!config.omr.allowedExtensions.isEmpty()
            && !config.omr.allowedExtensions.contains(info.suffix(), Qt::CaseInsensitive)) {
            showUnsupportedFormatError(config.omr.allowedExtensions);
            return false;
        }

        if (config.omr.maxFileSizeBytes > 0 && info.size() > config.omr.maxFileSizeBytes) {
            showFileTooLargeError(config.omr.maxFileSizeBytes);
            return false;
        }
    }

    return true;
}

void ConvertFileToScoreScenario::showFileTooLargeError(qint64 maxFileSizeBytes)
{
    std::string size = DataFormatter::formatFileSize(size_t(maxFileSizeBytes)).toStdString();
    std::string text = "The maximum file size is " + size
                       + ". Reduce the size of your file and try again.";
    showFileValidationError("This file is too large", text);
}

void ConvertFileToScoreScenario::showUnsupportedFormatError(const QStringList& allowedExtensions)
{
    QStringList upperExtensions;
    upperExtensions.reserve(allowedExtensions.size());
    for (const QString& ext : allowedExtensions) {
        upperExtensions << ext.toUpper();
    }

    std::string text = "Make sure the file you're uploading is a " + upperExtensions.join(", ").toStdString() + ".";
    showFileValidationError("This file type is not compatible", text);
}

void ConvertFileToScoreScenario::showFileValidationError(const std::string& title, const std::string& text)
{
    IInteractive::ButtonData guidelinesBtn(IInteractive::Button::CustomButton, "Uploading guidelines");
    IInteractive::ButtonData closeBtn(IInteractive::Button::Close, "Close", true /*accent*/);

    interactive()->error(title, text, { guidelinesBtn, closeBtn }, closeBtn.btn)
    .onResolve(this, [this, guidelinesBtn](const IInteractive::Result& res) {
        if (res.isButton(guidelinesBtn.btn)) {
            interactive()->openUrl(configuration()->scoreUploadingGuidelinesUrl());
        }
    });
}

void ConvertFileToScoreScenario::openFilesAndUpload(ConvertType type, const io::paths_t& paths)
{
    ConvertFileList files;
    files.reserve(paths.size());

    io::paths_t failedFiles;

    for (const io::path_t& path : paths) {
        auto file = std::make_shared<QFile>(path.toQString());
        if (!file->open(QIODevice::ReadOnly)) {
            failedFiles.push_back(path);
            continue;
        }

        ConvertFile convertFile;
        convertFile.data = file;
        convertFile.fileName = io::filename(path).toQString();
        files.push_back(convertFile);
    }

    if (!failedFiles.empty()) {
        IInteractive::Text text;
        text.text = "Could not open the following files";
        text.detailedText = io::pathsToString(failedFiles, "\n");
        interactive()->error("Conversion failed", text);

        Ret ret = make_ret(Err::FileOpenError);
        ret.setData(CONVERT_FAILED_FILES_KEY, failedFiles);
        finishConvert(ret);
        return;
    }

    upload(type, files, paths);
}

void ConvertFileToScoreScenario::upload(ConvertType type, const ConvertFileList& files, const io::paths_t& filePaths)
{
    ProgressPtr progress = museScoreComService()->convert()->uploadConvert(type, files);
    interactive()->showProgress("Uploading…", *progress);

    progress->finished().onReceive(this, [this, type, filePaths](const ProgressResult& res) {
        if (!res.ret) {
            LOGE() << "Could not upload the following files (type: " << convertTypeToString(type) << "): "
                   << io::pathsToString(filePaths, ", ") << ": " << res.ret.toString();
            Ret ret = res.ret;
            ret.setData(CONVERT_FAILED_FILES_KEY, filePaths);
            finishConvert(ret);
            return;
        }

        const int queueId = res.val.toMap()["id"].toInt();
        watch(queueId, type, filePaths);
    });
}

void ConvertFileToScoreScenario::watch(int queueId, ConvertType type, const io::paths_t& filePaths)
{
    m_watchedItems.insert_or_assign(queueId, WatchedItem { type, filePaths, DownloadStatus::NotStarted });
    saveWatchedItems();

    if (!m_timer.isActive()) {
        m_timer.start();
    }

    poll();
}

void ConvertFileToScoreScenario::poll()
{
    if (m_watchedItems.empty()) {
        m_timer.stop();
        return;
    }

    if (m_pollInProgress) {
        return;
    }

    m_pollInProgress = true;

    museScoreComService()->convert()->fetchConvertQueue().onResolve(this, [this](const RetVal<ConvertQueueList>& result) {
        m_pollInProgress = false;

        if (!result.ret) {
            ++m_pollFailureCount;

            if (m_pollFailureCount < MAX_CONSECUTIVE_POLL_FAILURES) {
                return;
            }

            LOGE() << "Could not check the conversion status after " << MAX_CONSECUTIVE_POLL_FAILURES
                   << " attempts, giving up on " << m_watchedItems.size() << " pending conversion(s): " << result.ret.toString();

            io::paths_t filePaths;
            for (const auto& pair : m_watchedItems) {
                filePaths.insert(filePaths.end(), pair.second.filePaths.begin(), pair.second.filePaths.end());
            }

            Ret ret = result.ret;
            ret.setData(CONVERT_FAILED_FILES_KEY, filePaths);

            m_timer.stop();
            m_watchedItems.clear();
            m_pollFailureCount = 0;
            saveWatchedItems();
            finishConvert(ret);
            return;
        }

        m_pollFailureCount = 0;

        for (auto it = m_watchedItems.begin(); it != m_watchedItems.end();) {
            int queueId = it->first;
            ConvertType type = it->second.type;

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

io::path_t ConvertFileToScoreScenario::pendingConvertsJsonPath() const
{
    return globalConfiguration()->userAppDataPath().appendingComponent("pending_converts.json");
}

void ConvertFileToScoreScenario::loadWatchedItems()
{
    TRACEFUNC;

    RetVal<ByteArray> data = fileSystem()->readFile(pendingConvertsJsonPath());
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

        io::paths_t filePaths;
        const JsonArray filePathsArray = obj.value("filePaths").toArray();
        for (size_t j = 0; j < filePathsArray.size(); ++j) {
            filePaths.push_back(io::path_t(filePathsArray.at(j).toStdString()));
        }

        m_watchedItems.insert_or_assign(queueId, WatchedItem { type, filePaths, DownloadStatus::NotStarted });
    }

    if (!m_watchedItems.empty()) {
        m_timer.start();
        poll();
    }
}

void ConvertFileToScoreScenario::saveWatchedItems()
{
    TRACEFUNC;

    JsonArray array;
    for (const auto& pair : m_watchedItems) {
        JsonArray filePaths;
        for (const io::path_t& filePath : pair.second.filePaths) {
            filePaths << filePath.toStdString();
        }

        JsonObject obj;
        obj["id"] = pair.first;
        obj["type"] = static_cast<int>(pair.second.type);
        obj["filePaths"] = filePaths;
        array << obj;
    }

    JsonDocument json(array);
    Ret ret = fileSystem()->writeFile(pendingConvertsJsonPath(), json.toJson());
    if (!ret) {
        LOGE() << "Could not save the pending conversions list: " << ret.toString();
    }
}

Ret ConvertFileToScoreScenario::attachFailedFiles(Ret ret, int queueId) const
{
    auto it = m_watchedItems.find(queueId);
    if (it != m_watchedItems.end()) {
        ret.setData(CONVERT_FAILED_FILES_KEY, it->second.filePaths);
    }
    return ret;
}

void ConvertFileToScoreScenario::onStatusChanged(const ConvertQueueItem& item)
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
        askReviewRating(item.type, item.id);
        downloadIfNotAlready(item.type, item.id);
        break;
    case ConvertStatus::Done:
        downloadIfNotAlready(item.type, item.id);
        break;
    case ConvertStatus::Failed:
        LOGE() << "Conversion failed: " << item << ", errorCode: " << errorCodeToString(item.errorCode);
        finishConvert(attachFailedFiles(make_ret(Err::ConvertProcessingFailed, errorCodeToString(item.errorCode)), item.id));
        break;
    }
}

bool ConvertFileToScoreScenario::shouldHandle(int queueId, ConvertStatus status)
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

void ConvertFileToScoreScenario::askReviewRating(ConvertType type, int queueId)
{
    using Button = IInteractive::Button;

    auto promise = interactive()->question("Review the converted score", "Does this look correct?",
                                           { Button::No, Button::Yes }, Button::Yes);

    promise.onResolve(this, [this, type, queueId](const IInteractive::Result& res) {
        OmrReviewRating rating = res.isButton(Button::Yes) ? OmrReviewRating::Good : OmrReviewRating::Bad;

        museScoreComService()->convert()->submitOmrReview(queueId, rating)
        .onResolve(this, [this, type, queueId](const RetVal<ConvertResult>& submitRes) {
            if (!submitRes.ret) {
                LOGE() << "Could not submit the review for conversion "
                       << convertLogId(queueId, type) << ": " << submitRes.ret.toString();
            }
        });
    });
}

void ConvertFileToScoreScenario::downloadIfNotAlready(ConvertType type, int queueId)
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

void ConvertFileToScoreScenario::fetchScoreUrlAndDownload(ConvertType type, int queueId)
{
    museScoreComService()->convert()->fetchMsczUrl(type, queueId)
    .onResolve(this, [this, type, queueId](const RetVal<SignedMsczUrl>& urlInfo) {
        if (!urlInfo.ret) {
            LOGE() << "Could not fetch the converted score for conversion "
                   << convertLogId(queueId, type) << ": " << urlInfo.ret.toString();
            clearDownloading(queueId);
            finishConvert(attachFailedFiles(urlInfo.ret, queueId));
            return;
        }

        if (urlInfo.val.expiresInSeconds <= 0) {
            Ret ret = make_ret(Err::DownloadLinkExpired, std::string("The download link has already expired"));
            ret = attachFailedFiles(ret, queueId);
            LOGW() << "Could not download the converted score for conversion "
                   << convertLogId(queueId, type) << ": " << ret.toString();
            m_watchedItems.erase(queueId);
            saveWatchedItems();
            finishConvert(ret);
            return;
        }

        downloadScoreAndFinish(urlInfo.val);
    });
}

void ConvertFileToScoreScenario::downloadScoreAndFinish(const SignedMsczUrl& urlInfo)
{
    io::path_t dir = globalConfiguration()->userAppDataPath() + "/converted_scores";
    fileSystem()->makePath(dir);

    auto scoreFile = std::make_shared<QTemporaryFile>(dir.toQString() + "/convertedScore_XXXXXX.mscz");
    scoreFile->setAutoRemove(false);

    if (!scoreFile->open()) {
        Ret ret = make_ret(Err::FileCreateError, std::string("Could not create a file for the converted score"));
        LOGE() << "Could not create a file for the converted score for conversion "
               << convertLogId(urlInfo.id, urlInfo.type) << ": " << ret.toString();
        clearDownloading(urlInfo.id);
        finishConvert(attachFailedFiles(ret, urlInfo.id));
        return;
    }

    const muse::io::path_t path = QFileInfo(*scoreFile).absoluteFilePath();
    ProgressPtr progress = museScoreComService()->convert()->downloadConvertedScore(urlInfo, scoreFile);
    interactive()->showProgress("Downloading…", *progress);

    progress->finished().onReceive(this, [this, path, queueId = urlInfo.id, type = urlInfo.type](const ProgressResult& res) {
        if (!res.ret) {
            LOGE() << "Could not download the converted score for conversion "
                   << convertLogId(queueId, type) << ": " << res.ret.toString();
            clearDownloading(queueId);
            finishConvert(attachFailedFiles(res.ret, queueId));
            return;
        }

        markDownloaded(queueId);
        finishConvert(make_ok(), path);
    });
}

void ConvertFileToScoreScenario::markDownloaded(int queueId)
{
    //! NOTE: the download is done, stop watching
    m_watchedItems.erase(queueId);
    saveWatchedItems();
}

void ConvertFileToScoreScenario::clearDownloading(int queueId)
{
    auto it = m_watchedItems.find(queueId);
    if (it != m_watchedItems.end() && it->second.downloadStatus == DownloadStatus::Downloading) {
        it->second.downloadStatus = DownloadStatus::NotStarted;
    }
}

void ConvertFileToScoreScenario::finishConvert(const Ret& ret, const io::path_t& path)
{
    m_convertFinished.send(ret, path);
}
