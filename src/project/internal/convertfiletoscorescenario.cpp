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

#include <QFile>
#include <QFileInfo>

#include "project/projecterrors.h"
#include "project/types/projecttypes.h"

#include "global/dataformatter.h"
#include "global/log.h"

using namespace mu::project;
using namespace muse;
using namespace muse::cloud;

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

    service()->convertFinished().onReceive(this, [this](const Ret& ret, const io::path_t& path) {
        m_convertFinished.send(ret, path);
    });

    service()->reviewRequested().onReceive(this, [this](int queueId, ConvertType) {
        askReviewRating(queueId);
    });
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

        //! NOTE: config is a client-side sanity check only; if it hasn't been fetched yet, fall back
        //! to letting the server enforce its own limits rather than blocking the conversion
        const ConvertConfig& config = service()->config();
        const ConvertType type = convertTypeFromPath(paths.front(), config);

        if (!validateAgainstConfig(type, paths, config)) {
            return resolve(RetVal<ConvertType>::make_ret(make_ret(Err::ConvertValidationFailed)));
        }

        return resolve(RetVal<ConvertType>::make_ok(type));
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
    std::string dialogText = muse::trc("project/convert", "Log in or create a free account on MuseScore.com to convert a file.");
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
    QString size = DataFormatter::formatFileSize(size_t(maxFileSizeBytes));
    std::string text = muse::qtrc("project/convert", "The maximum file size is %1. Reduce the size of your file and try again.")
                       .arg(size).toStdString();
    showFileValidationError(muse::trc("project/convert", "This file is too large"), text);
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
    IInteractive::ButtonData guidelinesBtn(IInteractive::Button::CustomButton,
                                           muse::trc("project/convert", "Uploading guidelines"));
    IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);

    interactive()->error(title, text, { guidelinesBtn, okBtn }, okBtn.btn)
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
        convertFile.path = path;
        files.push_back(convertFile);
    }

    if (!failedFiles.empty()) {
        IInteractive::Text text;
        text.text = "Could not open the following files";
        text.detailedText = io::pathsToString(failedFiles, "\n");
        interactive()->error("Conversion failed", text);

        Ret ret = make_ret(Err::FileOpenError);
        ret.setData(CONVERT_FAILED_FILES_KEY, failedFiles);
        m_convertFinished.send(ret, io::path_t());
        return;
    }

    service()->convert(type, files);
}

void ConvertFileToScoreScenario::askReviewRating(int queueId)
{
    static constexpr int goodBtn = int(IInteractive::Button::CustomButton) + 1;
    static constexpr int badBtn = int(IInteractive::Button::CustomButton) + 2;

    IInteractive::ButtonData good(goodBtn, muse::trc("project/convert", "Good"), /*accent*/ true);
    IInteractive::ButtonData bad(badBtn, muse::trc("project/convert", "Bad"));

    //! TODO: replace with toast
    auto promise = interactive()->question(
        muse::trc("project/convert", "How does your score look?"),
        muse::trc("project/convert", "We’re always improving our score conversion accuracy. Let us know how we did with this one."),
        { good, bad }, goodBtn);

    promise.onResolve(this, [this, queueId](const IInteractive::Result& res) {
        ReviewRating rating = res.isButton(goodBtn) ? ReviewRating::Good : ReviewRating::Bad;
        service()->submitReview(queueId, rating);
    });
}
