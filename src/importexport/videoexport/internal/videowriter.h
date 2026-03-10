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

#pragma once

#include "async/asyncable.h"

#include "engraving/style/style.h"

#include "modularity/ioc.h"
#include "global/iapplication.h"
#include "media/ivideoencoderresolver.h"
#include "../ivideoexportconfiguration.h"

#include "project/inotationwriter.h"
#include "project/inotationwritersregister.h"

#include "notation/notationtypes.h"

namespace mu::iex::videoexport {
class VideoWriter : public project::INotationWriter, public muse::Contextable, public muse::async::Asyncable
{
    muse::GlobalInject<IVideoExportConfiguration> configuration;
    muse::ContextInject<muse::IApplication> application = { this };
    muse::ContextInject<muse::media::IVideoEncoderResolver> videoEncodeResolver = { this };
    muse::ContextInject<project::INotationWritersRegister> writers = { this };

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

    Config makeConfig() const;

    void startVideoExport(muse::media::IVideoEncoderPtr encoder, notation::INotationPtr notation, const Config& cfg);
    void startAudioExport(notation::INotationPtr notation, const muse::io::path_t& audioPath);

    void doGenerate(muse::media::IVideoEncoderPtr encoder, notation::INotationPtr notation, const Config& config);

    struct ScoreRestoreData
    {
        engraving::MStyle style;
        notation::ViewMode viewMode = notation::ViewMode::PAGE;

        bool showFrames = true;
        bool showInstrumentNames = true;
        bool showInvisible = true;
        bool showPageborders = false;
        bool showUnprintable = true;
        bool showVBox = true;
    };

    std::optional<ScoreRestoreData> prepareScore(notation::INotationPtr notation, const Config& config);
    void restoreScore(notation::INotationPtr notation, const ScoreRestoreData& data);

    muse::Progress m_progress;
    bool m_abort = false;
    bool m_isCompleted = false;
    muse::Ret m_writeRet;

    project::INotationWriterPtr m_audioWriter;
    bool m_audioCompleted = false;
    muse::Ret m_audioRet;
};
}
