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
#include "importtef.h"
#include "measurehandler.h"
#include "tuplethandler.h"

#include "engraving/dom/box.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/measurebase.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/text.h"
#include "engraving/dom/timesig.h"
#include "log.h"

using namespace mu::engraving;

namespace mu::iex::tabledit {
int8_t TablEdit::readInt8()
{
    int8_t result;
    _file->read((uint8_t*)&result, 1);
    return result;
}

uint8_t TablEdit::readUInt8()
{
    uint8_t result;
    _file->read((uint8_t*)&result, 1);
    return result;
}

uint16_t TablEdit::readUInt16()
{
    uint16_t result;
    _file->read((uint8_t*)&result, 2);
    return result;
}

uint32_t TablEdit::readUInt32()
{
    uint32_t result;
    _file->read((uint8_t*)&result, 4);
    return result;
}

// read sized and null-terminated utf8 text from the current position

std::string TablEdit::readUtf8Text()
{
    std::string result;
    uint16_t size = readUInt16();
    for (uint16_t i = 0; i < size - 1; ++i) {
        result += readUInt8();
    }
    readUInt8(); // skip null
    LOGN("size %d result '%s'", size, result.c_str());
    return result;
}

// read sized and null-terminated utf8 text
// input is the position where the text's position in the file is stored

std::string TablEdit::readUtf8TextIndirect(uint32_t positionOfPosition)
{
    _file->seek(positionOfPosition);
    uint32_t position = readUInt32();
    if (position) {
        _file->seek(position);
        LOGN("position %d", position);
        return readUtf8Text();
    } else {
        return "";
    }
}

// return the part index for the instrument containing stringIdx

engraving::part_idx_t TablEdit::partIdx(size_t stringIdx, bool& ok) const
{
    ok = true;
    engraving::part_idx_t result { 0 };
    engraving::track_idx_t lowerBound { 1 };
    engraving::track_idx_t upperBound { 0 };

    for (const auto& instrument : tefInstruments) {
        upperBound += instrument.stringNumber;
        if (lowerBound <= stringIdx && stringIdx <= upperBound) {
            LOGN("string %zu lower %zu upper %zu found result %zu", stringIdx, lowerBound, upperBound, result);
            return result;
        }
        ++result;
        lowerBound += instrument.stringNumber;
    }
    ok = false;
    result = 0;
    LOGN("string %zu not found result %zu", stringIdx, result);
    return result;
}

// return total number of strings in previous parts

int TablEdit::stringNumberPreviousParts(part_idx_t partIdx) const
{
    int result { 0 };
    for (part_idx_t i = 0; i < partIdx; ++i) {
        result += tefInstruments.at(i).stringNumber;
    }
    LOGN("partIdx %zu result %d", partIdx, result);
    return result;
}

// debug: use color cr to show voice

#if 0
static muse::draw::Color toColor(const voice_idx_t voice)
{
    // debug: color notes based on voice number
    switch (voice) {
    case 0: return muse::draw::Color::BLUE;
    case 1: return muse::draw::Color::GREEN;
    case 2: return muse::draw::Color::RED;
    case 3: return { 150, 150, 0, 255 };
    default: return muse::draw::Color::BLACK;
    }
}

#else
static muse::draw::Color toColor(const voice_idx_t)
{
    // no debug: color notes black
    return muse::draw::Color::BLACK;
}

#endif

// create a VoiceAllocator for every instrument

void TablEdit::initializeVoiceAllocators(std::vector<VoiceAllocator>& allocators)
{
    for (size_t i = 0; i < tefInstruments.size(); ++i) {
        VoiceAllocator allocator;
        allocators.push_back(allocator);
    }
}

void TablEdit::allocateVoices(std::vector<VoiceAllocator>& allocators)
{
    std::vector<const TefNote*> column;
    int currentPosition { -1 };
    engraving::part_idx_t currentPart { 0 };
    for (const TefNote& tefNote : tefContents) {
        // a new column is started when either the postion or the part changes
        bool ok { true };
        const auto part = partIdx(tefNote.string, ok);
        if (!ok) {
            LOGD("error: invalid string %d", tefNote.string);
            continue;
        }
        if (tefNote.position != currentPosition || part != currentPart) {
            // handle the previous column
            allocators[currentPart].addColumn(column);
            // start a new column
            currentPosition = tefNote.position;
            currentPart = part;
            column.clear();
        }
        column.push_back(&tefNote);
    }
    // handle the last column
    allocators[currentPart].addColumn(column);
}

static void connectTie(mu::engraving::Chord* chord, Note* note)
{
    Segment* segment { chord->segment() };
    auto startTrack { VOICES* (chord->track() / VOICES) };
    auto endTrack { startTrack + VOICES - 1 };
    LOGN("segment %p tick %d string %d startTrack %zu track %zu endTrack %zu",
         segment, segment->tick().ticks(), note->string(), startTrack, chord->track(), endTrack);

    for (Segment* seg = segment->prev1(); seg; seg = seg->prev1()) {
        for (auto track = startTrack; track <= endTrack; ++track) {
            EngravingItem* el = seg->element(track);
            if (el && el->isChord()) {
                Chord* firstChord = toChord(el);
                LOGN("firstChord %p tick %d track %zu", firstChord, firstChord->tick().ticks(), track);
                for (Note* firstNote : firstChord->notes()) {
                    LOGN("- string %d", firstNote->string());
                    if (firstNote->string() == note->string()) {
                        Tie* tie = Factory::createTie(firstNote);
                        tie->setEndNote(note);
                        firstNote->add(tie);
                        LOGN(" -> tie %p", tie);
                        return;
                    }
                }
            }
        }
    }
}

static void addNoteToChord(mu::engraving::Chord* chord, track_idx_t track, int pitch, int fret, int string, bool tie,
                           muse::draw::Color color)
{
    LOGN("pitch %d", pitch);
    mu::engraving::Note* note = Factory::createNote(chord);
    if (note) {
        note->setTrack(track);
        note->setPitch(pitch);
        note->setTpcFromPitch(Prefer::NEAREST);
        note->setFret(fret);
        note->setString(string);
        note->setColor(color);
        if (tie) {
            connectTie(chord, note);
        }
        chord->add(note);
    }
}

static void addGraceNotesToChord(mu::engraving::Chord* chord, int pitch, int fret, int string, muse::draw::Color color)
{
    mu::engraving::TDuration durationType(mu::engraving::DurationType::V_INVALID);
    const int ticks { 240 };
    durationType.setVal(ticks);
    mu::engraving::Chord* cr = Factory::createChord(chord->score()->dummy()->segment());
    cr->setTrack(chord->track());
    cr->setNoteType(mu::engraving::NoteType::APPOGGIATURA);
    cr->setDurationType(mu::engraving::DurationType::V_EIGHTH);
    cr->setTicks(durationType.fraction());
    mu::engraving::Note* note = Factory::createNote(cr);
    note->setTrack(chord->track());
    //xmlSetPitch(note, sao.s.toLatin1(), sao.a, sao.o);
    note->setPitch(pitch);
    note->setTpcFromPitch(Prefer::NEAREST);
    note->setFret(fret);
    note->setString(string);
    note->setColor(color);
    cr->add(note);
    chord->add(cr);
}

static void addRest(Segment* segment, track_idx_t track, TDuration tDuration, Fraction length, muse::draw::Color color, bool visible = true)
{
    mu::engraving::Rest* rest = Factory::createRest(segment);
    if (rest) {
        rest->setTrack(track);
        rest->setDurationType(tDuration);
        rest->setTicks(length);
        rest->setColor(color);
        rest->setVisible(visible);
        segment->add(rest);
    }
}

void TablEdit::createContents(const MeasureHandler& measureHandler)
{
    if (tefInstruments.size() == 0) {
        LOGD("error: no instruments");
        return;
    }

    std::vector<VoiceAllocator> voiceAllocators;
    initializeVoiceAllocators(voiceAllocators);
    allocateVoices(voiceAllocators);

    for (size_t part = 0; part < tefInstruments.size(); ++part) {
        LOGN("part %zu", part);
        for (voice_idx_t voice = 0; voice < mu::engraving::VOICES; ++voice) {
            LOGN("- voice %zu", voice);
            auto& voiceContent { voiceAllocators.at(part).voiceContent(voice) };
            TupletHandler tupletHandler;
            for (size_t k = 0; k < voiceContent.size(); ++k) {
                LOGN("  - chord %zu", k);
                // tefNotes is either a rest or a chord of one or more notes
                const std::vector<const TefNote*>& tefNotes { voiceContent.at(k) };

                if (tefNotes.size() == 0) {
                    continue; // shouldn't happen
                }

                const TefNote* const firstNote { tefNotes.at(0) };
                Fraction length { firstNote->length, 64 }; // length is in 64th
                if (firstNote->dots == 1) {
                    length *= Fraction{ 3, 2 };
                } else if (firstNote->dots == 2) {
                    length *= Fraction{ 7, 4 };
                }

                TDuration tDuration(length);
                if (firstNote->dots) {
                    tDuration.setDots(firstNote->dots);
                }

                const auto idx { measureHandler.measureIndex(firstNote->position, tefMeasures) };
                const Fraction gapCorrection { measureHandler.sumPreviousGaps(idx), 64 };
                const auto positionCorrection = tupletHandler.doTuplet(firstNote);

                Fraction tick { firstNote->position, 64 }; // position is in 64th
                tick += positionCorrection;
                tick -= gapCorrection;
                LOGN("    positionCorrection %d/%d tick %d/%d length %d/%d",
                     positionCorrection.numerator(), positionCorrection.denominator(),
                     tick.numerator(), tick.denominator(),
                     length.numerator(), length.denominator()
                     );

                Measure* measure { score->tick2measure(tick) };
                if (!measure) {
                    LOGD("error: no measure");
                    continue;
                } else {
                    LOGN("measure %p", measure);
                }
                Segment* segment { measure->getSegment(mu::engraving::SegmentType::ChordRest, tick) };
                if (!segment) {
                    LOGD("error: no segment");
                    continue;
                }

                const auto track = part * VOICES + voice;
                if (segment->element(track)) {
                    LOGD("segment not empty");
                    continue;
                }

                if (firstNote->rest) {
                    LOGN("    - rest position %d string %d fret %d", firstNote->position, firstNote->string, firstNote->fret);
                    addRest(segment, track, tDuration, length, toColor(voice));
                } else {
                    LOGN("    - note(s) position %d string %d fret %d", firstNote->position, firstNote->string, firstNote->fret);
                    mu::engraving::Chord* chord { Factory::createChord(segment) };
                    if (chord) {
                        chord->setTrack(track);
                        chord->setDurationType(tDuration);
                        chord->setTicks(length);
                        segment->add(chord);

                        const TefInstrument& instrument { tefInstruments.at(part) };
                        if (instrument.stringNumber < 1 || 12 < instrument.stringNumber) {
                            LOGD("error: invalid instrument.stringNumber %d", instrument.stringNumber);
                            continue;
                        }

                        for (const auto note : tefNotes) {
                            const auto stringOffset = stringNumberPreviousParts(part);
                            // todo fix magical constant 96 and code duplication
                            int pitch = 96 - instrument.tuning.at(note->string - stringOffset - 1) + note->fret;
                            LOGN("      -> string %d fret %d pitch %d", note->string, note->fret, pitch);
                            // note TableEdit's strings start at 1, MuseScore's at 0
                            addNoteToChord(chord, track, pitch, note->fret, note->string - 1, note->tie, toColor(voice));
                            if (note->hasGrace) {
                                // todo fix magical constant 96 and code duplication
                                int gracePitch = 96 - instrument.tuning.at(/* todo */ note->string - stringOffset - 1) + note->graceFret;
                                addGraceNotesToChord(chord, gracePitch, note->graceFret, /* todo */ note->string - 1, toColor(voice));
                            }
                        }
                        tupletHandler.addCr(measure, chord);
                    }
                }
            }
        }
    }
}

void TablEdit::createLinkedTabs()
{
    constexpr size_t stavesInPart = 2;

    for (Part* part : score->parts()) {
        part->setStaves(static_cast<int>(stavesInPart));

        Staff* srcStaff = part->staff(0);
        Staff* dstStaff = part->staff(1);
        Excerpt::cloneStaff(srcStaff, dstStaff, false);

        static const std::vector<StaffTypes> types {
            StaffTypes::TAB_4SIMPLE,
            StaffTypes::TAB_5SIMPLE,
            StaffTypes::TAB_6SIMPLE,
            StaffTypes::TAB_7SIMPLE,
            StaffTypes::TAB_8SIMPLE,
            StaffTypes::TAB_9SIMPLE,
            StaffTypes::TAB_10SIMPLE,
        };

        Fraction fr = Fraction(0, 1);
        size_t lines = part->instrument()->stringData()->strings();
        size_t index = (lines >= 4 && lines <= 10) ? lines - 4 : 2;

        dstStaff->setStaffType(fr, *StaffType::preset(types.at(index)));
        dstStaff->setLines(fr, static_cast<int>(lines));
    }
}

static Fraction reducedActualLength(const int actual, const int nominalDenominator)
{
    Fraction res { actual, 64 };
    while (res.denominator() >= 2 * nominalDenominator && res.numerator() % 2 == 0) {
        res.setNumerator(res.numerator() / 2);
        res.setDenominator(res.denominator() / 2);
    }
    LOGN("actual %d nominalDenominator %d res %d/%d", actual, nominalDenominator, res.numerator(), res.denominator());
    return res;
}

void TablEdit::createMeasures(const MeasureHandler& measureHandler)
{
    int lastKey { 0 };               // safe default
    Fraction lastTimeSig { -1, -1 }; // impossible value
    Fraction tick { 0, 1 };
    for (size_t idx = 0; idx < tefMeasures.size(); ++idx) {
        TefMeasure& tefMeasure { tefMeasures.at(idx) };
        // create measure
        auto measure = Factory::createMeasure(score->dummy()->system());
        measure->setTick(tick);
        Fraction nominalLength{ tefMeasure.numerator, tefMeasure.denominator };
        Fraction actualLength{ reducedActualLength(measureHandler.actualSize(tefMeasures, idx), tefMeasure.denominator) };
        measure->setTimesig(nominalLength);
        measure->setTicks(actualLength);
        measure->setEndBarLineType(BarLineType::NORMAL, 0);
        LOGN("measure %p tick %d/%d nominalLength %d/%d actualLength %d/%d",
             measure,
             tick.numerator(), tick.denominator(),
             nominalLength.numerator(), nominalLength.denominator(),
             actualLength.numerator(), actualLength.denominator()
             );
        score->measures()->add(measure);

        if (tick == Fraction { 0, 1 }) {
            auto s1 = measure->getSegment(mu::engraving::SegmentType::KeySig, tick);
            for (size_t i = 0; i < tefInstruments.size(); ++i) {
                mu::engraving::KeySig* keysig = Factory::createKeySig(s1);
                keysig->setKey(Key(tefMeasure.key));
                keysig->setTrack(i * VOICES);
                s1->add(keysig);
            }
            lastKey = tefMeasure.key;

            auto s2 = measure->getSegment(mu::engraving::SegmentType::TimeSig, tick);
            for (size_t i = 0; i < tefInstruments.size(); ++i) {
                mu::engraving::TimeSig* timesig = Factory::createTimeSig(s2);
                timesig->setSig(nominalLength);
                timesig->setTrack(i * VOICES);
                s2->add(timesig);
            }
            lastTimeSig = nominalLength;
            createTempo();
        } else {
            if (tefMeasure.key != lastKey) {
                auto s1 = measure->getSegment(mu::engraving::SegmentType::KeySig, tick);
                for (size_t i = 0; i < tefInstruments.size(); ++i) {
                    mu::engraving::KeySig* keysig = Factory::createKeySig(s1);
                    keysig->setKey(Key(tefMeasure.key));
                    keysig->setTrack(i * VOICES);
                    s1->add(keysig);
                }
                lastKey = tefMeasure.key;
            }
            if (nominalLength != lastTimeSig) {
                auto s2 = measure->getSegment(mu::engraving::SegmentType::TimeSig, tick);
                for (size_t i = 0; i < tefInstruments.size(); ++i) {
                    mu::engraving::TimeSig* timesig = Factory::createTimeSig(s2);
                    timesig->setSig(nominalLength);
                    timesig->setTrack(i * VOICES);
                    s2->add(timesig);
                }
                lastTimeSig = nominalLength;
            }
        }

        tick += actualLength;
    }
    score->setUpTempoMap();
}

void TablEdit::createNotesFrame()
{
    if (!tefHeader.notes.empty()) {
        TBox* tbox = Factory::createTBox(score->dummy()->system());
        tbox->setTick(score->endTick());
        score->measures()->add(tbox);
        tbox->text()->setPlainText(muse::String::fromStdString(tefHeader.notes));
    }
}

void TablEdit::createParts()
{
    for (const auto& instrument : tefInstruments) {
        Part* part = new Part(score);
        score->appendPart(part);
        muse::String staffName { muse::String::fromStdString(instrument.name) };
        part->setPartName(staffName);
        part->setPlainLongName(staffName);

        StringData stringData;
        stringData.setFrets(25); // reasonable default (?)
        for (int i = 0; i < instrument.stringNumber; ++i) {
            int pitch = 96 - instrument.tuning.at(instrument.stringNumber - i - 1);
            LOGN("pitch %d", pitch);
            instrString str { pitch };
            stringData.stringList().push_back(str);
        }
        part->instrument()->setStringData(stringData);

        part->setMidiProgram(instrument.midiVoice);
        part->setMidiChannel(instrument.midiBank);

        Staff* staff = Factory::createStaff(part);
        ClefTypeList clefTypeList { ClefType::G8_VB, ClefType::G8_VB };
        staff->setDefaultClefType(clefTypeList);

        score->appendStaff(staff);
    }
}

void TablEdit::createProperties()
{
    if (!tefHeader.title.empty()) {
        score->setMetaTag(u"workTitle", muse::String::fromStdString(tefHeader.title));
    }
    if (!tefHeader.subTitle.empty()) {
        score->setMetaTag(u"subtitle", muse::String::fromStdString(tefHeader.subTitle));
    }
    if (!tefHeader.comment.empty()) {
        score->setMetaTag(u"comment", muse::String::fromStdString(tefHeader.comment));
    }
    if (!tefHeader.internetLink.empty()) {
        score->setMetaTag(u"source", muse::String::fromStdString(tefHeader.internetLink));
    }
    if (!tefHeader.copyright.empty()) {
        score->setMetaTag(u"copyright", muse::String::fromStdString(tefHeader.copyright));
    }
}

// convert the reading list into repeat and (todo) codas, segnos and voltas

void TablEdit::createRepeats()
{
    LOGN("reading list size %zu number of measures %zu", tefReadingList.size(), tefMeasures.size());
    // proof of concept: add repeat to whole score if
    // - reading list contains two items
    // - both spanning the entire score
    if (tefReadingList.size() == 2
        && tefReadingList.at(0).firstMeasure == 1 && tefReadingList.at(0).lastMeasure == static_cast<int>(tefMeasures.size())
        && tefReadingList.at(1).firstMeasure == 1 && tefReadingList.at(1).lastMeasure == static_cast<int>(tefMeasures.size())
        ) {
        LOGN("do it");
        if (score->measures()->empty()) {
            LOGE("no measures in score");
            return;
        }
        Measure* first { score->firstMeasure() };
        Measure* last { score->lastMeasure() };
        first->setRepeatStart(true);
        last->setRepeatEnd(true);
    } else {
        LOGN("no score repeat");
    }
}

static void setInstrumentIDs(const std::vector<Part*>& parts)
{
    for (Part* part : parts) {
        for (const auto& pair : part->instruments()) {
            pair.second->updateInstrumentId();
        }
    }
}

//---------------------------------------------------------
//   fillGap
//---------------------------------------------------------

// Fill one gap (tstart - tend) in this track in this measure with rest(s).

static void fillGap(Measure* measure, track_idx_t track, const Fraction& tstart, const Fraction& tend)
{
    Fraction ctick = tstart;
    Fraction restLen = tend - tstart;
    LOGN("measure %p track %zu tstart %d tend %d restLen %d len",
         measure, track, tstart.ticks(), tend.ticks(), restLen.ticks());
    auto durList = toDurationList(restLen, true);
    LOGN("durList.size %zu", durList.size());
    for (const auto& dur : durList) {
        LOGN("type %d dots %d fraction %d/%d", static_cast<int>(dur.type()), dur.dots(), dur.fraction().numerator(),
             dur.fraction().denominator());
        Segment* s = measure->getSegment(SegmentType::ChordRest, ctick);
        addRest(s, track, dur, dur.fraction(), muse::draw::Color::BLACK, false);
        ctick += dur.fraction();
    }
}

//---------------------------------------------------------
//   fillGapsInFirstVoices
//---------------------------------------------------------

// Fill gaps in first voice of every staff in this measure for this part with rest(s).

static void fillGapsInFirstVoices(MasterScore* score)
{
    IF_ASSERT_FAILED(score) {
        return;
    }

    for (staff_idx_t idx = 0; idx < score->nstaves(); ++idx) {
        for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
            Fraction measTick     = measure->tick();
            Fraction measLen      = measure->ticks();
            Fraction nextMeasTick = measTick + measLen;
            LOGN("measure %p idx %zu tick %d - %d (len %d)",
                 measure, idx, measTick.ticks(), nextMeasTick.ticks(), measLen.ticks());
            track_idx_t track = idx * VOICES;
            Fraction endOfLastCR = measTick;
            for (Segment* s = measure->first(); s; s = s->next()) {
                EngravingItem* el = s->element(track);
                if (el) {
                    if (s->isChordRestType()) {
                        ChordRest* cr  = static_cast<ChordRest*>(el);
                        Fraction crTick     = cr->tick();
                        Fraction crLen      = cr->globalTicks();
                        if (crTick > endOfLastCR) {
                            fillGap(measure, track, endOfLastCR, crTick);
                        }
                        endOfLastCR = crTick + crLen;
                    }
                }
            }
            if (nextMeasTick > endOfLastCR) {
                fillGap(measure, track, endOfLastCR, nextMeasTick);
            }
        }
    }
}

