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

#include <QUrl>

#include "actions/actiontypes.h"

#include "project/projecterrors.h"

#include "global/translation.h"
#include "global/dataformatter.h"
#include "global/log.h"

using namespace mu::project;
using namespace muse;
using namespace muse::cloud;

//! NOTE: attempt 4 is ~5 minutes into retrying
static constexpr int RETRY_TOAST_ATTEMPT_THRESHOLD = 4;

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
    selection.convertedScoreName = map.value("convertedScoreName").toString();

    if (type == ConvertType::Audio2Score && !link.isEmpty()) {
        selection.input = Audio2ScoreConvertInput { QUrl(link) };
    } else if (type == ConvertType::Audio2Score) {
        selection.input = Audio2ScoreConvertInput { paths };
    } else {
        selection.input = OmrConvertInput { paths };
    }

    return selection;
}

ConvertFileToScoreScenario::ConvertFileToScoreScenario(const muse::modularity::ContextPtr& iocCtx)
    : muse::Contextable(iocCtx)
{
}

void ConvertFileToScoreScenario::init()
{
    TRACEFUNC;

    service()->convertFinished().onReceive(this, [this](const Ret& ret, const WatchedScore& watched) {
        if (ret) {
            showScoreReadyNotification(watched);
        } else {
            showConvertFailedNotification(ret);
        }

        m_convertFinished.send(ret, watched);
    });

    service()->pollingFailed().onReceive(this, [this](const PollingFailure& failure) {
        if (failure.gaveUp) {
            showPollingGaveUpNotification();
            return;
        }

        if (failure.attempt == 1) {
            m_retryToastShown = false;
        }

        if (!m_retryToastShown && failure.attempt >= RETRY_TOAST_ATTEMPT_THRESHOLD) {
            m_retryToastShown = true;
            showPollingFailureNotification();
        }
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
    //! NOTE Guards against repeated clicks opening multiple dialogs
    //! while checkConvertIsAllowed is still pending
    if (m_convertFlowInProgress) {
        return;
    }

    m_convertFlowInProgress = true;

    checkConvertIsAllowed()
    .onResolve(this, [this, paths](const Ret& ret) {
        if (!ret) {
            m_convertFlowInProgress = false;
            return;
        }

        if (paths.empty()) {
            selectFilesToConvert()
            .onResolve(this, [this](const ConvertSelection& selection) {
                startConvert(selection.input, selection.convertedScoreName);
                m_convertFlowInProgress = false;
            })
            .onReject(this, [this](int, const std::string&) {
                m_convertFlowInProgress = false;
            });
            return;
        }

        RetVal<ConvertFilesValidation> validation = validateFiles(paths);
        if (!validation.ret) {
            m_convertFlowInProgress = false;
            return;
        }

        confirmConvert(paths, validation.val.type);
    })
    .onReject(this, [this](int, const std::string&) {
        m_convertFlowInProgress = false;
    });
}

async::Channel<Ret, WatchedScore> ConvertFileToScoreScenario::convertFinished() const
{
    return m_convertFinished;
}

ValNt<WatchedScoreList> ConvertFileToScoreScenario::watchedScores() const
{
    return service()->watchedScores();
}

bool ConvertFileToScoreScenario::isAwaitingReview(int scoreId) const
{
    const WatchedScore* watched = service()->watchedScoreById(scoreId);
    return watched && watched->conversion.status == ConvertStatus::AwaitingReview;
}

void ConvertFileToScoreScenario::cancelConversion(ConvertType type, int convertId)
{
    constexpr int keepConvertingBtn = int(IInteractive::Button::No);
    constexpr int cancelBtn = int(IInteractive::Button::Yes);

    IInteractive::ButtonData keepConverting(keepConvertingBtn, muse::trc("project/convert", "Continue converting"));
    keepConverting.role = IInteractive::ButtonRole::RejectRole;

    IInteractive::ButtonData cancel(cancelBtn, muse::trc("project/convert", "Yes, cancel"), /*accent*/ true);
    cancel.role = IInteractive::ButtonRole::DestructiveRole;

    interactive()->question(
        muse::trc("project/convert", "Are you sure you want to cancel this file conversion?"),
        muse::trc("project/convert", "Processing will be canceled and this score will be removed from your scores."),
        { keepConverting, cancel }, keepConvertingBtn, IInteractive::WithIcon)
    .onResolve(this, [this, type, convertId, cancelBtn](const IInteractive::Result& result) {
        if (result.isButton(cancelBtn)) {
            service()->deleteConversion(type, convertId);
        }
    });
}

async::Channel<PollingFailure> ConvertFileToScoreScenario::pollingFailed() const
{
    return service()->pollingFailed();
}

void ConvertFileToScoreScenario::retryPolling()
{
    service()->retryPolling();
}

async::Promise<Ret> ConvertFileToScoreScenario::checkConvertIsAllowed()
{
    return async::make_promise<Ret>([this](auto resolve, auto reject) {
        museScoreComService()->authorization()->checkCloudIsAvailable()
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
            m_convertFlowInProgress = false;
            return;
        }

        selectFilesToConvert(paths, type)
        .onResolve(this, [this](const ConvertSelection& selection) {
            startConvert(selection.input, selection.convertedScoreName);
            m_convertFlowInProgress = false;
        })
        .onReject(this, [this](int, const std::string&) {
            m_convertFlowInProgress = false;
        });
    })
    .onReject(this, [this](int, const std::string&) {
        m_convertFlowInProgress = false;
    });
}

Ret ConvertFileToScoreScenario::startConvert(const ConvertInput& input, const muse::String& convertedScoreName)
{
    Ret ret = service()->startConvert(input, convertedScoreName);
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
                                     "An internet connection is required for file conversion. Please check your internet connection or try again later."),
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
                           muse::trc("project/convert", "Make sure you’re importing a suitable PDF, image or audio file."),
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
                                     "Per conversion, you may select either one audio file, one PDF file, or multiple image files."),
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
    std::string text = muse::qtrc("project/convert", "You can convert up to %n audio file(s) at a time. Remove some files and try again.",
                                  nullptr, maxFiles).toStdString();
    interactive()->warning(muse::trc("project/convert", "Too many files selected"), text,
                           { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showTooManyImagesError(int maxImages)
{
    std::string text = muse::qtrc("project/convert", "You can convert up to %n image(s) at a time. Remove some images and try again.",
                                  nullptr, maxImages).toStdString();
    interactive()->warning(muse::trc("project/convert", "Too many images selected"), text,
                           { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showFileProcessingDialog()
{
    interactive()->open("musescore://project/convert/processing")
    .onResolve(this, [this](const Val& val) {
        const QVariantMap map = val.toQVariant().toMap();
        const QString action = map.value("action").toString();

        configuration()->setShowConvertFileProcessingDialog(map.value("showAgain").toBool());

        if (action == "convertMore") {
            convertFiles();
        } else if (action == "goToScores") {
            interactive()->open("musescore://home?section=scores&subSection=myOnlineScores");
        }
    });
}

void ConvertFileToScoreScenario::showScoreReadyNotification(const WatchedScore& watched)
{
    constexpr int openScoreBtn = int(toast::ToastActionCode::Custom) + 1;
    const int scoreId = watched.scoreId ? *watched.scoreId : 0;

    std::string msg = muse::qtrc("project/convert", "‘%1’ has finished processing and is ready to open.")
                      .arg(watched.name.toQString()).toStdString();

    toastService()->show(muse::trc("project/convert", "Your score is ready!"), msg,
                         muse::ui::IconCode::Code::TICK_FILLED, true,
    {
        { muse::trc("global", "Dismiss"), toast::ToastActionCode::Dismiss },
        { muse::trc("project/convert", "Open score"), openScoreBtn, /*accent*/ true },
    }).onResolve(this, [this, scoreId, openScoreBtn](const toast::ToastResult& result) {
        if (result.isCode(openScoreBtn)) {
            const QUrl url(QString("musescore://open-score/%1").arg(scoreId));
            dispatcher()->dispatch("file-open", actions::ActionData::make_arg1<QUrl>(url));
        }
    });
}

void ConvertFileToScoreScenario::showConvertFailedNotification(const Ret& ret)
{
    muse::String fileName = ret.data<muse::String>(CONVERT_FAILED_FILE_NAME_KEY, muse::String());
    if (fileName.isEmpty()) {
        return;
    }

    std::string msg = muse::qtrc("project/convert", "We weren’t able to convert ‘%1’. Please try again with a better quality file.")
                      .arg(fileName.toQString()).toStdString();

    toastService()->show(muse::trc("project/convert", "Error processing score"), msg,
                         muse::ui::IconCode::Code::ERROR_FILLED, true,
    {
        { muse::trc("project/convert", "Try another file"), toast::ToastActionCode::TryAgain },
        { muse::trc("global", "OK"), toast::ToastActionCode::Dismiss, /*accent*/ true },
    }).onResolve(this, [this](const toast::ToastResult& result) {
        if (result.isCode(toast::ToastActionCode::TryAgain)) {
            convertFiles();
        }
    });
}

void ConvertFileToScoreScenario::showPollingFailureNotification()
{
    toastService()->showWarning(
        muse::trc("project/convert", "We’re having trouble connecting to MuseScore.com."),
        muse::trc("project/convert", "We’ll keep trying intermittently."));
}

void ConvertFileToScoreScenario::showPollingGaveUpNotification()
{
    toastService()->show(muse::trc("project/convert", "Unable to connect to MuseScore.com"),
                         muse::trc("project/convert",
                                   "An internet connection is required for file conversion. Please check your internet connection or try again later."),
                         muse::ui::IconCode::Code::ERROR_FILLED, true,
    {
        { muse::trc("global", "Dismiss"), toast::ToastActionCode::Dismiss },
        { muse::trc("global", "Retry"), toast::ToastActionCode::TryAgain, /*accent*/ true },
    }).onResolve(this, [this](const toast::ToastResult& result) {
        if (result.isCode(toast::ToastActionCode::TryAgain)) {
            retryPolling();
        }
    });
}
