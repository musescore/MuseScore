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
#ifndef MU_IMPORTEXPORT_VIDEOWRITER_H
#define MU_IMPORTEXPORT_VIDEOWRITER_H

#include "modularity/ioc.h"
#include "../ivideoexportconfiguration.h"
#include "iapplication.h"

#include "project/inotationwriter.h"

namespace mu::iex::videoexport {
class VideoWriter : public project::INotationWriter, public muse::Contextable
{
    muse::GlobalInject<IVideoExportConfiguration> configuration;
    muse::ContextInject<muse::IApplication> application = { this };

public:
    explicit VideoWriter(const muse::modularity::ContextPtr& iocCtx)
        : Contextable(iocCtx) {}

    std::vector<UnitType> supportedUnitTypes() const override;
    bool supportsUnitType(UnitType unitType) const override;

    muse::Ret write(notation::INotationPtr notation, muse::io::IODevice& device, const Options& options = Options()) override;
    muse::Ret writeList(const notation::INotationPtrList& notations, muse::io::IODevice& device,
                        const Options& options = Options()) override;

    muse::Progress* progress() override;
    void abort() override;

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

    muse::Ret generatePagedOriginalVideo(notation::INotationPtr notation, const muse::io::path_t& filePath, const Config& config);

    muse::Progress m_progress;
    bool m_abort = false;
};
}

#endif // MU_IMPORTEXPORT_VIDEOWRITER_H