void TablEdit::createScore()
{
    MeasureHandler measureHandler;
    measureHandler.calculate(tefContents, tefMeasures);
    createProperties();
    createParts();
    createTitleFrame();
    createMeasures(measureHandler);
    createNotesFrame();
    createContents(measureHandler);
    fillGapsInFirstVoices(score);
    createRepeats();
    createTexts();
    createLinkedTabs();
    setInstrumentIDs(score->parts());
}

void TablEdit::createTempo()
{
    mu::engraving::Measure* measure = score->firstMeasure();
    mu::engraving::Segment* segment = measure->getSegment(mu::engraving::SegmentType::ChordRest, mu::engraving::Fraction(0, 1));
    mu::engraving::TempoText* tt = new mu::engraving::TempoText(segment);
    tt->setTempo(double(tefHeader.tempo) / 60.0);
    tt->setTrack(0);
    tt->setFollowText(true);
    muse::String tempoText = mu::engraving::TempoText::duration2tempoTextString(mu::engraving::DurationType::V_QUARTER);
    tempoText += u" = ";
    tempoText += muse::String::number(tefHeader.tempo);
    tt->setXmlText(tempoText);
    segment->add(tt);
}

void TablEdit::createTexts()
{
    for (const auto& textMarker : tefTextMarkers) {
        LOGN("position %d string %d text marker %d", textMarker.position, textMarker.string, textMarker.index);
        bool ok { true };
        const auto part = partIdx(textMarker.string, ok);
        if (!ok) {
            LOGD("error: invalid string %d", textMarker.string);
            continue;
        }
        const auto track = part * VOICES;
        LOGN("part %zu track %zu", part, track);

        Fraction tick { textMarker.position, 64 }; // position is in 64th
        Measure* measure { score->tick2measure(tick) };
        if (!measure) {
            LOGD("error: no measure");
            continue;
        } else {
            LOGN("measure %p", measure);
        }
        Segment* segment { measure->getSegment(mu::engraving::SegmentType::ChordRest, tick) };
        if (!segment) {
            LOGD("error: no segment");
            continue;
        }

        StaffText* staffText = Factory::createStaffText(segment);
        muse::String text { tefTexts.at(textMarker.index).c_str() };
        staffText->setPlainText(text);
        staffText->setTrack(track);
        segment->add(staffText);
    }
}

