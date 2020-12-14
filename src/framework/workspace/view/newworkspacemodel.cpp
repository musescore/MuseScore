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
//  but WITHOUT ANY:WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

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
    setImportUiPreferences(true);
    setImportUiArrangement(true);
    setImportPalettes(true);
    setImportToolbarCustomization(true);
}

QString NewWorkspaceModel::workspaceName() const
{
    return m_workspaceName;
}

bool NewWorkspaceModel::importUiPreferences() const
{
    return m_importUiPreferences;
}

bool NewWorkspaceModel::importUiArrangement() const
{
    return m_importUiArrangement;
}

bool NewWorkspaceModel::importPalettes() const
{
    return m_importPalettes;
}

bool NewWorkspaceModel::importToolbarCustomization() const
{
    return m_importToolbarCustomization;
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

void NewWorkspaceModel::setImportUiPreferences(bool needImport)
{
    if (m_importUiPreferences == needImport) {
        return;
    }

    m_importUiPreferences = needImport;
    emit dataChanged();
}

void NewWorkspaceModel::setImportUiArrangement(bool needImport)
{
    if (m_importUiArrangement == needImport) {
        return;
    }

    m_importUiArrangement = needImport;
    emit dataChanged();
}

void NewWorkspaceModel::setImportPalettes(bool needImport)
{
    if (m_importPalettes == needImport) {
        return;
    }

    m_importPalettes = needImport;
    emit dataChanged();
}

void NewWorkspaceModel::setImportToolbarCustomization(bool needImport)
{
    if (m_importToolbarCustomization == needImport) {
        return;
    }

    m_importToolbarCustomization = needImport;
    emit dataChanged();
}

QVariant NewWorkspaceModel::createWorkspace()
{
    IWorkspacePtr newWorkspace = workspaceCreator()->newWorkspace(m_workspaceName.toStdString());
    IWorkspacePtr currentWorkspace = workspaceManager()->currentWorkspace().val;

    QList<WorkspaceTag> importedTags;

    if (importUiPreferences()) {
        importedTags << WorkspaceTag::Settings;
    }

    if (importUiArrangement()) {
        importedTags << WorkspaceTag::UiArrangement;
    }

    if (importToolbarCustomization()) {
        importedTags << WorkspaceTag::Toolbar;
    }

    if (importPalettes()) {
        importedTags << WorkspaceTag::Palettes;
    }

    for (WorkspaceTag tag : importedTags) {
        for (AbstractDataPtr data : currentWorkspace->dataList(tag)) {
            newWorkspace->addData(data);
        }
    }

    return QVariant::fromValue(newWorkspace);
}
