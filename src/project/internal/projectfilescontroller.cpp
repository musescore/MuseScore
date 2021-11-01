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
#include "projectfilescontroller.h"

#include <QBuffer>
#include <QFileOpenEvent>

#include "translation.h"
#include "notation/notationerrors.h"
#include "projectconfiguration.h"

#include "log.h"

using namespace mu;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::framework;
using namespace mu::actions;

void ProjectFilesController::init()
{
    qApp->installEventFilter(this);

    dispatcher()->reg(this, "file-open", this, &ProjectFilesController::openProject);
    dispatcher()->reg(this, "file-new", this, &ProjectFilesController::newProject);
    dispatcher()->reg(this, "file-close", [this]() { closeOpenedProject(); });

    dispatcher()->reg(this, "file-save", [this]() { saveProject(); });
    dispatcher()->reg(this, "file-save-as", this, &ProjectFilesController::saveProjectAs);
    dispatcher()->reg(this, "file-save-a-copy", this, &ProjectFilesController::saveProjectCopy);
    dispatcher()->reg(this, "file-save-selection", this, &ProjectFilesController::saveSelection);
    dispatcher()->reg(this, "file-save-online", this, &ProjectFilesController::saveOnline);

    dispatcher()->reg(this, "file-export", this, &ProjectFilesController::exportScore);
    dispatcher()->reg(this, "file-import-pdf", this, &ProjectFilesController::importPdf);

    dispatcher()->reg(this, "clear-recent", this, &ProjectFilesController::clearRecentScores);

    dispatcher()->reg(this, "continue-last-session", this, &ProjectFilesController::continueLastSession);
}

bool ProjectFilesController::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == qApp && event->type() == QEvent::FileOpen) {
        QFileOpenEvent* openEvent = static_cast<QFileOpenEvent*>(event);
        openProject(openEvent->file());
    }

    return QObject::eventFilter(watched, event);
}

INotationProjectPtr ProjectFilesController::currentNotationProject() const
{
    return globalContext()->currentProject();
}

IMasterNotationPtr ProjectFilesController::currentMasterNotation() const
{
    return currentNotationProject() ? currentNotationProject()->masterNotation() : nullptr;
}

INotationPtr ProjectFilesController::currentNotation() const
{
    return currentMasterNotation() ? currentMasterNotation()->notation() : nullptr;
}

INotationInteractionPtr ProjectFilesController::currentInteraction() const
{
    return currentNotation() ? currentNotation()->interaction() : nullptr;
}

INotationSelectionPtr ProjectFilesController::currentNotationSelection() const
{
    return currentNotation() ? currentInteraction()->selection() : nullptr;
}

void ProjectFilesController::openProject(const actions::ActionData& args)
{
    io::path projectPath = !args.empty() ? args.arg<io::path>(0) : "";
    openProject(projectPath);
}

Ret ProjectFilesController::openProject(const io::path& projectPath_)
{
    //! Step 1. If no path is specified, ask the user to select a project
    io::path projectPath = projectPath_;
    if (projectPath.empty()) {
        projectPath = selectScoreOpeningFile();

        if (projectPath.empty()) {
            return make_ret(Ret::Code::Cancel);
        }
    }

    //! Sеep 2. If the project is already open in the current window, then just switch to showing the notation
    if (isProjectOpened(projectPath)) {
        if (!interactive()->isOpened("musescore://notation").val) {
            interactive()->open("musescore://notation");
        }
        return make_ret(Ret::Code::Ok);
    }

    //! Step 3. Check, if the project already opened in another window, then activate the window with the project
    if (multiInstancesProvider()->isProjectAlreadyOpened(projectPath)) {
        multiInstancesProvider()->activateWindowWithProject(projectPath);
        return make_ret(Ret::Code::Ok);
    }

    //! Step 4. Check, if a any project already opened in the current window,
    //! then we open new window
    if (globalContext()->currentProject()) {
        QStringList args;
        args << projectPath.toQString();
        multiInstancesProvider()->openNewAppInstance(args);
        return make_ret(Ret::Code::Ok);
    }

    //! Step 5. Open project in the current window
    return doOpenProject(projectPath);
}

