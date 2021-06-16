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
#ifndef MU_MI_IMULTIINSTANCESPROVIDER_H
#define MU_MI_IMULTIINSTANCESPROVIDER_H

#include <string>
#include <vector>

#include "modularity/imoduleexport.h"
#include "io/path.h"
#include "mitypes.h"
#include "async/notification.h"
#include "val.h"

namespace mu::mi {
class IMultiInstancesProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMultiInstancesProvider)
public:
    virtual ~IMultiInstancesProvider() = default;

    // Score opening
    virtual bool isScoreAlreadyOpened(const io::path& scorePath) const = 0;
    virtual void activateWindowWithScore(const io::path& scorePath) = 0;

    // Settings
    virtual bool isPreferencesAlreadyOpened() const = 0;
    virtual void activateWindowWithOpenedPreferences() const = 0;
    virtual void settingsBeginTransaction() = 0;
    virtual void settingsCommitTransaction() = 0;
    virtual void settingsRollbackTransaction() = 0;
    virtual void settingsSetValue(const std::string& key, const Val& value) = 0;

    //! NOTE Technical
    virtual const std::string& selfID() const = 0;
    virtual std::vector<InstanceMeta> instances() const = 0;
    virtual async::Notification instancesChanged() const = 0;
};
}

#endif // MU_MI_IMULTIINSTANCESPROVIDER_H
