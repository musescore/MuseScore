/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "filescorecontroller.h"

#include <QObject>
#include <QBuffer>

#include "translation.h"
#include "notation/notationerrors.h"

#include "userscoresconfiguration.h"

#include "log.h"

using namespace mu;
using namespace mu::userscores;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::framework;
using namespace mu::actions;

void FileScoreController::init()
{
    dispatcher()->reg(this, "file-open", this, &FileScoreController::openProject);
    dispatcher()->reg(this, "file-new", this, &FileScoreController::newProject);
    dispatcher()->reg(this, "file-close", [this]() { closeOpenedProject(); });

    dispatcher()->reg(this, "file-save", this, &FileScoreController::saveScore);
    dispatcher()->reg(this, "file-save-as", this, &FileScoreController::saveScoreAs);
    dispatcher()->reg(this, "file-save-a-copy", this, &FileScoreController::saveScoreCopy);
    dispatcher()->reg(this, "file-save-selection", this, &FileScoreController::saveSelection);
    dispatcher()->reg(this, "file-save-online", this, &FileScoreController::saveOnline);

    dispatcher()->reg(this, "file-export", this, &FileScoreController::exportScore);
    dispatcher()->reg(this, "file-import-pdf", this, &FileScoreController::importPdf);

    dispatcher()->reg(this, "clear-recent", this, &FileScoreController::clearRecentScores);

    dispatcher()->reg(this, "continue-last-session", this, &FileScoreController::continueLastSession);
}

INotationProjectPtr FileScoreController::currentNotationProject() const
{
    return globalContext()->currentNotationProject();
}

IMasterNotationPtr FileScoreController::currentMasterNotation() const
{
    return currentNotationProject() ? currentNotationProject()->masterNotation() : nullptr;
}

INotationPtr FileScoreController::currentNotation() const
{
    return currentMasterNotation() ? currentMasterNotation()->notation() : nullptr;
}

INotationInteractionPtr FileScoreController::currentInteraction() const
{
    return currentNotation() ? currentNotation()->interaction() : nullptr;
}

INotationSelectionPtr FileScoreController::currentNotationSelection() const
{
    return currentNotation() ? currentInteraction()->selection() : nullptr;
}

Ret FileScoreController::openProject(const io::path& projectPath)
{
    return doOpenProject(projectPath);
}

bool FileScoreController::isProjectOpened(const io::path& scorePath) const
{
    auto project = globalContext()->currentNotationProject();
    if (!project) {
        return false;
    }

    LOGD() << "project->path: " << project->path() << ", check path: " << scorePath;
    if (project->path() == scorePath) {
        return true;
    }

    return false;
}

void FileScoreController::openProject(const actions::ActionData& args)
{
    io::path scorePath = args.count() > 0 ? args.arg<io::path>(0) : "";

    if (scorePath.empty()) {
        scorePath = selectScoreOpeningFile();

        if (scorePath.empty()) {
            return;
        }
    }

    doOpenProject(scorePath);
}

void FileScoreController::newProject()
{
    Ret ret = interactive()->open("musescore://userscores/newscore").ret;

    if (ret) {
        ret = interactive()->open("musescore://notation").ret;
    }

    if (!ret) {
        LOGE() << ret.toString();
    }
}

bool FileScoreController::closeOpenedProject()
{
    INotationProjectPtr project = currentNotationProject();
    if (!project) {
        return true;
    }

    if (project->needSave().val) {
        IInteractive::Button btn = askAboutSavingScore(project->path());

        if (btn == IInteractive::Button::Cancel) {
            return false;
        } else if (btn == IInteractive::Button::Save) {
            saveScore();
        }
    }

    globalContext()->setCurrentNotationProject(nullptr);
    return true;
}

IInteractive::Button FileScoreController::askAboutSavingScore(const io::path& filePath)
{
    if (!configuration()->needShowWarningAboutUnsavedScore()) {
        return IInteractive::Button::DontSave;
    }

    std::string title = qtrc("userscores", "Do you want to save changes to the score “%1” before closing?")
                        .arg(io::completebasename(filePath).toQString()).toStdString();

    std::string body = trc("userscores", "Your changes will be lost if you don’t save them.");

    IInteractive::Options options {
        IInteractive::Option::WithIcon | IInteractive::Option::WithShowAgain
    };

    IInteractive::Result result = interactive()->warning(title, body, {
        IInteractive::Button::DontSave,
        IInteractive::Button::Cancel,
        IInteractive::Button::Save
    }, IInteractive::Button::Save, options);

    configuration()->setNeedShowWarningAboutUnsavedScore(result.showAgain());

    return result.standartButton();
}

