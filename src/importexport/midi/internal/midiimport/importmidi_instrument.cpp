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

#include <set>

#include "importmidi_chord.h"
#include "importmidi_inner.h"
#include "importmidi_instrument_names.h"
#include "importmidi_operations.h"

#include "engraving/dom/drumset.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/dom/staff.h"

using namespace mu::engraving;

namespace mu::engraving {
extern std::vector<const InstrumentGroup*> instrumentGroups;
}

namespace mu::iex::midi {
namespace MidiInstr {
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

void findNotEmptyDrumPitches(std::set<int>& drumPitches, const InstrumentTemplate* templ)
{
    for (int i = 0; i != DRUM_INSTRUMENTS; ++i) {
        if (!templ->drumset->name(i).empty()) {
            drumPitches.insert(i);
        }
    }
}

bool hasNotDefinedDrumPitch(const std::set<int>& trackPitches, const std::set<int>& drumPitches)
{
    bool hasNotDefinedPitch = false;
    for (const int pitch: trackPitches) {
        if (drumPitches.find(pitch) == drumPitches.end()) {
            hasNotDefinedPitch = true;
            break;
        }
    }

    return hasNotDefinedPitch;
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

// find instrument with maximum MIDI program
// that is less than the track MIDI program, i.e. suitable instrument
const InstrumentTemplate* findClosestInstrument(const MTrack& track)
{
    int maxLessProgram = -1;
    const InstrumentTemplate* closestTemplate = nullptr;

    for (const InstrumentGroup* group: instrumentGroups) {
        for (const InstrumentTemplate* templ: group->instrumentTemplates) {
            if (templ->staffGroup == StaffGroup::TAB) {
                continue;
            }
            const bool isDrumTemplate = templ->useDrumset;
            if (track.mtrack->drumTrack() != isDrumTemplate) {
                continue;
            }
            for (const auto& channel: templ->channel) {
                if (channel.program() < track.program
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

std::vector<const InstrumentTemplate*> findInstrumentsForProgram(const MTrack& track)
{
    std::vector<const InstrumentTemplate*> suitableTemplates;
    const int program = track.program;

    std::set<int> trackPitches;
    if (track.mtrack->drumTrack()) {
        trackPitches = findAllPitches(track);
    }

    for (const InstrumentGroup* group: instrumentGroups) {
        for (const InstrumentTemplate* templ: group->instrumentTemplates) {
            if (templ->staffGroup == StaffGroup::TAB) {
                continue;
            }
            const bool isDrumTemplate = templ->useDrumset;
            if (track.mtrack->drumTrack() != isDrumTemplate) {
                continue;
            }

            std::set<int> drumPitches;
            if (isDrumTemplate && templ->drumset) {
                findNotEmptyDrumPitches(drumPitches, templ);
            }

            for (const auto& channel: templ->channel) {
                if (channel.program() == program) {
                    if (isDrumTemplate && templ->drumset) {
                        if (hasNotDefinedDrumPitch(trackPitches, drumPitches)) {
                            break;
                        }
                    }
                    suitableTemplates.push_back(templ);
                    break;
                }
            }
        }
    }

    if (suitableTemplates.empty()) {
        // Ms instruments with the desired MIDI program were not found
        // so we will find the most matching instrument

        if (program == 55) {             // GM "Orchestra Hit" sound
            auto instr = findInstrument(u"electronic-instruments", u"brass-synthesizer");
            if (instr) {
                suitableTemplates.push_back(instr);
            }
        } else if (program == 110) {          // GM "Fiddle" sound
            auto instr = findInstrument(u"strings", u"violin");
            if (instr) {
                suitableTemplates.push_back(instr);
            }
        } else if (program >= 80 && program <= 103) {
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
        } else if (program == 36 || program == 37) {
            // slightly improve slap bass match:
            // match to the instruments with program 33
            // instead of 35 according to the algorithm below
            auto instr = findInstrument(u"plucked-strings", u"electric-bass");
            if (instr) {
                suitableTemplates.push_back(instr);
            }
            instr = findInstrument(u"plucked-strings", u"5-string-electric-bass");
            if (instr) {
                suitableTemplates.push_back(instr);
            }
        } else {            // find instrument with maximum MIDI program
                            // that is less than the track MIDI program, i.e. suitable instrument
            auto instr = findClosestInstrument(track);
            if (instr) {
                suitableTemplates.push_back(instr);
            }
        }
    }

    return suitableTemplates;
}

std::pair<int, int> findMinMaxPitch(const MTrack& track)
{
    int minPitch = std::numeric_limits<int>::max();
    int maxPitch = -1;

    for (const auto& chord: track.chords) {
        for (const auto& note: chord.second.notes) {
            if (note.pitch < minPitch) {
                minPitch = note.pitch;
            }
            if (note.pitch > maxPitch) {
                maxPitch = note.pitch;
            }
        }
    }
    return std::make_pair(minPitch, maxPitch);
}

int findMaxPitchDiff(const std::pair<int, int>& minMaxPitch, const InstrumentTemplate* templ)
{
    int diff = 0;
    if (minMaxPitch.first < templ->minPitchP) {
        diff = templ->minPitchP - minMaxPitch.first;
    }
    if (minMaxPitch.second > templ->maxPitchP) {
        diff = qMax(diff, minMaxPitch.second - templ->maxPitchP);
    }

    Q_ASSERT(diff >= 0);

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
    if (!instr->trackName.isEmpty()) {
        return instr->trackName;
    }
    if (!instr->longNames.empty()) {
        return instr->longNames.front().name();
    }
    return "";
}
} // namespace MidiInstr
} // namespace mu::iex::midi
