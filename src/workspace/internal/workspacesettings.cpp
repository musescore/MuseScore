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
#include "workspacesettings.h"

#include "log.h"

using namespace mu::workspace;

void WorkspaceSettings::init()
{
    manager()->currentWorkspace().ch.onReceive(this, [this](const IWorkspacePtr) {
        m_valuesChanged.notify();
    });
}

bool WorkspaceSettings::isManage(WorkspaceTag tag) const
{
    if (tag != WorkspaceTag::Settings && tag != WorkspaceTag::UiArrangement) {
        return false;
    }

    IWorkspacePtr currentWorkspace = manager()->currentWorkspace().val;
    if (!currentWorkspace) {
        return false;
    }

    return containsTag(currentWorkspace->tags(), tag);
}

mu::Val WorkspaceSettings::value(WorkspaceTag tag, const std::string& key) const
{
    if (!currentWorkspace()) {
        return Val();
    }

    AbstractDataPtr abstractData = currentWorkspace()->data(tag);
    SettingsDataPtr settingsData = std::dynamic_pointer_cast<SettingsData>(abstractData);
    if (settingsData && settingsData->values.find(key) != settingsData->values.end()) {
        return settingsData->values[key];
    }

    return Val();
}

void WorkspaceSettings::setValue(WorkspaceTag tag, const std::string& key, const mu::Val& value) const
{
    if (!currentWorkspace()) {
        return;
    }

    AbstractDataPtr abstractData = currentWorkspace()->data(tag);
    SettingsDataPtr settingsData = std::dynamic_pointer_cast<SettingsData>(abstractData);

    if (settingsData) {
        settingsData->values[key] = value;
    } else {
        settingsData = std::make_shared<SettingsData>();
        settingsData->tag = tag;
        settingsData->values.insert({ key, value });
    }

    currentWorkspace()->addData(settingsData);
}

mu::async::Notification WorkspaceSettings::valuesChanged() const
{
    return m_valuesChanged;
}

IWorkspacePtr WorkspaceSettings::currentWorkspace() const
{
    return manager()->currentWorkspace().val;
}
