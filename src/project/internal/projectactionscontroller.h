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
#ifndef MU_PROJECT_PROJECTACTIONSCONTROLLER_H
#define MU_PROJECT_PROJECTACTIONSCONTROLLER_H

#include "iprojectfilescontroller.h"

#include <QObject>

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "context/iglobalcontext.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "cloud/iuploadingservice.h"
#include "playback/iplaybackcontroller.h"
#include "print/iprintprovider.h"
#include "inotationreadersregister.h"

#include "async/asyncable.h"

#include "iprojectconfiguration.h"
#include "iprojectcreator.h"
#include "iplatformrecentfilescontroller.h"
#include "iprojectautosaver.h"

namespace mu::project {
class ProjectActionsController : public IProjectFilesController, public QObject, public actions::Actionable, public async::Asyncable
{
    INJECT(project, IProjectConfiguration, configuration)
    INJECT(project, INotationReadersRegister, readers)
    INJECT(project, IProjectCreator, projectCreator)
    INJECT(project, IPlatformRecentFilesController, platformRecentFilesController)
    INJECT(project, IProjectAutoSaver, projectAutoSaver)

    INJECT(project, actions::IActionsDispatcher, dispatcher)
    INJECT(project, framework::IInteractive, interactive)
    INJECT(project, context::IGlobalContext, globalContext)
    INJECT(project, mi::IMultiInstancesProvider, multiInstancesProvider)
    INJECT(project, cloud::IUploadingService, uploadingService)
    INJECT(project, playback::IPlaybackController, playbackController)
    INJECT(project, print::IPrintProvider, printProvider)

public:
    void init();

    bool isFileSupported(const io::path& path) const override;
    Ret openProject(const io::path& projectPath) override;
    bool closeOpenedProject(bool quitApp = false) override;
    bool isProjectOpened(const io::path& scorePath) const override;
    bool isAnyProjectOpened() const override;
    bool saveProject(const io::path& path = io::path()) override;

private:
    void setupConnections();

    project::INotationProjectPtr currentNotationProject() const;
    notation::IMasterNotationPtr currentMasterNotation() const;
    notation::INotationPtr currentNotation() const;
    notation::INotationInteractionPtr currentInteraction() const;
    notation::INotationSelectionPtr currentNotationSelection() const;

    void openProject(const actions::ActionData& args);
    void newProject();

    bool checkCanIgnoreError(const Ret& ret, const io::path& filePath);
    framework::IInteractive::Button askAboutSavingScore(const io::path& filePath);

    void saveProjectAs();
    void saveProjectCopy();
    void saveSelection();
    void saveOnline();

    void importPdf();

    void clearRecentScores();

    void continueLastSession();

    io::path selectScoreOpeningFile();
    io::path selectScoreSavingFile(const io::path& defaultFilePath, const QString& saveTitle);

    Ret doOpenProject(const io::path& filePath);
    bool doSaveScore(const io::path& filePath = io::path(), project::SaveMode saveMode = project::SaveMode::Save);

    Ret openNotationPageIfNeed();

    void exportScore();
    void printScore();

    io::path defaultSavingFilePath() const;

    void prependToRecentScoreList(const io::path& filePath);

    bool isProjectOpened() const;
    bool isNeedSaveScore() const;
    bool hasSelection() const;

    bool m_isProjectProcessing = false;
};
}

#endif // MU_PROJECT_PROJECTACTIONSCONTROLLER_H
