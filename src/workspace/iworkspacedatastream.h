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
#ifndef MU_WORKSPACE_IWORKSPACEDATASTREAM_H
#define MU_WORKSPACE_IWORKSPACEDATASTREAM_H

#include <memory>

#include "workspacetypes.h"
#include "io/device.h"

namespace mu::workspace {
class IWorkspaceDataStream
{
public:
    virtual ~IWorkspaceDataStream() = default;

    virtual AbstractDataPtrList read(io::Device& sourceDevice) const = 0;
    virtual void write(const AbstractDataPtrList& dataList, io::Device& destinationDevice) const = 0;

    virtual WorkspaceTag tag() const = 0;
};

using IWorkspaceDataStreamPtr = std::shared_ptr<IWorkspaceDataStream>;
using IWorkspaceDataStreamPtrList = std::vector<IWorkspaceDataStreamPtr>;
}

#endif // MU_WORKSPACE_IWORKSPACEDATASTREAM_H
