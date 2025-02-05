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

#include "newworkspacemodel.h"

#include "global/translation.h"
#include "global/io/path.h"

#include "ui/uitypes.h"
#include "palette/palettetypes.h"

#include "log.h"

using namespace muse::workspace;

NewWorkspaceModel::NewWorkspaceModel(QObject* parent)
    : QObject(parent)
{
}

void NewWorkspaceModel::load(const QString& workspaceNames)
{
    m_workspaceNames = workspaceNames.split(',');

    setWorkspaceName(muse::qtrc("workspace", "My new workspace"));
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

QString NewWorkspaceModel::errorMessage() const
{
    return m_errorMessage;
}

bool NewWorkspaceModel::isWorkspaceNameAllowed() const
{
    return !m_workspaceName.isEmpty() && m_errorMessage.isEmpty();
}

void NewWorkspaceModel::setWorkspaceName(const QString& name)
{
    if (m_workspaceName == name) {
        return;
    }

    m_workspaceName = name;
    validateWorkspaceName();
    emit workspaceNameChanged();
}

void NewWorkspaceModel::validateWorkspaceName()
{
    m_errorMessage.clear();

    if (m_workspaceNames.contains(m_workspaceName)) {
        m_errorMessage = muse::qtrc("workspace", "A workspace with the name “%1” already exists. Please choose a different name.")
                         .arg(m_workspaceName);
        return;
    }

    //! NOTE A file will be created with this name, so let's check if the name is valid for the file name
    if (!muse::io::isAllowedFileName(muse::io::path_t(m_workspaceName))) {
        m_errorMessage = muse::qtrc("workspace", "“%1” cannot be used as a workspace name. Please choose a different name.")
                         .arg(m_workspaceName);
        return;
    }
}

void NewWorkspaceModel::setUseUiPreferences(bool needUse)
{
    if (m_useUiPreferences == needUse) {
        return;
    }

    m_useUiPreferences = needUse;
    emit useUiPreferencesChanged();
}

void NewWorkspaceModel::setUseUiArrangement(bool needUse)
{
    if (m_useUiArrangement == needUse) {
        return;
    }

    m_useUiArrangement = needUse;
    emit useUiArrangementChanged();
}

void NewWorkspaceModel::setUsePalettes(bool needUse)
{
    if (m_usePalettes == needUse) {
        return;
    }

    m_usePalettes = needUse;
    emit usePalettesChanged();
}

void NewWorkspaceModel::setUseToolbarCustomization(bool needUse)
{
    if (m_useToolbarCustomization == needUse) {
        return;
    }

    m_useToolbarCustomization = needUse;
    emit useToolbarCustomizationChanged();
}

QVariant NewWorkspaceModel::createWorkspace() const
{
    QVariantMap meta;
    meta["name"] = m_workspaceName;
    meta[muse::ui::WS_UiSettings] = useUiPreferences();
    meta[muse::ui::WS_UiStates] = useUiArrangement();
    meta[muse::ui::WS_UiToolConfigs] = useToolbarCustomization();
//    meta[palette::WS_Palettes] = usePalettes(); // todo
    return meta;
}