void TablEdit::createTitleFrame()
{
    VBox* vbox = Factory::createTitleVBox(score->dummy()->system());
    vbox->setTick(mu::engraving::Fraction(0, 1));
    score->measures()->add(vbox);
    if (!tefHeader.title.empty()) {
        Text* s = Factory::createText(vbox, TextStyleType::TITLE);
        s->setPlainText(muse::String::fromStdString(tefHeader.title));
        vbox->add(s);
    }
    if (!tefHeader.subTitle.empty()) {
        Text* s = Factory::createText(vbox, TextStyleType::SUBTITLE);
        s->setPlainText(muse::String::fromStdString(tefHeader.subTitle));
        vbox->add(s);
    }
}

/*
 * encoding of note durations
 * 0 whole
 * 1 half dotted
 * 2 whole triplet
 * 3 half
 * 4 quarter dotted
 * 5 half triplet
 * 6 quarter
 * 7 eight dotted
 * 8 quarter triplet
 * ...
 * 18 1/64th note
 * 19 half double dotted
 * 22 quarter double dotted
 * 25 eighth double dotted
 * 28 16th double dotted
 */

// return TablEdit note length in 64th
// (including triplets rounded down to nearest note length)
// TODO: remove code duplication with voiceallocator.cpp durationToInt()

