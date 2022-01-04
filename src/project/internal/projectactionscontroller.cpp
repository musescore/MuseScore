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
#include "projectactionscontroller.h"

#include <QBuffer>

#include "translation.h"
#include "defer.h"
#include "notation/notationerrors.h"
#include "projectconfiguration.h"
#include "engraving/infrastructure/io/mscio.h"

#include "log.h"

using namespace mu;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::framework;
using namespace mu::actions;

static mu::Uri NOTATION_PAGE_URI("musescore://notation");
static mu::Uri NEW_SCORE_URI("musescore://project/newscore");

void ProjectActionsController::init()
{
    dispatcher()->reg(this, "file-open", this, &ProjectActionsController::openProject);
    dispatcher()->reg(this, "file-new", this, &ProjectActionsController::newProject);

    dispatcher()->reg(this, "file-close", [this]() {
        bool quitApp = multiInstancesProvider()->instances().size() > 1;
        closeOpenedProject(quitApp);
    });

    dispatcher()->reg(this, "file-save", [this]() { saveProject(); });
    dispatcher()->reg(this, "file-save-as", this, &ProjectActionsController::saveProjectAs);
    dispatcher()->reg(this, "file-save-a-copy", this, &ProjectActionsController::saveProjectCopy);
    dispatcher()->reg(this, "file-save-selection", this, &ProjectActionsController::saveSelection);
    dispatcher()->reg(this, "file-save-online", this, &ProjectActionsController::saveOnline);

    dispatcher()->reg(this, "file-export", this, &ProjectActionsController::exportScore);
    dispatcher()->reg(this, "file-import-pdf", this, &ProjectActionsController::importPdf);

    dispatcher()->reg(this, "print", this, &ProjectActionsController::printScore);

    dispatcher()->reg(this, "clear-recent", this, &ProjectActionsController::clearRecentScores);

    dispatcher()->reg(this, "continue-last-session", this, &ProjectActionsController::continueLastSession);
}

INotationProjectPtr ProjectActionsController::currentNotationProject() const
{
    return globalContext()->currentProject();
}

IMasterNotationPtr ProjectActionsController::currentMasterNotation() const
{
    return currentNotationProject() ? currentNotationProject()->masterNotation() : nullptr;
}

INotationPtr ProjectActionsController::currentNotation() const
{
    return currentMasterNotation() ? currentMasterNotation()->notation() : nullptr;
}

INotationInteractionPtr ProjectActionsController::currentInteraction() const
{
    return currentNotation() ? currentNotation()->interaction() : nullptr;
}

INotationSelectionPtr ProjectActionsController::currentNotationSelection() const
{
    return currentNotation() ? currentInteraction()->selection() : nullptr;
}

bool ProjectActionsController::isFileSupported(const io::path& path) const
{
    std::string suffix = io::suffix(path);
    if (engraving::isMuseScoreFile(suffix)) {
        return true;
    }

    if (readers()->reader(suffix)) {
        return true;
    }

    return false;
}

void ProjectActionsController::openProject(const actions::ActionData& args)
{
    io::path projectPath = !args.empty() ? args.arg<io::path>(0) : "";
    openProject(projectPath);
}