void FileScoreController::saveScore()
{
    if (!currentNotationProject()->created().val) {
        doSaveScore();
        return;
    }

    io::path defaultFilePath = defaultSavingFilePath();

    io::path filePath = selectScoreSavingFile(defaultFilePath, qtrc("userscores", "Save Score"));
    if (filePath.empty()) {
        return;
    }

    if (io::suffix(filePath).empty()) {
        filePath = filePath + UserScoresConfiguration::DEFAULT_FILE_SUFFIX;
    }

    doSaveScore(filePath);
}

void FileScoreController::saveScoreAs()
{
    io::path defaultFilePath = defaultSavingFilePath();
    io::path selectedFilePath = selectScoreSavingFile(defaultFilePath, qtrc("userscores", "Save Score"));
    if (selectedFilePath.empty()) {
        return;
    }

    doSaveScore(selectedFilePath, SaveMode::SaveAs);
}

void FileScoreController::saveScoreCopy()
{
    io::path defaultFilePath = defaultSavingFilePath();
    io::path selectedFilePath = selectScoreSavingFile(defaultFilePath, qtrc("userscores", "Save a Copy"));
    if (selectedFilePath.empty()) {
        return;
    }

    doSaveScore(selectedFilePath, SaveMode::SaveCopy);
}

void FileScoreController::saveSelection()
{
    io::path defaultFilePath = defaultSavingFilePath();
    io::path selectedFilePath = selectScoreSavingFile(defaultFilePath, qtrc("userscores", "Save Selection"));
    if (selectedFilePath.empty()) {
        return;
    }

    Ret ret = currentNotationProject()->save(selectedFilePath, SaveMode::SaveSelection);
    if (!ret) {
        LOGE() << ret.toString();
    }
}

void FileScoreController::saveOnline()
{
    INotationProjectPtr project = globalContext()->currentNotationProject();
    if (!project) {
        return;
    }

    QBuffer* projectData = new QBuffer();
    projectData->open(QIODevice::WriteOnly);

    Ret ret = project->writeToDevice(projectData);
    if (!ret) {
        LOGE() << ret.toString();
        delete projectData;
        return;
    }

    projectData->close();

    projectData->open(QIODevice::ReadOnly);

    ProgressChannel progressCh = uploadingService()->progressChannel();
    progressCh.onReceive(this, [](const Progress& progress) {
        LOGD() << "Uploading progress: " << progress.current << "/" << progress.total;
    }, Asyncable::AsyncMode::AsyncSetRepeat);

    async::Channel<QUrl> sourceUrlCh = uploadingService()->sourceUrlReceived();
    sourceUrlCh.onReceive(this, [project, projectData](const QUrl& url) {
        projectData->deleteLater();

        LOGD() << "Source url received: " << url;
        QString newSource = url.toString();

        Meta meta = project->metaInfo();
        if (meta.source == newSource) {
            return;
        }

        meta.source = newSource;
        project->setMetaInfo(meta);

        if (project->created().val) {
            project->save();
        }
    }, Asyncable::AsyncMode::AsyncSetRepeat);

    Meta meta = project->metaInfo();
    uploadingService()->uploadScore(*projectData, meta.title, meta.source);
}

bool FileScoreController::checkCanIgnoreError(const Ret& ret, const io::path& filePath)
{
    static const QList<Err> ignorableErrors {
        Err::FileTooOld,
        Err::FileTooNew,
        Err::FileCorrupted,
        Err::FileOld300Format
    };

    std::string title = trc("userscores", "Open Error");
    std::string body = qtrc("userscores", "Cannot open file %1:\n%2")
                       .arg(filePath.toQString())
                       .arg(QString::fromStdString(ret.text())).toStdString();

    IInteractive::Options options;
    options.setFlag(IInteractive::Option::WithIcon);

    bool canIgnore = ignorableErrors.contains(static_cast<Err>(ret.code()));

    if (!canIgnore) {
        interactive()->error(title, body, {
            IInteractive::Button::Ok
        }, IInteractive::Button::Ok, options);

        return false;
    }

    IInteractive::Result result = interactive()->warning(title, body, {
        IInteractive::Button::Cancel,
        IInteractive::Button::Ignore
    }, IInteractive::Button::Ignore, options);

    return result.standartButton() == IInteractive::Button::Ignore;
}

void FileScoreController::importPdf()
{
    interactive()->openUrl("https://musescore.com/import");
}

void FileScoreController::clearRecentScores()
{
    configuration()->setRecentScorePaths({});
    platformRecentFilesController()->clearRecentFiles();
}

void FileScoreController::continueLastSession()
{
    io::paths recentScorePaths = configuration()->recentScorePaths().val;

    if (recentScorePaths.empty()) {
        return;
    }

    io::path lastScorePath = recentScorePaths.front();
    openProject(lastScorePath);
}

void FileScoreController::exportScore()
{
    interactive()->open("musescore://userscores/export");
}

