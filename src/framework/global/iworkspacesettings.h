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
#ifndef MU_FRAMEWORK_IWORKSPACESETTINGS_H
#define MU_FRAMEWORK_IWORKSPACESETTINGS_H

#include "modularity/imoduleexport.h"
#include "val.h"
#include "async/channel.h"
#include "async/notification.h"
#include "workspace/workspacetypes.h"

namespace mu::framework {
class IWorkspaceSettings : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IWorkspaceSettings)

public:
    virtual ~IWorkspaceSettings() = default;

    enum class Tag {
        Undefined = 0,
        Settings,
        UiArrangement
    };

    struct Key
    {
        Tag tag = Tag::Undefined;
        std::string key;

        bool operator==(const Key& other) const { return tag == other.tag && key == other.key; }
        bool operator<(const Key& other) const
        {
            if (tag != other.tag) {
                return tag < other.tag;
            }
            return key < other.key;
        }
    };

    virtual bool isManage(Tag tag) const = 0;

    virtual Val value(const Key& key) const = 0;
    virtual void setValue(const Key& key, const Val& value) = 0;

    virtual async::Channel<Val> valueChanged(const Key& key) const = 0;
    virtual async::Notification valuesChanged() const = 0;
};
}

#endif // MU_FRAMEWORK_IWORKSPACESETTINGS_H
