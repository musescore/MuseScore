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
#ifndef MU_IMPORTEXPORT_ABSTRACTAUDIOWRITER_H
#define MU_IMPORTEXPORT_ABSTRACTAUDIOWRITER_H

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "audio/main/iplayback.h"
#include "audio/main/istartaudiocontroller.h"
#include "iaudioexportconfiguration.h"
#include "context/iglobalcontext.h"
#include "playback/iplaybackcontroller.h"
#include "global/iapplication.h"

#include "project/iprojectwriter.h"

namespace mu::iex::audioexport {
class AbstractAudioWriter : public project::IProjectWriter, public muse::Contextable, public muse::async::Asyncable
{
public:
    muse::GlobalInject<IAudioExportConfiguration> configuration;
    muse::ContextInject<muse::audio::IPlayback> playback = { this };
    muse::ContextInject<context::IGlobalContext> globalContext = { this };
    muse::ContextInject<muse::audio::IStartAudioController> startAudioController = { this };
    muse::ContextInject<playback::IPlaybackController> playbackController  = { this };
    muse::GlobalInject<muse::IApplication> application;

public:
    AbstractAudioWriter(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    std::vector<project::WriteUnitType> supportedUnitTypes() const override;
    bool supportsUnitType(project::WriteUnitType unitType) const override;

    muse::Ret write(project::INotationProjectPtr project, muse::io::IODevice& dstDevice,
                    const project::WriteOptions& options = project::WriteOptions()) override;
    muse::Ret write(project::INotationProjectPtr project, const muse::io::path_t& filePath,
                    const project::WriteOptions& options = project::WriteOptions()) override;

    muse::Progress* progress() override;
    void abort() override;

protected:
    muse::Ret doWriteAndWait(project::INotationProjectPtr project, muse::io::IODevice& dstDevice,
                             const muse::audio::SoundTrackFormat& format);

private:
    void doWrite(muse::io::IODevice& dstDevice, const muse::audio::SoundTrackFormat& format);

    project::WriteUnitType unitTypeFromOptions(const project::WriteOptions& options) const;

    muse::Progress m_progress;
    bool m_isCompleted = false;
    muse::Ret m_writeRet;
};
}

#endif // MU_IMPORTEXPORT_ABSTRACTAUDIOWRITER_H
