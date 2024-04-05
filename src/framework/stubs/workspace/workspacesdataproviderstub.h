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
#ifndef MUSE_WORKSPACE_WORKSPACESDATAPROVIDERSTUB_H
#define MUSE_WORKSPACE_WORKSPACESDATAPROVIDERSTUB_H

#include "workspace/iworkspacesdataprovider.h"

namespace muse::workspace {
class WorkspacesDataProviderStub : public IWorkspacesDataProvider
{
public:
    WorkspacesDataProviderStub() = default;

    RetVal<QByteArray> rawData(DataKey key) const override;
    Ret setRawData(DataKey key, const QByteArray& data) override;
    async::Notification dataChanged(DataKey key) const override;

    async::Notification workspaceChanged() const override;
};
}

#endif // MUSE_WORKSPACE_WORKSPACESDATAPROVIDERSTUB_H
