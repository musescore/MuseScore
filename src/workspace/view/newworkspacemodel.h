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

#ifndef MU_WORKSPACE_NEWWORKSPACEMODEL_H
#define MU_WORKSPACE_NEWWORKSPACEMODEL_H

#include <QObject>

namespace mu::workspace {
class NewWorkspaceModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString workspaceName READ workspaceName WRITE setWorkspaceName NOTIFY dataChanged)
    Q_PROPERTY(bool useUiPreferences READ useUiPreferences WRITE setUseUiPreferences NOTIFY dataChanged)
    Q_PROPERTY(bool useUiArrangement READ useUiArrangement WRITE setUseUiArrangement NOTIFY dataChanged)
    Q_PROPERTY(bool usePalettes READ usePalettes WRITE setUsePalettes NOTIFY dataChanged)
    Q_PROPERTY(bool useToolbarCustomization READ useToolbarCustomization WRITE setUseToolbarCustomization NOTIFY dataChanged)
    Q_PROPERTY(bool canCreateWorkspace READ canCreateWorkspace NOTIFY canCreateWorkspaceChanged)

public:
    explicit NewWorkspaceModel(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE QVariant createWorkspace();

    QString workspaceName() const;
    bool useUiPreferences() const;
    bool useUiArrangement() const;
    bool usePalettes() const;
    bool useToolbarCustomization() const;
    bool canCreateWorkspace() const;

public slots:
    void setWorkspaceName(const QString& name);
    void setUseUiPreferences(bool needUse);
    void setUseUiArrangement(bool needUse);
    void setUsePalettes(bool needUse);
    void setUseToolbarCustomization(bool needUse);

signals:
    void dataChanged();
    void canCreateWorkspaceChanged();

private:

    QString m_workspaceName;
    bool m_useUiPreferences = false;
    bool m_useUiArrangement = false;
    bool m_usePalettes = false;
    bool m_useToolbarCustomization = false;
};
}

#endif // MU_WORKSPACE_NEWWORKSPACEMODEL_H
