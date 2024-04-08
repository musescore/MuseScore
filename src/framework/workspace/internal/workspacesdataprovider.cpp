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
#include "workspacesdataprovider.h"

#include "log.h"

using namespace muse;
using namespace muse::workspace;

void WorkspacesDataProvider::init()
{
    manager()->currentWorkspaceChanged().onNotify(this, [this]() {
        m_workspaceChanged.notify();
        for (auto& n : m_dataNotifications) {
            n.second.notify();
        }
    });
}

RetVal<QByteArray> WorkspacesDataProvider::rawData(DataKey key) const
{
    IWorkspacePtr current = manager()->currentWorkspace();
    IF_ASSERT_FAILED(current) {
        return RetVal<QByteArray>(make_ret(Ret::Code::InternalError));
    }

    if (current->isManaged(key)) {
        LOGD() << "get data from current workspace, key: " << key;
        return current->rawData(key);
    }

    IWorkspacePtr def = manager()->defaultWorkspace();
    IF_ASSERT_FAILED(def) {
        return RetVal<QByteArray>(make_ret(Ret::Code::InternalError));
    }

    LOGD() << "get data from default workspace, key: " << key;
    return def->rawData(key);
}

Ret WorkspacesDataProvider::setRawData(DataKey key, const QByteArray& data)
{
    IWorkspacePtr current = manager()->currentWorkspace();
    IF_ASSERT_FAILED(current) {
        return make_ret(Ret::Code::InternalError);
    }

    Ret ret = false;
    if (current->isManaged(key)) {
        LOGD() << "set data to current workspace, key: " << key;
        ret = current->setRawData(key, data);
    }

    if (!ret) {
        IWorkspacePtr def = manager()->defaultWorkspace();
        IF_ASSERT_FAILED(def) {
            return make_ret(Ret::Code::InternalError);
        }

        LOGD() << "set data to default workspace, key: " << key;
        ret = def->setRawData(key, data);
    }

    if (ret) {
        auto n = m_dataNotifications.find(key);
        if (n != m_dataNotifications.end()) {
            n->second.notify();
        }
    }

    return ret;
}

async::Notification WorkspacesDataProvider::dataChanged(DataKey key) const
{
    return m_dataNotifications[key];
}

async::Notification WorkspacesDataProvider::workspaceChanged() const
{
    return m_workspaceChanged;
}
