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
#ifndef MUSE_WORKSPACE_IWORKSPACE_H
#define MUSE_WORKSPACE_IWORKSPACE_H

#include <memory>
#include <QByteArray>

#include "types/retval.h"
#include "workspacetypes.h"
#include "async/channel.h"

namespace muse::workspace {
class IWorkspace
{
public:
    virtual ~IWorkspace() = default;

    virtual std::string name() const = 0;
    virtual std::string title() const = 0;

    virtual bool isBuiltin() const = 0;
    virtual bool isEdited() const = 0;

    virtual RetVal<QByteArray> rawData(const DataKey& key) const = 0;
    virtual Ret setRawData(const DataKey& key, const QByteArray& data) = 0;

    virtual void reset() = 0;

    virtual async::Notification reloadNotification() = 0;
};

using IWorkspacePtr = std::shared_ptr<IWorkspace>;
using IWorkspacePtrList = std::vector<IWorkspacePtr>;
}

#endif // MUSE_WORKSPACE_IWORKSPACE_H
