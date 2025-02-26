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

    return current->rawData(key);
}

Ret WorkspacesDataProvider::setRawData(DataKey key, const QByteArray& data)
{
    IWorkspacePtr current = manager()->currentWorkspace();
    IF_ASSERT_FAILED(current) {
        return make_ret(Ret::Code::InternalError);
    }

    Ret ret = current->setRawData(key, data);

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
