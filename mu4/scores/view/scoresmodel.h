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
#ifndef MU_SCORES_SCORESMODEL_H
#define MU_SCORES_SCORESMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"
#include "../iscoresconfiguration.h"
#include "domain/notation/imsczmetareader.h"

namespace mu {
namespace scores {
class ScoresModel : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(scores, actions::IActionsDispatcher, dispatcher)
    INJECT(scores, IScoresConfiguration, scoresConfiguration)
    INJECT(scores, domain::notation::IMsczMetaReader, msczMetaReader)

    Q_PROPERTY(QVariantList recentList READ recentList NOTIFY recentListChanged)

public:
    ScoresModel(QObject* parent = nullptr);

    Q_INVOKABLE void openScore();
    Q_INVOKABLE void importScore();

    QVariantList recentList();
    void setRecentList(const QVariantList& recentList);

signals:
    void recentListChanged(QVariantList recentList);

private:
    void updateRecentList(const QStringList& recentList);

    QVariantList m_recentList;
};
}
}

#endif // MU_SCORES_SCORESMODEL_H
