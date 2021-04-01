//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_USERSCORES_FILESCORECONTROLLER_H
#define MU_USERSCORES_FILESCORECONTROLLER_H

#include "../ifilescorecontroller.h"
#include "iuserscoresconfiguration.h"
#include "modularity/ioc.h"
#include "iinteractive.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "async/asyncable.h"
#include "notation/inotationcreator.h"
#include "context/iglobalcontext.h"

namespace mu::userscores {
class FileScoreController : public IFileScoreController, public actions::Actionable, public async::Asyncable
{
    INJECT(scores, actions::IActionsDispatcher, dispatcher)
    INJECT(scores, framework::IInteractive, interactive)
    INJECT(scores, notation::INotationCreator, notationCreator)
    INJECT(scores, context::IGlobalContext, globalContext)
    INJECT(scores, IUserScoresConfiguration, configuration)

public:
    void init();

    Ret openScore(const io::path& scorePath) override;

private:
    void setupConnections();

    notation::IMasterNotationPtr currentMasterNotation() const;
    notation::INotationPtr currentNotation() const;
    notation::INotationInteractionPtr currentInteraction() const;
    notation::INotationSelectionPtr currentNotationSelection() const;

    void openScore(const actions::ActionData& args);
    void importScore();
    void newScore();

    void saveScore();
    void saveScoreAs();
    void saveScoreCopy();
    void saveSelection();

    void importPdf();

    void clearRecentScores();

    void continueLastSession();

    io::path selectScoreOpenningFile(const QStringList& filter);
    io::path selectScoreSavingFile(const io::path& defaultFilePath, const QString& saveTitle);
    Ret doOpenScore(const io::path& filePath);
    void doSaveScore(const io::path& filePath = io::path(), notation::SaveMode saveMode = notation::SaveMode::Save);

    io::path defaultSavingFilePath() const;

    void prependToRecentScoreList(const io::path& filePath);

    bool isScoreOpened() const;
    bool isNeedSaveScore() const;
    bool hasSelection() const;
};
}

#endif // MU_USERSCORES_FILESCORECONTROLLER_H
