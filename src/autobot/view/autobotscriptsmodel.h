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
#ifndef MU_AUTOBOT_AUTOBOTSCRIPTSMODEL_H
#define MU_AUTOBOT_AUTOBOTSCRIPTSMODEL_H

#include <QAbstractListModel>
#include <QList>

#include "modularity/ioc.h"
#include "../iautobotscriptsrepository.h"

namespace mu::autobot {
class AutobotScriptsModel : public QAbstractListModel
{
    Q_OBJECT

    INJECT(autobot, IAutobotScriptsRepository, scriptsRepository)

public:
    explicit AutobotScriptsModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void runScript(int scriptIndex);

private:

    enum Roles {
        ScriptTitle = Qt::UserRole + 1,
        ScriptIndex,
    };

    Scripts m_scripts;
};
}

#endif // MU_AUTOBOT_AUTOBOTSCRIPTSMODEL_H