static int duration2length(const int duration)
{
    if (0 <= duration && duration <= 18) {
        // remove dot and triplet
        int noteType { 0 };
        int dotOrTriplet { duration % 3 };
        switch (dotOrTriplet) {
        case 0: noteType = duration / 3;
            break;
        case 1: noteType = (duration + 2) / 3;
            break;
        case 2: noteType = (duration - 2) / 3;
            break;
        default: LOGD("impossible value %d", dotOrTriplet);
        }
        switch (noteType) {
        case 0: return 64; // 1/1
        case 1: return 32; // 1/2
        case 2: return 16; // 1/4
        case 3: return 8;  // 1/8
        case 4: return 4;  // 1/16
        case 5: return 2;  // 1/32
        case 6: return 1;  // 1/64
        // as long as the if statement above is correct, this cannot actually happen
        default: LOGD("impossible value %d", dotOrTriplet);
        }
    } else {
        switch (duration) {
        case 19: return 32; // 1/2
        case 22: return 16; // 1/4
        case 25: return 8;  // 1/8
        case 28: return 4;  // 1/16
        default: LOGD("impossible value %d", duration);
        }
    }
    LOGD("invalid note duration %d", duration);
    return 0; // invalid (result is weird layout)
}

// return the number of dots
// TODO: remove code duplication with voiceallocator.cpp durationToInt()

