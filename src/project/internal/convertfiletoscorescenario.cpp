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
#include <QUrl>

#include "actions/actiontypes.h"

#include "project/projecterrors.h"

#include "global/dataformatter.h"
#include "global/log.h"

using namespace mu::project;
using namespace muse;
using namespace muse::cloud;

static ConvertSelection toConvertSelection(const Val& val)
{
    const QVariantMap map = val.toQVariant().toMap();
    const ConvertType type = static_cast<ConvertType>(map.value("type").toInt());
    const QString link = map.value("link").toString();
    const QStringList pathsList = map.value("paths").toStringList();

    io::paths_t paths;
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

    return selection;
}

void ConvertFileToScoreScenario::init()
{
    TRACEFUNC;

    service()->convertFinished().onReceive(this, [this](const Ret& ret, const io::path_t& path) {
        if (ret) {
            showScoreReadyNotification(path);
        } else {
            showConvertFailedNotification(ret);
        }

        m_convertFinished.send(ret, path);
    });

    service()->reviewRequested().onReceive(this, [this](ConvertType, int) {
        // TODO: Temporarily disabled to prevent dialog spam until we replace it with a toast
        // askReviewRating(type, queueId);
    });
}

const ConvertConfig& ConvertFileToScoreScenario::config() const
{
    return service()->config();
}

bool ConvertFileToScoreScenario::isFileSupported(const io::path_t& path) const
{
    return service()->isFileSupported(path);
}

RetVal<ConvertFilesValidation> ConvertFileToScoreScenario::validateFiles(const io::paths_t& paths)
{
    RetVal<ConvertFilesValidation> result = service()->validateFiles(paths);
    if (!result.ret) {
        showValidationError(result.ret);
    }

    return result;
}

Ret ConvertFileToScoreScenario::validateLink(const QUrl& link)
{
    Ret ret = service()->validateLink(link);
    if (!ret) {
        showUnsupportedLinkError();
    }

    return ret;
}

void ConvertFileToScoreScenario::convertFiles(const io::paths_t& paths)
{
    checkConvertIsAllowed()
    .onResolve(this, [this, paths](const Ret& ret) {
        if (!ret) {
            return;
        }

        if (paths.empty()) {
            selectFilesToConvert()
            .onResolve(this, [this](const ConvertSelection& selection) {
                startConvert(selection.input, selection.convertedFileName);
            });
            return;
        }

        RetVal<ConvertFilesValidation> validation = validateFiles(paths);
        if (!validation.ret) {
            return;
        }

        confirmConvert(paths, validation.val.type);
    });
}

async::Channel<Ret, io::path_t> ConvertFileToScoreScenario::convertFinished() const
{
    return m_convertFinished;
}

async::Promise<Ret> ConvertFileToScoreScenario::checkConvertIsAllowed()
{
    return async::make_promise<Ret>([this](auto resolve, auto reject) {
        museScoreComService()->authorization()->checkCloudIsAvailableAsync()
        .onResolve(this, [this, resolve, reject](const Ret& ret) {
            if (!ret) {
                showCloudIsNotAvailableError();
                (void)resolve(ret);
                return;
            }

            ensureAuthorization()
            .onResolve(this, [resolve](const Ret& ret) {
                (void)resolve(ret);
            })
            .onReject(this, [reject](int code, const std::string& msg) {
                (void)reject(code, msg);
            });
        });

        return async::Promise<Ret>::dummy_result();
    });
}

async::Promise<Ret> ConvertFileToScoreScenario::ensureAuthorization()
{
    return async::make_promise<Ret>([this](auto resolve, auto reject) {
        IAuthorizationServicePtr authorizationService = museScoreComService()->authorization();
        if (authorizationService->userAuthorized().val) {
            return resolve(make_ok());
        }

        std::string dialogText = muse::trc("project/convert", "Log in or create a free account on MuseScore.com to convert a file.");

        UriQuery query("muse://cloud/requireauthorization");
        query.addParam("text", Val(dialogText));
        query.addParam("cloudCode", Val(authorizationService->cloudInfo().code));
        query.addParam("publishingScore", Val(false));

        interactive()->open(query)
        .onResolve(this, [resolve](const Val&) {
            (void)resolve(make_ok());
        })
        .onReject(this, [reject](int code, const std::string& msg) {
            (void)reject(code, msg);
        });

        return async::Promise<Ret>::dummy_result();
    });
}

