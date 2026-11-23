/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <functional>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "audio/main/iplayback.h"
#include "audio/main/istartaudiocontroller.h"
#include "iaudioexportconfiguration.h"
#include "project/iprojectsession.h"
#include "playback/iplaybackcontroller.h"
#include "global/iapplication.h"

#include "project/inotationwriter.h"

namespace mu::iex::audioexport {
class AbstractAudioWriter : public project::INotationWriter, public muse::Contextable, public muse::async::Asyncable
{
public:
    muse::GlobalInject<IAudioExportConfiguration> configuration;
    muse::GlobalInject<muse::IApplication> application;
    muse::GlobalInject<muse::audio::IStartAudioController> startAudioController;

public:
    AbstractAudioWriter(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    std::vector<UnitType> supportedUnitTypes() const override;
    bool supportsUnitType(UnitType unitType) const override;

    muse::Ret writeSync(notation::INotationPtr notation, muse::io::IODevice& dstDevice, const Options& options = Options()) override;
    muse::async::Promise<muse::Ret> write(notation::INotationPtr notation, muse::io::IODevice& dstDevice,
                                          const Options& options = Options()) override;
    muse::Ret writeListSync(const notation::INotationPtrList& notations, muse::io::IODevice& dstDevice,
                            const Options& options = Options()) override;

    muse::Progress* progress() override;
    void abort() override;

protected:
    virtual muse::audio::SoundTrackFormat soundTrackFormat() const = 0;

private:
    using Finished = std::function<void (const muse::Ret&)>;

    void doWriteAsync(notation::INotationPtr notation, muse::io::IODevice& dstDevice, const Options& options, const Finished& onFinished);
    void doWrite(muse::io::IODevice& dstDevice, const muse::audio::SoundTrackFormat& format, uint64_t writeId);
    void finishSaving(const muse::Ret& ret);
    void finish(const muse::Ret& ret);

    //! NOTE Calls the function now if the condition is met, otherwise when the channel notifies and the condition is met
    void whenReady(
        const std::function<bool()>& isReady, const std::function<muse::async::Channel<bool>()>& changed,
        const std::function<void()>& callback);

    UnitType unitTypeFromOptions(const Options& options) const;

    muse::modularity::ContextPtr m_iocContext;
    muse::Progress m_progress;
    Finished m_onFinished;

    notation::INotationPtr m_notationForRestore;

    //! NOTE The id of the current write, the callbacks of a canceled (or replaced) write are ignored
    uint64_t m_writeId = 0;
    bool m_isSaving = false;
};
}

#endif // MU_IMPORTEXPORT_ABSTRACTAUDIOWRITER_H
