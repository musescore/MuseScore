/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited and others
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

#include "scorespagemodel.h"

#include <QString>

#include "project/projectcommands.h"
#include "log.h"

using namespace mu::project;

ScoresPageModel::ScoresPageModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void ScoresPageModel::createNewScore()
{
    commandDispatcher()->dispatch(project::PROJECT_NEW_COMMAND);
}

void ScoresPageModel::convertFileToScore()
{
    commandDispatcher()->dispatch(project::PROJECT_CONVERT_TO_SCORE_COMMAND);
}

void ScoresPageModel::openOther()
{
    commandDispatcher()->dispatch(project::PROJECT_OPEN_COMMAND);
}

void ScoresPageModel::openScore(const QString& scorePath, const QString& displayNameOverride)
{
    commandDispatcher()->dispatch(project::PROJECT_OPEN_COMMAND, { { "url", muse::Val(QUrl::fromLocalFile(
                                                                                          scorePath).toString()) }, { "display_name", muse::Val(
                                                                                                                          displayNameOverride) } });
}

void ScoresPageModel::revealInFileBrowser(const QString& scorePath)
{
    muse::Ret ret = platformInteractive()->revealInFileBrowser(scorePath);
    if (!ret) {
        LOGE() << ret.toString();
    }
}

void ScoresPageModel::viewOnline(int scoreId)
{
    if (scoreId <= 0) {
        return;
    }

    museScoreComService()->downloadScoreInfo(scoreId)
    .onResolve(this, [this, scoreId](const muse::RetVal<muse::cloud::ScoreInfo>& scoreInfo) {
        if (!scoreInfo.ret) {
            LOGE() << scoreInfo.ret.toString();
            return;
        }

        QUrl scoreUrl = QUrl::fromUserInput(scoreInfo.val.url);
        if (!scoreUrl.isValid() || scoreUrl.isEmpty()) {
            LOGE() << "Invalid score URL for cloud score" << scoreId << ":" << scoreInfo.val.url;
            return;
        }

        platformInteractive()->openUrl(scoreUrl);
    });
}

void ScoresPageModel::deleteScore(int scoreId, const QString& scoreName)
{
    deleteCloudScoreScenario()->deleteScore(scoreId, scoreName);
}

void ScoresPageModel::openScoreManager()
{
    platformInteractive()->openUrl(museScoreComService()->scoreManagerUrl());
}

int ScoresPageModel::tabIndex() const
{
    return configuration()->homeScoresPageTabIndex();
}

void ScoresPageModel::setTabIndex(int index)
{
    if (index == tabIndex()) {
        return;
    }

    configuration()->setHomeScoresPageTabIndex(index);
    emit tabIndexChanged();
}

ScoresPageModel::ViewType ScoresPageModel::viewType() const
{
    return static_cast<ViewType>(configuration()->homeScoresPageViewType());
}

void ScoresPageModel::setViewType(ViewType type)
{
    if (viewType() == type) {
        return;
    }

    configuration()->setHomeScoresPageViewType(IProjectConfiguration::HomeScoresPageViewType(type));
    emit viewTypeChanged();
}