Ret ProjectActionsController::openProject(const io::path& projectPath_)
{
    //! NOTE This method is synchronous,
    //! but inside `multiInstancesProvider` there can be an event loop
    //! to wait for the responces from other instances, accordingly,
    //! the events (like user click) can be executed and this method can be called several times,
    //! before the end of the current call.
    //! So we ignore all subsequent calls until the current one completes.
    if (m_isProjectProcessing) {
        return make_ret(Ret::Code::InternalError);
    }
    m_isProjectProcessing = true;

    Defer defer([this]() {
        m_isProjectProcessing = false;
    });

    //! Step 1. If no path is specified, ask the user to select a project
    io::path projectPath = projectPath_;
    if (projectPath.empty()) {
        projectPath = selectScoreOpeningFile();

        if (projectPath.empty()) {
            return make_ret(Ret::Code::Cancel);
        }
    }

    //! Step 2. If the project is already open in the current window, then just switch to showing the notation
    if (isProjectOpened(projectPath)) {
        return openNotationPageIfNeed();
    }

    //! Step 3. Check, if the project already opened in another window, then activate the window with the project
    if (multiInstancesProvider()->isProjectAlreadyOpened(projectPath)) {
        multiInstancesProvider()->activateWindowWithProject(projectPath);
        return make_ret(Ret::Code::Ok);
    }

    //! Step 4. Check, if a any project is already open in the current window,
    //! then create a new instance
    if (globalContext()->currentProject()) {
        QStringList args;
        args << projectPath.toQString();
        multiInstancesProvider()->openNewAppInstance(args);
        return make_ret(Ret::Code::Ok);
    }

    //! Step 5. Open project in the current window
    return doOpenProject(projectPath);
}

Ret ProjectActionsController::doOpenProject(const io::path& filePath)
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

    return openNotationPageIfNeed();
}

mu::Ret ProjectActionsController::openNotationPageIfNeed()
{
    if (interactive()->isOpened(NOTATION_PAGE_URI).val) {
        return make_ret(Ret::Code::Ok);
    }

    return interactive()->open(NOTATION_PAGE_URI).ret;
}

bool ProjectActionsController::isProjectOpened(const io::path& scorePath) const
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

bool ProjectActionsController::isAnyProjectOpened() const
{
    auto project = globalContext()->currentProject();
    if (project) {
        return true;
    }
    return false;
}

void ProjectActionsController::newProject()
{
    //! NOTE This method is synchronous,
    //! but inside `multiInstancesProvider` there can be an event loop
    //! to wait for the responces from other instances, accordingly,
    //! the events (like user click) can be executed and this method can be called several times,
    //! before the end of the current call.
    //! So we ignore all subsequent calls until the current one completes.
    if (m_isProjectProcessing) {
        return;
    }
    m_isProjectProcessing = true;

    Defer defer([this]() {
        m_isProjectProcessing = false;
    });

    if (globalContext()->currentProject()) {
        //! Check, if any project is already open in the current window
        //! and there is already a created instance without a project, then activate it
        if (multiInstancesProvider()->isHasAppInstanceWithoutProject()) {
            multiInstancesProvider()->activateWindowWithoutProject();
            return;
        }

        //! Otherwise, we will create a new instance
        QStringList args;
        args << "--session-type" << "start-with-new";
        multiInstancesProvider()->openNewAppInstance(args);
        return;
    }

    Ret ret = interactive()->open(NEW_SCORE_URI).ret;

    if (ret) {
        ret = openNotationPageIfNeed();
    }

    if (!ret) {
        LOGE() << ret.toString();
    }
}

bool ProjectActionsController::closeOpenedProject(bool quitApp)
{
    INotationProjectPtr project = currentNotationProject();
    if (!project) {
        return true;
    }

    if (playbackController()->isPlaying()) {
        playbackController()->reset();
    }

    bool result = true;

    if (project->needSave().val) {
        IInteractive::Button btn = askAboutSavingScore(project->path());

        if (btn == IInteractive::Button::Cancel) {
            result = false;
        } else if (btn == IInteractive::Button::Save) {
            result = saveProject();
        } else if (btn == IInteractive::Button::DontSave) {
            result = true;
        }
    }

    if (result) {
        globalContext()->setCurrentProject(nullptr);

        if (quitApp) {
            dispatcher()->dispatch("quit", actions::ActionData::make_arg1<bool>(false));
        }
    }

    return result;
}

IInteractive::Button ProjectActionsController::askAboutSavingScore(const io::path& filePath)
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

