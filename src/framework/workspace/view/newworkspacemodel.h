/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include <QObject>
#include <QVariant>

namespace muse::workspace {
class NewWorkspaceModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString workspaceName READ workspaceName WRITE setWorkspaceName NOTIFY workspaceNameChanged)
    Q_PROPERTY(bool isWorkspaceNameAllowed READ isWorkspaceNameAllowed NOTIFY workspaceNameChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY workspaceNameChanged)

public:
    explicit NewWorkspaceModel(QObject* parent = nullptr);

    Q_INVOKABLE void load(const QString& workspaceNames);
    Q_INVOKABLE QVariant createWorkspace() const;

    QString workspaceName() const;
    QString errorMessage() const;
    bool isWorkspaceNameAllowed() const;

public slots:
    void setWorkspaceName(const QString& name);

signals:
    void workspaceNameChanged();

private:
    void validateWorkspaceName();

    QString m_workspaceName;
    QString m_errorMessage;

    QStringList m_workspaceNames;
};
}
