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
#include "abstractaudiowriter.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QThread>

#include "audio/iaudiooutput.h"

#include "log.h"

using namespace mu::iex::audioexport;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::framework;

std::vector<INotationWriter::UnitType> AbstractAudioWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool AbstractAudioWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

mu::Ret AbstractAudioWriter::write(INotationPtr, QIODevice&, const Options& options)
{
    IF_ASSERT_FAILED(unitTypeFromOptions(options) != UnitType::MULTI_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    if (supportsUnitType(options.value(OptionKey::UNIT_TYPE, Val(UnitType::PER_PAGE)).toEnum<UnitType>())) {
        NOT_IMPLEMENTED;
        return Ret(Ret::Code::NotImplemented);
    }

    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

mu::Ret AbstractAudioWriter::writeList(const INotationPtrList&, QIODevice&, const Options& options)
{
    IF_ASSERT_FAILED(unitTypeFromOptions(options) == UnitType::MULTI_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    if (supportsUnitType(options.value(OptionKey::UNIT_TYPE, Val(UnitType::PER_PAGE)).toEnum<UnitType>())) {
        NOT_IMPLEMENTED;
        return Ret(Ret::Code::NotImplemented);
    }

    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

void AbstractAudioWriter::abort()
{
    NOT_IMPLEMENTED;
}

bool AbstractAudioWriter::supportsProgressNotifications() const
{
    return true;
}

mu::framework::Progress AbstractAudioWriter::progress() const
{
    return m_progress;
}

void AbstractAudioWriter::doWriteAndWait(INotationPtr notation, QIODevice& destinationDevice, const audio::SoundTrackFormat& format)
{
    //!Note Temporary workaround, since QIODevice is the alias for QIODevice, which falls with SIGSEGV
    //!     on any call from background thread. Once we have our own implementation of QIODevice
    //!     we can pass QIODevice directly into IPlayback::IAudioOutput::saveSoundTrack
    QFile* file = qobject_cast<QFile*>(&destinationDevice);

    QFileInfo info(*file);
    QString path = info.absoluteFilePath();

    m_isCompleted = false;

    playbackController()->setNotation(notation);
    playbackController()->setIsExportingAudio(true);

    m_progress.finished.onReceive(this, [this](const auto&) {
        playbackController()->setIsExportingAudio(false);
        playbackController()->setNotation(globalContext()->currentNotation());
    });

    playback()->sequenceIdList()
    .onResolve(this, [this, path, &format](const audio::TrackSequenceIdList& sequenceIdList) {
        m_progress.started.notify();

        for (const audio::TrackSequenceId sequenceId : sequenceIdList) {
            playback()->audioOutput()->saveSoundTrackProgress(sequenceId).progressChanged
            .onReceive(this, [this](int64_t current, int64_t total, std::string title) {
                m_progress.progressChanged.send(current, total, title);
            });

            playback()->audioOutput()->saveSoundTrack(sequenceId, io::path_t(path), std::move(format))
            .onResolve(this, [this, path](const bool /*result*/) {
                LOGD() << "Successfully saved sound track by path: " << path;
                m_isCompleted = true;
                m_progress.finished.send(make_ok());
            })
            .onReject(this, [this](int errorCode, const std::string& msg) {
                m_isCompleted  = true;
                m_progress.finished.send(make_ret(errorCode, msg));
            });
        }
    })
    .onReject(this, [](int errorCode, const std::string& msg) {
        LOGE() << "errorCode: " << errorCode << ", " << msg;
    });

    while (!m_isCompleted) {
        QApplication::instance()->processEvents();
        QThread::yieldCurrentThread();
    }
}

INotationWriter::UnitType AbstractAudioWriter::unitTypeFromOptions(const Options& options) const
{
    std::vector<UnitType> supported = supportedUnitTypes();
    IF_ASSERT_FAILED(!supported.empty()) {
        return UnitType::PER_PART;
    }

    UnitType defaultUnitType = supported.front();
    UnitType unitType = options.value(OptionKey::UNIT_TYPE, Val(defaultUnitType)).toEnum<UnitType>();
    if (!supportsUnitType(unitType)) {
        return defaultUnitType;
    }

    return unitType;
}
