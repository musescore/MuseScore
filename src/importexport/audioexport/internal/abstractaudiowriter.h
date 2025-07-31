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
#include "iaudioexportconfiguration.h"
#include "context/iglobalcontext.h"
#include "playback/iplaybackcontroller.h"

#include "project/inotationwriter.h"

namespace mu::iex::audioexport {
class AbstractAudioWriter : public project::INotationWriter, public muse::Injectable, public muse::async::Asyncable
{
public:
    muse::Inject<muse::audio::IPlayback> playback = { this };
    muse::Inject<IAudioExportConfiguration> configuration = { this };
    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<playback::IPlaybackController> playbackController  = { this };

public:
    AbstractAudioWriter(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    std::vector<UnitType> supportedUnitTypes() const override;
    bool supportsUnitType(UnitType unitType) const override;

    muse::Ret write(notation::INotationPtr notation, muse::io::IODevice& dstDevice, const Options& options = Options()) override;
    muse::Ret writeList(const notation::INotationPtrList& notations, muse::io::IODevice& dstDevice,
                        const Options& options = Options()) override;

    muse::Progress* progress() override;
    void abort() override;

protected:
    muse::Ret doWriteAndWait(notation::INotationPtr notation, muse::io::IODevice& dstDevice, const muse::audio::SoundTrackFormat& format);

private:
    void doWrite(const QString& path, const muse::audio::SoundTrackFormat& format, bool startProgress = true);

    UnitType unitTypeFromOptions(const Options& options) const;

    muse::Progress m_progress;
    bool m_isCompleted = false;
    muse::Ret m_writeRet;
};
}

#endif // MU_IMPORTEXPORT_ABSTRACTAUDIOWRITER_H