async::Promise<ConvertSelection> ConvertFileToScoreScenario::selectFilesToConvert(const io::paths_t& paths, ConvertType type)
{
    UriQuery query("musescore://project/convert/selectfiles");

    if (!paths.empty()) {
        ValList pathsList;
        pathsList.reserve(paths.size());
        for (const io::path_t& path : paths) {
            pathsList.push_back(Val(path));
        }
        query.addParam("initialPaths", Val(pathsList));
        query.addParam("initialConvertType", Val(type));
    }

    return interactive()->open(query)
           .then<ConvertSelection>(this, [](const Val& val, auto resolve) {
        return resolve(toConvertSelection(val));
    });
}

void ConvertFileToScoreScenario::confirmConvert(const io::paths_t& paths, ConvertType type)
{
    constexpr int proceedBtn = int(IInteractive::Button::CustomButton) + 1;

    IInteractive::ButtonData cancel = interactive()->buttonData(IInteractive::Button::Cancel);
    cancel.role = IInteractive::ButtonRole::RejectRole;

    IInteractive::ButtonData proceed(proceedBtn, muse::trc("global", "Proceed"), /*accent*/ true);
    proceed.role = IInteractive::ButtonRole::AcceptRole;

    interactive()->question(muse::trc("project/convert", "Would you like to convert this file to a score?"),
                            muse::trc("project/convert",
                                      "This file needs to be converted online before it can be edited. Would you like to proceed?"),
                            { cancel, proceed }, proceedBtn)
    .onResolve(this, [this, paths, type, proceedBtn](const IInteractive::Result& result) {
        if (!result.isButton(proceedBtn)) {
            return;
        }

        selectFilesToConvert(paths, type)
        .onResolve(this, [this](const ConvertSelection& selection) {
            startConvert(selection.input, selection.convertedFileName);
        });
    });
}

Ret ConvertFileToScoreScenario::startConvert(const ConvertInput& input, const QString& convertedFileName)
{
    Ret ret = service()->startConvert(input, convertedFileName);
    if (!ret) {
        showUnknownError();
        return ret;
    }

    if (configuration()->showConvertFileProcessingDialog()) {
        showFileProcessingDialog();
    }

    return ret;
}

void ConvertFileToScoreScenario::showValidationError(const Ret& ret)
{
    const ConvertConfig& config = service()->config();

    switch (static_cast<Err>(ret.code())) {
    case Err::ConvertUnsupportedFormat:
        showUnsupportedFormatError();
        break;
    case Err::ConvertMixedFileTypes:
        showMixedFileTypesError();
        break;
    case Err::ConvertMultiplePdfFiles:
        showMultiplePdfFilesError();
        break;
    case Err::ConvertAudioFileTooLarge:
        showFileTooLargeError(config.audio2score.file.maxFileSizeBytes);
        break;
    case Err::ConvertFileTooLarge:
        showFileTooLargeError(config.omr.pdf.maxFileSizeBytes);
        break;
    case Err::ConvertCombinedImageTooLarge:
        showCombinedImageSizeTooLargeError(config.omr.images.maxFileSizeBytes);
        break;
    case Err::ConvertTooManyAudioFiles:
        showTooManyAudioFilesError(config.audio2score.file.maxFiles);
        break;
    case Err::ConvertTooManyImages:
        showTooManyImagesError(config.omr.images.maxFiles);
        break;
    default:
        showUnknownError();
        break;
    }
}

