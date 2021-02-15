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
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "currentworkspacemodel.h"

using namespace mu::workspace;

CurrentWorkspaceModel::CurrentWorkspaceModel(QObject* parent)
    : QObject(parent)
{
}

void CurrentWorkspaceModel::load()
{
    emit currentWorkspaceNameChanged();

    configuration()->currentWorkspaceName().ch.onReceive(this, [this](const std::string&) {
        emit currentWorkspaceNameChanged();
    });
}

void CurrentWorkspaceModel::selectWorkspace()
{
    RetVal<Val> result = interactive()->open("musescore://workspace/select?sync=true");
    if (!result.ret) {
        return;
    }

    std::string selectedWorkspace = result.val.toString();

    if (configuration()->currentWorkspaceName().val == selectedWorkspace || selectedWorkspace.empty()) {
        return;
    }

    configuration()->setCurrentWorkspaceName(selectedWorkspace);
}

QString CurrentWorkspaceModel::currentWorkspaceName() const
{
    return QString::fromStdString(configuration()->currentWorkspaceName().val);
}
