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

#include "mp3writer.h"

#include "log.h"

using namespace mu::iex::audioexport;
using namespace mu::framework;

mu::Ret Mp3Writer::write(notation::INotationPtr notation, io::Device& destinationDevice, const Options& options)
{
    UNUSED(notation)
    UNUSED(options)

    //!Note Temporary workaround, since io::Device is the alias for QIODevice, which falls with SIGSEGV
    //!     on any call from background thread. Once we have our own implementation of io::Device
    //!     we can pass io::Device directly into IPlayback::IAudioOutput::saveSoundTrack
    QFile* file = qobject_cast<QFile*>(&destinationDevice);

    QFileInfo info(*file);
    QString path = info.absoluteFilePath();

    //TODO Take actual data
    audio::TrackSequenceId currentSequenceId = 0;
    audio::SoundTrackFormat format { audio::SoundTrackType::MP3, 44100, 2, 128 };

    playback()->audioOutput()->saveSoundTrack(currentSequenceId, io::path(info.absoluteFilePath()), std::move(format))
    .onResolve(this, [path](const bool /*result*/) {
        LOGD() << "Successfully saved sound track by path: " << path;
    })
    .onReject(this, [](int errorCode, const std::string& msg) {
        LOGE() << "errorCode: " << errorCode << ", " << msg;
    });

    return make_ret(Ret::Code::Ok);
}
