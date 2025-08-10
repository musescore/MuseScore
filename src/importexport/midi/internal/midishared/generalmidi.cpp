/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "generalmidi.h"

#include <array>
#include <string_view>

using namespace std::literals;

namespace mu::iex::midi {
namespace {
struct Gm1ProgramData {
    GM1Program program;
    std::string_view midiName;
};

static constexpr std::array GM1_PROGRAM_DATA = {
    Gm1ProgramData{ GM1Program::AcousticGrandPiano, "Grand Piano"sv },
    Gm1ProgramData{ GM1Program::BrightAcousticPiano, "Bright Piano"sv },
    Gm1ProgramData{ GM1Program::ElectricGrandPiano, "E.Grand"sv },
    Gm1ProgramData{ GM1Program::HonkyTonkPiano, "Honky-tonk"sv },
    Gm1ProgramData{ GM1Program::ElectricPiano1, "E.Piano"sv },
    Gm1ProgramData{ GM1Program::ElectricPiano2, "E.Piano 2"sv },
    Gm1ProgramData{ GM1Program::Harpsichord, "Harpsichord"sv },
    Gm1ProgramData{ GM1Program::Clavi, "Clav."sv },
    Gm1ProgramData{ GM1Program::Celesta, "Celesta"sv },
    Gm1ProgramData{ GM1Program::Glockenspiel, "Glockenspiel"sv },
    Gm1ProgramData{ GM1Program::MusicBox, "Music Box"sv },
    Gm1ProgramData{ GM1Program::Vibraphone, "Vibraphone"sv },
    Gm1ProgramData{ GM1Program::Marimba, "Marimba"sv },
    Gm1ProgramData{ GM1Program::Xylophone, "Xylophone"sv },
    Gm1ProgramData{ GM1Program::TubularBells, "Tubular Bells"sv },
    Gm1ProgramData{ GM1Program::Dulcimer, "Dulcimer"sv },
    Gm1ProgramData{ GM1Program::DrawbarOrgan, "Drawbar Organ"sv },
    Gm1ProgramData{ GM1Program::PercussiveOrgan, "Perc. Organ"sv },
    Gm1ProgramData{ GM1Program::RockOrgan, "Rock Organ"sv },
    Gm1ProgramData{ GM1Program::ChurchOrgan, "Church Organ"sv },
    Gm1ProgramData{ GM1Program::ReedOrgan, "Reed Organ"sv },
    Gm1ProgramData{ GM1Program::Accordion, "Akkordion"sv },
    Gm1ProgramData{ GM1Program::Harmonica, "Harmonica"sv },
    Gm1ProgramData{ GM1Program::TangoAccordion, "Bandoneon"sv },
    Gm1ProgramData{ GM1Program::AcousticGuitarNylon, "Nylon Gtr."sv },
    Gm1ProgramData{ GM1Program::AcousticGuitarSteel, "Steel Gtr."sv },
    Gm1ProgramData{ GM1Program::ElectricGuitarJazz, "Jazz Guitar"sv },
    Gm1ProgramData{ GM1Program::ElectricGuitarClean, "Clean Guitar"sv },
    Gm1ProgramData{ GM1Program::ElectricGuitarMuted, "Muted Guitar"sv },
    Gm1ProgramData{ GM1Program::OverdrivenGuitar, "Overdrive Gtr"sv },
    Gm1ProgramData{ GM1Program::DistortionGuitar, "DistortionGtr"sv },
    Gm1ProgramData{ GM1Program::GuitarHarmonics, "Gtr.Harmonics"sv },
    Gm1ProgramData{ GM1Program::AcousticBass, "Acoustic Bass"sv },
    Gm1ProgramData{ GM1Program::ElectricBassFinger, "Fingered Bass"sv },
    Gm1ProgramData{ GM1Program::ElectricBassPick, "Picked Bass"sv },
    Gm1ProgramData{ GM1Program::FretlessBass, "Fretless Bass"sv },
    Gm1ProgramData{ GM1Program::SlapBass1, "Slap Bass 1"sv },
    Gm1ProgramData{ GM1Program::SlapBass2, "Slap Bass 2"sv },
    Gm1ProgramData{ GM1Program::SynthBass1, "Synth Bass 1"sv },
    Gm1ProgramData{ GM1Program::SynthBass2, "Synth Bass 2"sv },
    Gm1ProgramData{ GM1Program::Violin, "Violin"sv },
    Gm1ProgramData{ GM1Program::Viola, "Viola"sv },
    Gm1ProgramData{ GM1Program::Cello, "Cello"sv },
    Gm1ProgramData{ GM1Program::Contrabass, "Contrabass"sv },
    Gm1ProgramData{ GM1Program::TremoloStrings, "Tremolo Str."sv },
    Gm1ProgramData{ GM1Program::PizzicatoStrings, "PizzicatoStr."sv },
    Gm1ProgramData{ GM1Program::OrchestralHarp, "Harp"sv },
    Gm1ProgramData{ GM1Program::Timpani, "Timpani"sv },
    Gm1ProgramData{ GM1Program::StringEnsemble1, "Strings 1"sv },
    Gm1ProgramData{ GM1Program::StringEnsemble2, "Strings 2"sv },
    Gm1ProgramData{ GM1Program::SynthStrings1, "Syn.Strings 1"sv },
    Gm1ProgramData{ GM1Program::SynthStrings2, "Syn.Strings 2"sv },
    Gm1ProgramData{ GM1Program::ChoirAahs, "Choir Aahs"sv },
    Gm1ProgramData{ GM1Program::VoiceOohs, "Voice Oohs"sv },
    Gm1ProgramData{ GM1Program::SynthVoice, "Synth Voice"sv },
    Gm1ProgramData{ GM1Program::OrchestraHit, "Orchestra Hit"sv },
    Gm1ProgramData{ GM1Program::Trumpet, "Trumpet"sv },
    Gm1ProgramData{ GM1Program::Trombone, "Trombone"sv },
    Gm1ProgramData{ GM1Program::Tuba, "Tuba"sv },
    Gm1ProgramData{ GM1Program::MutedTrumpet, "Muted Trumpet"sv },
    Gm1ProgramData{ GM1Program::FrenchHorn, "French Horn"sv },
    Gm1ProgramData{ GM1Program::BrassSection, "Brass Section"sv },
    Gm1ProgramData{ GM1Program::SynthBrass1, "Synth Brass 1"sv },
    Gm1ProgramData{ GM1Program::SynthBrass2, "Synth Brass 2"sv },
    Gm1ProgramData{ GM1Program::SopranoSax, "Soprano Sax"sv },
    Gm1ProgramData{ GM1Program::AltoSax, "Alto Sax"sv },
    Gm1ProgramData{ GM1Program::TenorSax, "Tenor Sax"sv },
    Gm1ProgramData{ GM1Program::BaritoneSax, "Baritone Sax"sv },
    Gm1ProgramData{ GM1Program::Oboe, "Oboe"sv },
    Gm1ProgramData{ GM1Program::EnglishHorn, "English Horn"sv },
    Gm1ProgramData{ GM1Program::Bassoon, "Bassoon"sv },
    Gm1ProgramData{ GM1Program::Clarinet, "Clarinet"sv },
    Gm1ProgramData{ GM1Program::Piccolo, "Piccolo"sv },
    Gm1ProgramData{ GM1Program::Flute, "Flute"sv },
    Gm1ProgramData{ GM1Program::Recorder, "Recorder"sv },
    Gm1ProgramData{ GM1Program::PanFlute, "Pan Flute"sv },
    Gm1ProgramData{ GM1Program::BlownBottle, "Blown Bottle"sv },
    Gm1ProgramData{ GM1Program::Shakuhachi, "Shakuhachi"sv },
    Gm1ProgramData{ GM1Program::Whistle, "Whistle"sv },
    Gm1ProgramData{ GM1Program::Ocarina, "Ocarina"sv },
    Gm1ProgramData{ GM1Program::Lead1Square, "Square Wave"sv },
    Gm1ProgramData{ GM1Program::Lead2Sawtooth, "Saw Wave"sv },
    Gm1ProgramData{ GM1Program::Lead3Calliope, "Calliope"sv },
    Gm1ProgramData{ GM1Program::Lead4Chiff, "Chiffer Lead"sv },
    Gm1ProgramData{ GM1Program::Lead5Charang, "Charang"sv },
    Gm1ProgramData{ GM1Program::Lead6Voice, "Solo Vox"sv },
    Gm1ProgramData{ GM1Program::Lead7Fifths, "Fifth Saw"sv },
    Gm1ProgramData{ GM1Program::Lead8BassAndLead, "Bass Lead"sv },
    Gm1ProgramData{ GM1Program::Pad1NewAge, "New Age Pad"sv },
    Gm1ProgramData{ GM1Program::Pad2Warm, "Warm Pad"sv },
    Gm1ProgramData{ GM1Program::Pad3Polysynth, "Polysynth Pad"sv },
    Gm1ProgramData{ GM1Program::Pad4Choir, "Choir Pad"sv },
    Gm1ProgramData{ GM1Program::Pad5Bowed, "Bowed Pad"sv },
    Gm1ProgramData{ GM1Program::Pad6Metallic, "Metallic Pad"sv },
    Gm1ProgramData{ GM1Program::Pad7Halo, "Halo Pad"sv },
    Gm1ProgramData{ GM1Program::Pad8Sweep, "Sweep Pad"sv },
    Gm1ProgramData{ GM1Program::FX1Rain, "Rain"sv },
    Gm1ProgramData{ GM1Program::FX2Soundtrack, "Soundtrack"sv },
    Gm1ProgramData{ GM1Program::FX3Crystal, "Crystal"sv },
    Gm1ProgramData{ GM1Program::FX4Atmosphere, "Athmosphere"sv },
    Gm1ProgramData{ GM1Program::FX5Brightness, "Brightness"sv },
    Gm1ProgramData{ GM1Program::FX6Goblins, "Goblins"sv },
    Gm1ProgramData{ GM1Program::FX7Echoes, "Echoes"sv },
    Gm1ProgramData{ GM1Program::FX8SciFi, "Sci-Fi"sv },
    Gm1ProgramData{ GM1Program::Sitar, "Sitar"sv },
    Gm1ProgramData{ GM1Program::Banjo, "Banjo"sv },
    Gm1ProgramData{ GM1Program::Shamisen, "Shamisen"sv },
    Gm1ProgramData{ GM1Program::Koto, "Koto"sv },
    Gm1ProgramData{ GM1Program::Kalimba, "Kalimba"sv },
    Gm1ProgramData{ GM1Program::BagPipe, "Bagpipe"sv },
    Gm1ProgramData{ GM1Program::Fiddle, "Fiddle"sv },
    Gm1ProgramData{ GM1Program::Shanai, "Shanai"sv },
    Gm1ProgramData{ GM1Program::TinkleBell, "Tinkle Bell"sv },
    Gm1ProgramData{ GM1Program::Agogo, "Agogo"sv },
    Gm1ProgramData{ GM1Program::SteelDrums, "Steel Drums"sv },
    Gm1ProgramData{ GM1Program::Woodblock, "Woodblock"sv },
    Gm1ProgramData{ GM1Program::TaikoDrum, "Taiko Drum"sv },
    Gm1ProgramData{ GM1Program::MelodicTom, "Melodic Drum"sv },
    Gm1ProgramData{ GM1Program::SynthDrum, "Synth Drum"sv },
    Gm1ProgramData{ GM1Program::ReverseCymbal, "Rev. Cymbal"sv },
    Gm1ProgramData{ GM1Program::GuitarFretNoise, "GtrFret Noise"sv },
    Gm1ProgramData{ GM1Program::BreathNoise, "Breath Noise"sv },
    Gm1ProgramData{ GM1Program::Seashore, "Seashore"sv },
    Gm1ProgramData{ GM1Program::BirdTweet, "Bird Tweed"sv },
    Gm1ProgramData{ GM1Program::TelephoneRing, "Telephone"sv },
    Gm1ProgramData{ GM1Program::Helicopter, "Helicopter"sv },
    Gm1ProgramData{ GM1Program::Applause, "Applaus"sv },
    Gm1ProgramData{ GM1Program::Gunshot, "Gunshot"sv },
};
static_assert(GM1_PROGRAM_DATA.size() == 128);
} // namespace
} // namespace mu::iex::midi

std::string_view mu::iex::midi::getMidiName(const GM1Program program)
{
    const auto& data = GM1_PROGRAM_DATA[static_cast<std::size_t>(program)];

    return data.midiName;
}
