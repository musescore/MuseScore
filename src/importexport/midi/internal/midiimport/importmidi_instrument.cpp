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
#include "importmidi_instrument.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <limits>
#include <set>
#include <string_view>
#include <utility>
#include <vector>

#include "importmidi_chord.h"
#include "importmidi_inner.h"
#include "importmidi_instrument_names.h"
#include "importmidi_operations.h"

#include "engraving/dom/drumset.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/dom/staff.h"

#include "internal/midishared/generalmidi.h"

using namespace std::literals;
using namespace mu::engraving;

namespace mu::engraving {
extern std::vector<const InstrumentGroup*> instrumentGroups;
}

namespace mu::iex::midi {
namespace MidiInstr {
namespace {
struct GM1ProgramData {
    GM1Program program = {};
    std::string_view preferredInstrumentId = {};
};

// General MIDI Level 1
constexpr std::array GM1_SOUND_DATA {
    GM1ProgramData{ GM1Program::AcousticGrandPiano, "grand-piano"sv },
    GM1ProgramData{ GM1Program::BrightAcousticPiano, "piano"sv },
    GM1ProgramData{ GM1Program::ElectricGrandPiano, "electric-piano"sv },
    GM1ProgramData{ GM1Program::HonkyTonkPiano, "honky-tonk-piano"sv },
    GM1ProgramData{ GM1Program::ElectricPiano1, "electric-piano"sv },
    GM1ProgramData{ GM1Program::ElectricPiano2, "electric-piano"sv },
    GM1ProgramData{ GM1Program::Harpsichord, "harpsichord"sv },
    GM1ProgramData{ GM1Program::Clavi, "clavichord"sv },
    GM1ProgramData{ GM1Program::Celesta, "celesta"sv },
    GM1ProgramData{ GM1Program::Glockenspiel, "glockenspiel"sv },
    GM1ProgramData{ GM1Program::MusicBox, "crotales"sv },
    GM1ProgramData{ GM1Program::Vibraphone, "vibraphone"sv },
    GM1ProgramData{ GM1Program::Marimba, "marimba"sv },
    GM1ProgramData{ GM1Program::Xylophone, "xylophone"sv },
    GM1ProgramData{ GM1Program::TubularBells, "tubular-bells"sv },
    GM1ProgramData{ GM1Program::Dulcimer, "dulcimer"sv },
    GM1ProgramData{ GM1Program::DrawbarOrgan, "hammond-organ"sv },
    GM1ProgramData{ GM1Program::PercussiveOrgan, "percussive-organ"sv },
    GM1ProgramData{ GM1Program::RockOrgan, "rotary-organ"sv },
    GM1ProgramData{ GM1Program::ChurchOrgan, "pipe-organ"sv },
    GM1ProgramData{ GM1Program::ReedOrgan, "reed-organ"sv },
    GM1ProgramData{ GM1Program::Accordion, "accordion"sv },
    GM1ProgramData{ GM1Program::Harmonica, "harmonica"sv },
    GM1ProgramData{ GM1Program::TangoAccordion, "bandoneon"sv },
    GM1ProgramData{ GM1Program::AcousticGuitarNylon, "guitar-nylon"sv },
    GM1ProgramData{ GM1Program::AcousticGuitarSteel, "guitar-steel"sv },
    GM1ProgramData{ GM1Program::ElectricGuitarJazz, "electric-guitar"sv },
    GM1ProgramData{ GM1Program::ElectricGuitarClean, "electric-guitar"sv },
    GM1ProgramData{ GM1Program::ElectricGuitarMuted, "electric-guitar"sv },
    GM1ProgramData{ GM1Program::OverdrivenGuitar, "electric-guitar"sv },
    GM1ProgramData{ GM1Program::DistortionGuitar, "electric-guitar"sv },
    GM1ProgramData{ GM1Program::GuitarHarmonics, "electric-guitar"sv },
    GM1ProgramData{ GM1Program::AcousticBass, "acoustic-bass"sv },
    GM1ProgramData{ GM1Program::ElectricBassFinger, "electric-bass"sv },
    GM1ProgramData{ GM1Program::ElectricBassPick, "bass-guitar"sv },
    GM1ProgramData{ GM1Program::FretlessBass, "fretless-electric-bass"sv },
    GM1ProgramData{ GM1Program::SlapBass1, "electric-bass"sv },
    GM1ProgramData{ GM1Program::SlapBass2, "electric-bass"sv },
    GM1ProgramData{ GM1Program::SynthBass1, "bass-synthesizer"sv },
    GM1ProgramData{ GM1Program::SynthBass2, "bass-synthesizer"sv },
    GM1ProgramData{ GM1Program::Violin, "violin"sv },
    GM1ProgramData{ GM1Program::Viola, "viola"sv },
    GM1ProgramData{ GM1Program::Cello, "violoncello"sv },
    GM1ProgramData{ GM1Program::Contrabass, "contrabass"sv },
    GM1ProgramData{ GM1Program::TremoloStrings, "strings"sv },
    GM1ProgramData{ GM1Program::PizzicatoStrings, "strings"sv },
    GM1ProgramData{ GM1Program::OrchestralHarp, "harp"sv },
    GM1ProgramData{ GM1Program::Timpani, "timpani"sv },
    GM1ProgramData{ GM1Program::StringEnsemble1, "strings"sv },
    GM1ProgramData{ GM1Program::StringEnsemble2, "strings"sv },
    GM1ProgramData{ GM1Program::SynthStrings1, "string-synthesizer"sv },
    GM1ProgramData{ GM1Program::SynthStrings2, "string-synthesizer"sv },
    GM1ProgramData{ GM1Program::ChoirAahs, "soprano"sv },
    GM1ProgramData{ GM1Program::VoiceOohs, "soprano"sv },
    GM1ProgramData{ GM1Program::SynthVoice, "voice"sv },
    GM1ProgramData{ GM1Program::OrchestraHit, "brass-synthesizer"sv },
    GM1ProgramData{ GM1Program::Trumpet, "trumpet"sv },
    GM1ProgramData{ GM1Program::Trombone, "trombone"sv },
    GM1ProgramData{ GM1Program::Tuba, "tuba"sv },
    GM1ProgramData{ GM1Program::MutedTrumpet, "trumpet"sv },
    GM1ProgramData{ GM1Program::FrenchHorn, "horn"sv },
    GM1ProgramData{ GM1Program::BrassSection, "brass"sv },
    GM1ProgramData{ GM1Program::SynthBrass1, "brass-synthesizer"sv },
    GM1ProgramData{ GM1Program::SynthBrass2, "brass-synthesizer"sv },
    GM1ProgramData{ GM1Program::SopranoSax, "soprano-saxophone"sv },
    GM1ProgramData{ GM1Program::AltoSax, "alto-saxophone"sv },
    GM1ProgramData{ GM1Program::TenorSax, "tenor-saxophone"sv },
    GM1ProgramData{ GM1Program::BaritoneSax, "baritone-saxophone"sv },
    GM1ProgramData{ GM1Program::Oboe, "oboe"sv },
    GM1ProgramData{ GM1Program::EnglishHorn, "english-horn"sv },
    GM1ProgramData{ GM1Program::Bassoon, "bassoon"sv },
    GM1ProgramData{ GM1Program::Clarinet, "clarinet"sv },
    GM1ProgramData{ GM1Program::Piccolo, "piccolo"sv },
    GM1ProgramData{ GM1Program::Flute, "flute"sv },
    GM1ProgramData{ GM1Program::Recorder, "recorder"sv },
    GM1ProgramData{ GM1Program::PanFlute, "pan-flute"sv },
    GM1ProgramData{ GM1Program::BlownBottle, "pan-flute"sv },
    GM1ProgramData{ GM1Program::Shakuhachi, "shakuhachi"sv },
    GM1ProgramData{ GM1Program::Whistle, "c-tin-whistle"sv },
    GM1ProgramData{ GM1Program::Ocarina, "ocarina"sv },
    GM1ProgramData{ GM1Program::Lead1Square, "square-synth"sv },
    GM1ProgramData{ GM1Program::Lead2Sawtooth, "saw-synth"sv },
    GM1ProgramData{ GM1Program::Lead3Calliope, ""sv },
    GM1ProgramData{ GM1Program::Lead4Chiff, ""sv },
    GM1ProgramData{ GM1Program::Lead5Charang, "tuned-klaxon-horns"sv },
    GM1ProgramData{ GM1Program::Lead6Voice, "kazoo"sv },
    GM1ProgramData{ GM1Program::Lead7Fifths, ""sv },
    GM1ProgramData{ GM1Program::Lead8BassAndLead, ""sv },
    GM1ProgramData{ GM1Program::Pad1NewAge, "new-age-synth"sv },
    GM1ProgramData{ GM1Program::Pad2Warm, "warm-synth"sv },
    GM1ProgramData{ GM1Program::Pad3Polysynth, "poly-synth"sv },
    GM1ProgramData{ GM1Program::Pad4Choir, "choir-synth"sv },
    GM1ProgramData{ GM1Program::Pad5Bowed, "bowed-synth"sv },
    GM1ProgramData{ GM1Program::Pad6Metallic, "metallic-synth"sv },
    GM1ProgramData{ GM1Program::Pad7Halo, "halo-synth"sv },
    GM1ProgramData{ GM1Program::Pad8Sweep, "sweep-synth"sv },
    GM1ProgramData{ GM1Program::FX1Rain, "rain-synth"sv },
    GM1ProgramData{ GM1Program::FX2Soundtrack, "soundtrack-synth"sv },
    GM1ProgramData{ GM1Program::FX3Crystal, "crystal-synth"sv },
    GM1ProgramData{ GM1Program::FX4Atmosphere, "atmosphere-synth"sv },
    GM1ProgramData{ GM1Program::FX5Brightness, "brightness-synth"sv },
    GM1ProgramData{ GM1Program::FX6Goblins, "goblins-synth"sv },
    GM1ProgramData{ GM1Program::FX7Echoes, "echoes-synth"sv },
    GM1ProgramData{ GM1Program::FX8SciFi, "sci-fi-synth"sv },
    GM1ProgramData{ GM1Program::Sitar, "sitar"sv },
    GM1ProgramData{ GM1Program::Banjo, "banjo"sv },
    GM1ProgramData{ GM1Program::Shamisen, "shamisen"sv },
    GM1ProgramData{ GM1Program::Koto, "koto"sv },
    GM1ProgramData{ GM1Program::Kalimba, "kalimba"sv },
    GM1ProgramData{ GM1Program::BagPipe, "bagpipe"sv },
    GM1ProgramData{ GM1Program::Fiddle, "violin"sv },
    GM1ProgramData{ GM1Program::Shanai, "shenai"sv },
    GM1ProgramData{ GM1Program::TinkleBell, "hand-bells"sv },
    GM1ProgramData{ GM1Program::Agogo, "hand-bells"sv },
    GM1ProgramData{ GM1Program::SteelDrums, "steel-drums"sv },
    GM1ProgramData{ GM1Program::Woodblock, "temple-blocks"sv },
    GM1ProgramData{ GM1Program::TaikoDrum, ""sv },
    GM1ProgramData{ GM1Program::MelodicTom, "roto-toms"sv },
    GM1ProgramData{ GM1Program::SynthDrum, ""sv },
    GM1ProgramData{ GM1Program::ReverseCymbal, ""sv },
    GM1ProgramData{ GM1Program::GuitarFretNoise, ""sv },
    GM1ProgramData{ GM1Program::BreathNoise, ""sv },
    GM1ProgramData{ GM1Program::Seashore, ""sv },
    GM1ProgramData{ GM1Program::BirdTweet, ""sv },
    GM1ProgramData{ GM1Program::TelephoneRing, ""sv },
    GM1ProgramData{ GM1Program::Helicopter, ""sv },
    GM1ProgramData{ GM1Program::Applause, ""sv },
    GM1ProgramData{ GM1Program::Gunshot, "cannon"sv },
};
static_assert(GM1_SOUND_DATA.size() == 128);

const InstrumentTemplate* getPreferredInstrument(const GM1Program sound)
{
    const std::string_view id = GM1_SOUND_DATA[static_cast<std::uint8_t>(sound)].preferredInstrumentId;
    InstrumentIndex idx = searchTemplateIndexForId(String::fromUtf8(id));
    if (idx.groupIndex == -1) {
        return nullptr;
    }

    return idx.instrTemplate;
}
}

QString instrumentName(MidiType type, int program, bool isDrumTrack)
{
    if (isDrumTrack) {
        return "Percussion";
    }

    int hbank = -1, lbank = -1;
    if (program == -1) {
        program = 0;
    } else {
        hbank = (program >> 16);
        lbank = (program >> 8) & 0xff;
        program = program & 0xff;
    }
    return MidiInstrument::instrName(int(type), hbank, lbank, program);
}

bool isGrandStaff(const MTrack& t1, const MTrack& t2)
{
    return t1.mtrack->outChannel() == t2.mtrack->outChannel()
           && MChord::isGrandStaffProgram(t1.program)
           && !t1.mtrack->drumTrack() && !t2.mtrack->drumTrack();
}

bool isSameChannel(const MTrack& t1, const MTrack& t2)
{
    return t1.mtrack->outChannel() == t2.mtrack->outChannel();
}

bool is3StaffOrgan(int program)
{
    return program >= 16 && program <= 20;
}

bool areNext2GrandStaff(int currentTrack, const QList<MTrack>& tracks)
{
    if (currentTrack + 1 >= tracks.size()) {
        return false;
    }
    return isGrandStaff(tracks[currentTrack], tracks[currentTrack + 1]);
}

bool areNext3OrganStaff(int currentTrack, const QList<MTrack>& tracks)
{
    if (currentTrack + 2 >= tracks.size()) {
        return false;
    }
    if (!is3StaffOrgan(tracks[currentTrack].program)) {
        return false;
    }

    return isGrandStaff(tracks[currentTrack], tracks[currentTrack + 1])
           && isSameChannel(tracks[currentTrack + 1], tracks[currentTrack + 2]);
}

QString concatenateWithComma(const QString& left, const QString& right)
{
    if (left.isEmpty()) {
        return right;
    }
    if (right.isEmpty()) {
        return left;
    }
    return left + ", " + right;
}

// set program equal to all staves, as it should be
// often only first stave in Grand Staff have correct program, other - default (piano)
// also handle track names
void setGrandStaffProgram(QList<MTrack>& tracks)
{
    for (int i = 0; i < tracks.size(); ++i) {
        MTrack& mt = tracks[i];

        if (areNext3OrganStaff(i, tracks)) {
            tracks[i + 1].program = mt.program;
            tracks[i + 2].program = mt.program;

            mt.name = concatenateWithComma(mt.name, "Manual");
            tracks[i + 1].name = "";
            tracks[i + 2].name = concatenateWithComma(tracks[i + 2].name, "Pedal");

            i += 2;
        } else if (areNext2GrandStaff(i, tracks)) {
            tracks[i + 1].program = mt.program;
            if (mt.name != tracks[i + 1].name) {
                mt.name = "";                     // only one name place near bracket is available
                tracks[i + 1].name = "";          // so instrument name will be used instead
            }
            i += 1;
        }
    }
}

std::set<int> findAllPitches(const MTrack& track)
{
    std::set<int> pitches;
    for (const auto& chord: track.chords) {
        for (const auto& note: chord.second.notes) {
            pitches.insert(note.pitch);
        }
    }
    return pitches;
}

std::set<int> findNotEmptyDrumPitches(const Drumset* drumset)
{
    std::set<int> drumPitches = {};
    for (int i = 0; i != DRUM_INSTRUMENTS; ++i) {
        if (!drumset->name(i).empty()) {
            drumPitches.insert(i);
        }
    }

    return drumPitches;
}

static bool hasNotDefinedDrumPitch(const std::set<int>& trackPitches, const std::set<int>& drumPitches)
{
    for (const int pitch : trackPitches) {
        if (drumPitches.find(pitch) == drumPitches.end()) {
            return true;
        }
    }

    return false;
}

static const InstrumentTemplate* findInstrument(const String& groupId, const String& instrId)
{
    const InstrumentTemplate* instr = nullptr;

    for (const InstrumentGroup* group: instrumentGroups) {
        if (group->id == groupId) {
            for (const InstrumentTemplate* templ: group->instrumentTemplates) {
                if (templ->id == instrId) {
                    instr = templ;
                    break;
                }
            }
            break;
        }
    }
    return instr;
}

// returns number of pitches which are present in p1, but missing in p2
static int countMissingPitches(const std::set<int>& p1, const std::set<int>& p2)
{
    std::vector<int> missingPitches = {};
    missingPitches.reserve(p1.size());

    std::set_difference(p1.begin(), p1.end(), p2.begin(), p2.end(), std::back_inserter(missingPitches));

    return static_cast<int>(missingPitches.size());
}

// find instrument with maximum MIDI program
// that is less than the track MIDI program, i.e. suitable instrument
static const InstrumentTemplate* findClosestInstrument(const int program, const std::set<int>& pitches,
                                                       const bool isDrumTrack)
{
    int maxLessProgram = -1;
    int minMissingPitches = std::numeric_limits<int>::max();
    const InstrumentTemplate* closestTemplate = nullptr;

    for (const InstrumentGroup* group: instrumentGroups) {
        for (const InstrumentTemplate* templ: group->instrumentTemplates) {
            if (templ->staffGroup == StaffGroup::TAB) {
                continue;
            }
            if (isDrumTrack != templ->useDrumset) {
                continue;
            }

            if (isDrumTrack) {
                int missingPitches = std::numeric_limits<int>::max();
                if (templ->drumset) {
                    const std::set<int> drumPitches = findNotEmptyDrumPitches(templ->drumset);
                    missingPitches = countMissingPitches(pitches, drumPitches);
                }

                for (const auto& channel: templ->channel) {
                    if (channel.program() < program
                        && missingPitches < minMissingPitches) {
                        maxLessProgram = channel.program();
                        minMissingPitches = missingPitches;

                        closestTemplate = templ;
                        break;
                    }
                }

                continue;
            }

            for (const auto& channel: templ->channel) {
                if (channel.program() < program
                    && channel.program() > maxLessProgram) {
                    maxLessProgram = channel.program();
                    closestTemplate = templ;
                    break;
                }
            }
        }
    }
    return closestTemplate;
}

static std::vector<const InstrumentTemplate*> findInstrumentsForProgram(const MTrack& track)
{
    std::vector<const InstrumentTemplate*> suitableTemplates;
    const int program = track.program;
    const bool isDrumTrack = track.mtrack->drumTrack();

    const InstrumentTemplate* prefInstr = getPreferredInstrument(GM1Program { static_cast<std::uint8_t>(program) });
    if (prefInstr && !isDrumTrack) {
        suitableTemplates.push_back(prefInstr);
    }

    std::set<int> trackPitches;
    if (isDrumTrack) {
        trackPitches = findAllPitches(track);
    }

    for (const InstrumentGroup* group : instrumentGroups) {
        for (const InstrumentTemplate* templ: group->instrumentTemplates) {
            if (prefInstr && templ->id == prefInstr->id) {
                continue;
            }
            if (templ->staffGroup == StaffGroup::TAB) {
                continue;
            }
            if (isDrumTrack != templ->useDrumset) {
                continue;
            }

            if (isDrumTrack) {
                std::set<int> drumPitches;
                if (templ->drumset) {
                    drumPitches = findNotEmptyDrumPitches(templ->drumset);
                }

                for (const auto& channel: templ->channel) {
                    if (channel.program() == program) {
                        if (templ->drumset) {
                            if (hasNotDefinedDrumPitch(trackPitches, drumPitches)) {
                                break;
                            }
                        }
                        suitableTemplates.push_back(templ);
                        break;
                    }
                }

                continue;
            }

            for (const auto& channel: templ->channel) {
                if (channel.program() == program) {
                    suitableTemplates.push_back(templ);
                    break;
                }
            }
        }
    }

    if (suitableTemplates.empty()) {
        // Ms instruments with the desired MIDI program were not found
        // so we will find the most matching instrument

        if (program >= 80 && program <= 103) {
            const InstrumentTemplate* instr = nullptr;
            if (track.mtrack->drumTrack()) {
                instr = findInstrument(u"electronic-instruments", u"percussion-synthesizer");
            } else {
                instr = findInstrument(u"electronic-instruments", u"effect-synth");
            }
            if (instr) {
                suitableTemplates.push_back(instr);               // generic synth
            }
        } else if (program >= 112 && program <= 127) {
            auto instr = findInstrument(u"unpitched-percussion", u"snare-drum");
            if (instr) {
                suitableTemplates.push_back(instr);               // 1-line percussion staff
            }
        } else {            // find instrument with maximum MIDI program
                            // that is less than the track MIDI program, i.e. suitable instrument
            auto instr = findClosestInstrument(program, trackPitches, isDrumTrack);
            if (instr) {
                suitableTemplates.push_back(instr);
            }
        }
    }

    return suitableTemplates;
}

static std::pair<int, int> findMinMaxPitch(const MTrack& track)
{
    int minPitch = std::numeric_limits<int>::max();
    int maxPitch = -1;

    for (const auto& chord : track.chords) {
        for (const auto& note : chord.second.notes) {
            minPitch = std::min(minPitch, note.pitch);
            maxPitch = std::max(maxPitch, note.pitch);
        }
    }
    return std::pair(minPitch, maxPitch);
}

static int findMaxPitchDiff(const std::pair<int, int>& minMaxPitch, const InstrumentTemplate* templ)
{
    int diff = 0;
    if (minMaxPitch.first < templ->minPitchP) {
        diff = templ->minPitchP - minMaxPitch.first;
    }
    if (minMaxPitch.second > templ->maxPitchP) {
        diff = std::max(diff, minMaxPitch.second - templ->maxPitchP);
    }

    return diff;
}

static bool hasCommonGenre(const std::list<const InstrumentGenre*>& genres)
{
    for (const InstrumentGenre* genre : genres) {
        if (genre->id == "common") {
            return true;
        }
    }
    return false;
}

void sortInstrumentTemplates(
    std::vector<const InstrumentTemplate*>& templates,
    const std::pair<int, int>& minMaxPitch)
{
    std::stable_sort(templates.begin(), templates.end(),
                     [minMaxPitch](const InstrumentTemplate* templ1, const InstrumentTemplate* templ2) {
        const int diff1 = findMaxPitchDiff(minMaxPitch, templ1);
        const int diff2 = findMaxPitchDiff(minMaxPitch, templ2);

        if (diff1 != diff2) {
            return diff1 < diff2;
        }
        // if drumset is not null - it's a particular drum instrument
        // if drum set is null - it's a common drumset
        // so prefer particular drum instruments
        if (templ1->drumset && !templ2->drumset) {
            return true;
        }
        if (!templ1->drumset && templ2->drumset) {
            return false;
        }
        // prefer instruments with the "common" genre
        const bool hasCommon1 = hasCommonGenre(templ1->genres);
        const bool hasCommon2 = hasCommonGenre(templ2->genres);
        if (hasCommon1 && !hasCommon2) {
            return true;
        }
        if (!hasCommon1 && hasCommon2) {
            return false;
        }
        return templ1->genres.size() > templ2->genres.size();
    });
}

std::vector<const InstrumentTemplate*> findSuitableInstruments(const MTrack& track)
{
    std::vector<const InstrumentTemplate*> templates = findInstrumentsForProgram(track);
    if (templates.empty()) {
        return templates;
    }

    const std::pair<int, int> minMaxPitch = findMinMaxPitch(track);
    sortInstrumentTemplates(templates, minMaxPitch);

    // add empty instrument to show no-instrument option
    if (!templates.empty()) {
        templates.push_back(nullptr);
    }

    return templates;
}

void findInstrumentsForAllTracks(const QList<MTrack>& tracks, bool forceReload)
{
    auto& opers = midiImportOperations;
    auto& instrListOption = opers.data()->trackOpers.msInstrList;

    if (forceReload) {
        instrListOption.clear();
    }

    if (opers.data()->processingsOfOpenedFile == 0 || forceReload) {
        // create instrument list on MIDI file opening
        for (const auto& track: tracks) {
            instrListOption.setValue(track.indexOfOperation,
                                     findSuitableInstruments(track));
            if (!instrListOption.value(track.indexOfOperation).empty()) {
                const int defaultInstrIndex = 0;
                opers.data()->trackOpers.msInstrIndex.setDefaultValue(defaultInstrIndex);
            }
        }
    }
}

void instrumentTemplatesChanged()
{
    QStringList files(midiImportOperations.allMidiFiles());
    for (const QString& file : std::as_const(files)) {
        MidiOperations::CurrentMidiFileSetter s(midiImportOperations, file);
        MidiOperations::FileData* data = midiImportOperations.data();
        if (data) {
            findInstrumentsForAllTracks(data->tracks, /* forceReload */ true);
        }
    }
}

void createInstruments(Score* score, QList<MTrack>& tracks)
{
    const auto& opers = midiImportOperations;
    const auto& instrListOption = opers.data()->trackOpers.msInstrList;

    const int ntracks = tracks.size();
    for (int idx = 0; idx < ntracks; ++idx) {
        MTrack& track = tracks[idx];
        Part* part = new Part(score);

        const auto& instrList = instrListOption.value(track.indexOfOperation);
        const InstrumentTemplate* instr = nullptr;
        if (!instrList.empty()) {
            const int instrIndex = opers.data()->trackOpers.msInstrIndex.value(
                track.indexOfOperation);
            instr = instrList[instrIndex];
            if (instr) {
                part->initFromInstrTemplate(instr);
            }
        }

        if (areNext3OrganStaff(idx, tracks)) {
            part->setStaves(3);
        } else if (areNext2GrandStaff(idx, tracks)) {
            part->setStaves(2);
        } else {
            part->setStaves(1);
        }

        if (part->nstaves() == 1) {
            if (track.mtrack->drumTrack()) {
                part->staff(0)->setStaffType(Fraction(0, 1), *StaffType::preset(StaffTypes::PERC_DEFAULT));
                if (!instr) {
                    part->instrument()->setDrumset(smDrumset);
                }
            }
        } else {
            if (!instr) {
                part->staff(0)->setBarLineSpan(true);
                part->staff(0)->setBracketType(0, BracketType::BRACE);
            } else {
                part->staff(0)->setBarLineSpan(instr->barlineSpan[0]);
                part->staff(0)->setBracketType(0, instr->bracket[0]);
            }
            part->staff(0)->setBracketSpan(0, 2);
        }

        if (instr) {
            for (size_t i = 0; i != part->nstaves(); ++i) {
                if (instr->staffTypePreset) {
                    part->staff(i)->init(instr, nullptr, static_cast<int>(i));
                    part->staff(i)->setStaffType(Fraction(0, 1), *(instr->staffTypePreset));
                }
//                        part->staff(i)->setLines(0, instr->staffLines[i]);
//                        part->staff(i)->setSmall(0, instr->smallStaff[i]);
                part->staff(i)->setDefaultClefType(instr->clefTypes[i]);
            }
        }

        for (size_t i = 0; i != part->nstaves(); ++i) {
            if (i > 0) {
                ++idx;
            }
            tracks[idx].staff = part->staff(i);
        }

        // only importing a single volume per track here, skip when multiple volumes
        // are defined, or the single volume is not defined on tick 0.
        if (track.volumes.size() == 1) {
            for (auto& i: track.volumes) {
                if (i.first == ReducedFraction(0, 1)) {
                    part->instrument()->channel(0)->setVolume(i.second);
                }
            }
        }

        score->appendPart(part);
    }
}

QString msInstrName(int trackIndex)
{
    const auto& opers = midiImportOperations.data()->trackOpers;

    const int instrIndex = opers.msInstrIndex.value(trackIndex);
    const auto& trackInstrList = opers.msInstrList.value(trackIndex);
    const InstrumentTemplate* instr = (trackInstrList.empty())
                                      ? nullptr : trackInstrList[instrIndex];
    if (!instr) {
        return "";
    }
    if (!instr->longNames.empty()) {
        return instr->longNames.front().name();
    }
    if (!instr->trackName.isEmpty()) {
        return instr->trackName;
    }

    return "";
}
} // namespace MidiInstr
} // namespace mu::iex::midi