bool ProjectActionsController::saveProject(const io::path& path)
{
    if (!currentNotationProject()) {
        LOGW() << "no current project";
        return false;
    }

    if (!currentNotationProject()->created().val) {
        return doSaveScore();
    }

    io::path filePath;
    if (!path.empty()) {
        filePath = path;
    } else {
        io::path defaultFilePath = defaultSavingFilePath();
        filePath = selectScoreSavingFile(defaultFilePath, qtrc("project", "Save score"));
    }

    if (filePath.empty()) {
        return false;
    }

    if (io::suffix(filePath).empty()) {
        filePath = filePath + ProjectConfiguration::DEFAULT_FILE_SUFFIX;
    }

    doSaveScore(filePath);
    return true;
}

void ProjectActionsController::saveProjectAs()
{
    io::path defaultFilePath = defaultSavingFilePath();
    io::path selectedFilePath = selectScoreSavingFile(defaultFilePath, qtrc("project", "Save score"));
    if (selectedFilePath.empty()) {
        return;
    }

    doSaveScore(selectedFilePath, SaveMode::SaveAs);
}

void ProjectActionsController::saveProjectCopy()
{
    io::path defaultFilePath = defaultSavingFilePath();
    io::path selectedFilePath = selectScoreSavingFile(defaultFilePath, qtrc("project", "Save a copy"));
    if (selectedFilePath.empty()) {
        return;
    }

    doSaveScore(selectedFilePath, SaveMode::SaveCopy);
}

void ProjectActionsController::saveSelection()
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

void ProjectActionsController::saveOnline()
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

bool ProjectActionsController::checkCanIgnoreError(const Ret& ret, const io::path& filePath)
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

void ProjectActionsController::importPdf()
{
    interactive()->openUrl("https://musescore.com/import");
}

void ProjectActionsController::clearRecentScores()
{
    configuration()->setRecentProjectPaths({});
    platformRecentFilesController()->clearRecentFiles();
}

void ProjectActionsController::continueLastSession()
{
    io::paths recentScorePaths = configuration()->recentProjectPaths();

    if (recentScorePaths.empty()) {
        return;
    }

    io::path lastScorePath = recentScorePaths.front();
    openProject(lastScorePath);
}

void ProjectActionsController::exportScore()
{
    interactive()->open("musescore://project/export");
}

void ProjectActionsController::printScore()
{
    INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    printProvider()->printNotation(notation);
}

io::path ProjectActionsController::selectScoreOpeningFile()
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

    return interactive()->selectOpeningFile(qtrc("project", "Score"), configuration()->userProjectsPath(), filter.join(";;"));
}

io::path ProjectActionsController::selectScoreSavingFile(const io::path& defaultFilePath, const QString& saveTitle)
{
    QString filter = QObject::tr("MuseScore File") + " (*.mscz)";
    io::path filePath = interactive()->selectSavingFile(saveTitle, defaultFilePath, filter);

    return filePath;
}

bool ProjectActionsController::doSaveScore(const io::path& filePath, project::SaveMode saveMode)
{
    io::path oldPath = currentNotationProject()->metaInfo().filePath;

    Ret ret = currentNotationProject()->save(filePath, saveMode);
    if (!ret) {
        LOGE() << ret.toString();
        return false;
    }

    if (oldPath != filePath) {
        globalContext()->currentMasterNotationChanged().notify();
    }

    prependToRecentScoreList(filePath);
    return true;
}

io::path ProjectActionsController::defaultSavingFilePath() const
{
    ProjectMeta scoreMetaInfo = currentNotationProject()->metaInfo();

    io::path fileName = scoreMetaInfo.title;
    if (fileName.empty()) {
        fileName = scoreMetaInfo.fileName;
    }

    return configuration()->defaultSavingFilePath(fileName);
}

void ProjectActionsController::prependToRecentScoreList(const io::path& filePath)
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

bool ProjectActionsController::isProjectOpened() const
{
    return currentNotationProject() != nullptr;
}

bool ProjectActionsController::isNeedSaveScore() const
{
    return currentNotationProject() && currentNotationProject()->needSave().val;
}

bool ProjectActionsController::hasSelection() const
{
    return currentNotationSelection() ? !currentNotationSelection()->isNone() : false;
}
