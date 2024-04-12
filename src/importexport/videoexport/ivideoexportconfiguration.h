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
#ifndef MU_IMPORTEXPORT_IVIDEOEXPORTCONFIGURATION_H
#define MU_IMPORTEXPORT_IVIDEOEXPORTCONFIGURATION_H

#include <string>
#include <optional>

#include "modularity/imoduleinterface.h"

#include "videoexporttypes.h"

namespace mu::iex::videoexport {
class IVideoExportConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IVideoExportConfiguration)

public:
    virtual ~IVideoExportConfiguration() = default;

    virtual ViewMode viewMode() const = 0;
    virtual void setViewMode(std::optional<ViewMode> viewMode) = 0;

    virtual bool showPiano() const = 0;
    virtual void setShowPiano(std::optional<bool> showPiano) = 0;
    virtual PianoPosition pianoPosition() const = 0;
    virtual void setPianoPosition(std::optional<PianoPosition> position) = 0;

    virtual std::string resolution() const = 0;
    virtual void setResolution(std::optional<std::string> resolution) = 0;

    virtual int fps() const = 0;
    virtual void setFps(std::optional<int> fps) = 0;

    virtual double leadingSec() const = 0;
    virtual void setLeadingSec(std::optional<double> leadingSec) = 0;

    virtual double trailingSec() const = 0;
    virtual void setTrailingSec(std::optional<double> trailingSec) = 0;
};
}

#endif // MU_IMPORTEXPORT_IVIDEOEXPORTCONFIGURATION_H