io::path FileScoreController::selectScoreOpeningFile()
{
    QString allExt = "*.mscz *.mxl *.musicxml *.xml *.mid *.midi *.kar *.md *.mgu *.sgu *.cap *.capx"
                     "*.ove *.scw *.bmw *.bww *.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp *.ptb *.mscx *.mscs *.mscz,";

    QStringList filter;
    filter << QObject::tr("All Supported Files") + " (" + allExt + ")"
           << QObject::tr("MuseScore File") + " (*.mscz)"
           << QObject::tr("MusicXML Files") + " (*.mxl *.musicxml *.xml)"
           << QObject::tr("MIDI Files") + " (*.mid *.midi *.kar)"
           << QObject::tr("MuseData Files") + " (*.md)"
           << QObject::tr("Capella Files") + " (*.cap *.capx)"
           << QObject::tr("BB Files (experimental)") + " (*.mgu *.sgu)"
           << QObject::tr("Overture / Score Writer Files (experimental)") + " (*.ove *.scw)"
           << QObject::tr("Bagpipe Music Writer Files (experimental)") + " (*.bmw *.bww)"
           << QObject::tr("Guitar Pro Files") + " (*.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp)"
           << QObject::tr("Power Tab Editor Files (experimental)") + " (*.ptb)"
           << QObject::tr("MuseScore Unpack Files") + " (*.mscx)"
           << QObject::tr("MuseScore Dev Files") + " (*.mscs)"
           << QObject::tr("MuseScore Backup Files") + " (*.mscz,)";

    return interactive()->selectOpeningFile(qtrc("userscores", "Score"), "", filter.join(";;"));
}

io::path FileScoreController::selectScoreSavingFile(const io::path& defaultFilePath, const QString& saveTitle)
{
    QString filter = QObject::tr("MuseScore File") + " (*.mscz)";
    io::path filePath = interactive()->selectSavingFile(saveTitle, defaultFilePath, filter);

    return filePath;
}

Ret FileScoreController::doOpenProject(const io::path& filePath)
{
    TRACEFUNC;

    if (multiInstancesProvider()->isScoreAlreadyOpened(filePath)) {
        multiInstancesProvider()->activateWindowWithScore(filePath);
        return make_ret(Ret::Code::Ok);
    }

    auto project = notationCreator()->newNotationProject();
    IF_ASSERT_FAILED(project) {
        return make_ret(Ret::Code::InternalError);
    }

    Ret ret = project->load(filePath);

    if (!ret && checkCanIgnoreError(ret, filePath)) {
        constexpr auto NO_STYLE = "";
        constexpr bool FORCE_MODE = true;
        ret = project->load(filePath, NO_STYLE, FORCE_MODE);
    }

    if (!ret) {
        return ret;
    }

    if (!globalContext()->containsNotationProject(filePath)) {
        globalContext()->addNotationProject(project);
    }

    globalContext()->setCurrentNotationProject(project);

    prependToRecentScoreList(filePath);

    interactive()->open("musescore://notation");

    return make_ret(Ret::Code::Ok);
}

void FileScoreController::doSaveScore(const io::path& filePath, project::SaveMode saveMode)
{
    io::path oldPath = currentNotationProject()->metaInfo().filePath;

    Ret ret = currentNotationProject()->save(filePath, saveMode);
    if (!ret) {
        LOGE() << ret.toString();
        return;
    }

    if (saveMode == SaveMode::SaveAs && oldPath != filePath) {
        globalContext()->currentMasterNotationChanged().notify();
    }

    prependToRecentScoreList(filePath);
}

io::path FileScoreController::defaultSavingFilePath() const
{
    Meta scoreMetaInfo = currentNotationProject()->metaInfo();

    io::path fileName = scoreMetaInfo.title;
    if (fileName.empty()) {
        fileName = scoreMetaInfo.fileName;
    }

    return configuration()->defaultSavingFilePath(fileName);
}

void FileScoreController::prependToRecentScoreList(const io::path& filePath)
{
    if (filePath.empty()) {
        return;
    }

    io::paths recentScorePaths = configuration()->recentScorePaths().val;

    auto it = std::find(recentScorePaths.begin(), recentScorePaths.end(), filePath);
    if (it != recentScorePaths.end()) {
        recentScorePaths.erase(it);
    }

    recentScorePaths.insert(recentScorePaths.begin(), filePath);
    configuration()->setRecentScorePaths(recentScorePaths);
    platformRecentFilesController()->addRecentFile(filePath);
}

bool FileScoreController::isProjectOpened() const
{
    return currentNotationProject() != nullptr;
}

bool FileScoreController::isNeedSaveScore() const
{
    return currentNotationProject() && currentNotationProject()->needSave().val;
}

bool FileScoreController::hasSelection() const
{
    return currentNotationSelection() ? !currentNotationSelection()->isNone() : false;
}