void ConvertFileToScoreScenario::showCloudIsNotAvailableError()
{
    interactive()->warning(muse::trc("project/convert", "Unable to connect to MuseScore.com"),
                           muse::trc("project/convert",
                                     "An internet connection is required to convert a file. Please check your internet connection or try again later."),
                           { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showUnknownError()
{
    interactive()->warning(muse::trc("project/convert", "Something went wrong"),
                           muse::trc("project/convert", "Check your internet connection and try again."),
                           { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showFileTooLargeError(qint64 maxFileSizeBytes)
{
    QString size = DataFormatter::formatFileSize(size_t(maxFileSizeBytes));
    std::string text = muse::qtrc("project/convert", "The maximum file size is %1. Reduce the size of your file and try again.")
                       .arg(size).toStdString();
    interactive()->warning(muse::trc("project/convert", "This file is too large"), text,
                           { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showCombinedImageSizeTooLargeError(qint64 maxFileSizeBytes)
{
    QString size = DataFormatter::formatFileSize(size_t(maxFileSizeBytes));
    std::string text = muse::qtrc("project/convert",
                                  "The maximum combined file size for all images is %1. Choose a smaller file or remove some images to continue.")
                       .arg(size).toStdString();
    interactive()->warning(muse::trc("project/convert", "Maximum file size exceeded"), text,
                           { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showUnsupportedFormatError()
{
    interactive()->warning(muse::trc("project/convert", "This file type is not compatible"),
                           muse::trc("project/convert", "Make sure you’re importing a suitable PDF, image or MP3 file."),
                           { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showUnsupportedLinkError()
{
    const LinkSources configured = service()->config().audio2score.link.allowedSources;
    const LinkSources sources = configured ? configured : (LinkSource::YouTube | LinkSource::AudioCom);

    std::string text;
    if (sources.testFlag(LinkSource::YouTube) && sources.testFlag(LinkSource::AudioCom)) {
        text = muse::trc("project/convert", "Make sure you’re using a valid link from YouTube or Audio.com.");
    } else if (sources.testFlag(LinkSource::YouTube)) {
        text = muse::trc("project/convert", "Make sure you’re using a valid link from YouTube.");
    } else if (sources.testFlag(LinkSource::AudioCom)) {
        text = muse::trc("project/convert", "Make sure you’re using a valid link from Audio.com.");
    }

    interactive()->warning(muse::trc("project/convert", "Please use a compatible URL"), text,
                           { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showMixedFileTypesError()
{
    interactive()->warning(muse::trc("project/convert", "Please select files of the same type"),
                           muse::trc("project/convert",
                                     "Per conversion, you may select either one MP3 file, one PDF file, or multiple image files."),
                           { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showMultiplePdfFilesError()
{
    interactive()->warning(muse::trc("project/convert", "Please select a single PDF file"),
                           muse::trc("project/convert", "Only one PDF file can be converted at a time."),
                           { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showTooManyAudioFilesError(int maxFiles)
{
    std::string text = muse::qtrc("project/convert", "You can convert up to %1 audio files at a time. Remove some files and try again.")
                       .arg(maxFiles).toStdString();
    interactive()->warning(muse::trc("project/convert", "Too many files selected"), text,
                           { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showTooManyImagesError(int maxImages)
{
    std::string text = muse::qtrc("project/convert", "You can convert up to %1 images at a time. Remove some images and try again.")
                       .arg(maxImages).toStdString();
    interactive()->warning(muse::trc("project/convert", "Too many images selected"), text,
                           { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showFileProcessingDialog()
{
    constexpr int convertMoreBtn = int(IInteractive::Button::CustomButton) + 1;
    constexpr int goToScoresBtn = int(IInteractive::Button::CustomButton) + 2;

    IInteractive::ButtonData convertMore(convertMoreBtn, muse::trc("project/convert", "Convert more"));
    IInteractive::ButtonData goToScores(goToScoresBtn, muse::trc("project/convert", "Go to scores"));
    IInteractive::ButtonData ok = interactive()->buttonData(IInteractive::Button::Ok);

    std::string msg = muse::trc("project/convert",
                                "We’ll notify you once the score is ready to open. "
                                "You can check the status of the score in Home > Scores.");

    interactive()->info(muse::trc("project/convert", "Your score is being processed"), msg,
                        { convertMore, goToScores, ok }, static_cast<int>(IInteractive::Button::Ok),
                        IInteractive::Option::WithDontShowAgainCheckBox)
    .onResolve(this, [this, convertMoreBtn, goToScoresBtn](const IInteractive::Result& result) {
        configuration()->setShowConvertFileProcessingDialog(result.showAgain());

        if (result.isButton(convertMoreBtn)) {
            convertFiles();
        } else if (result.isButton(goToScoresBtn)) {
            interactive()->open("musescore://home?section=scores");
        }
    });
}

void ConvertFileToScoreScenario::showScoreReadyNotification(const io::path_t& path)
{
    constexpr int openScoreBtn = int(IInteractive::Button::CustomButton) + 1;
    constexpr int dismissBtn = int(IInteractive::Button::CustomButton) + 2;

    IInteractive::ButtonData openScore(openScoreBtn, muse::trc("project/convert", "Open score"), /*accent*/ true);
    openScore.role = IInteractive::ButtonRole::AcceptRole;

    IInteractive::ButtonData dismiss(dismissBtn, muse::trc("global", "Dismiss"));
    dismiss.role = IInteractive::ButtonRole::RejectRole;

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

void ConvertFileToScoreScenario::showConvertFailedNotification(const Ret& ret)
{
    constexpr int tryAgainBtn = int(IInteractive::Button::CustomButton) + 1;
    constexpr int dismissBtn = int(IInteractive::Button::CustomButton) + 2;

    IInteractive::ButtonData dismiss(dismissBtn, muse::trc("global", "Dismiss"));
    dismiss.role = IInteractive::ButtonRole::RejectRole;

    IInteractive::ButtonData tryAgain(tryAgainBtn, muse::trc("global", "Try again"), /*accent*/ true);
    tryAgain.role = IInteractive::ButtonRole::AcceptRole;

    QString fileName = ret.data<QString>(CONVERT_FAILED_FILE_NAME_KEY, QString());
    std::string msg = muse::qtrc("project/convert", "We weren’t able to convert ‘%1’. Please try again with a better quality file.")
                      .arg(fileName).toStdString();

    //! TODO: replace with toast
    interactive()->warning(muse::trc("project/convert", "Error processing score"), msg,
                           { dismiss, tryAgain }, dismissBtn)
    .onResolve(this, [this, tryAgainBtn](const IInteractive::Result& result) {
        if (result.isButton(tryAgainBtn)) {
            convertFiles();
        }
    });
}

void ConvertFileToScoreScenario::askReviewRating(ConvertType type, int queueId)
{
    static constexpr int goodBtn = int(IInteractive::Button::CustomButton) + 1;
    static constexpr int badBtn = int(IInteractive::Button::CustomButton) + 2;

    //: Button to rate the quality of a converted score as good
    IInteractive::ButtonData good(goodBtn, muse::trc("project/convert", "Good"), /*accent*/ true);
    //: Button to rate the quality of a converted score as bad
    IInteractive::ButtonData bad(badBtn, muse::trc("project/convert", "Bad"));

    //! TODO: replace with toast
    auto promise = interactive()->question(
        muse::trc("project/convert", "How does your score look?"),
        muse::trc("project/convert", "We’re always improving our score conversion accuracy. Let us know how we did with this one."),
        { good, bad }, goodBtn);

    promise.onResolve(this, [this, queueId, type](const IInteractive::Result& res) {
        ReviewRating rating = res.isButton(goodBtn) ? ReviewRating::Good : ReviewRating::Bad;
        service()->submitReview(type, queueId, rating);
    });
}
