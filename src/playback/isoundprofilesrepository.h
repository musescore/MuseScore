/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#ifndef MU_PLAYBACK_ISOUNDPROFILESREPOSITORY_H
#define MU_PLAYBACK_ISOUNDPROFILESREPOSITORY_H

#include "modularity/imoduleinterface.h"

#include "playbacktypes.h"

namespace mu::playback {
class ISoundProfilesRepository : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISoundProfilesRepository)

public:
    virtual ~ISoundProfilesRepository() = default;

    virtual void refresh() = 0;

    virtual const SoundProfile& profile(const SoundProfileName& name) const = 0;
    virtual bool containsProfile(const SoundProfileName& name) const = 0;
    virtual const SoundProfilesMap& availableProfiles() const = 0;

    virtual void addProfile(const SoundProfile& profile) = 0;
    virtual void removeProfile(const SoundProfileName& name) = 0;
};
}

#endif // MU_PLAYBACK_ISOUNDPROFILESREPOSITORY_H
