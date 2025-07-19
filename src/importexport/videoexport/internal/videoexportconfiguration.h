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
#ifndef MU_IMPORTEXPORT_VIDEOEXPORTCONFIGURATION_H
#define MU_IMPORTEXPORT_VIDEOEXPORTCONFIGURATION_H

#include "../ivideoexportconfiguration.h"

#include "../videoexporttypes.h"

namespace mu::iex::videoexport {
class VideoExportConfiguration : public IVideoExportConfiguration
{
public:
    VideoExportConfiguration() = default;

    ViewMode viewMode() const override;
    void setViewMode(std::optional<ViewMode> viewMode) override;

    bool showPiano() const override;
    void setShowPiano(std::optional<bool> showPiano) override;
    PianoPosition pianoPosition() const override;
    void setPianoPosition(std::optional<PianoPosition> position) override;

    std::string resolution() const override;
    void setResolution(std::optional<std::string> resolution) override;

    int fps() const override;
    void setFps(std::optional<int> fps) override;

    double leadingSec() const override;
    void setLeadingSec(std::optional<double> leadingSec) override;

    double trailingSec() const override;
    void setTrailingSec(std::optional<double> trailingSec) override;

private:
    std::optional<ViewMode> m_viewMode = std::nullopt;
    std::optional<bool> m_showPiano = std::nullopt;
    std::optional<PianoPosition> m_pianoPosition = std::nullopt;
    std::optional<std::string> m_resolution = std::nullopt;
    std::optional<int> m_fps = std::nullopt;
    std::optional<double> m_leadingSec = std::nullopt;
    std::optional<double> m_trailingSec = std::nullopt;
};
}

#endif // MU_IMPORTEXPORT_VIDEOEXPORTCONFIGURATION_H
