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

#include "notationmidiwriter.h"

#include "log.h"
#include "midiexport/exportmidi.h"

using namespace mu::iex::midi;
using namespace mu::io;
using namespace mu::notation;

mu::Ret NotationMidiWriter::write(INotationPtr notation, io::Device& destinationDevice, const Options&)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ms::Score* score = notation->elements()->msScore();

    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ms::ExportMidi exportMidi(score);

    bool isPlayRepeatsEnabled = notationConfiguration()->isPlayRepeatsEnabled();
    bool isMidiExportRpns = midiImportExportConfiguration()->isMidiExportRpns();
    Ms::SynthesizerState synthesizerState = score->synthesizerState();

    bool ok = exportMidi.write(&destinationDevice, isPlayRepeatsEnabled, isMidiExportRpns, synthesizerState);

    return ok ? make_ret(Ret::Code::Ok) : make_ret(Ret::Code::InternalError);
}
