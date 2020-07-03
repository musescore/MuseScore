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
#include "scoresmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "view/scoresmodel.h"
#include "internal/openscorecontroller.h"
#include "internal/scoresconfiguration.h"

using namespace mu::scores;

static OpenScoreController* m_openController = new OpenScoreController();
static ScoresConfiguration* m_scoresConfiguration = new ScoresConfiguration();

static void scores_init_qrc()
{
    Q_INIT_RESOURCE(scores);
}

std::string ScoresModule::moduleName() const
{
    return "scores";
}

void ScoresModule::registerExports()
{
    framework::ioc()->registerExport<IOpenScoreController>(moduleName(), m_openController);
    framework::ioc()->registerExport<IScoresConfiguration>(moduleName(), m_scoresConfiguration);
}

void ScoresModule::registerResources()
{
    scores_init_qrc();
}

void ScoresModule::registerUiTypes()
{
    qmlRegisterType<ScoresModel>("MuseScore.Scores", 1, 0, "ScoresModel");
}

void ScoresModule::onInit()
{
    m_scoresConfiguration->init();
    m_openController->init();
}
