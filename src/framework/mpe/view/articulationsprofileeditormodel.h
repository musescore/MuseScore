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

#ifndef MU_MPE_ARTICULATIONSPROFILEEDITORMODEL_H
#define MU_MPE_ARTICULATIONSPROFILEEDITORMODEL_H

#include <QAbstractListModel>
#include <QList>

#include "modularity/ioc.h"
#include "global/iinteractive.h"
#include "io/path.h"

#include "iarticulationprofilesrepository.h"
#include "internal/articulationpatternsscopeitem.h"

namespace mu::mpe {
class ArticulationsProfileEditorModel : public QAbstractListModel
{
    Q_OBJECT

    INJECT(mpe, framework::IInteractive, interactive)
    INJECT(mpe, IArticulationProfilesRepository, profilesRepository)

public:
    enum RoleNames {
        PatternsScopeItem = Qt::UserRole + 1
    };

    explicit ArticulationsProfileEditorModel(QObject* parent = nullptr);

    Q_INVOKABLE void requestToOpenProfile();
    Q_INVOKABLE void requestToSaveProfile();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void setProfile(const ArticulationsProfilePtr ptr);

    void loadItems();

    io::path m_profilePath;

    ArticulationsProfilePtr m_profile = nullptr;
    QList<ArticulationPatternsScopeItem*> m_items;
};
}

#endif // MU_MPE_ARTICULATIONSPROFILEEDITORMODEL_H