Ret ProjectFilesController::doOpenProject(const io::path& filePath)
{
    TRACEFUNC;

    auto project = projectCreator()->newProject();
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

    globalContext()->setCurrentProject(project);

    prependToRecentScoreList(filePath);

    interactive()->open("musescore://notation");

    return make_ret(Ret::Code::Ok);
}

bool ProjectFilesController::isProjectOpened(const io::path& scorePath) const
{
    auto project = globalContext()->currentProject();
    if (!project) {
        return false;
    }

    LOGD() << "project->path: " << project->path() << ", check path: " << scorePath;
    if (project->path() == scorePath) {
        return true;
    }

    return false;
}

void ProjectFilesController::newProject()
{
    //! Check, if a any project already opened in the current window,
    //! then we open new window
    if (globalContext()->currentProject()) {
        QStringList args;
        args << "--session-type" << "start-with-new";
        multiInstancesProvider()->openNewAppInstance(args);
        return;
    }

    Ret ret = interactive()->open("musescore://project/newscore").ret;

    if (ret) {
        ret = interactive()->open("musescore://notation").ret;
    }

    if (!ret) {
        LOGE() << ret.toString();
    }
}

bool ProjectFilesController::closeOpenedProject()
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
            saveProject();
        }
    }

    globalContext()->setCurrentProject(nullptr);
    return true;
}

IInteractive::Button ProjectFilesController::askAboutSavingScore(const io::path& filePath)
{
    QString scoreName = qtrc("project", "Untitled");
    if (!filePath.empty()) {
        scoreName = io::completebasename(filePath).toQString();
    }
    std::string title = qtrc("project", "Do you want to save changes to the score “%1” before closing?")
                        .arg(scoreName).toStdString();

    std::string body = trc("project", "Your changes will be lost if you don’t save them.");

    IInteractive::Options options {
        IInteractive::Option::WithIcon
    };

    IInteractive::Result result = interactive()->warning(title, body, {
        IInteractive::Button::DontSave,
        IInteractive::Button::Cancel,
        IInteractive::Button::Save
    }, IInteractive::Button::Save, options);

    return result.standardButton();
}

void ProjectFilesController::saveProject(const io::path& path)
{
    if (!currentNotationProject()) {
        LOGW() << "no current project";
        return;
    }

    if (!currentNotationProject()->created().val) {
        doSaveScore();
        return;
    }

    io::path filePath;
    if (!path.empty()) {
        filePath = path;
    } else {
        io::path defaultFilePath = defaultSavingFilePath();
        filePath = selectScoreSavingFile(defaultFilePath, qtrc("project", "Save score"));
    }

    if (filePath.empty()) {
        return;
    }

    if (io::suffix(filePath).empty()) {
        filePath = filePath + ProjectConfiguration::DEFAULT_FILE_SUFFIX;
    }

    doSaveScore(filePath);
}

void ProjectFilesController::saveProjectAs()
{
    io::path defaultFilePath = defaultSavingFilePath();
    io::path selectedFilePath = selectScoreSavingFile(defaultFilePath, qtrc("project", "Save score"));
    if (selectedFilePath.empty()) {
        return;
    }

    doSaveScore(selectedFilePath, SaveMode::SaveAs);
}

void ProjectFilesController::saveProjectCopy()
{
    io::path defaultFilePath = defaultSavingFilePath();
    io::path selectedFilePath = selectScoreSavingFile(defaultFilePath, qtrc("project", "Save a copy"));
    if (selectedFilePath.empty()) {
        return;
    }

    doSaveScore(selectedFilePath, SaveMode::SaveCopy);
}

void ProjectFilesController::saveSelection()
{
    io::path defaultFilePath = defaultSavingFilePath();
    io::path selectedFilePath = selectScoreSavingFile(defaultFilePath, qtrc("project", "Save selection"));
    if (selectedFilePath.empty()) {
        return;
    }

    Ret ret = currentNotationProject()->save(selectedFilePath, SaveMode::SaveSelection);
    if (!ret) {
        LOGE() << ret.toString();
    }
}

