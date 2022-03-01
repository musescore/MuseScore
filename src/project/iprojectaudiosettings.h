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

#ifndef MU_PROJECT_IPROJECTAUDIOSETTINGS_H
#define MU_PROJECT_IPROJECTAUDIOSETTINGS_H

#include <memory>

#include "id.h"
#include "audio/audiotypes.h"

namespace mu::project {
class IProjectAudioSettings
{
public:
    virtual ~IProjectAudioSettings() = default;

    virtual audio::AudioOutputParams masterAudioOutputParams() const = 0;
    virtual void setMasterAudioOutputParams(const audio::AudioOutputParams& params) = 0;

    virtual audio::AudioInputParams trackInputParams(const ID& partId) const = 0;
    virtual void setTrackInputParams(const ID& partId, const audio::AudioInputParams& params) = 0;

    virtual audio::AudioOutputParams trackOutputParams(const ID& partId) const = 0;
    virtual void setTrackOutputParams(const ID& partId, const audio::AudioOutputParams& params) = 0;

    virtual void removeTrackParams(const ID& partId) = 0;

    virtual mu::ValNt<bool> needSave() const = 0;
};

using IProjectAudioSettingsPtr = std::shared_ptr<IProjectAudioSettings>;
}

#endif // MU_PROJECT_IPROJECTAUDIOSETTINGS_H
