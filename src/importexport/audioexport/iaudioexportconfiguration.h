/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_IMPORTEXPORT_IAUDIOEXPORTCONFIGURATION_H
#define MU_IMPORTEXPORT_IAUDIOEXPORTCONFIGURATION_H

#include <optional>

#include "modularity/imoduleinterface.h"

#include "audio/audiotypes.h"

namespace mu::iex::audioexport {
class IAudioExportConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioExportConfiguration)

public:
    virtual ~IAudioExportConfiguration() = default;

    virtual int exportMp3Bitrate() const = 0;
    virtual void setExportMp3Bitrate(int bitrate) = 0;
    virtual void setExportMp3BitrateOverride(std::optional<int> bitrate) = 0;
    virtual const std::vector<int>& availableMp3BitRates() const = 0;

    virtual int exportSampleRate() const = 0;
    virtual void setExportSampleRate(int rate) = 0;
    virtual const std::vector<int>& availableSampleRates() const = 0;

    virtual muse::audio::samples_t exportBufferSize() const = 0;
};
}

#endif // MU_IMPORTEXPORT_IAUDIOEXPORTCONFIGURATION_H