void ProjectFilesController::saveOnline()
{
    INotationProjectPtr project = globalContext()->currentProject();
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

        ProjectMeta meta = project->metaInfo();
        if (meta.source == newSource) {
            return;
        }

        meta.source = newSource;
        project->setMetaInfo(meta);

        if (project->created().val) {
            project->save();
        }
    }, Asyncable::AsyncMode::AsyncSetRepeat);

    ProjectMeta meta = project->metaInfo();
    uploadingService()->uploadScore(*projectData, meta.title, meta.source);
}

bool ProjectFilesController::checkCanIgnoreError(const Ret& ret, const io::path& filePath)
{
    static const QList<notation::Err> ignorableErrors {
        notation::Err::FileTooOld,
        notation::Err::FileTooNew,
        notation::Err::FileCorrupted,
        notation::Err::FileOld300Format
    };

    std::string title = trc("project", "Open Error");
    std::string body = qtrc("project", "Cannot open file %1:\n%2")
                       .arg(filePath.toQString())
                       .arg(QString::fromStdString(ret.text())).toStdString();

    IInteractive::Options options;
    options.setFlag(IInteractive::Option::WithIcon);

    bool canIgnore = ignorableErrors.contains(static_cast<notation::Err>(ret.code()));

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

    return result.standardButton() == IInteractive::Button::Ignore;
}

void ProjectFilesController::importPdf()
{
    interactive()->openUrl("https://musescore.com/import");
}

void ProjectFilesController::clearRecentScores()
{
    configuration()->setRecentProjectPaths({});
    platformRecentFilesController()->clearRecentFiles();
}

void ProjectFilesController::continueLastSession()
{
    io::paths recentScorePaths = configuration()->recentProjectPaths();

    if (recentScorePaths.empty()) {
        return;
    }

    io::path lastScorePath = recentScorePaths.front();
    openProject(lastScorePath);
}

void ProjectFilesController::exportScore()
{
    interactive()->open("musescore://project/export");
}

io::path ProjectFilesController::selectScoreOpeningFile()
{
    QString allExt = "*.mscz *.mxl *.musicxml *.xml *.mid *.midi *.kar *.md *.mgu *.sgu *.cap *.capx"
                     "*.ove *.scw *.bmw *.bww *.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp *.ptb *.mscx *.mscs *.mscz~";

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
           << QObject::tr("MuseScore Backup Files") + " (*.mscz~)";

    return interactive()->selectOpeningFile(qtrc("project", "Score"), "", filter.join(";;"));
}

io::path ProjectFilesController::selectScoreSavingFile(const io::path& defaultFilePath, const QString& saveTitle)
{
    QString filter = QObject::tr("MuseScore File") + " (*.mscz)";
    io::path filePath = interactive()->selectSavingFile(saveTitle, defaultFilePath, filter);

    return filePath;
}

void ProjectFilesController::doSaveScore(const io::path& filePath, project::SaveMode saveMode)
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

io::path ProjectFilesController::defaultSavingFilePath() const
{
    ProjectMeta scoreMetaInfo = currentNotationProject()->metaInfo();

    io::path fileName = scoreMetaInfo.title;
    if (fileName.empty()) {
        fileName = scoreMetaInfo.fileName;
    }

    return configuration()->defaultSavingFilePath(fileName);
}

void ProjectFilesController::prependToRecentScoreList(const io::path& filePath)
{
    if (filePath.empty()) {
        return;
    }

    io::paths recentScorePaths = configuration()->recentProjectPaths();

    auto it = std::find(recentScorePaths.begin(), recentScorePaths.end(), filePath);
    if (it != recentScorePaths.end()) {
        recentScorePaths.erase(it);
    }

    recentScorePaths.insert(recentScorePaths.begin(), filePath);
    configuration()->setRecentProjectPaths(recentScorePaths);
    platformRecentFilesController()->addRecentFile(filePath);
}

bool ProjectFilesController::isProjectOpened() const
{
    return currentNotationProject() != nullptr;
}

bool ProjectFilesController::isNeedSaveScore() const
{
    return currentNotationProject() && currentNotationProject()->needSave().val;
}

bool ProjectFilesController::hasSelection() const
{
    return currentNotationSelection() ? !currentNotationSelection()->isNone() : false;
}