static int duration2dots(const int duration)
{
    if (0 <= duration && duration <= 18 && (duration % 3) == 0) {
        return 0;
    } else if (0 <= duration && duration <= 18 && (duration % 3) == 1) {
        return 1;
    } else if (duration == 19 || duration == 22 || duration == 25 || duration == 28) {
        return 2;
    }
    LOGD("invalid note duration %d", duration);
    return 0; // invalid
}

// check if 3:2 tuplet

static bool duration2triplet(const int duration)
{
    if (0 <= duration && duration <= 18) {
        return duration % 3 == 2;
    } else {
        LOGD("invalid note duration %d", duration);
        return false; // invalid
    }
}

Voice extractVoice(const uint8_t byte3)
{
    switch ((byte3 & 0x30) / 0x10) {
    case 0: return Voice::DEFAULT;
    case 2: return Voice::UPPER;
    case 3: return Voice::LOWER;
    default: LOGE("unknown voice");
        return Voice::DEFAULT;
    }
}

void TablEdit::readTefContents()
{
    // calculate the total number of strings
    // instruments must have been read before reading contents
    if (tefInstruments.empty()) {
        LOGD("no instruments");
        return;
    }
    int totalNumberOfStrings { 0 };
    for (const auto& instrument : tefInstruments) {
        totalNumberOfStrings += instrument.stringNumber;
    }
    LOGN("totalNumberOfStrings %d", totalNumberOfStrings);

    _file->seek(OFFSET_CONTENTS);
    uint32_t position = readUInt32();
    _file->seek(position);
    uint32_t offset = readUInt32();
    LOGN("position %d offset %d", position, offset);
    while (offset != 0xFFFFFFFF) {
        uint8_t byte1 = readUInt8();
        uint8_t byte2 = readUInt8();
        uint8_t byte3 = readUInt8();
        uint8_t byte4 = readUInt8();
        /* uint8_t byte5 = */ readUInt8();
        /* uint8_t byte6 = */ readUInt8();
        /* uint8_t byte7 = */ readUInt8();
        /* uint8_t byte8 = */ readUInt8();
        TefNote note;
        note.position = (offset >> 3) / totalNumberOfStrings;
        const auto noteRestMarker = byte1 & 0x3F;
        if (noteRestMarker < 0x33) {
            note.string = ((offset >> 3) % totalNumberOfStrings) + 1;
            note.fret = noteRestMarker - 1;
        } else if (noteRestMarker == 0x33) {
            note.string = ((offset >> 3) % totalNumberOfStrings) + 1;
            note.rest = true;
        } else {
            // not a note or rest
            //LOGD("marker %d duration %d length %d dots %d", noteRestMarker, note.duration, note.length, note.dots);
        }
        if (noteRestMarker <= 0x33) {
            note.duration = byte2 & 0x1F;
            note.length = duration2length(note.duration);
            note.dots = duration2dots(note.duration);
            note.triplet = duration2triplet(note.duration);
            note.voice = extractVoice(byte3);
            note.tie = (byte2 / 0x20) == 7;
            if (byte1 & 0x40) {
                note.graceEffect = byte4 / 0x20;
                note.graceFret = byte4 & 0x1F;
                note.hasGrace = true;
                //LOGD("graceEffect %d graceFret %d", note.graceEffect, note.graceFret);
            }
            tefContents.push_back(note);
        } else if (noteRestMarker == 0x39) {
            TefTextMarker tefTextMarker;
            tefTextMarker.position = (offset >> 3) / totalNumberOfStrings;
            tefTextMarker.string = ((offset >> 3) % totalNumberOfStrings) + 1;
            tefTextMarker.index = static_cast<int>((byte3 << 8) + byte2);
            LOGN("text marker %d", tefTextMarker.index);
            tefTextMarkers.push_back(tefTextMarker);
        }
        offset = readUInt32();
    }
}

