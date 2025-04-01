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

#include "midiexport/exportmidi.h"

#include "log.h"

using namespace mu::iex::midi;
using namespace muse;
using namespace muse::io;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::engraving;

std::vector<INotationWriter::UnitType> NotationMidiWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART };
}

bool NotationMidiWriter::supportsUnitType(UnitType unitType) const
{
    std::vector<UnitType> unitTypes = supportedUnitTypes();
    return std::find(unitTypes.cbegin(), unitTypes.cend(), unitType) != unitTypes.cend();
}

Ret NotationMidiWriter::write(INotationPtr notation, io::IODevice& destinationDevice, const Options&)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    Score* score = notation->elements()->msScore();

    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    ExportMidi exportMidi(score);

    bool isPlayRepeatsEnabled = midiImportExportConfiguration()->isExpandRepeats();
    bool isMidiExportRpns = midiImportExportConfiguration()->isMidiExportRpns();
    bool isMidiSpaceLyrics = midiImportExportConfiguration()->isMidiSpaceLyrics();
    SynthesizerState synthesizerState = score->synthesizerState();

    QByteArray qdata;
    QBuffer buf(&qdata);
    buf.open(QIODevice::WriteOnly);

    bool ok = exportMidi.write(&buf, isPlayRepeatsEnabled, isMidiExportRpns, synthesizerState, isMidiSpaceLyrics);
    if (ok) {
        ByteArray data = ByteArray::fromQByteArrayNoCopy(qdata);
        destinationDevice.write(data);
    }

    return ok ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}

Ret NotationMidiWriter::writeList(const notation::INotationPtrList&, io::IODevice&, const Options&)
{
    NOT_SUPPORTED;
    return Ret(Ret::Code::NotSupported);
}
