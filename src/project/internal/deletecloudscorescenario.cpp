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
#include "deletecloudscorescenario.h"

#include <QString>
#include <QVariantMap>

#include "translation.h"
#include "types/uri.h"
#include "types/val.h"

#include "log.h"

using namespace mu::project;
using namespace muse;

static const Uri DELETE_CLOUD_SCORE_URI("musescore://project/deletecloudscore");

DeleteCloudScoreScenario::DeleteCloudScoreScenario(const modularity::ContextPtr& iocCtx)
    : Contextable(iocCtx)
{
}

void DeleteCloudScoreScenario::deleteScore(int scoreId, const QString& scoreName)
{
    if (scoreId <= 0) {
        return;
    }

    UriQuery query(DELETE_CLOUD_SCORE_URI);
    query.addParam("scoreName", Val(scoreName));

    interactive()->open(query)
    .onResolve(this, [this, scoreId](const Val& val) {
        const QVariantMap map = val.toQVariant().toMap();
        if (map.value("agreesToDelete").toBool()) {
            doDeleteScore(scoreId);
        }
    });
}

async::Channel<int> DeleteCloudScoreScenario::scoreDeleted() const
{
    return m_scoreDeleted;
}

void DeleteCloudScoreScenario::doDeleteScore(int scoreId)
{
    museScoreComService()->deleteScore(scoreId)
    .onResolve(this, [this, scoreId](const Ret& ret) {
        if (!ret) {
            LOGE() << ret.toString();
            interactive()->error(muse::trc("project/cloud", "Unable to delete the score"),
                                 muse::trc("global", "Please check your internet connection or try again later."),
                                 { IInteractive::Button::Ok });
            return;
        }

        const io::path_t localPath = configuration()->cloudProjectPath(scoreId);
        recentFilesController()->removeRecentFile(localPath);

        const Ret removeRet = fileSystem()->remove(localPath);
        if (!removeRet) {
            LOGE() << removeRet.toString();
        }

        m_scoreDeleted.send(scoreId);
    });
}
