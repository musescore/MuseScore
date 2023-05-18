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
#ifndef MU_WORKSPACE_WORKSPACESDATAPROVIDER_H
#define MU_WORKSPACE_WORKSPACESDATAPROVIDER_H

#include <map>

#include "../iworkspacesdataprovider.h"

#include "modularity/ioc.h"
#include "iworkspacemanager.h"
#include "async/asyncable.h"

namespace mu::workspace {
class WorkspacesDataProvider : public IWorkspacesDataProvider, public async::Asyncable
{
    INJECT(IWorkspaceManager, manager)

public:
    WorkspacesDataProvider() = default;

    void init();

    RetVal<QByteArray> rawData(DataKey key) const override;
    Ret setRawData(DataKey key, const QByteArray& data) override;
    async::Notification dataChanged(DataKey key) const override;

    async::Notification workspaceChanged() const override;

private:

    mutable std::map<DataKey, async::Notification> m_dataNotifications;
    async::Notification m_workspaceChanged;
};
}
#endif // MU_WORKSPACE_WORKSPACEPROVIDER_H