// tuning to MIDI: 96 - tuning[string] with string 0 is highest
// MIDI E2 = 40 E4 = 64
// todo: check interaction with fMiddleC

void TablEdit::readTefInstruments()
{
    _file->seek(OFFSET_INSTRUMENTS);
    uint32_t position = readUInt32();
    _file->seek(position);
    uint16_t structSize = readUInt16();
    uint16_t numberOfInstruments = readUInt16();
    //uint32_t zero = readUInt16();
    LOGN("structSize %d numberOfInstruments %d", structSize, numberOfInstruments);
    for (uint16_t i = 0; i < numberOfInstruments; ++i) {
        TefInstrument instrument;
        instrument.stringNumber = readUInt16();
        instrument.firstString = readUInt16();
        instrument.available16U = readUInt16();
        instrument.verticalSpacing = readUInt16();
        instrument.midiVoice = readUInt8();
        instrument.midiBank = readUInt8();
        instrument.nBanjo5 = readUInt8();
        instrument.uSpec = readUInt8();
        instrument.nCapo = readUInt16();
        instrument.fMiddleC = readUInt8();
        instrument.fClef = readUInt8();
        instrument.output = readUInt16();
        instrument.options = readUInt16();
        for (uint16_t j = 0; j < 12; ++j) {
            auto n = readUInt8();
            instrument.tuning[j] = n;
        }
        // name is a zero-terminated utf8 string
        bool atEnd = false;
        for (uint16_t j = 0; j < 36; ++j) {
            auto c = readUInt8();
            if (c == 0) {
                // stop appending, but continue reading
                atEnd = true;
            }
            if (0x20 <= c && c <= 0x7E && !atEnd) {
                instrument.name += static_cast<char>(c);
            }
        }
        tefInstruments.push_back(instrument);
    }
}

