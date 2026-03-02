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

#include "notationmidiwriter.h"

#include <QBuffer>

#include "global/io/file.h"

#include "midiexport/exportmidi.h"

#include "log.h"

using namespace mu::iex::midi;
using namespace muse;
using namespace muse::io;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::engraving;

std::vector<WriteUnitType> NotationMidiWriter::supportedUnitTypes() const
{
    return { WriteUnitType::PER_PART };
}

bool NotationMidiWriter::supportsUnitType(WriteUnitType unitType) const
{
    std::vector<WriteUnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

Ret NotationMidiWriter::write(INotationProjectPtr project, muse::io::IODevice& destinationDevice, const WriteOptions& /*options*/)
{
    IF_ASSERT_FAILED(project) {
        return make_ret(Ret::Code::UnknownError);
    }

    Score* score = project->masterNotation()->notation()->elements()->msScore();

    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    ExportMidi exportMidi(score);

    bool isPlayRepeatsEnabled = midiImportExportConfiguration()->isExpandRepeats();
    bool isMidiExportRpns = midiImportExportConfiguration()->isMidiExportRpns();
    SynthesizerState synthesizerState = score->synthesizerState();

    QByteArray qdata;
    QBuffer buf(&qdata);
    buf.open(QIODevice::WriteOnly);

    bool ok = exportMidi.write(&buf, isPlayRepeatsEnabled, isMidiExportRpns, synthesizerState);
    if (ok) {
        ByteArray data = ByteArray::fromQByteArrayNoCopy(qdata);
        destinationDevice.write(data);
    }

    return ok ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret NotationMidiWriter::write(INotationProjectPtr project, const muse::io::path_t& filePath, const WriteOptions& options)
{
    muse::io::File file(filePath);
    if (!file.open(IODevice::WriteOnly)) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ret ret = write(project, file, options);
    file.close();
    return ret;
}
