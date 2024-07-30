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
#ifndef MUSE_WORKSPACE_WORKSPACESDATAPROVIDER_H
#define MUSE_WORKSPACE_WORKSPACESDATAPROVIDER_H

#include <map>

#include "../iworkspacesdataprovider.h"

#include "modularity/ioc.h"
#include "iworkspacemanager.h"
#include "async/asyncable.h"

namespace muse::workspace {
class WorkspacesDataProvider : public IWorkspacesDataProvider, public Injectable, public async::Asyncable
{
    Inject<IWorkspaceManager> manager = { this };

public:
    WorkspacesDataProvider(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

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
#endif // MUSE_WORKSPACE_WORKSPACEPROVIDER_H
