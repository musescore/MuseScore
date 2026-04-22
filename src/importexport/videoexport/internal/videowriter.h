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

#include <memory>

#include "async/asyncable.h"

#include "io/filestream.h"

#include "engraving/style/style.h"

#include "modularity/ioc.h"
#include "io/ifilesystem.h"
#include "global/iapplication.h"
#include "media/ivideoencoderresolver.h"
#include "../ivideoexportconfiguration.h"
#include "context/iglobalcontext.h"

#include "project/inotationwriter.h"
#include "project/inotationwritersregister.h"

class QImage;

namespace muse::draw {
class Painter;
}

namespace mu::iex::videoexport {
class VideoWriter : public project::INotationWriter, public muse::Contextable, public muse::async::Asyncable
{
    muse::GlobalInject<IVideoExportConfiguration> configuration;
    muse::GlobalInject<muse::io::IFileSystem> fileSystem;
    muse::GlobalInject<muse::IApplication> application;
    muse::GlobalInject<muse::media::IVideoEncoderResolver> videoEncodeResolver;
    muse::GlobalInject<project::INotationWritersRegister> writers;
    muse::ContextInject<context::IGlobalContext> globalContext = { this };

public:
    VideoWriter(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

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
        double canvasDpi = 300.0;
    };

    Config makeConfig() const;

    void startVideoExport(muse::media::IVideoEncoderPtr encoder, notation::INotationPtr notation, const Config& cfg);
    void startAudioExport(notation::INotationPtr notation, const muse::io::path_t& audioPath, const Config& cfg);

    void doGenerate(muse::media::IVideoEncoderPtr encoder, notation::INotationPtr notation, const Config& config);

    bool generateLeadingFrames(muse::media::IVideoEncoderPtr encoder, notation::INotationPtr notation, muse::draw::Painter& painter,
                               QImage& frame, const Config& config, int totalFrameCount);

    bool generateScoreFrames(muse::media::IVideoEncoderPtr encoder, notation::INotationPtr notation, muse::draw::Painter& painter,
                             QImage& frame, const Config& config, float totalPlayTimeSec, int leadingFrameCount, int totalFrameCount);

    bool generateTrailingFrames(muse::media::IVideoEncoderPtr encoder, const Config& config);

    struct ScoreRestoreData
    {
        engraving::MStyle style;
        engraving::LayoutMode layoutMode = engraving::LayoutMode::PAGE;

        bool showFrames = true;
        bool showInstrumentNames = true;
        bool showInvisible = true;
        bool showPageborders = false;
        bool showUnprintable = true;
        bool showVBox = true;
    };

    std::optional<ScoreRestoreData> prepareScore(notation::INotationPtr notation, Config& config);
    void restoreScore(notation::INotationPtr notation, const ScoreRestoreData& data);

    muse::Progress m_progress;
    bool m_abort = false;
    bool m_isCompleted = false;
    muse::Ret m_writeRet;

    project::INotationWriterPtr m_audioWriter;
    std::unique_ptr<muse::io::FileStream> m_audioFile;
    bool m_audioCompleted = false;
    muse::Ret m_audioRet;
};
}
