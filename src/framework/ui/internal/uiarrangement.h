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

#ifndef MU_UI_UIARRANGEMENT_H
#define MU_UI_UIARRANGEMENT_H

#include <QString>
#include <QByteArray>
#include <QList>
#include <QJsonObject>

#include "modularity/ioc.h"
#include "workspace/iworkspacemanager.h"
#include "async/asyncable.h"
#include "actions/actiontypes.h"

namespace mu::ui {
class UiArrangement : public async::Asyncable
{
    INJECT(ui, workspace::IWorkspaceManager, workspaceManager)
public:
    UiArrangement() = default;

    void init();

    QString value(const QString& key) const;
    void setValue(const QString& key, const QString& val);
    mu::async::Notification valueChanged(const QString& key) const;

    QByteArray state(const QString& key) const;
    void setState(const QString& key, const QByteArray& data);

    actions::ActionCodeList toolbarActions(const QString& toolbarName) const;
    void setToolbarActions(const QString& toolbarName, const actions::ActionCodeList& actions);
    mu::async::Notification toolbarActionsChanged(const QString& toolbarName) const;

private:
    workspace::IWorkspacePtr currentWorkspace() const;

    void updateData();
    void saveData();

    Ret writeToFile(const QJsonObject& data);
    QJsonObject readFromFile() const;

    QJsonObject m_settings;
    QJsonObject m_states;
    QJsonObject m_toolactions;
};
}

#endif // MU_UI_UIARRANGEMENT_H
