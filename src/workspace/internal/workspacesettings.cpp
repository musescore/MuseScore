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
using namespace mu::async;

void WorkspaceSettings::init()
{
    manager()->currentWorkspace().ch.onReceive(this, [this](const IWorkspacePtr) {
        m_valuesChanged.notify();

        for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
            it->second.send(value(it->first));
        }
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

mu::Val WorkspaceSettings::value(const Key& key) const
{
    if (!currentWorkspace()) {
        return Val();
    }

    AbstractDataPtr abstractData = currentWorkspace()->data(key.tag);
    SettingsDataPtr settingsData = std::dynamic_pointer_cast<SettingsData>(abstractData);
    if (settingsData && settingsData->values.find(key.key) != settingsData->values.end()) {
        return settingsData->values[key.key];
    }

    return Val();
}

void WorkspaceSettings::setValue(const Key& key, const mu::Val& value) const
{
    if (!currentWorkspace()) {
        return;
    }

    AbstractDataPtr abstractData = currentWorkspace()->data(key.tag);
    SettingsDataPtr settingsData = std::dynamic_pointer_cast<SettingsData>(abstractData);

    if (settingsData) {
        settingsData->values[key.key] = value;
    } else {
        settingsData = std::make_shared<SettingsData>();
        settingsData->tag = key.tag;
        settingsData->values.insert({ key.key, value });
    }

    currentWorkspace()->addData(settingsData);

    auto it = m_channels.find(key);
    if (it != m_channels.end()) {
        Channel<Val> channel = it->second;
        channel.send(value);
    }
}

Channel<mu::Val> WorkspaceSettings::valueChanged(const Key& key) const
{
    return m_channels[key];
}

Notification WorkspaceSettings::valuesChanged() const
{
    return m_valuesChanged;
}

IWorkspacePtr WorkspaceSettings::currentWorkspace() const
{
    return manager()->currentWorkspace().val;
}
