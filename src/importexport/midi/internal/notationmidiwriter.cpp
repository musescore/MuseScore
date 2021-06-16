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
using namespace mu::system;

mu::Ret NotationMidiWriter::write(notation::INotationPtr notation, IODevice& destinationDevice, const Options& options)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ms::Score* score = notation->elements()->msScore();

    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ms::ExportMidi em(score);
    em.write(&destinationDevice, notationConfiguration()->isPlayRepeatsEnabled(), false, score->synthesizerState());
//    return em.write(device, preferences.getBool(PREF_IO_MIDI_EXPANDREPEATS), preferences.getBool(PREF_IO_MIDI_EXPORTRPNS),
//                    synthesizerState());

    return make_ret(Ret::Code::NotImplemented);
}
