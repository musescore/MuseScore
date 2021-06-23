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
#ifndef MU_WORKSPACE_WORKSPACEUSETTINGS_H
#define MU_WORKSPACE_WORKSPACEUSETTINGS_H

#include <map>
#include <QJsonObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "global/iworkspacesettings.h"
#include "iworkspacemanager.h"

namespace mu::workspace {
class WorkspaceSettings : public framework::IWorkspaceSettings, public async::Asyncable
{
    INJECT(workspace, IWorkspaceManager, manager)

public:
    void init();

    bool isManage(Tag tag) const override;

    Val value(const Key& key) const override;
    void setValue(const Key& key, const Val& value) override;

    async::Channel<Val> valueChanged(const Key& key) const override;
    async::Notification valuesChanged() const override;

private:
    IWorkspacePtr currentWorkspace() const;
    QJsonObject m_data;
    mutable std::map<Key, async::Channel<Val> > m_channels;
    async::Notification m_valuesChanged;
};
}

#endif // MU_WORKSPACE_WORKSPACEUSETTINGS_H
