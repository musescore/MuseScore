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
#include "importfiletoscorescenario.h"

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

static std::string errorCodeToString(ImportErrorCode code)
{
    switch (code) {
    case ImportErrorCode::Unknown: return "Something went wrong";
    case ImportErrorCode::UnsupportedFormat: return "This file format is not supported";
    case ImportErrorCode::FileTooLarge: return "The file is too large";
    case ImportErrorCode::TooManyFiles: return "Too many files were provided";
    case ImportErrorCode::RateLimited: return "Too many import requests, please try again later";
    case ImportErrorCode::MsczNotReady: return "The score is not ready yet";
    case ImportErrorCode::MetaLocked: return "The score information can no longer be changed";
    case ImportErrorCode::NoNeedReview: return "This score does not require a review";
    case ImportErrorCode::SearchRequired: return "A song search is required";
    case ImportErrorCode::InvalidInput: return "The provided input is invalid";
    case ImportErrorCode::InvalidFileType: return "The file type is invalid";
    case ImportErrorCode::InvalidFormat: return "The file format is invalid";
    case ImportErrorCode::FileProcessingError: return "The file could not be processed";
    case ImportErrorCode::ModelExecutionError: return "The conversion model failed to run";
    case ImportErrorCode::ConversionError: return "The file could not be converted";
    case ImportErrorCode::ResourceNotFound: return "The requested resource was not found";
    case ImportErrorCode::InternalServerError: return "A server error occurred";
    }
    return std::string();
}

static std::string importLogId(int queueId, ImportType type)
{
    return std::to_string(queueId) + " (type: " + importTypeToString(type) + ")";
}

static ImportType importTypeFromPath(const io::path_t& path, const ImportConfig& config)
{
    const QString ext = QFileInfo(path.toQString()).suffix();

    if (!config.audio2score.allowedExtensions.isEmpty()) {
        return config.audio2score.allowedExtensions.contains(ext, Qt::CaseInsensitive)
               ? ImportType::Audio2Score : ImportType::Omr;
    }

    //! NOTE: config unavailable, fall back to a best-effort guess
    return isAudioFileSuffix(ext.toLower().toStdString()) ? ImportType::Audio2Score : ImportType::Omr;
}

void ImportFileToScoreScenario::init()
{
    TRACEFUNC;

    m_timer.setInterval(1 * 60000); // poll once a minute

    QObject::connect(&m_timer, &QTimer::timeout, [this]() { poll(); });

    //! NOTE: prefetch and cache import config
    museScoreComService()->import()->fetchImportConfig().onResolve(this, [](const RetVal<ImportConfig>& config) {
        if (!config.ret) {
            LOGW() << "Could not prefetch import config: " << config.ret.toString();
        }
    });
}

void ImportFileToScoreScenario::resumeImport()
{
    //! NOTE: resuming polling can show dialogs (errors, review prompts), so it must wait
    //! until the main window is up rather than running during onInit()
    loadWatchedItems();
}

async::Promise<ImportSelection> ImportFileToScoreScenario::selectFilesToImport()
{
    return async::Promise<ImportSelection>([this](auto resolve, auto reject) {
        Ret authRet = ensureAuthorization();
        if (!authRet) {
            return reject(authRet.code(), authRet.text());
        }

        interactive()->open("musescore://project/import/selectfiles")
        .then<ImportSelection>(this, [](const Val& val, auto innerResolve) {
            QVariantMap map = val.toQVariant().toMap();

            ImportSelection selection;
            selection.type = static_cast<ImportType>(map.value("type").toInt());

            const QStringList paths = map.value("paths").toStringList();
            selection.paths.reserve(paths.size());
            for (const QString& path : paths) {
                selection.paths.push_back(io::path_t(path));
            }

            return innerResolve(selection);
        })
        .onResolve(this, [resolve](const ImportSelection& selection) {
            (void)resolve(selection);
        })
        .onReject(this, [reject](int code, const std::string& msg) {
            (void)reject(code, msg);
        });

        return async::Promise<ImportSelection>::dummy_result();
    });
}