void TablEdit::readTefMeasures()
{
    _file->seek(OFFSET_MEASURES);
    uint32_t position = readUInt32();
    _file->seek(position);
    /* uint16_t structSize = */ readUInt16();
    uint16_t numberOfMeasures = readUInt16();
    /* uint32_t zero = */ readUInt32();
    for (uint16_t i = 0; i < numberOfMeasures; ++i) {
        TefMeasure measure;
        measure.flag = readUInt8();
        measure.isPickup = measure.flag & 0x08;
        /* uint8_t uTmp = */ readUInt8();
        measure.key = readInt8();
        measure.size = readUInt8();
        measure.denominator = readUInt8();
        measure.numerator = readUInt8();
        /* uint16_t margins = */ readUInt16();
        tefMeasures.push_back(measure);
    }
}

void TablEdit::readTefReadingList()
{
    _file->seek(OFFSET_READINGLIST);
    uint32_t position = readUInt32();
    if (!position) {
        return;
    }

    _file->seek(position);
    uint16_t structSize = readUInt16();
    uint16_t numberOfItems = readUInt16();
    for (uint16_t i = 0; i < numberOfItems; ++i) {
        TefReadingListItem item;
        item.firstMeasure = readUInt16();
        item.lastMeasure = readUInt16();
        // skip the label
        for (uint16_t j = 4; j < structSize; ++j) {
            readUInt8();
        }
        tefReadingList.push_back(item);
    }
}

