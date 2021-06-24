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

#include "newworkspacemodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::workspace;

NewWorkspaceModel::NewWorkspaceModel(QObject* parent)
    : QObject(parent)
{
}

void NewWorkspaceModel::load()
{
    setWorkspaceName(qtrc("workspaces", "My new workspace"));
    setUseUiPreferences(true);
    setUseUiArrangement(true);
    setUsePalettes(true);
    setUseToolbarCustomization(true);
}

QString NewWorkspaceModel::workspaceName() const
{
    return m_workspaceName;
}

bool NewWorkspaceModel::useUiPreferences() const
{
    return m_useUiPreferences;
}

bool NewWorkspaceModel::useUiArrangement() const
{
    return m_useUiArrangement;
}

bool NewWorkspaceModel::usePalettes() const
{
    return m_usePalettes;
}

bool NewWorkspaceModel::useToolbarCustomization() const
{
    return m_useToolbarCustomization;
}

bool NewWorkspaceModel::canCreateWorkspace() const
{
    return !m_workspaceName.isEmpty();
}

void NewWorkspaceModel::setWorkspaceName(const QString& name)
{
    if (m_workspaceName == name) {
        return;
    }

    m_workspaceName = name;
    emit dataChanged();
    emit canCreateWorkspaceChanged();
}

void NewWorkspaceModel::setUseUiPreferences(bool needUse)
{
    if (m_useUiPreferences == needUse) {
        return;
    }

    m_useUiPreferences = needUse;
    emit dataChanged();
}

void NewWorkspaceModel::setUseUiArrangement(bool needUse)
{
    if (m_useUiArrangement == needUse) {
        return;
    }

    m_useUiArrangement = needUse;
    emit dataChanged();
}

void NewWorkspaceModel::setUsePalettes(bool needUse)
{
    if (m_usePalettes == needUse) {
        return;
    }

    m_usePalettes = needUse;
    emit dataChanged();
}

void NewWorkspaceModel::setUseToolbarCustomization(bool needUse)
{
    if (m_useToolbarCustomization == needUse) {
        return;
    }

    m_useToolbarCustomization = needUse;
    emit dataChanged();
}

QVariant NewWorkspaceModel::createWorkspace()
{
    IWorkspacePtr newWorkspace = workspaceCreator()->newWorkspace(m_workspaceName.toStdString());

    newWorkspace->setIsManaged(DataKey::UiSettings, useUiPreferences());
    newWorkspace->setIsManaged(DataKey::UiStates, useUiArrangement());
    newWorkspace->setIsManaged(DataKey::UiToolConfigs, useToolbarCustomization());
    newWorkspace->setIsManaged(DataKey::Palettes, usePalettes());

    return QVariant::fromValue(newWorkspace);
}