async::Promise<RetVal<ImportType> > ImportFileToScoreScenario::validateFiles(const io::paths_t& paths)
{
    return async::Promise<RetVal<ImportType> >([this, paths](auto resolve, auto) {
        if (paths.empty()) {
            return resolve(RetVal<ImportType>::make_ret(make_ret(Err::ImportValidationFailed)));
        }

        museScoreComService()->import()->fetchImportConfig().onResolve(this, [this, paths, resolve](const RetVal<ImportConfig>& config) {
            if (!config.ret) {
                LOGW() << "Could not fetch import config: " << config.ret.toString();
            }

            const ImportType type = importTypeFromPath(paths.front(), config.val);

            //! NOTE: config is a client-side sanity check only; if it can't be fetched, fall back
            //! to letting the server enforce its own limits rather than blocking the import
            if (config.ret && !validateAgainstConfig(type, paths, config.val)) {
                (void)resolve(RetVal<ImportType>::make_ret(make_ret(Err::ImportValidationFailed)));
                return;
            }

            (void)resolve(RetVal<ImportType>::make_ok(type));
        });

        return async::Promise<RetVal<ImportType> >::dummy_result();
    });
}

bool ImportFileToScoreScenario::importFiles(ImportType type, const io::paths_t& paths)
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

async::Channel<Ret, io::path_t> ImportFileToScoreScenario::importFinished() const
{
    return m_importFinished;
}

Ret ImportFileToScoreScenario::ensureAuthorization()
{
    std::string dialogText = "Log in or create a free account on MuseScore.com to convert a file.";
    return museScoreComService()->authorization()->ensureAuthorization(false, dialogText).ret;
}

