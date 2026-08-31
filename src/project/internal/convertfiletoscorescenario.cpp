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

#include <optional>

#include <QFileInfo>
#include <QUrl>

#include "actions/actiontypes.h"

#include "project/projecterrors.h"

#include "global/dataformatter.h"
#include "global/io/path.h"
#include "global/log.h"

using namespace mu::project;
using namespace muse;
using namespace muse::cloud;

static bool isSupportedExtension(const muse::io::path_t& path, const ConvertConfig& config)
{
    return fileCategoryFromPath(path, config) != FileCategory::Unknown;
}

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

async::Promise<ConvertSelection> ConvertFileToScoreScenario::selectFilesToConvert()
{
    return interactive()->open("musescore://project/convert/selectfiles")
           .then<ConvertSelection>(this, [](const Val& val, auto resolve) {
        return resolve(toConvertSelection(val));
    });
}

bool ConvertFileToScoreScenario::isFileSupported(const io::path_t& path) const
{
    return isSupportedExtension(path, service()->config());
}

RetVal<ConvertType> ConvertFileToScoreScenario::validateFiles(const io::paths_t& paths)
{
    if (paths.empty()) {
        return RetVal<ConvertType>::make_ret(make_ret(Err::ConvertValidationFailed));
    }

    //! NOTE: config is a client-side sanity check only; if it hasn't been fetched yet, fall back
    //! to letting the server enforce its own limits rather than blocking the conversion
    const ConvertConfig& config = service()->config();
    const ConvertType type = fileCategoryFromPath(paths.front(), config) == FileCategory::Audio
                             ? ConvertType::Audio2Score : ConvertType::Omr;

    if (!validateAgainstConfig(paths, config)) {
        return RetVal<ConvertType>::make_ret(make_ret(Err::ConvertValidationFailed));
    }

    return RetVal<ConvertType>::make_ok(type);
}

