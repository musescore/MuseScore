//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "workspaceuiarrangment.h"

#include "log.h"

using namespace mu::workspace;

void WorkspaceUiArrangment::init()
{
    manager()->currentWorkspace().ch.onReceive(this, [this](const IWorkspacePtr) {
        m_valuesChanged.notify();
    });
}

mu::Val WorkspaceUiArrangment::value(const std::string& key) const
{
    if (!isWorkspaceManageUi()) {
        return Val();
    }

    AbstractDataPtr abstractData = currentWorkspace()->data(WorkspaceTag::UiArrangement);
    UiArrangmentDataPtr uiArrangmentData = std::dynamic_pointer_cast<UiArrangmentData>(abstractData);
    if (!uiArrangmentData) {
        LOGE() << "Failed to get data for " << key;
        return Val();
    }

    return uiArrangmentData->values[key];
}

bool WorkspaceUiArrangment::setValue(const std::string& key, const mu::Val& value) const
{
    if (!isWorkspaceManageUi()) {
        return false;
    }

    AbstractDataPtr abstractData = currentWorkspace()->data(WorkspaceTag::UiArrangement);
    UiArrangmentDataPtr uiArrangmentData = std::dynamic_pointer_cast<UiArrangmentData>(abstractData);
    if (!uiArrangmentData) {
        LOGE() << "Failed to get data for " << key;
        return false;
    }

    uiArrangmentData->values[key] = value;
    currentWorkspace()->addData(uiArrangmentData);

    return true;
}

mu::async::Notification WorkspaceUiArrangment::valuesChanged() const
{
    return m_valuesChanged;
}

IWorkspacePtr WorkspaceUiArrangment::currentWorkspace() const
{
    return manager()->currentWorkspace().val;
}

bool WorkspaceUiArrangment::isWorkspaceManageUi() const
{
    if (!currentWorkspace()) {
        return false;
    }

    return !currentWorkspace()->dataList(WorkspaceTag::UiArrangement).empty();
}
