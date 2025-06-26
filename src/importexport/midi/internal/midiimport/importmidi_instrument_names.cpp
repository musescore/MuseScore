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

#include "importmidi_instrument_names.h"

namespace mu::iex::midi {
static constexpr MidiInstrument minstr[] = {
    // Piano
    { 0, "Grand Piano" },
    { 1, "Bright Piano" },
    { 2, "E.Grand" },
    { 3, "Honky-tonk" },
    { 4, "E.Piano" },
    { 5, "E.Piano 2" },
    { 6, "Harpsichord" },
    { 7, "Clav." },

// Chromatic Perc
    { 8, "Celesta" },
    { 9, "Glockenspiel" },
    { 10, "Music Box" },
    { 11, "Vibraphone" },
    { 12, "Marimba" },
    { 13, "Xylophone" },
    { 14, "Tubular Bells" },
    { 15, "Dulcimer" },
// Organ
    { 16, "Drawbar Organ" },
    { 17, "Perc. Organ" },
    { 18, "Rock Organ" },
    { 19, "Church Organ" },
    { 20, "Reed Organ" },
    { 21, "Akkordion" },
    { 22, "Harmonica" },
    { 23, "Bandoneon" },
// Guitar
    { 24, "Nylon Gtr." },
    { 25, "Steel Gtr." },
    { 26, "Jazz Guitar" },
    { 27, "Clean Guitar" },
    { 28, "Muted Guitar" },
    { 29, "Overdrive Gtr" },
    { 30, "DistortionGtr" },
    { 31, "Gtr.Harmonics" },
// Bass
    { 32, "Acoustic Bass" },
    { 33, "Fingered Bass" },
    { 34, "Picked Bass" },
    { 35, "Fretless Bass" },
    { 36, "Slap Bass 1" },
    { 37, "Slap Bass 2" },
    { 38, "Synth Bass 1" },
    { 39, "Synth Bass 2" },
// Strings/Orch
    { 40, "Violin" },
    { 41, "Viola" },
    { 42, "Cello" },
    { 43, "Contrabass" },
    { 44, "Tremolo Str." },
    { 45, "PizzicatoStr." },
    { 46, "Harp" },
    { 47, "Timpani" },
// Ensemble
    { 48, "Strings 1" },
    { 49, "Strings 2" },
    { 50, "Syn.Strings 1" },
    { 51, "Syn.Strings 2" },
    { 52, "Choir Aahs" },
    { 53, "Voice Oohs" },
    { 54, "Synth Voice" },
    { 55, "Orchestra Hit" },
// Brass
    { 56, "Trumpet" },
    { 57, "Trombone" },
    { 58, "Tuba" },
    { 59, "Muted Trumpet" },
    { 60, "French Horn" },
    { 61, "Brass Section" },
    { 62, "Synth Brass 1" },
    { 63, "Synth Brass 2" },
// Reed
    { 64, "Soprano Sax" },
    { 65, "Alto Sax" },
    { 66, "Tenor Sax" },
    { 67, "Baritone Sax" },
    { 68, "Oboe" },
    { 69, "English Horn" },
    { 70, "Bassoon" },
    { 71, "Clarinet" },
// Pipe
    { 72, "Piccolo" },
    { 73, "Flute" },
    { 74, "Recorder" },
    { 75, "Pan Flute" },
    { 76, "Blown Bottle" },
    { 77, "Shakuhachi" },
    { 78, "Whistle" },
    { 79, "Ocarina" },
// SynthLead
    { 80, "Square Wave" },
    { 81, "Saw Wave" },
    { 82, "Calliope" },
    { 83, "Chiffer Lead" },
    { 84, "Charang" },
    { 85, "Solo Vox" },
    { 86, "Fifth Saw" },
    { 87, "Bass Lead" },
// Synth Pad
    { 88, "New Age Pad" },
    { 89, "Warm Pad" },
    { 90, "Polysynth Pad" },
    { 91, "Choir Pad" },
    { 92, "Bowed Pad" },
    { 93, "Metallic Pad" },
    { 94, "Halo Pad" },
    { 95, "Sweep Pad" },
// Synth FX
    { 96, "Rain" },
    { 97, "Soundtrack" },
    { 98, "Crystal" },
    { 99, "Athmosphere" },
    { 100, "Brightness" },
    { 101, "Goblins" },
    { 102, "Echoes" },
    { 103, "Sci-Fi" },
// Ethnic
    { 104, "Sitar" },
    { 105, "Banjo" },
    { 106, "Shamisen" },
    { 107, "Koto" },
    { 108, "Kalimba" },
    { 109, "Bagpipe" },
    { 110, "Fiddle" },
    { 111, "Shanai" },
// Percussive
    { 112, "Tinkle Bell" },
    { 113, "Agogo" },
    { 114, "Steel Drums" },
    { 115, "Woodblock" },
    { 116, "Taiko Drum" },
    { 117, "Melodic Drum" },
    { 118, "Synth Drum" },
    { 119, "Rev. Cymbal" },
// Special FX
    { 120, "GtrFret Noise" },
    { 121, "Breath Noise" },
    { 122, "Seashore" },
    { 123, "Bird Tweed" },
    { 124, "Telephone" },
    { 125, "Helicopter" },
    { 126, "Applaus" },
    { 127, "Gunshot" },
};

QString MidiInstrument::instrName(const GM1Program program)
{
    const int p = toMidiData(program);

    for (unsigned int i = 0; i < sizeof(minstr) / sizeof(*minstr); ++i) {
        const MidiInstrument& mi = minstr[i];
        if (mi.patch == p) {
            return QString(mi.name);
        }
    }

    return QString();
}
} // namespace namespace mu::iex::midi
