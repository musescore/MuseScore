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

#include "dataformatter.h"
#include "log.h"

#include "projecterrors.h"
#include "types/projecttypes.h"

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

static const char* importStatusToString(ImportStatus status)
{
    switch (status) {
    case ImportStatus::Processing: return "processing";
    case ImportStatus::AwaitingMeta: return "awaiting meta";
    case ImportStatus::AwaitingReview: return "awaiting review";
    case ImportStatus::Done: return "done";
    case ImportStatus::Failed: return "failed";
    case ImportStatus::Unknown: return "unknown";
    }
    return "";
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
    m_timer.setInterval(1 * 60000); // poll once a minute

    QObject::connect(&m_timer, &QTimer::timeout, [this]() { poll(); });

    //! NOTE: prefetch and cache import config
    museScoreComService()->import()->fetchImportConfig().onResolve(this, [](const RetVal<ImportConfig>& config) {
        if (!config.ret) {
            LOGW() << "Could not prefetch import config: " << config.ret.toString();
        }
    });
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
            failedFiles.push_back(io::filename(path));
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
        finishImport(make_ret(Err::FileOpenError));
        return;
    }

    upload(type, files);
}

void ImportFileToScoreScenario::upload(ImportType type, const ImportFileList& files)
{
    ProgressPtr progress = museScoreComService()->import()->uploadImport(type, files);
    interactive()->showProgress("Uploading…", *progress);

    progress->finished().onReceive(this, [this, type, files](const ProgressResult& res) {
        if (!res.ret) {
            QStringList fileNames;
            fileNames.reserve(int(files.size()));
            for (const ImportFile& file : files) {
                fileNames << file.fileName;
            }

            IInteractive::Text text;
            text.text = "Could not upload the following files";
            text.detailedText = fileNames.join(", ").toStdString() + "\n" + res.ret.toString();
            interactive()->error("Upload failed", text);
            finishImport(res.ret);
            return;
        }

        int queueId = res.val.toMap()["id"].toInt();
        if (queueId <= 0) {
            Ret ret = make_ret(Err::MalformedImportResponse, std::string("The server did not return a valid import id"));
            interactive()->error("Upload failed", ret.text());
            finishImport(ret);
            return;
        }

        watch(queueId, type);
    });
}

void ImportFileToScoreScenario::watch(int queueId, ImportType type)
{
    m_watchedItems.insert_or_assign(queueId, WatchedItem { type, DownloadStatus::NotStarted });

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

            IInteractive::Text text;
            text.text = "Could not check the import status";
            text.detailedText = result.ret.toString();
            interactive()->error("Import failed", text);

            m_timer.stop();
            m_watchedItems.clear();
            m_pollFailureCount = 0;
            finishImport(result.ret);
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

                if (found->status == ImportStatus::Done || found->status == ImportStatus::Failed) {
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

            it = m_watchedItems.erase(it);
        }
    });
}

void ImportFileToScoreScenario::onStatusChanged(const ImportQueueItem& item)
{
    if (!shouldHandle(item.id, item.status)) {
        return;
    }

    const char* typeStr = item.type == ImportType::Omr ? "Omr" : "Audio2Score";
    LOGI() << "Import " << item.id << " (\"" << item.filename << "\", type = " << typeStr
           << ", scoreId = " << item.scoreId << ", createdAt = " << item.createdAt.toString(Qt::ISODate)
           << ", updatedAt = " << item.updatedAt.toString(Qt::ISODate)
           << ") is " << importStatusToString(item.status);

    switch (item.status) {
    case ImportStatus::Processing:
    case ImportStatus::Unknown:
        break;
    case ImportStatus::AwaitingMeta:
        //! NOTE: the MSCZ is already available at this point; submitting meta is optional,
        //! so it doesn't gate the download
        submitMeta(item.id);
        downloadIfNotAlready(item.type, item.id);
        break;
    case ImportStatus::AwaitingReview:
        //! NOTE: same as AwaitingMeta above - the review rating doesn't gate the download
        askReviewRating(item.id);
        downloadIfNotAlready(item.type, item.id);
        break;
    case ImportStatus::Done:
        downloadIfNotAlready(item.type, item.id);
        break;
    case ImportStatus::Failed: {
        IInteractive::Text text;
        text.text = "Could not import \"" + item.filename.toStdString() + "\"";
        text.detailedText = errorCodeToString(item.errorCode);
        interactive()->error("Import failed", text);
        finishImport(make_ret(Ret::Code::UnknownError, errorCodeToString(item.errorCode)));
        break;
    }
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

void ImportFileToScoreScenario::submitMeta(int queueId)
{
    //! NOTE: dummy meta until the meta-fill dialog is built
    OmrMeta meta;
    meta.id = queueId;
    meta.title = "Untitled";
    meta.isOriginComposition = true;

    museScoreComService()->import()->submitOmrMeta(meta).onResolve(this, [this](const RetVal<ImportResult>& res) {
        if (res.ret) {
            return;
        }

        IInteractive::Text text;
        text.text = "Could not submit the score information";
        text.detailedText = res.ret.toString();
        interactive()->error("Import failed", text);
    });
}

void ImportFileToScoreScenario::askReviewRating(int queueId)
{
    using Button = IInteractive::Button;

    auto promise = interactive()->question("Review the imported score", "Does this look correct?",
                                           { Button::No, Button::Yes }, Button::Yes);

    promise.onResolve(this, [this, queueId](const IInteractive::Result& res) {
        OmrReviewRating rating = res.isButton(Button::Yes) ? OmrReviewRating::Good : OmrReviewRating::Bad;
        submitReview(queueId, rating);
    });
}

void ImportFileToScoreScenario::submitReview(int queueId, OmrReviewRating rating)
{
    museScoreComService()->import()->submitOmrReview(queueId, rating).onResolve(this, [this](const RetVal<ImportResult>& res) {
        if (res.ret) {
            return;
        }

        IInteractive::Text text;
        text.text = "Could not submit the review";
        text.detailedText = res.ret.toString();
        interactive()->error("Review failed", text);
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
    museScoreComService()->import()->fetchMsczUrl(type, queueId).onResolve(this, [this, queueId](const RetVal<SignedMsczUrl>& urlInfo) {
        if (!urlInfo.ret) {
            interactive()->error("Import failed", "Could not fetch the imported score: " + urlInfo.ret.toString());
            clearDownloading(queueId);
            finishImport(urlInfo.ret);
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
        interactive()->error("Import failed", ret.text());
        clearDownloading(urlInfo.id);
        finishImport(ret);
        return;
    }

    const muse::io::path_t path = QFileInfo(*scoreFile).absoluteFilePath();
    ProgressPtr progress = museScoreComService()->import()->downloadImportedScore(urlInfo, scoreFile);
    interactive()->showProgress("Downloading…", *progress);

    progress->finished().onReceive(this, [this, path, queueId = urlInfo.id](const ProgressResult& res) {
        if (!res.ret) {
            interactive()->error("Import failed", "Could not download the imported score: " + res.ret.toString());
            clearDownloading(queueId);
            finishImport(res.ret);
            return;
        }

        markDownloaded(queueId);
        finishImport(make_ok(), path);
    });
}

void ImportFileToScoreScenario::markDownloaded(int queueId)
{
    auto it = m_watchedItems.find(queueId);
    if (it != m_watchedItems.end()) {
        it->second.downloadStatus = DownloadStatus::Downloaded;
    }
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
