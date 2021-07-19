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
#ifndef MU_USERSCORES_FILESCORECONTROLLER_H
#define MU_USERSCORES_FILESCORECONTROLLER_H

#include "../ifilescorecontroller.h"
#include "iuserscoresconfiguration.h"
#include "modularity/ioc.h"
#include "iinteractive.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "async/asyncable.h"
#include "project/iprojectcreator.h"
#include "context/iglobalcontext.h"
#include "iplatformrecentfilescontroller.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "cloud/iuploadingservice.h"

namespace mu::userscores {
class FileScoreController : public IFileScoreController, public actions::Actionable, public async::Asyncable
{
    INJECT(userscores, actions::IActionsDispatcher, dispatcher)
    INJECT(userscores, framework::IInteractive, interactive)
    INJECT(userscores, project::IProjectCreator, notationCreator)
    INJECT(userscores, context::IGlobalContext, globalContext)
    INJECT(userscores, IUserScoresConfiguration, configuration)
    INJECT(userscores, IPlatformRecentFilesController, platformRecentFilesController)
    INJECT(userscores, mi::IMultiInstancesProvider, multiInstancesProvider)
    INJECT(userscores, cloud::IUploadingService, uploadingService)

public:
    void init();

    Ret openProject(const io::path& projectPath) override;
    bool closeOpenedProject() override;
    bool isProjectOpened(const io::path& scorePath) const override;

private:
    void setupConnections();

    project::INotationProjectPtr currentNotationProject() const;
    notation::IMasterNotationPtr currentMasterNotation() const;
    notation::INotationPtr currentNotation() const;
    notation::INotationInteractionPtr currentInteraction() const;
    notation::INotationSelectionPtr currentNotationSelection() const;

    void openProject(const actions::ActionData& args);
    void importScore();
    void newProject();

    bool checkCanIgnoreError(const Ret& ret, const io::path& filePath);
    framework::IInteractive::Button askAboutSavingScore(const io::path& filePath);

    void saveScore();
    void saveScoreAs();
    void saveScoreCopy();
    void saveSelection();
    void saveOnline();

    void importPdf();

    void clearRecentScores();

    void continueLastSession();

    io::path selectScoreOpeningFile();
    io::path selectScoreSavingFile(const io::path& defaultFilePath, const QString& saveTitle);

    Ret doOpenProject(const io::path& filePath);
    void doSaveScore(const io::path& filePath = io::path(), project::SaveMode saveMode = project::SaveMode::Save);

    void exportScore();

    io::path defaultSavingFilePath() const;

    void prependToRecentScoreList(const io::path& filePath);

    bool isProjectOpened() const;
    bool isNeedSaveScore() const;
    bool hasSelection() const;
};
}

#endif // MU_USERSCORES_FILESCORECONTROLLER_H
