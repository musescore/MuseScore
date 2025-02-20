/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#pragma once

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "notation/inotation.h"

#include "layoutpanelutils.h"

namespace mu::instrumentsscene {
class SystemObjectsLayerSettingsModel : public QAbstractListModel, public muse::Injectable
{
    Q_OBJECT

    muse::Inject<context::IGlobalContext> context = { this };

public:
    explicit SystemObjectsLayerSettingsModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load(const QString& staffId);
    Q_INVOKABLE void setSystemObjectsGroupVisible(int index, bool visible);

private:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        VisibilityRole,
    };

    std::vector<SystemObjectsGroup> m_systemObjectGroups;
};
}