void TablEdit::readTefTexts()
{
    _file->seek(OFFSET_TEXTS);
    uint32_t position = readUInt32();
    if (!position) {
        return;
    }

    _file->seek(position);
    uint16_t numberOfTexts = readUInt16();
    for (uint16_t i = 0; i < numberOfTexts; ++i) {
        std::string text { readUtf8Text() };
        LOGN("i %d text '%s'", i, text.c_str());
        tefTexts.push_back(text);
    }
}

void TablEdit::readTefHeader()
{
    readUInt16(); // skip private01
    tefHeader.version = readUInt16();
    tefHeader.subVersion = readUInt16();
    tefHeader.tempo = readUInt16();
    tefHeader.chorus = readUInt16();
    tefHeader.reverb = readUInt16();
    readUInt16(); // todo syncope (is signed)
    readUInt16(); // skip private02
    tefHeader.securityCode = readUInt32();
    tefHeader.securityFlags = readUInt32();
    _file->seek(OFFSET_TBED);
    tefHeader.tbed = readUInt32();
    readUInt32(); // skip contents
    auto titlePtr = readUInt32();
    LOGN("titlePtr %d", titlePtr);
    _file->seek(titlePtr);
    tefHeader.title = readUtf8TextIndirect(OFFSET_TITLE);
    tefHeader.subTitle = readUtf8TextIndirect(OFFSET_SUBTITLE);
    tefHeader.comment = readUtf8TextIndirect(OFFSET_COMMENT);
    tefHeader.notes = readUtf8TextIndirect(OFFSET_NOTES);
    tefHeader.internetLink = readUtf8TextIndirect(OFFSET_INTERNETLINK);
    tefHeader.copyright = readUtf8TextIndirect(OFFSET_COPYRIGHT);
    _file->seek(OFFSET_OLDNUM);
    tefHeader.wOldNum = readUInt16();
    tefHeader.wFormat = readUInt16();
}

//---------------------------------------------------------
//   import
//
//   see src/importexport/guitarpro/internal/importgtp.cpp createLinkedTabs()
//   for linked staves
//---------------------------------------------------------

Err TablEdit::import()
{
    readTefHeader();
    LOGN("version %d subversion %d", tefHeader.version, tefHeader.subVersion);
    LOGN("tempo %d chorus %d reverb %d", tefHeader.tempo, tefHeader.chorus, tefHeader.reverb);
    LOGN("securityCode %d securityFlags %d", tefHeader.securityCode, tefHeader.securityFlags);
    LOGN("title '%s'", tefHeader.title.c_str());
    LOGN("subTitle '%s'", tefHeader.subTitle.c_str());
    LOGN("comment '%s'", tefHeader.comment.c_str());
    LOGN("notes '%s'", tefHeader.notes.c_str());
    LOGN("internetLink '%s'", tefHeader.internetLink.c_str());
    LOGN("copyright '%s'", tefHeader.copyright.c_str());
    LOGN("tbed %d wOldNum %d wFormat %d", tefHeader.tbed, tefHeader.wOldNum, tefHeader.wFormat);
    if ((tefHeader.wFormat >> 8) < 10) {
        return Err::FileBadFormat;
        //return Err::FileTooOld; // TODO: message is too specific for MuseScore format
    }
    if ((tefHeader.wFormat >> 8) > 10) {
        return Err::FileBadFormat;
        //return Err::FileTooNew;
    }
    if (tefHeader.securityCode != 0) {
        return Err::FileBadFormat; // todo "file is protected" message ?
    }
    readTefTexts();
    for (const auto& text : tefTexts) {
        LOGN("text: '%s'", text.c_str());
    }
    readTefMeasures();
    for (const auto& measure : tefMeasures) {
        LOGN("flag %d key %d size %d numerator %d denominator %d",
             measure.flag, measure.key, measure.size, measure.numerator, measure.denominator);
    }
    readTefInstruments();
    for (const auto& instrument : tefInstruments) {
        LOGN("stringNumber %d firstString %d midiVoice %d midiBank %d",
             instrument.stringNumber, instrument.firstString, instrument.midiVoice, instrument.midiBank);
    }
    readTefReadingList();
    for (const auto& item : tefReadingList) {
        LOGN("firstMeasure %d lastMeasure %d", item.firstMeasure, item.lastMeasure);
    }
    readTefContents();
    for (const auto& note : tefContents) {
        LOGN("position %d rest %d string %d fret %d duration %d length %d dots %d tie %d triplet %d voice %d",
             note.position, note.rest, note.string, note.fret,
             note.duration, note.length, note.dots,
             note.tie, note.triplet, static_cast<int>(note.voice));
    }
    for (const auto& textMarker : tefTextMarkers) {
        LOGN("position %d string %d text marker %d", textMarker.position, textMarker.string, textMarker.index);
    }
    createScore();
    return Err::NoError;
}
} // namespace mu::iex::tabledit
