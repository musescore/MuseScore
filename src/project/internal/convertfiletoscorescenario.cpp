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

#include <QFileInfo>

#include "actions/actiontypes.h"

#include "project/projecterrors.h"
#include "project/types/projecttypes.h"

#include "global/dataformatter.h"
#include "global/io/path.h"
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
        if (ret) {
            showScoreReadyNotification(path);
        } else {
            LOGE() << ret.text();
        }

        m_convertFinished.send(ret, path);
    });

    service()->reviewRequested().onReceive(this, [this](int queueId, ConvertType) {
        askReviewRating(queueId);
    });
}

const ConvertConfig& ConvertFileToScoreScenario::convertConfig() const
{
    return service()->config();
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

            const ConvertType type = static_cast<ConvertType>(map.value("type").toInt());
            const QString link = map.value("link").toString();

            io::paths_t paths;
            const QStringList pathsList = map.value("paths").toStringList();
            paths.reserve(pathsList.size());
            for (const QString& path : pathsList) {
                paths.push_back(io::path_t(path));
            }

            ConvertSelection selection;
            selection.convertedFileName = map.value("convertedFileName").toString();

            if (type == ConvertType::Audio2Score && !link.isEmpty()) {
                selection.input = Audio2ScoreConvertInput { link };
            } else if (type == ConvertType::Audio2Score) {
                selection.input = Audio2ScoreConvertInput { paths };
            } else {
                selection.input = OmrConvertInput { paths };
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

async::Promise<RetVal<ConvertType> > ConvertFileToScoreScenario::validate(const io::paths_t& paths)
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

Ret ConvertFileToScoreScenario::convert(const ConvertInput& input, const QString& convertedFileName)
{
    IF_ASSERT_FAILED(io::isAllowedFileName(io::path_t(convertedFileName))) {
        return make_ret(Err::ConvertValidationFailed);
    }

    IF_ASSERT_FAILED(!convertPathsOf(input).empty() || !convertLinkOf(input).isEmpty()) {
        return make_ret(Err::ConvertValidationFailed);
    }

    Ret authRet = ensureAuthorization();
    if (!authRet) {
        return authRet;
    }

    return startUpload(input, convertedFileName);
}

async::Channel<Ret, io::path_t> ConvertFileToScoreScenario::convertFinished() const
{
    return m_convertFinished;
}

Ret ConvertFileToScoreScenario::ensureAuthorization()
{
    IAuthorizationServicePtr authorizationService = museScoreComService()->authorization();
    if (authorizationService->userAuthorized().val) {
        return make_ok();
    }

    std::string dialogText = muse::trc("project/convert", "Log in or create a free account on MuseScore.com to convert a file.");

    UriQuery query("muse://cloud/requireauthorization");
    query.addParam("text", Val(dialogText));
    query.addParam("cloudCode", Val(authorizationService->cloudInfo().code));
    query.addParam("publishingScore", Val(false));

    return interactive()->openSync(query).ret;
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

Ret ConvertFileToScoreScenario::startUpload(const ConvertInput& input, const QString& convertedFileName)
{
    if (configuration()->showConvertFileProcessingDialog()) {
        showFileProcessingDialog();
    }

    return service()->convert(input, convertedFileName);
}

void ConvertFileToScoreScenario::showFileProcessingDialog()
{
    constexpr int uploadMoreBtn = int(IInteractive::Button::CustomButton) + 1;
    constexpr int goToScoresBtn = int(IInteractive::Button::CustomButton) + 2;

    IInteractive::ButtonData uploadMore(uploadMoreBtn, muse::trc("project/convert", "Upload more"));
    IInteractive::ButtonData goToScores(goToScoresBtn, muse::trc("project/convert", "Go to scores"));
    IInteractive::ButtonData ok = interactive()->buttonData(IInteractive::Button::Ok);

    std::string msg = muse::trc("project/convert",
                                "We’ll notify you once the score is ready to open. "
                                "You can check the status of the score in Home > Scores.");

    interactive()->info(muse::trc("project/convert", "Your score is being processed"), msg,
                        { uploadMore, goToScores, ok }, static_cast<int>(IInteractive::Button::Ok),
                        IInteractive::Option::WithDontShowAgainCheckBox)
    .onResolve(this, [this, uploadMoreBtn, goToScoresBtn](const IInteractive::Result& result) {
        configuration()->setShowConvertFileProcessingDialog(result.showAgain());

        if (result.isButton(uploadMoreBtn)) {
            selectFilesToConvert()
            .onResolve(this, [this](const ConvertSelection& selection) {
                convert(selection.input, selection.convertedFileName);
            });
        } else if (result.isButton(goToScoresBtn)) {
            interactive()->openUrl(museScoreComService()->scoreManagerUrl());
        }
    });
}

void ConvertFileToScoreScenario::showScoreReadyNotification(const io::path_t& path)
{
    constexpr int openScoreBtn = int(IInteractive::Button::CustomButton) + 1;
    constexpr int dismissBtn = int(IInteractive::Button::CustomButton) + 2;

    IInteractive::ButtonData openScore(openScoreBtn, muse::trc("project/convert", "Open score"), /*accent*/ true);
    IInteractive::ButtonData dismiss(dismissBtn, muse::trc("project/convert", "Dismiss"));

    QString scoreName = QFileInfo(path.toQString()).completeBaseName();
    std::string msg = muse::qtrc("project/convert", "‘%1’ has finished processing and is ready to open.")
                      .arg(scoreName).toStdString();

    //! TODO: replace with toast
    interactive()->info(muse::trc("project/convert", "Your score is ready!"), msg,
                        { dismiss, openScore }, dismissBtn, IInteractive::Option::WithIcon)
    .onResolve(this, [this, path, openScoreBtn](const IInteractive::Result& result) {
        if (result.isButton(openScoreBtn)) {
            dispatcher()->dispatch("file-open", actions::ActionData::make_arg1<QUrl>(path.toQUrl()));
        }
    });
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