Ret ConvertFileToScoreScenario::validateLink(const QUrl& link)
{
    //! NOTE: config is a client-side sanity check only; if it hasn't been fetched yet, fall back
    //! to a best-effort guess covering all known sources
    const ConvertConfig& config = service()->config();
    const LinkSources sources = config.audio2score.allowedLinkSources
                                ? config.audio2score.allowedLinkSources
                                : LinkSource::YouTube | LinkSource::AudioCom;

    if (!link.isValid()) {
        const QString err = link.errorString();
        if (!err.isEmpty()) {
            LOGW() << "Invalid link: " << err;
        }
        showUnsupportedLinkError(sources);
        return make_ret(Err::ConvertValidationFailed);
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

    showUnsupportedLinkError(sources);
    return make_ret(Err::ConvertValidationFailed);
}

void ConvertFileToScoreScenario::convertFiles(const io::paths_t& paths)
{
    checkConvertIsAllowed()
    .onResolve(this, [this, paths](const Ret& ret) {
        if (!ret) {
            return;
        }

        RetVal<ConvertType> validation = validateFiles(paths);
        if (!validation.ret) {
            return;
        }

        confirmConvert(paths, validation.val);
    });
}

void ConvertFileToScoreScenario::confirmConvert(const io::paths_t& paths, ConvertType type)
{
    constexpr int proceedBtn = int(IInteractive::Button::CustomButton) + 1;

    IInteractive::ButtonData cancel = interactive()->buttonData(IInteractive::Button::Cancel);
    IInteractive::ButtonData proceed(proceedBtn, muse::trc("global", "Proceed"), /*accent*/ true);

    interactive()->question(muse::trc("project/convert", "Would you like to convert this file to a score?"),
                            muse::trc("project/convert",
                                      "This file needs to be converted online before it can be edited. Would you like to proceed?"),
                            { cancel, proceed }, proceedBtn)
    .onResolve(this, [this, paths, type, proceedBtn](const IInteractive::Result& result) {
        if (!result.isButton(proceedBtn)) {
            return;
        }

        ConvertInput input = type == ConvertType::Audio2Score
                             ? ConvertInput(Audio2ScoreConvertInput { paths })
                             : ConvertInput(OmrConvertInput { paths });

        startConvert(input, QFileInfo(paths.front().toQString()).completeBaseName());
    });
}

Ret ConvertFileToScoreScenario::startConvert(const ConvertInput& input, const QString& convertedFileName)
{
    Ret ret = service()->startConvert(input, convertedFileName);
    if (!ret) {
        // return ret; // TODO: always fails right now
    }

    if (configuration()->showConvertFileProcessingDialog()) {
        showFileProcessingDialog();
    }

    return ret;
}

async::Channel<Ret, io::path_t> ConvertFileToScoreScenario::convertFinished() const
{
    return m_convertFinished;
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

bool ConvertFileToScoreScenario::validateAgainstConfig(const io::paths_t& paths, const ConvertConfig& config)
{
    std::optional<FileCategory> firstCategory;
    qint64 totalSizeBytes = 0;

    for (const io::path_t& path : paths) {
        if (!isSupportedExtension(path, config)) {
            showUnsupportedFormatError();
            return false;
        }

        const FileCategory category = fileCategoryFromPath(path, config);
        if (!firstCategory) {
            firstCategory = category;
        } else if (category != firstCategory) {
            showMixedFileTypesError();
            return false;
        }

        const qint64 fileSizeBytes = QFileInfo(path.toQString()).size();

        if (category == FileCategory::Audio
            && config.audio2score.maxFileSizeBytes > 0 && fileSizeBytes > config.audio2score.maxFileSizeBytes) {
            showFileTooLargeError(config.audio2score.maxFileSizeBytes);
            return false;
        }

        totalSizeBytes += fileSizeBytes;
    }

    if (firstCategory == FileCategory::Pdf && paths.size() > 1) {
        showMultiplePdfFilesError();
        return false;
    }

    if (firstCategory == FileCategory::Audio) {
        if (config.audio2score.maxFiles > 0 && int(paths.size()) > config.audio2score.maxFiles) {
            showTooManyAudioFilesError(config.audio2score.maxFiles);
            return false;
        }

        return true;
    }

    if (config.omr.maxImages > 0 && paths.size() > 1 && int(paths.size()) > config.omr.maxImages) {
        showTooManyImagesError(config.omr.maxImages);
        return false;
    }

    //! NOTE: maxFileSizeBytes is a combined budget across all selected images
    //! (or the single file's own size, for a PDF)
    if (config.omr.maxFileSizeBytes > 0 && totalSizeBytes > config.omr.maxFileSizeBytes) {
        if (firstCategory == FileCategory::Image) {
            showCombinedImageSizeTooLargeError(config.omr.maxFileSizeBytes);
        } else {
            showFileTooLargeError(config.omr.maxFileSizeBytes);
        }
        return false;
    }

    return true;
}

void ConvertFileToScoreScenario::showCloudIsNotAvailableError()
{
    interactive()->error(muse::trc("project/convert", "Unable to connect to MuseScore.com"),
                         muse::trc("project/convert",
                                   "An internet connection is required to convert a file. Please check your internet connection or try again later."),
                         { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showFileTooLargeError(qint64 maxFileSizeBytes)
{
    QString size = DataFormatter::formatFileSize(size_t(maxFileSizeBytes));
    std::string text = muse::qtrc("project/convert", "The maximum file size is %1. Reduce the size of your file and try again.")
                       .arg(size).toStdString();
    interactive()->error(muse::trc("project/convert", "This file is too large"), text,
                         { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showCombinedImageSizeTooLargeError(qint64 maxFileSizeBytes)
{
    QString size = DataFormatter::formatFileSize(size_t(maxFileSizeBytes));
    std::string text = muse::qtrc("project/convert",
                                  "The maximum combined file size for all images is %1. Choose a smaller file or remove some images to continue.")
                       .arg(size).toStdString();
    interactive()->error(muse::trc("project/convert", "Maximum file size exceeded"), text,
                         { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showUnsupportedFormatError()
{
    interactive()->error(muse::trc("project/convert", "This file type is not compatible"),
                         muse::trc("project/convert", "Make sure you’ve selected a PDF, image or MP3 file."),
                         { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showUnsupportedLinkError(LinkSources sources)
{
    std::string text;
    if (sources.testFlag(LinkSource::YouTube) && sources.testFlag(LinkSource::AudioCom)) {
        text = muse::trc("project/convert", "Make sure you’re using a valid link from YouTube or Audio.com.");
    } else if (sources.testFlag(LinkSource::YouTube)) {
        text = muse::trc("project/convert", "Make sure you’re using a valid link from YouTube.");
    } else if (sources.testFlag(LinkSource::AudioCom)) {
        text = muse::trc("project/convert", "Make sure you’re using a valid link from Audio.com.");
    }

    interactive()->error(muse::trc("project/convert", "Please use a compatible URL"), text,
                         { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showMixedFileTypesError()
{
    interactive()->error(muse::trc("project/convert", "Please select files of the same type"),
                         muse::trc("project/convert",
                                   "Per conversion, you may select either one MP3 file, one PDF file, or multiple image files."),
                         { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showMultiplePdfFilesError()
{
    interactive()->error(muse::trc("project/convert", "Please select a single PDF file"),
                         muse::trc("project/convert", "Only one PDF file can be converted at a time."),
                         { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showTooManyAudioFilesError(int maxFiles)
{
    std::string text = muse::qtrc("project/convert", "You can convert up to %1 audio files at a time. Remove some files and try again.")
                       .arg(maxFiles).toStdString();
    interactive()->error(muse::trc("project/convert", "Too many files selected"), text,
                         { interactive()->buttonData(IInteractive::Button::Ok) });
}

void ConvertFileToScoreScenario::showTooManyImagesError(int maxImages)
{
    std::string text = muse::qtrc("project/convert", "You can convert up to %1 images at a time. Remove some images and try again.")
                       .arg(maxImages).toStdString();
    interactive()->error(muse::trc("project/convert", "Too many images selected"), text,
                         { interactive()->buttonData(IInteractive::Button::Ok) });
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
            checkConvertIsAllowed()
            .onResolve(this, [this](const Ret& ret) {
                if (!ret) {
                    return;
                }

                selectFilesToConvert()
                .onResolve(this, [this](const ConvertSelection& selection) {
                    startConvert(selection.input, selection.convertedFileName);
                });
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
