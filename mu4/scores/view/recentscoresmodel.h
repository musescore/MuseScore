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
#ifndef MU_SCORES_RECENTSCORESMODEL_H
#define MU_SCORES_RECENTSCORESMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"
#include "iscoresconfiguration.h"
#include "domain/notation/imsczmetareader.h"

namespace mu {
namespace scores {
class RecentScoresModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(scores, actions::IActionsDispatcher, dispatcher)
    INJECT(scores, IScoresConfiguration, scoresConfiguration)
    INJECT(scores, domain::notation::IMsczMetaReader, msczMetaReader)

public:
    RecentScoresModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void newScore();
    Q_INVOKABLE void openScore();
    Q_INVOKABLE void openRecentScore(int index);

private:
    enum Roles {
        RoleTitle = Qt::UserRole + 1,
        RoleScore
    };

    void updateRecentScores(const QStringList& recentScoresPathList);
    void setRecentScores(const QVariantList& recentScores);

    QVariantList m_recentScores;
    QHash<int, QByteArray> m_roles;
};
}
}

#endif // MU_SCORES_RECENTSCORESMODEL_H