bool ImportFileToScoreScenario::validateAgainstConfig(ImportType type, const io::paths_t& paths, const ImportConfig& config)
{
    for (const io::path_t& path : paths) {
        if (importTypeFromPath(path, config) != type) {
            showFileValidationError("Import failed", "All files must be of the same type");
            return false;
        }
    }

    if (type == ImportType::Audio2Score) {
        if (config.audio2score.maxFiles > 0 && int(paths.size()) > config.audio2score.maxFiles) {
            std::string text = "Only up to " + std::to_string(config.audio2score.maxFiles)
                               + " audio file(s) can be imported at a time";
            showFileValidationError("Import failed", text);
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
                           + " image(s) can be imported at a time";
        showFileValidationError("Import failed", text);
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

void ImportFileToScoreScenario::showFileTooLargeError(qint64 maxFileSizeBytes)
{
    std::string size = DataFormatter::formatFileSize(size_t(maxFileSizeBytes)).toStdString();
    std::string text = "The maximum file size is " + size
                       + ". Reduce the size of your file and try again.";
    showFileValidationError("This file is too large", text);
}

void ImportFileToScoreScenario::showUnsupportedFormatError(const QStringList& allowedExtensions)
{
    QStringList upperExtensions;
    upperExtensions.reserve(allowedExtensions.size());
    for (const QString& ext : allowedExtensions) {
        upperExtensions << ext.toUpper();
    }

    std::string text = "Make sure the file you're uploading is a " + upperExtensions.join(", ").toStdString() + ".";
    showFileValidationError("This file type is not compatible", text);
}

void ImportFileToScoreScenario::showFileValidationError(const std::string& title, const std::string& text)
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

void ImportFileToScoreScenario::openFilesAndUpload(ImportType type, const io::paths_t& paths)
{
    ImportFileList files;
    files.reserve(paths.size());

    io::paths_t failedFiles;

    for (const io::path_t& path : paths) {
        auto file = std::make_shared<QFile>(path.toQString());
        if (!file->open(QIODevice::ReadOnly)) {
            failedFiles.push_back(path);
            continue;
        }

        ImportFile importFile;
        importFile.data = file;
        importFile.fileName = io::filename(path).toQString();
        files.push_back(importFile);
    }

    if (!failedFiles.empty()) {
        IInteractive::Text text;
        text.text = "Could not open the following files";
        text.detailedText = io::pathsToString(failedFiles, "\n");
        interactive()->error("Import failed", text);

        Ret ret = make_ret(Err::FileOpenError);
        ret.setData(IMPORT_FAILED_FILES_KEY, failedFiles);
        finishImport(ret);
        return;
    }

    upload(type, files, paths);
}

void ImportFileToScoreScenario::upload(ImportType type, const ImportFileList& files, const io::paths_t& filePaths)
{
    ProgressPtr progress = museScoreComService()->import()->uploadImport(type, files);
    interactive()->showProgress("Uploading…", *progress);

    progress->finished().onReceive(this, [this, type, filePaths](const ProgressResult& res) {
        if (!res.ret) {
            LOGE() << "Could not upload the following files (type: " << importTypeToString(type) << "): "
                   << io::pathsToString(filePaths, ", ") << ": " << res.ret.toString();
            Ret ret = res.ret;
            ret.setData(IMPORT_FAILED_FILES_KEY, filePaths);
            finishImport(ret);
            return;
        }

        const int queueId = res.val.toMap()["id"].toInt();
        watch(queueId, type, filePaths);
    });
}

void ImportFileToScoreScenario::watch(int queueId, ImportType type, const io::paths_t& filePaths)
{
    m_watchedItems.insert_or_assign(queueId, WatchedItem { type, filePaths, DownloadStatus::NotStarted });
    saveWatchedItems();

    if (!m_timer.isActive()) {
        m_timer.start();
    }

    poll();
}

void ImportFileToScoreScenario::poll()
{
    if (m_watchedItems.empty()) {
        m_timer.stop();
        return;
    }

    if (m_pollInProgress) {
        return;
    }

    m_pollInProgress = true;

    museScoreComService()->import()->fetchImportQueue().onResolve(this, [this](const RetVal<ImportQueueList>& result) {
        m_pollInProgress = false;

        if (!result.ret) {
            ++m_pollFailureCount;

            if (m_pollFailureCount < MAX_CONSECUTIVE_POLL_FAILURES) {
                return;
            }

            LOGE() << "Could not check the import status after " << MAX_CONSECUTIVE_POLL_FAILURES
                   << " attempts, giving up on " << m_watchedItems.size() << " pending import(s): " << result.ret.toString();

            io::paths_t filePaths;
            for (const auto& pair : m_watchedItems) {
                filePaths.insert(filePaths.end(), pair.second.filePaths.begin(), pair.second.filePaths.end());
            }

            Ret ret = result.ret;
            ret.setData(IMPORT_FAILED_FILES_KEY, filePaths);

            m_timer.stop();
            m_watchedItems.clear();
            m_pollFailureCount = 0;
            saveWatchedItems();
            finishImport(ret);
            return;
        }

        m_pollFailureCount = 0;

        for (auto it = m_watchedItems.begin(); it != m_watchedItems.end();) {
            int queueId = it->first;
            ImportType type = it->second.type;

            auto found = std::find_if(result.val.begin(), result.val.end(), [queueId](const ImportQueueItem& item) {
                return item.id == queueId;
            });

            if (found != result.val.end()) {
                onStatusChanged(*found);

                //! NOTE: Done doesn't erase the item yet - it stays watched (and persisted)
                //! until its download actually resolves, so a crash mid-download can be resumed
                if (found->status == ImportStatus::Failed) {
                    it = m_watchedItems.erase(it);
                } else {
                    ++it;
                }
                continue;
            }

            //! NOTE: a finished entity drops out of the queue entirely rather than
            //! reporting a final "done" status, per the OMR/A2S API contract
            ImportQueueItem doneItem;
            doneItem.id = queueId;
            doneItem.type = type;
            doneItem.status = ImportStatus::Done;
            onStatusChanged(doneItem);

            ++it;
        }

        saveWatchedItems();
    });
}

io::path_t ImportFileToScoreScenario::pendingImportsJsonPath() const
{
    return globalConfiguration()->userAppDataPath().appendingComponent("pending_imports.json");
}

void ImportFileToScoreScenario::loadWatchedItems()
{
    TRACEFUNC;

    RetVal<ByteArray> data = fileSystem()->readFile(pendingImportsJsonPath());
    if (!data.ret || data.val.empty()) {
        if (!data.ret && data.ret.code() != static_cast<int>(io::Err::FSNotExist)) {
            LOGE() << "Could not read the pending imports file: " << data.ret;
        }
        return;
    }

    std::string err;
    const JsonDocument json = JsonDocument::fromJson(data.val, &err);
    if (!err.empty() || !json.isArray()) {
        if (!err.empty()) {
            LOGE() << "Could not parse the pending imports file: " << err;
        }
        return;
    }

    const JsonArray array = json.rootArray();
    for (size_t i = 0; i < array.size(); ++i) {
        const JsonObject obj = array.at(i).toObject();
        const int queueId = obj.value("id").toInt();
        const ImportType type = static_cast<ImportType>(obj.value("type").toInt());

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

void ImportFileToScoreScenario::saveWatchedItems()
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
    Ret ret = fileSystem()->writeFile(pendingImportsJsonPath(), json.toJson());
    if (!ret) {
        LOGE() << "Could not save the pending imports list: " << ret.toString();
    }
}

Ret ImportFileToScoreScenario::attachFailedFiles(Ret ret, int queueId) const
{
    auto it = m_watchedItems.find(queueId);
    if (it != m_watchedItems.end()) {
        ret.setData(IMPORT_FAILED_FILES_KEY, it->second.filePaths);
    }
    return ret;
}

void ImportFileToScoreScenario::onStatusChanged(const ImportQueueItem& item)
{
    if (!shouldHandle(item.id, item.status)) {
        return;
    }

    LOGI() << "Import status changed: " << item;

    switch (item.status) {
    case ImportStatus::Processing:
    case ImportStatus::Unknown:
        break;
    case ImportStatus::AwaitingMeta:
        //! NOTE: the MSCZ is already available at this point; submitting meta is optional,
        //! so it doesn't gate the download
        submitMeta(item.type, item.id);
        downloadIfNotAlready(item.type, item.id);
        break;
    case ImportStatus::AwaitingReview:
        //! NOTE: same as AwaitingMeta above - the review rating doesn't gate the download
        askReviewRating(item.type, item.id);
        downloadIfNotAlready(item.type, item.id);
        break;
    case ImportStatus::Done:
        downloadIfNotAlready(item.type, item.id);
        break;
    case ImportStatus::Failed:
        LOGE() << "Import failed: " << item << ", errorCode: " << errorCodeToString(item.errorCode);
        finishImport(attachFailedFiles(make_ret(Err::ImportProcessingFailed, errorCodeToString(item.errorCode)), item.id));
        break;
    }
}

bool ImportFileToScoreScenario::shouldHandle(int queueId, ImportStatus status)
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

void ImportFileToScoreScenario::submitMeta(ImportType type, int queueId)
{
    //! NOTE: dummy meta until the meta-fill dialog is built
    OmrMeta meta;
    meta.id = queueId;
    meta.title = "Untitled";
    meta.isOriginComposition = true;

    museScoreComService()->import()->submitOmrMeta(meta).onResolve(this, [this, type, queueId](const RetVal<ImportResult>& res) {
        if (!res.ret) {
            LOGE() << "Could not submit the score information for import "
                   << importLogId(queueId, type) << ": " << res.ret.toString();
        }
    });
}

void ImportFileToScoreScenario::askReviewRating(ImportType type, int queueId)
{
    using Button = IInteractive::Button;

    auto promise = interactive()->question("Review the imported score", "Does this look correct?",
                                           { Button::No, Button::Yes }, Button::Yes);

    promise.onResolve(this, [this, type, queueId](const IInteractive::Result& res) {
        OmrReviewRating rating = res.isButton(Button::Yes) ? OmrReviewRating::Good : OmrReviewRating::Bad;

        museScoreComService()->import()->submitOmrReview(queueId, rating)
        .onResolve(this, [this, type, queueId](const RetVal<ImportResult>& submitRes) {
            if (!submitRes.ret) {
                LOGE() << "Could not submit the review for import "
                       << importLogId(queueId, type) << ": " << submitRes.ret.toString();
            }
        });
    });
}

void ImportFileToScoreScenario::downloadIfNotAlready(ImportType type, int queueId)
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

void ImportFileToScoreScenario::fetchScoreUrlAndDownload(ImportType type, int queueId)
{
    museScoreComService()->import()->fetchMsczUrl(type, queueId)
    .onResolve(this, [this, type, queueId](const RetVal<SignedMsczUrl>& urlInfo) {
        if (!urlInfo.ret) {
            LOGE() << "Could not fetch the imported score for import "
                   << importLogId(queueId, type) << ": " << urlInfo.ret.toString();
            clearDownloading(queueId);
            finishImport(attachFailedFiles(urlInfo.ret, queueId));
            return;
        }

        if (urlInfo.val.expiresInSeconds <= 0) {
            Ret ret = make_ret(Err::DownloadLinkExpired, std::string("The download link has already expired"));
            ret = attachFailedFiles(ret, queueId);
            LOGW() << "Could not download the imported score for import "
                   << importLogId(queueId, type) << ": " << ret.toString();
            m_watchedItems.erase(queueId);
            saveWatchedItems();
            finishImport(ret);
            return;
        }

        downloadScoreAndFinish(urlInfo.val);
    });
}

void ImportFileToScoreScenario::downloadScoreAndFinish(const SignedMsczUrl& urlInfo)
{
    io::path_t dir = globalConfiguration()->userAppDataPath() + "/imported_scores";
    fileSystem()->makePath(dir);

    auto scoreFile = std::make_shared<QTemporaryFile>(dir.toQString() + "/importedScore_XXXXXX.mscz");
    scoreFile->setAutoRemove(false);

    if (!scoreFile->open()) {
        Ret ret = make_ret(Err::FileCreateError, std::string("Could not create a file for the imported score"));
        LOGE() << "Could not create a file for the imported score for import "
               << importLogId(urlInfo.id, urlInfo.type) << ": " << ret.toString();
        clearDownloading(urlInfo.id);
        finishImport(attachFailedFiles(ret, urlInfo.id));
        return;
    }

    const muse::io::path_t path = QFileInfo(*scoreFile).absoluteFilePath();
    ProgressPtr progress = museScoreComService()->import()->downloadImportedScore(urlInfo, scoreFile);
    interactive()->showProgress("Downloading…", *progress);

    progress->finished().onReceive(this, [this, path, queueId = urlInfo.id, type = urlInfo.type](const ProgressResult& res) {
        if (!res.ret) {
            LOGE() << "Could not download the imported score for import "
                   << importLogId(queueId, type) << ": " << res.ret.toString();
            clearDownloading(queueId);
            finishImport(attachFailedFiles(res.ret, queueId));
            return;
        }

        markDownloaded(queueId);
        finishImport(make_ok(), path);
    });
}

void ImportFileToScoreScenario::markDownloaded(int queueId)
{
    //! NOTE: the download is done, stop watching
    m_watchedItems.erase(queueId);
    saveWatchedItems();
}

void ImportFileToScoreScenario::clearDownloading(int queueId)
{
    auto it = m_watchedItems.find(queueId);
    if (it != m_watchedItems.end() && it->second.downloadStatus == DownloadStatus::Downloading) {
        it->second.downloadStatus = DownloadStatus::NotStarted;
    }
}

void ImportFileToScoreScenario::finishImport(const Ret& ret, const io::path_t& path)
{
    m_importFinished.send(ret, path);
}
