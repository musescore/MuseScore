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

    Q_PROPERTY(bool useUiPreferences READ useUiPreferences WRITE setUseUiPreferences NOTIFY useUiPreferencesChanged)
    Q_PROPERTY(bool useUiArrangement READ useUiArrangement WRITE setUseUiArrangement NOTIFY useUiArrangementChanged)
    Q_PROPERTY(bool usePalettes READ usePalettes WRITE setUsePalettes NOTIFY usePalettesChanged)
    Q_PROPERTY(bool useToolbarCustomization READ useToolbarCustomization
               WRITE setUseToolbarCustomization NOTIFY useToolbarCustomizationChanged)

public:
    explicit NewWorkspaceModel(QObject* parent = nullptr);

    Q_INVOKABLE void load(const QString& workspaceNames);
    Q_INVOKABLE QVariant createWorkspace() const;

    QString workspaceName() const;
    QString errorMessage() const;
    bool isWorkspaceNameAllowed() const;

    bool useUiPreferences() const;
    bool useUiArrangement() const;
    bool usePalettes() const;
    bool useToolbarCustomization() const;

public slots:
    void setWorkspaceName(const QString& name);
    void setUseUiPreferences(bool needUse);
    void setUseUiArrangement(bool needUse);
    void setUsePalettes(bool needUse);
    void setUseToolbarCustomization(bool needUse);

signals:
    void workspaceNameChanged();
    void useUiPreferencesChanged();
    void useUiArrangementChanged();
    void usePalettesChanged();
    void useToolbarCustomizationChanged();

private:
    void validateWorkspaceName();

    QString m_workspaceName;
    QString m_errorMessage;

    bool m_useUiPreferences = false;
    bool m_useUiArrangement = false;
    bool m_usePalettes = false;
    bool m_useToolbarCustomization = false;

    QStringList m_workspaceNames;
};
}
