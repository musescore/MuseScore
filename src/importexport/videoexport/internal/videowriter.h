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
#ifndef MU_IMPORTEXPORT_VIDEOWRITER_H
#define MU_IMPORTEXPORT_VIDEOWRITER_H

#include "modularity/ioc.h"
#include "project/iprojectwriter.h"
#include "../ivideoexportconfiguration.h"

namespace mu::iex::videoexport {
class VideoWriter : public project::IProjectWriter
{
    INJECT(IVideoExportConfiguration, configuration)
public:
    VideoWriter() = default;

    std::vector<UnitType> supportedUnitTypes() const override;
    bool supportsUnitType(UnitType unitType) const override;

    Ret write(project::INotationProjectPtr project, QIODevice& device, const Options& options = Options()) override;
    Ret write(project::INotationProjectPtr project, const io::path_t& filePath, const Options& options = Options()) override;

private:

    struct Config
    {
        int width = 1920;
        int height = 1080;
        int fps = 24;
        int bitrate = 800000;
        float leadingSec = 3.;
        float trailingSec = 3.;
    };

    Ret generatePagedOriginalVideo(project::INotationProjectPtr project, const io::path_t& filePath, const Config& config);
};
}

#endif // MU_IMPORTEXPORT_VIDEOWRITER_H
