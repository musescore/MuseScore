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

#include "log.h"

using namespace mu::project;
using namespace muse;
using namespace muse::cloud;

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

static ImportType importTypeFromPath(const io::path_t& path)
{
    QString ext = QFileInfo(path.toQString()).suffix().toLower();
    return ext == "mp3" ? ImportType::Audio2Score : ImportType::Omr;
}

static constexpr int MAX_CONSECUTIVE_POLL_FAILURES = 5;

void ImportFileToScoreScenario::init()
{
    m_timer.setInterval(1 * 60000); // poll once a minute

    QObject::connect(&m_timer, &QTimer::timeout, [this]() { poll(); });
}

async::Promise<io::paths_t> ImportFileToScoreScenario::selectFilesToImport()
{
    return interactive()->open("musescore://project/import/selectfiles").then<io::paths_t>(this, [](const Val& val, auto resolve) {
        QStringList paths = val.toQVariant().toStringList();

        io::paths_t result;
        result.reserve(paths.size());
        for (const QString& path : paths) {
            result.push_back(io::path_t(path));
        }

        return resolve(result);
    });
}

bool ImportFileToScoreScenario::isImportInProgress() const
{
    return m_importInProgress;
}

async::Channel<Ret, io::path_t> ImportFileToScoreScenario::importFinished() const
{
    return m_importFinished;
}

bool ImportFileToScoreScenario::importFiles(const io::paths_t& paths)
{
    if (m_importInProgress) {
        return false;
    }

    if (paths.empty()) {
        return false;
    }

    ImportType type = importTypeFromPath(paths.front());

    for (const io::path_t& path : paths) {
        if (importTypeFromPath(path) != type) {
            interactive()->error("Import failed", "All files must be of the same type");
            return false;
        }
    }

    ImportFileList files;
    files.reserve(paths.size());

    QStringList failedFiles;

    for (const io::path_t& path : paths) {
        auto file = std::make_shared<QFile>(path.toQString());
        if (!file->open(QIODevice::ReadOnly)) {
            failedFiles << QFileInfo(path.toQString()).fileName();
            continue;
        }

        ImportFile importFile;
        importFile.data = file;
        importFile.fileName = QFileInfo(path.toQString()).fileName();
        files.push_back(importFile);
    }

    if (!failedFiles.isEmpty()) {
        IInteractive::Text text;
        text.text = "Could not open the following files";
        text.detailedText = failedFiles.join(", ").toStdString();
        interactive()->error("Import failed", text);
        return false;
    }

    m_importInProgress = true;
    upload(type, files);
    return true;
}

void ImportFileToScoreScenario::upload(ImportType type, const ImportFileList& files)
{
    ProgressPtr progress = museScoreComService()->import()->uploadImport(type, files);

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
            Ret ret = make_ret(Ret::Code::UnknownError, std::string("The server did not return a valid import id"));
            interactive()->error("Upload failed", ret.text());
            finishImport(ret);
            return;
        }

        watch(queueId, type);
    });
}

void ImportFileToScoreScenario::watch(int queueId, ImportType type)
{
    m_watchedItems.push_back({ queueId, type });
    m_pollFailureCount = 0;

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

    museScoreComService()->import()->fetchImportQueue().onResolve(this, [this](const RetVal<ImportQueueList>& result) {
        if (!result.ret) {
            ++m_pollFailureCount;

            if (m_pollFailureCount < MAX_CONSECUTIVE_POLL_FAILURES) {
                return;
            }

            IInteractive::Text text;
            text.text = "Could not check the import status";
            text.detailedText = result.ret.toString();
            interactive()->error("Import failed", text);

            m_watchedItems.clear();
            m_timer.stop();
            finishImport(result.ret);
            return;
        }

        m_pollFailureCount = 0;

        for (auto it = m_watchedItems.begin(); it != m_watchedItems.end();) {
            const WatchedItem& watched = *it;

            auto found = std::find_if(result.val.begin(), result.val.end(), [&watched](const ImportQueueItem& item) {
                return item.id == watched.queueId;
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
            doneItem.id = watched.queueId;
            doneItem.type = watched.type;
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

    switch (item.status) {
    case ImportStatus::Processing:
        LOGI() << "Import " << item.id << " (\"" << item.filename << "\", type = "
               << (item.type == ImportType::Omr ? "Omr" : "Audio2Score") << ") is processing";
        break;
    case ImportStatus::AwaitingMeta:
        submitMeta(item.id);
        break;
    case ImportStatus::AwaitingReview:
        askReviewRating(item.id);
        break;
    case ImportStatus::Done:
        fetchScoreUrlAndDownload(item.type, item.id);
        break;
    case ImportStatus::Failed: {
        IInteractive::Text text;
        text.text = "Could not import \"" + item.filename.toStdString() + "\"";
        text.detailedText = errorCodeToString(item.errorCode);
        interactive()->error("Import failed", text);
        finishImport(make_ret(Ret::Code::UnknownError, errorCodeToString(item.errorCode)));
        break;
    }
    case ImportStatus::Unknown:
        break;
    }
}

bool ImportFileToScoreScenario::shouldHandle(int queueId, ImportStatus status)
{
    auto it = m_lastHandledStatus.find(queueId);
    if (it != m_lastHandledStatus.end()) {
        if (it->second == status) {
            return false;
        }
        it->second = status;
        return true;
    }

    m_lastHandledStatus.emplace(queueId, status);
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
        finishImport(res.ret);
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
        finishImport(res.ret);
    });
}

void ImportFileToScoreScenario::fetchScoreUrlAndDownload(ImportType type, int queueId)
{
    museScoreComService()->import()->fetchMsczUrl(type, queueId).onResolve(this, [this](const RetVal<SignedMsczUrl>& urlInfo) {
        if (!urlInfo.ret) {
            interactive()->error("Import failed", "Could not fetch the imported score: " + urlInfo.ret.toString());
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
        Ret ret = make_ret(Ret::Code::UnknownError, std::string("Could not create a file for the imported score"));
        interactive()->error("Import failed", ret.text());
        finishImport(ret);
        return;
    }

    const muse::io::path_t path = QFileInfo(*scoreFile).absoluteFilePath();
    ProgressPtr progress = museScoreComService()->import()->downloadImportedScore(urlInfo, scoreFile);

    progress->finished().onReceive(this, [this, path](const ProgressResult& res) {
        if (!res.ret) {
            interactive()->error("Import failed", "Could not download the imported score: " + res.ret.toString());
            finishImport(res.ret);
            return;
        }

        finishImport(make_ok(), path);
    });
}

void ImportFileToScoreScenario::finishImport(const Ret& ret, const io::path_t& path)
{
    m_importInProgress = false;
    m_importFinished.send(ret, path);
}
