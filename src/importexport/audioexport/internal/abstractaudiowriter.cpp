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
#include "abstractaudiowriter.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QThread>

#include "global/containers.h"
#include "audio/iaudiooutput.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace mu::iex::audioexport;
using namespace mu::project;
using namespace mu::notation;

std::vector<INotationWriter::UnitType> AbstractAudioWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool AbstractAudioWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

Ret AbstractAudioWriter::write(INotationPtr, io::IODevice&, const Options& options)
{
    IF_ASSERT_FAILED(unitTypeFromOptions(options) != UnitType::MULTI_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    if (supportsUnitType(muse::value(options, OptionKey::UNIT_TYPE, Val(UnitType::PER_PAGE)).toEnum<UnitType>())) {
        NOT_IMPLEMENTED;
        return Ret(Ret::Code::NotImplemented);
    }

    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

Ret AbstractAudioWriter::writeList(const INotationPtrList&, io::IODevice&, const Options& options)
{
    IF_ASSERT_FAILED(unitTypeFromOptions(options) == UnitType::MULTI_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    if (supportsUnitType(muse::value(options, OptionKey::UNIT_TYPE, Val(UnitType::PER_PAGE)).toEnum<UnitType>())) {
        NOT_IMPLEMENTED;
        return Ret(Ret::Code::NotImplemented);
    }

    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

void AbstractAudioWriter::abort()
{
    playback()->audioOutput()->abortSavingAllSoundTracks();
}

muse::Progress* AbstractAudioWriter::progress()
{
    return &m_progress;
}

Ret AbstractAudioWriter::doWriteAndWait(INotationPtr notation,
                                        io::IODevice& destinationDevice,
                                        const SoundTrackFormat& format)
{
    //!Note Temporary workaround, since QIODevice is the alias for QIODevice, which falls with SIGSEGV
    //!     on any call from background thread. Once we have our own implementation of QIODevice
    //!     we can pass QIODevice directly into IPlayback::IAudioOutput::saveSoundTrack

    QString path = QString::fromStdString(destinationDevice.meta("file_path"));
    IF_ASSERT_FAILED(!path.isEmpty()) {
        return make_ret(Ret::Code::InternalError);
    }

    m_isCompleted = false;
    m_writeRet = muse::Ret();

    playbackController()->setNotation(notation);
    playbackController()->setIsExportingAudio(true);

    m_progress.finished.onReceive(this, [this](const auto&) {
        playbackController()->setIsExportingAudio(false);
        playbackController()->setNotation(globalContext()->currentNotation());
    });

    playback()->sequenceIdList()
    .onResolve(this, [this, path, &format](const TrackSequenceIdList& sequenceIdList) {
        m_progress.started.notify();

        for (const TrackSequenceId sequenceId : sequenceIdList) {
            playback()->audioOutput()->saveSoundTrackProgress(sequenceId).progressChanged
            .onReceive(this, [this](int64_t current, int64_t total, std::string title) {
                m_progress.progressChanged.send(current, total, title);
            });

            playback()->audioOutput()->saveSoundTrack(sequenceId, muse::io::path_t(path), std::move(format))
            .onResolve(this, [this, path](const bool /*result*/) {
                LOGD() << "Successfully saved sound track by path: " << path;
                m_writeRet = muse::make_ok();
                m_isCompleted = true;
                m_progress.finished.send(muse::make_ok());
            })
            .onReject(this, [this](int errorCode, const std::string& msg) {
                m_writeRet = Ret(errorCode, msg);
                m_isCompleted = true;
                m_progress.finished.send(make_ret(errorCode, msg));
            });
        }
    })
    .onReject(this, [](int errorCode, const std::string& msg) {
        LOGE() << "errorCode: " << errorCode << ", " << msg;
    });

    while (!m_isCompleted) {
        qApp->processEvents();
        QThread::yieldCurrentThread();
    }

    return m_writeRet;
}

INotationWriter::UnitType AbstractAudioWriter::unitTypeFromOptions(const Options& options) const
{
    std::vector<UnitType> supported = supportedUnitTypes();
    IF_ASSERT_FAILED(!supported.empty()) {
        return UnitType::PER_PART;
    }

    UnitType defaultUnitType = supported.front();
    UnitType unitType = muse::value(options, OptionKey::UNIT_TYPE, Val(defaultUnitType)).toEnum<UnitType>();
    if (!supportsUnitType(unitType)) {
        return defaultUnitType;
    }

    return unitType;
}
