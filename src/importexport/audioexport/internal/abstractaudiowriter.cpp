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

#include <QThread>

#include "global/io/file.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace mu::iex::audioexport;
using namespace mu::project;
using namespace mu::notation;

std::vector<WriteUnitType> AbstractAudioWriter::supportedUnitTypes() const
{
    return { WriteUnitType::PER_PART };
}

bool AbstractAudioWriter::supportsUnitType(WriteUnitType unitType) const
{
    std::vector<WriteUnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

Ret AbstractAudioWriter::write(INotationProjectPtr, io::IODevice&, const WriteOptions&)
{
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}

Ret AbstractAudioWriter::write(INotationProjectPtr project, const muse::io::path_t& filePath, const WriteOptions& options)
{
    muse::io::File file(filePath);
    if (!file.open(muse::io::IODevice::WriteOnly)) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ret ret = write(project, file, options);
    file.close();
    return ret;
}

void AbstractAudioWriter::abort()
{
    playback()->abortSavingAllSoundTracks();
    m_isCompleted = true;
}

muse::Progress* AbstractAudioWriter::progress()
{
    return &m_progress;
}

Ret AbstractAudioWriter::doWriteAndWait(project::INotationProjectPtr project,
                                        io::IODevice& dstDevice,
                                        const SoundTrackFormat& format)
{
    IF_ASSERT_FAILED(project) {
        return make_ret(Ret::Code::UnknownError);
    }

    //! NOTE Waiting for the audio system to start if it is not already running
    while (!startAudioController()->isAudioStarted()) {
        application()->processEvents();
        QThread::yieldCurrentThread();
    }

    m_isCompleted = false;
    m_writeRet = muse::Ret();

    INotationPtr notation = project->masterNotation()->notation();

    playbackController()->setNotation(notation);
    playbackController()->setIsExportingAudio(true);

    doWrite(dstDevice, format);

    while (!m_isCompleted) {
        application()->processEvents();
        QThread::yieldCurrentThread();
    }

    playbackController()->setIsExportingAudio(false);
    playbackController()->setNotation(globalContext()->currentNotation());

    return m_writeRet;
}

void AbstractAudioWriter::doWrite(io::IODevice& dstDevice, const SoundTrackFormat& format)
{
    const std::string processingOnlineSoundsMsg = trc("iex_audio", "Processing online sounds…");

    auto sendProgress = [this, processingOnlineSoundsMsg](int64_t current, int64_t total, SaveSoundTrackStage stage) {
        switch (stage) {
        case SaveSoundTrackStage::ProcessingOnlineSounds:
            m_progress.progress(current, total, processingOnlineSoundsMsg);
            break;
        case SaveSoundTrackStage::WritingSoundTrack:
        case SaveSoundTrackStage::Unknown:
            m_progress.progress(current, total);
            break;
        }
    };

    playback()->sequenceIdList()
    .onResolve(this, [this, &dstDevice, format, sendProgress](const TrackSequenceIdList& sequenceIdList) {
        m_progress.start();

        for (const TrackSequenceId sequenceId : sequenceIdList) {
            playback()->saveSoundTrackProgressChanged(sequenceId)
            .onReceive(this, [sendProgress](int64_t current, int64_t total, SaveSoundTrackStage stage) {
                sendProgress(current, total, stage);
            });

            playback()->saveSoundTrack(sequenceId, std::move(format), dstDevice)
            .onResolve(this, [this, sequenceId](const bool /*result*/) {
                LOGI() << "Successfully saved sound track";
                m_writeRet = muse::make_ok();
                m_isCompleted = true;
                m_progress.finish(muse::make_ok());
                playback()->saveSoundTrackProgressChanged(sequenceId).disconnect(this);
            })
            .onReject(this, [this, sequenceId](int errorCode, const std::string& msg) {
                m_writeRet = Ret(errorCode, msg);
                m_isCompleted = true;
                m_progress.finish(make_ret(errorCode, msg));
                playback()->saveSoundTrackProgressChanged(sequenceId).disconnect(this);
            });
        }
    })
    .onReject(this, [this](int errorCode, const std::string& msg) {
        LOGE() << "errorCode: " << errorCode << ", " << msg;
        m_isCompleted = true;
    });
}

WriteUnitType AbstractAudioWriter::unitTypeFromOptions(const WriteOptions& options) const
{
    std::vector<WriteUnitType> supported = supportedUnitTypes();
    IF_ASSERT_FAILED(!supported.empty()) {
        return WriteUnitType::PER_PART;
    }

    WriteUnitType defaultUnitType = supported.front();
    WriteUnitType unitType
        = static_cast<WriteUnitType>(value(options, WriteOptionKey::UNIT_TYPE, Val(static_cast<int>(defaultUnitType))).toInt());
    if (!supportsUnitType(unitType)) {
        return defaultUnitType;
    }

    return unitType;
}
