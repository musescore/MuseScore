/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <gtest/gtest.h>

#include "engraving/dom/beam.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/stringdata.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tuplet.h"
#include "engraving/editing/editbeam.h"
#include "engraving/editing/editcrossstaff.h"
#include "engraving/editing/edittuplet.h"
#include "engraving/editing/noteinput.h"
#include "engraving/editing/splitjoinmeasure.h"
#include "engraving/editing/transaction/transaction.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu::engraving;

static const String TUPLET_DATA_DIR(u"tuplet_data/");

class Engraving_TupletTests : public ::testing::Test
{
public:
    struct LongNoteEntry {
        int bar;
        int chordRest;
        Fraction duration;
        std::string rhythm;
        bool rest = false;
    };

    bool createTuplet(int n, ChordRest* cr);
    void tuplet(const char16_t* p1, const char16_t* p2);
    void split(const char16_t* p1, const char16_t* p2);
    void enterLongNotes(const String& file, const std::vector<LongNoteEntry>& entries);
};

bool Engraving_TupletTests::createTuplet(int n, ChordRest* cr)
{
    if (cr->durationType() < TDuration(DurationType::V_128TH)) {
        return false;
    }

    Fraction f(cr->ticks());
    Fraction tick = cr->tick();
    Tuplet* ot    = cr->tuplet();

    f.reduce();         //measure duration might not be reduced
    Fraction ratio(n, f.numerator());
    Fraction fr(1, f.denominator());
    while (ratio.numerator() >= ratio.denominator() * 2) {
        ratio *= Fraction(1, 2);
        fr    *= Fraction(1, 2);
    }

    Tuplet* tuplet = Factory::createTuplet(cr->score()->dummy());
    tuplet->setRatio(ratio);

    //
    // "fr" is the fraction value of one tuple element
    //
    // "tuplet time" is "normal time" / tuplet->ratio()
    //    Example: an 1/8 has 240 midi ticks, in an 1/8 triplet the note
    //             has a tick duration of 240 / (3/2) = 160 ticks
    //             (assume tpq = 480)
    //

    tuplet->setTicks(f);
    TDuration baseLen(fr);
    tuplet->setBaseLen(baseLen);

    tuplet->setTrack(cr->track());
    tuplet->setTick(tick);
    Measure* measure = cr->measure();
    tuplet->setOwnershipParent(measure);

    if (ot) {
        tuplet->setTuplet(ot);
    }
    cr->score()->startCmd(TranslatableString::untranslatable("Engraving tuplet tests"));
    cr->score()->cmdCreateTuplet(cr, tuplet);
    cr->score()->endCmd();
    return true;
}

void Engraving_TupletTests::tuplet(const char16_t* p1, const char16_t* p2)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + p1);
    Measure* m1 = score->firstMeasure();
    ASSERT_TRUE(m1);
    Measure* m2 = m1->nextMeasure();
    ASSERT_TRUE(m2);

    EXPECT_TRUE(m1 != m2);

    Segment* s = m2->first(SegmentType::ChordRest);
    ASSERT_TRUE(s);
    Chord* c = toChord(s->element(0));
    ASSERT_TRUE(c);

    EXPECT_TRUE(createTuplet(3, c));

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, p1, TUPLET_DATA_DIR + p2));
    delete score;
}

TEST_F(Engraving_TupletTests, join1)
{
    tuplet(u"tuplet1.mscx", u"tuplet1-ref.mscx");
}

void Engraving_TupletTests::split(const char16_t* p1, const char16_t* p2)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + p1);
    Measure* m         = score->firstMeasure();
    TimeSig* ts        = Factory::createTimeSig(score->dummy());
    ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving tuplet tests"), [&](Transaction& tx) {
        EditData dd(0);
        dd.dropElement = ts;
        dd.modifiers = {};
        dd.dragOffset = QPointF();
        dd.track = 0;
        m->drop(tx, dd);
    });

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, p1, TUPLET_DATA_DIR + p2));
    delete score;
}

TEST_F(Engraving_TupletTests, split1)
{
    split(u"split1.mscx",   u"split1-ref.mscx");
}

TEST_F(Engraving_TupletTests, split2)
{
    split(u"split2.mscx",   u"split2-ref.mscx");
}

TEST_F(Engraving_TupletTests, split3)
{
    split(u"split3.mscx",   u"split3-ref.mscx");
}

TEST_F(Engraving_TupletTests, split4)
{
    split(u"split4.mscx",   u"split4-ref.mscx");
}

//---------------------------------------------------------
//   addStaff
//    Checks that after adding a staff the resulting
//    score is equal to the reference score
//---------------------------------------------------------

TEST_F(Engraving_TupletTests, addStaff)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + "nestedTuplets_addStaff.mscx");
    ASSERT_TRUE(score);

    // add a staff to the existing staff
    // (copied and adapted from void MuseScore::editInstrList() in mscore/instrdialog.cpp)
    Staff* oldStaff   = score->staff(0);
    Staff* newStaff   = Factory::createStaff(oldStaff->part());
    newStaff->setPart(oldStaff->part());
    newStaff->initFromStaffType(oldStaff->staffType(Fraction(0, 1)));
    newStaff->setDefaultClefType(ClefTypeList(ClefType::F));
    KeySigEvent ke = oldStaff->keySigEvent(Fraction(0, 1));
    newStaff->setKey(Fraction(0, 1), ke);
    score->undoInsertStaff(newStaff, 0, true);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"nestedTuplets_addStaff.mscx", TUPLET_DATA_DIR + u"nestedTuplets_addStaff-ref.mscx"));
    delete score;
}

//-----------------------------------------
//    saveLoad
//     checks that properties persist after loading and saving
//-----------------------------------------
TEST_F(Engraving_TupletTests, saveLoad)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + "save-load.mscx");
    ASSERT_TRUE(score);
    //simply load and save
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"save-load.mscx", TUPLET_DATA_DIR + u"save-load.mscx"));
    delete score;
}

// the rhythm of track 0 in a measure, e.g. "3:5[8~ 8 r8] 8. 4": durations as note values
// (r = rest, . = dot, ~ = tie, R = measure rest), tuplets as actual:normal[...]
static std::string rhythm(Measure* measure)
{
    std::string result;
    std::vector<const Tuplet*> open;
    for (Segment* s = measure->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        const ChordRest* cr = toChordRest(s->element(0));
        if (!cr) {
            continue;
        }

        std::vector<const Tuplet*> tuplets;
        for (const Tuplet* t = cr->tuplet(); t; t = t->tuplet()) {
            tuplets.insert(tuplets.begin(), t);
        }
        size_t common = 0;
        while (common < open.size() && common < tuplets.size() && open[common] == tuplets[common]) {
            ++common;
        }
        for (; open.size() > common; open.pop_back()) {
            result += "]";
        }
        for (size_t i = common; i < tuplets.size(); ++i) {
            result += " " + std::to_string(tuplets[i]->ratio().numerator()) + ":"
                      + std::to_string(tuplets[i]->ratio().denominator()) + "[";
            open.push_back(tuplets[i]);
        }

        result += (result.empty() || result.back() == '[') ? "" : " ";
        if (cr->durationType().type() == DurationType::V_MEASURE) {
            result += "R";
            continue;
        }
        result += cr->isRest() ? "r" : "";
        result += std::to_string(1 << (static_cast<int>(cr->durationType().type()) - static_cast<int>(DurationType::V_WHOLE)));
        result += std::string(cr->durationType().dots(), '.');
        result += cr->isChord() && toChord(cr)->upNote()->tieFor() ? "~" : "";
    }
    result += std::string(open.size(), ']');
    return result.substr(result.find_first_not_of(' '));
}

//-----------------------------------------
//    longNoteIntoSpaceLeft
//     a note or rest longer than the space left in a tuplet writes
//     all of that space as tuplet notes, even when it needs two values,
//     and continues with ordinary notes or in the next or outer tuplet
//-----------------------------------------

void Engraving_TupletTests::enterLongNotes(const String& file, const std::vector<LongNoteEntry>& entries)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + file + u".mscx");
    ASSERT_TRUE(score);

    for (const LongNoteEntry& e : entries) {
        Measure* m = score->firstMeasure();
        for (int i = 0; i < e.bar; ++i) {
            m = m->nextMeasure();
        }
        Segment* s = m->first(SegmentType::ChordRest);
        for (int i = 0; i < e.chordRest; ++i) {
            s = s->next(SegmentType::ChordRest);
        }
        ASSERT_TRUE(toChordRest(s->element(0))->tuplet());

        score->startCmd(TranslatableString::untranslatable("Engraving tuplet tests"));
        score->setNoteRest(s, 0, e.rest ? NoteVal() : NoteVal(72), e.duration, DirectionV::AUTO);
        score->endCmd();
        EXPECT_EQ(rhythm(m), e.rhythm) << "bar " << e.bar + 1;
    }

    EXPECT_TRUE(score->sanityCheck());
    delete score;
}

TEST_F(Engraving_TupletTests, longNoteIntoSpaceLeft)
{
    enterLongNotes(u"spaceLeft_longNote", {
        // 16th quintuplet: space left 5/16, two values
        { 0, 0, Fraction(1, 2), "5:4[4~ 16~] 8. r16 r2" },
        // eighth quintuplet: space left 5/8, two values
        { 1, 0, Fraction(1, 1), "5:4[2~ 8~] 4. r8" },
        // 16th quintuplet, a rest: space left 5/16, two values
        { 2, 0, Fraction(1, 2), "5:4[r4 r16] r8. r16 r2", true },
        // eighth quintuplet, a rest: space left 5/8, two values
        { 3, 0, Fraction(1, 1), "5:4[r2 r8] r4. r8", true },
        // 16th quintuplet from its 2nd note: space left 4/16
        { 4, 1, Fraction(1, 2), "5:4[r16 4~] 4 r2" },
        // 16th septuplet: space left 7/16
        { 5, 0, Fraction(1, 2), "7:4[4..~] 16 r16 r8 r2" },
        // spills from one eighth triplet into the next
        { 6, 0, Fraction(1, 1), "3:2[4.~] 3:2[4.~] 4 r4" },
        // 16th sextuplet from its 2nd note: space left 5/16
        { 7, 1, Fraction(1, 2), "6:4[r16 4~ 16~] 8. r16 r2" },
        // 9:8 16ths: space left 9/16
        { 8, 0, Fraction(1, 1), "9:8[2~ 16~] 4.. r16" },
        // two 16th quintuplets: two values in each
        { 9, 0, Fraction(1, 1), "5:4[4~ 16~] 5:4[4~ 16~] 4. r8" },
        // 5:3 eighths in 6/8: space left 5/8
        { 10, 0, Fraction(3, 4), "5:3[2~ 8~] 8 r8 r8" },
    });
}

//-----------------------------------------
//    longNoteIntoNestedSpaceLeft
//     the same, from the innermost of three nested tuplets,
//     continuing in the tuplets it is nested in,
//     and replacing a nested tuplet in its way as a whole
//-----------------------------------------

TEST_F(Engraving_TupletTests, longNoteIntoNestedSpaceLeft)
{
    enterLongNotes(u"spaceLeft_nested", {
        // half triplet > eighth quintuplet > 16th septuplet, from its 3rd note
        { 0, 2, Fraction(1, 1), "3:2[ 5:4[ 7:4[r16 r16 4~ 16~] 4.~] 4~ 16 r16 r8 r2]" },
        // the same nested tuplets, a rest
        { 1, 2, Fraction(1, 1), "3:2[ 5:4[ 7:4[r16 r16 r4 r16] r4.] r4 r16 r16 r8 r2]", true },
        // quarter quintuplet > eighth triplet > 32nd sextuplet, from its 2nd note
        { 2, 1, Fraction(1, 1), "5:4[ 3:2[ 6:4[r32 8~ 32~] 4~] 2~ 16. r32 r8 r4]" },
        // quarter quintuplet > two eighth triplets, from the 1st triplet's 3rd note into the 2nd triplet
        { 3, 2, Fraction(1, 2), "5:4[ 3:2[r8 r8 8~] 4. r8 r4 r4]" },
    });
}

//-----------------------------------------
//    nestedTupletsLengthen
//     lengthening a chord inside a tuplet over a nested tuplet
//     replaces the nested tuplet instead of overfilling the measure
//-----------------------------------------
TEST_F(Engraving_TupletTests, nestedTupletsLengthen)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"nestedTuplets_lengthen.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure();

    // 9:8 eighth tuplet: three eighths, a nested eighth triplet, four eighths
    ASSERT_EQ(rhythm(m1), "9:8[8 8 8 3:2[8 8 8] 8 8 8 8]");
    Segment* s = m1->first(SegmentType::ChordRest);
    s = s->next(SegmentType::ChordRest)->next(SegmentType::ChordRest);
    ChordRest* third = toChordRest(s->element(0));
    ASSERT_TRUE(third && third->tuplet());

    // a quarter reaches into the nested triplet
    score->startCmd(TranslatableString::untranslatable("Engraving tuplet tests"));
    score->changeCRlen(third, TDuration(DurationType::V_QUARTER));
    score->endCmd();

    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "9:8[8 8 4 r8 8 8 8 8]");

    score->undoRedo(true, nullptr);
    EXPECT_EQ(rhythm(m1), "9:8[8 8 8 3:2[8 8 8] 8 8 8 8]");
    delete score;
}

TEST_F(Engraving_TupletTests, nestedTupletStartDoesNotSplitTuplet)
{
    // quarter, a quarter triplet starting with an eighth triplet, quarter
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    is.setNoteEntryMode(true);
    is.setNoteEntryMethod(NoteEntryMethod::BY_DURATION);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        for (DurationType duration : { DurationType::V_QUARTER, DurationType::V_HALF, DurationType::V_QUARTER }) {
            is.setDuration(duration);
            NoteVal nval(60);
            NoteInput::addPitch(tx, score, nval, false);
        }
    });
    Measure* m1 = score->firstMeasure();
    auto crAt = [&](const Fraction& tick) {
        Segment* s = m1->findSegment(SegmentType::ChordRest, tick);
        return s ? toChordRest(s->element(0)) : nullptr;
    };
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        score->addTuplet(crAt(Fraction(1, 4)), Fraction(3, 2), TupletNumberType::SHOW_NUMBER, TupletBracketType::AUTO_BRACKET);
    });
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        score->addTuplet(crAt(Fraction(1, 4)), Fraction(3, 2), TupletNumberType::SHOW_NUMBER, TupletBracketType::AUTO_BRACKET);
    });
    ASSERT_EQ(rhythm(m1), "4 3:2[ 3:2[8 r8 r8] r4 r4] 4");

    // only the start of both tuplets doesn't split one
    Segment* start = m1->findSegment(SegmentType::ChordRest, Fraction(1, 4));
    EXPECT_FALSE(start->splitsTuplet());
    EXPECT_TRUE(m1->findSegment(SegmentType::ChordRest, Fraction(1, 4) + Fraction(1, 18))->splitsTuplet());
    EXPECT_TRUE(m1->findSegment(SegmentType::ChordRest, Fraction(1, 4) + Fraction(1, 6))->splitsTuplet());

    // Insert adds a quarter before the tuplets
    is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    is.setSegment(start);
    score->select(toChord(crAt(Fraction(1, 4)))->upNote());
    is.setTrack(0);
    is.setDuration(DurationType::V_QUARTER);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        NoteInput::addPitch(tx, score, NoteInputParams { 0 }, false, false);
    });
    EXPECT_EQ(rhythm(m1), "4 4 3:2[ 3:2[8 r8 r8] r4 r4] 4");
    score->undoRedo(true, nullptr);
    ASSERT_EQ(rhythm(m1), "4 3:2[ 3:2[8 r8 r8] r4 r4] 4");

    // the measure splits before the tuplets
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        SplitJoinMeasure::splitMeasure(tx, score, Fraction(1, 4));
    });
    m1 = score->firstMeasure();
    EXPECT_EQ(rhythm(m1), "4");
    EXPECT_EQ(rhythm(m1->nextMeasure()), "3:2[ 3:2[8 r8 r8] r4 r4] 4");
    EXPECT_TRUE(score->sanityCheck());

    delete score;
}

TEST_F(Engraving_TupletTests, insertTimeMovesLaterTuplets)
{
    // voice 1: four quarters; voice 2: an eighth triplet on beat 2
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    is.setDuration(DurationType::V_QUARTER);
    is.setNoteEntryMode(true);
    is.setNoteEntryMethod(NoteEntryMethod::BY_DURATION);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        for (int pitch : { 60, 62, 64, 65 }) {
            NoteVal nval(pitch);
            NoteInput::addPitch(tx, score, nval, false);
        }
    });
    is.setTrack(1);
    is.setSegment(score->tick2segment(Fraction(1, 4), false, SegmentType::ChordRest));
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        NoteVal nval(55);
        Note* note = NoteInput::addPitch(tx, score, nval, false);
        score->addTuplet(note->chord(), Fraction(3, 2), TupletNumberType::SHOW_NUMBER, TupletBracketType::AUTO_BRACKET);
    });
    Measure* m1 = score->firstMeasure();
    ChordRest* first = toChordRest(m1->findSegment(SegmentType::ChordRest, Fraction(1, 4))->element(1));
    ASSERT_TRUE(first && first->tuplet());
    Tuplet* tuplet = first->tuplet();
    ASSERT_EQ(tuplet->tick(), Fraction(1, 4));

    // a quarter inserted on beat 1 moves the triplet to beat 3
    is.setTrack(0);
    is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->select(toChord(is.cr())->upNote());
    is.setTrack(0);
    is.setDuration(DurationType::V_QUARTER);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        NoteInput::addPitch(tx, score, NoteInputParams { 0 }, false, false);
    });
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(m1->ticks(), Fraction(5, 4));
    EXPECT_EQ(first->tick(), Fraction(2, 4));
    EXPECT_EQ(tuplet->tick(), Fraction(2, 4));

    score->undoRedo(true, nullptr);
    EXPECT_EQ(tuplet->tick(), Fraction(1, 4));
    score->undoRedo(false, nullptr);
    EXPECT_EQ(tuplet->tick(), Fraction(2, 4));

    // deleting the triplet leaves a rest in its place
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        score->cmdDeleteTuplet(tuplet, true);
    });
    EXPECT_TRUE(score->sanityCheck());
    Segment* segment = m1->findSegment(SegmentType::ChordRest, Fraction(2, 4));
    ASSERT_TRUE(segment);
    ChordRest* rest = toChordRest(segment->element(1));
    ASSERT_TRUE(rest);
    EXPECT_TRUE(rest->isRest());
    EXPECT_EQ(rest->ticks(), Fraction(1, 4));

    delete score;
}

TEST_F(Engraving_TupletTests, insertTimeThenDeleteTupletKeepsMeasureComplete)
{
    // four quarters, the second made into an eighth triplet
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    is.setDuration(DurationType::V_QUARTER);
    is.setNoteEntryMode(true);
    is.setNoteEntryMethod(NoteEntryMethod::BY_DURATION);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        for (int pitch : { 60, 62, 64, 65 }) {
            NoteVal nval(pitch);
            NoteInput::addPitch(tx, score, nval, false);
        }
    });
    Measure* m1 = score->firstMeasure();
    Chord* second = toChord(m1->findSegment(SegmentType::ChordRest, Fraction(1, 4))->element(0));
    ASSERT_TRUE(second);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        score->addTuplet(second, Fraction(3, 2), TupletNumberType::SHOW_NUMBER, TupletBracketType::AUTO_BRACKET);
    });
    ChordRest* tupletFirst = toChordRest(m1->findSegment(SegmentType::ChordRest, Fraction(1, 4))->element(0));
    ASSERT_TRUE(tupletFirst && tupletFirst->tuplet());
    Tuplet* tuplet = tupletFirst->tuplet();

    // a quarter inserted on beat 1 moves the triplet to beat 3
    is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->select(toChord(is.cr())->upNote());
    is.setTrack(0);
    is.setDuration(DurationType::V_QUARTER);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        NoteInput::addPitch(tx, score, NoteInputParams { 0 }, false, false);
    });
    ASSERT_EQ(m1->ticks(), Fraction(5, 4));

    // deleting the triplet leaves a rest on beat 3 and the first quarter on beat 2
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        score->cmdDeleteTuplet(tuplet, true);
    });
    EXPECT_TRUE(score->sanityCheck());
    ChordRest* beat2 = toChordRest(m1->findSegment(SegmentType::ChordRest, Fraction(1, 4))->element(0));
    ASSERT_TRUE(beat2 && beat2->isChord());
    EXPECT_EQ(toChord(beat2)->upNote()->pitch(), 60);
    Segment* beat3 = m1->findSegment(SegmentType::ChordRest, Fraction(2, 4));
    ASSERT_TRUE(beat3 && beat3->element(0));
    EXPECT_TRUE(beat3->element(0)->isRest());
    EXPECT_EQ(toChordRest(beat3->element(0))->ticks(), Fraction(1, 4));

    delete score;
}

TEST_F(Engraving_TupletTests, insertMeasureMovesOuterTuplets)
{
    // a quarter triplet made of three eighth triplets, then a half
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    is.setDuration(DurationType::V_HALF);
    is.setNoteEntryMode(true);
    is.setNoteEntryMethod(NoteEntryMethod::BY_DURATION);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        for (int pitch : { 60, 62 }) {
            NoteVal nval(pitch);
            NoteInput::addPitch(tx, score, nval, false);
        }
    });
    auto crAt = [&](const Fraction& tick) {
        Segment* s = score->tick2segment(tick, false, SegmentType::ChordRest);
        return s ? toChordRest(s->element(0)) : nullptr;
    };
    for (const Fraction& tick : { Fraction(0, 1), Fraction(0, 1), Fraction(1, 6), Fraction(2, 6) }) {
        score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
            score->addTuplet(crAt(tick), Fraction(3, 2), TupletNumberType::SHOW_NUMBER, TupletBracketType::AUTO_BRACKET);
        });
    }
    ASSERT_TRUE(crAt(Fraction(0, 1))->tuplet());
    Tuplet* outer = crAt(Fraction(0, 1))->tuplet()->tuplet();
    ASSERT_TRUE(outer);
    for (const DurationElement* element : outer->elements()) {
        ASSERT_TRUE(element->isTuplet());
    }

    // a measure inserted before moves the outer triplet along with the inner ones
    score->startCmd(TranslatableString::untranslatable("Tuplet tests"));
    score->insertMeasure(score->firstMeasure());
    score->endCmd();
    EXPECT_EQ(outer->tick(), Fraction(1, 1));

    // deleting the outer triplet leaves a half rest in its place
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        score->cmdDeleteTuplet(outer, true);
    });
    EXPECT_TRUE(score->sanityCheck());
    ChordRest* rest = crAt(Fraction(1, 1));
    EXPECT_TRUE(rest && rest->isRest() && rest->ticks() == Fraction(1, 2));
    ChordRest* half = crAt(Fraction(3, 2));
    EXPECT_TRUE(half && half->isChord());
    auto insertedMeasureIsRest = [&]() {
        ChordRest* cr = crAt(Fraction(0, 1));
        return cr && cr->isRest() && cr->ticks() == Fraction(1, 1);
    };
    EXPECT_TRUE(insertedMeasureIsRest());

    score->undoRedo(true, nullptr);
    EXPECT_EQ(outer->tick(), Fraction(1, 1));
    EXPECT_TRUE(insertedMeasureIsRest());
    EXPECT_TRUE(score->sanityCheck());
    score->undoRedo(true, nullptr);
    EXPECT_EQ(outer->tick(), Fraction(0, 1));
    EXPECT_TRUE(score->sanityCheck());

    delete score;
}

//-----------------------------------------
//    addTuplet
//     creates tuplets whose total duration differs from the
//     chord/rest they start at, using addTuplet.mscx:
//      m1: eight eighths, m2: half note + half rest,
//      m3: a 9:8 eighth tuplet, m4: eight eighths
//-----------------------------------------

static ChordRest* chordRestAt(Measure* measure, size_t index)
{
    size_t i = 0;
    for (Segment* s = measure->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        if (i++ == index) {
            return toChordRest(s->element(0));
        }
    }
    return nullptr;
}

static Measure* measureAt(Score* score, int index)
{
    Measure* m = score->firstMeasure();
    for (int i = 0; m && i < index; ++i) {
        m = m->nextMeasure();
    }
    return m;
}

static bool addTuplet(ChordRest* cr, const Fraction& ratio, const TDuration& baseLen)
{
    Score* score = cr->score();
    score->startCmd(TranslatableString::untranslatable("Engraving tuplet tests"));
    Tuplet* tuplet = score->addTuplet(cr, ratio, baseLen, TupletNumberType::SHOW_NUMBER, TupletBracketType::AUTO_BRACKET);
    score->endCmd();
    return tuplet;
}

TEST_F(Engraving_TupletTests, addTupletLongerThanChordRest)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = measureAt(score, 0);

    // 3 eighths in the time of 5, over the first five eighths of m1
    ASSERT_TRUE(addTuplet(chordRestAt(m1, 0), Fraction(3, 5), DurationType::V_EIGHTH));
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "3:5[8 r8 r8] 8 8 8");

    score->undoRedo(true, nullptr);
    EXPECT_EQ(rhythm(m1), "8 8 8 8 8 8 8 8");
    delete score;
}

TEST_F(Engraving_TupletTests, addTupletShorterThanChordRest)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    Measure* m2 = measureAt(score, 1);

    // an eighth triplet on the half note of m2: the remaining quarter becomes a rest
    ASSERT_TRUE(addTuplet(chordRestAt(m2, 0), Fraction(3, 2), DurationType::V_EIGHTH));
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m2), "3:2[8 r8 r8] r4 r2");

    score->undoRedo(true, nullptr);
    EXPECT_EQ(rhythm(m2), "2 r2");
    delete score;
}

TEST_F(Engraving_TupletTests, addTupletShorterDotted)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    Measure* m2 = measureAt(score, 1);

    // 2 dotted sixteenths in the time of 3 on the half note of m2: the tuplet's 9/32 isn't a
    // single note value, and the remaining 7/32 becomes rests
    TDuration dottedSixteenth(DurationType::V_16TH);
    dottedSixteenth.setDots(1);
    ASSERT_TRUE(addTuplet(chordRestAt(m2, 0), Fraction(2, 3), dottedSixteenth));
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m2), "2:3[16. r16.] r32 r16 r8 r2");

    score->undoRedo(true, nullptr);
    EXPECT_EQ(rhythm(m2), "2 r2");
    delete score;
}

TEST_F(Engraving_TupletTests, addTupletNested)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    // 3 eighths in the time of 5, over the last five elements of the 9:8 tuplet in m3 (#18980)
    ASSERT_TRUE(addTuplet(chordRestAt(measureAt(score, 2), 4), Fraction(3, 5), DurationType::V_EIGHTH));
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 2)), "9:8[8 8 8 8 3:5[8 r8 r8]]");

    // the nested tuplet survives a save and reload
    ASSERT_TRUE(ScoreRW::saveScore(score, u"addTuplet-nested.mscx"));
    delete score;
    score = ScoreRW::readScore(u"addTuplet-nested.mscx", true);
    ASSERT_TRUE(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 2)), "9:8[8 8 8 8 3:5[8 r8 r8]]");
    delete score;
}

TEST_F(Engraving_TupletTests, addTupletKeepsSelection)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = measureAt(score, 0);
    Note* note = toChord(chordRestAt(m1, 7))->upNote();
    score->select(note);
    score->inputState().setDuration(DurationType::V_QUARTER);

    // the caller decides what to select, e.g. note input calls selectNewTuplet()
    score->startCmd(TranslatableString::untranslatable("Engraving tuplet tests"));
    Tuplet* tuplet = score->addTuplet(chordRestAt(m1, 0), Fraction(3, 2), TupletNumberType::SHOW_NUMBER,
                                      TupletBracketType::AUTO_BRACKET);
    score->endCmd();
    ASSERT_TRUE(tuplet);
    EXPECT_EQ(rhythm(m1), "3:2[16 r16 r16] 8 8 8 8 8 8 8");
    EXPECT_EQ(score->selection().element(), note);
    EXPECT_EQ(score->inputState().duration(), TDuration(DurationType::V_QUARTER));
    delete score;
}

TEST_F(Engraving_TupletTests, addTupletKeepsTabString)
{
    // staff 0 as a guitar TAB staff
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    Staff* staff = score->staff(0);
    staff->setStaffType(Fraction(0, 1), *StaffType::preset(StaffTypes::TAB_6COMMON));
    int strings[] = { 40, 45, 50, 55, 59, 64 };
    staff->part()->instrument()->setStringData(StringData(24, 6, strings));

    // G4 at fret 8 on string 1, not its default string 0 (fret 3): the tuplet's first note keeps it
    InputState& is = score->inputState();
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    is.setDuration(DurationType::V_QUARTER);
    is.setNoteEntryMode(true);
    is.setString(1);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving tuplet tests"), [&](Transaction& tx) {
        NoteInput::addFret(tx, score, 8);
    });
    ASSERT_TRUE(addTuplet(chordRestAt(measureAt(score, 0), 0), Fraction(3, 2), DurationType::V_QUARTER));
    const Chord* chord = toChord(chordRestAt(measureAt(score, 0), 0));
    ASSERT_TRUE(chord->tuplet());
    EXPECT_EQ(chord->upNote()->string(), 1);
    EXPECT_EQ(chord->upNote()->fret(), 8);

    delete score;
}

// Divides the selected chords/rests, as the tuplet commands do
static std::vector<Tuplet*> addTuplets(Score* score, const Fraction& ratio)
{
    std::vector<Tuplet*> tuplets;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving tuplet tests"), [&](Transaction& tx) {
        tuplets = EditTuplet::addTuplets(tx, score, ratio, TupletNumberType::SHOW_NUMBER, TupletBracketType::AUTO_BRACKET,
                                         /*autoBaseLen*/ true);
    });
    return tuplets;
}

// Adds a second staff to the first staff's part, linked to it or with measure rests
static void addSecondStaff(Score* score, bool linked)
{
    score->startCmd(TranslatableString::untranslatable("Engraving tuplet tests"));
    Staff* staff = score->staff(0);
    Staff* newStaff = Factory::createStaff(staff->part());
    newStaff->setPart(staff->part());
    score->undoInsertStaff(newStaff, 1, /*createRests*/ !linked);
    if (linked) {
        Excerpt::cloneStaff(staff, newStaff);
    }
    score->endCmd();
}

TEST_F(Engraving_TupletTests, addTuplets)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = measureAt(score, 0);

    // 3 eighths, selected from last to first: the tuplets come back in score order
    score->select(toChord(chordRestAt(m1, 5))->upNote(), SelectType::SINGLE);
    score->select(toChord(chordRestAt(m1, 2))->upNote(), SelectType::ADD);
    score->select(toChord(chordRestAt(m1, 0))->upNote(), SelectType::ADD);
    const std::vector<Tuplet*> tuplets = addTuplets(score, Fraction(3, 2));
    ASSERT_EQ(tuplets.size(), 3);
    EXPECT_EQ(tuplets[0]->tick(), Fraction(0, 1));
    EXPECT_EQ(tuplets[1]->tick(), Fraction(1, 4));
    EXPECT_EQ(tuplets[2]->tick(), Fraction(5, 8));
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "3:2[16 r16 r16] 8 3:2[16 r16 r16] 8 8 3:2[16 r16 r16] 8 8");

    // the start of the first tuplet is selected, and its note value becomes the input duration
    EXPECT_TRUE(score->selection().isSingle());
    EXPECT_EQ(score->selection().element(), toChord(tuplets[0]->elements().front())->upNote());
    EXPECT_EQ(score->inputState().duration(), TDuration(DurationType::V_16TH));

    score->undoRedo(true, nullptr);
    EXPECT_EQ(rhythm(m1), "8 8 8 8 8 8 8 8");

    delete score;
}

TEST_F(Engraving_TupletTests, addTupletsKeepsRange)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = measureAt(score, 0);

    score->select(chordRestAt(m1, 0), SelectType::RANGE);
    score->select(chordRestAt(m1, 1), SelectType::RANGE);
    ASSERT_EQ(score->getSelectedChordRests().size(), 2);
    EXPECT_EQ(addTuplets(score, Fraction(3, 2)).size(), 2);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "3:2[16 r16 r16] 3:2[16 r16 r16] 8 8 8 8 8 8");

    const Selection& selection = score->selection();
    EXPECT_TRUE(selection.isRange());
    EXPECT_EQ(selection.tickStart(), Fraction(0, 1));
    EXPECT_EQ(selection.tickEnd(), Fraction(1, 4));
    EXPECT_EQ(selection.staffStart(), 0);
    EXPECT_EQ(selection.staffEnd(), 1);
    EXPECT_EQ(score->getSelectedChordRests().size(), 6);

    delete score;
}

TEST_F(Engraving_TupletTests, addTupletsRangeAcrossStaves)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    addSecondStaff(score, /*linked*/ false);
    Measure* m1 = measureAt(score, 0);

    // the first 2 eighths of m1 and the measure rest below them
    score->selection().setRange(chordRestAt(m1, 0)->segment(), chordRestAt(m1, 2)->segment(), 0, 2);
    score->selection().updateSelectedElements();
    ASSERT_EQ(score->getSelectedChordRests().size(), 3);
    const std::vector<Tuplet*> tuplets = addTuplets(score, Fraction(3, 2));
    EXPECT_TRUE(score->sanityCheck());

    // score order: by tick, then by track
    ASSERT_EQ(tuplets.size(), 3);
    EXPECT_EQ(tuplets[0]->tick(), Fraction(0, 1));
    EXPECT_EQ(tuplets[0]->staffIdx(), 0);
    EXPECT_EQ(tuplets[1]->tick(), Fraction(0, 1));
    EXPECT_EQ(tuplets[1]->staffIdx(), 1);
    EXPECT_EQ(tuplets[2]->tick(), Fraction(1, 8));
    EXPECT_EQ(tuplets[2]->staffIdx(), 0);

    const Selection& selection = score->selection();
    EXPECT_TRUE(selection.isRange());
    EXPECT_EQ(selection.tickStart(), Fraction(0, 1));
    EXPECT_EQ(selection.tickEnd(), Fraction(1, 4));
    EXPECT_EQ(selection.staffStart(), 0);
    EXPECT_EQ(selection.staffEnd(), 2);

    delete score;
}

TEST_F(Engraving_TupletTests, addTupletsLinkedStaff)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    addSecondStaff(score, /*linked*/ true);
    Measure* m1 = measureAt(score, 0);

    // the first eighth of m1 on both staves: one tuplet, which the linked staff gets too
    Segment* first = m1->first(SegmentType::ChordRest);
    score->select(first->cr(0), SelectType::RANGE);
    score->select(first->cr(VOICES), SelectType::RANGE);
    ASSERT_EQ(score->getSelectedChordRests().size(), 2);
    EXPECT_EQ(addTuplets(score, Fraction(3, 2)).size(), 1);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "3:2[16 r16 r16] 8 8 8 8 8 8 8");

    const Tuplet* linkedTuplet = first->cr(VOICES)->tuplet();
    ASSERT_TRUE(linkedTuplet);
    EXPECT_EQ(linkedTuplet->ratio(), Fraction(3, 2));
    EXPECT_FALSE(linkedTuplet->tuplet());
    EXPECT_EQ(linkedTuplet->elements().size(), 3);
    EXPECT_TRUE(score->selection().isRange());
    EXPECT_EQ(score->selection().staffEnd(), 2);

    delete score;
}

TEST_F(Engraving_TupletTests, canAddTuplet)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    const TDuration eighth(DurationType::V_EIGHTH);

    // fits exactly up to the barline
    EXPECT_TRUE(score->canAddTuplet(chordRestAt(measureAt(score, 3), 3), Fraction(3, 5), eighth));
    // would cross the barline
    EXPECT_FALSE(score->canAddTuplet(chordRestAt(measureAt(score, 3), 4), Fraction(3, 5), eighth));
    // fits exactly up to the end of the parent tuplet
    EXPECT_TRUE(score->canAddTuplet(chordRestAt(measureAt(score, 2), 4), Fraction(3, 5), eighth));
    // would cross the end of the parent tuplet
    EXPECT_FALSE(score->canAddTuplet(chordRestAt(measureAt(score, 2), 5), Fraction(3, 5), eighth));
    // invalid ratio
    EXPECT_FALSE(score->canAddTuplet(chordRestAt(measureAt(score, 0), 0), Fraction(0, 5), eighth));

    delete score;
}

TEST_F(Engraving_TupletTests, canAddTupletInEmptyVoice)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    // halfway through m1, where voice 2 is empty
    const Segment* segment = chordRestAt(measureAt(score, 0), 4)->segment();
    ASSERT_FALSE(segment->cr(1));

    // fits exactly up to the barline
    EXPECT_TRUE(score->canAddTuplet(segment, 1, Fraction(3, 2), DurationType::V_QUARTER));
    // would cross the barline
    EXPECT_FALSE(score->canAddTuplet(segment, 1, Fraction(3, 2), DurationType::V_HALF));
    // with a chord/rest in the track, the same as for that chord/rest: would cross the barline
    EXPECT_FALSE(score->canAddTuplet(chordRestAt(measureAt(score, 3), 4)->segment(), 0, Fraction(3, 5), DurationType::V_EIGHTH));
    // invalid ratio
    EXPECT_FALSE(score->canAddTuplet(segment, 1, Fraction(0, 2), DurationType::V_QUARTER));
    // no track
    EXPECT_FALSE(score->canAddTuplet(segment, muse::nidx, Fraction(3, 2), DurationType::V_QUARTER));

    delete score;
}

TEST_F(Engraving_TupletTests, canAddTupletInEmptyVoiceDotted)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    TDuration dottedEighth(DurationType::V_EIGHTH);
    dottedEighth.setDots(1);
    TDuration doubleDottedEighth(DurationType::V_EIGHTH);
    doubleDottedEighth.setDots(2);

    // halfway through m1, where voice 2 is empty
    const Segment* segment = chordRestAt(measureAt(score, 0), 4)->segment();
    // 3 dotted eighths in the time of 2: 3/8 fits
    EXPECT_TRUE(score->canAddTuplet(segment, 1, Fraction(3, 2), dottedEighth));
    // 2 dotted eighths in the time of 3: 9/16 would cross the barline
    EXPECT_FALSE(score->canAddTuplet(segment, 1, Fraction(2, 3), dottedEighth));
    // 3 double-dotted eighths in the time of 2: 7/16 fits
    EXPECT_TRUE(score->canAddTuplet(segment, 1, Fraction(3, 2), doubleDottedEighth));
    // 2 double-dotted eighths in the time of 3: 21/32 would cross the barline
    EXPECT_FALSE(score->canAddTuplet(segment, 1, Fraction(2, 3), doubleDottedEighth));

    delete score;
}

TEST_F(Engraving_TupletTests, canAddTupletInsideLongerChordRest)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = measureAt(score, 0);

    // voice 2 of m1: 3 quarter rests in the time of 2 over the first half
    Segment* first = m1->first(SegmentType::ChordRest);
    score->startCmd(TranslatableString::untranslatable("Engraving tuplet tests"));
    score->expandVoice(first, 1);
    score->endCmd();
    ASSERT_TRUE(first->cr(1));
    ASSERT_TRUE(addTuplet(first->cr(1), Fraction(3, 2), DurationType::V_QUARTER));

    // the second eighth of m1, inside the first rest of that tuplet
    const Segment* segment = m1->findSegment(SegmentType::ChordRest, Fraction(1, 8));
    ASSERT_TRUE(segment);
    ASSERT_FALSE(segment->cr(1));

    // 3 eighths in the time of 2 fit in the tuplet
    EXPECT_TRUE(score->canAddTuplet(segment, 1, Fraction(3, 2), DurationType::V_EIGHTH));
    // 4 quarters in the time of 3 would cross the end of the tuplet, though not the barline
    EXPECT_FALSE(score->canAddTuplet(segment, 1, Fraction(4, 3), DurationType::V_QUARTER));

    delete score;
}

TEST_F(Engraving_TupletTests, tupletEndTick)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    const TDuration eighth(DurationType::V_EIGHTH);

    // 3 eighths in the time of 2, from the start of m1
    EXPECT_EQ(score->tupletEndTick(chordRestAt(measureAt(score, 0), 0), Fraction(3, 2), eighth), Fraction(1, 4));
    // 3 eighths in the time of 5, from halfway through m1: it ends after the barline
    EXPECT_EQ(score->tupletEndTick(chordRestAt(measureAt(score, 0), 4), Fraction(3, 5), eighth), Fraction(9, 8));
    // 3 eighths in the time of 5 in the 9:8 tuplet of m3: it ends with the 9:8 tuplet
    EXPECT_EQ(score->tupletEndTick(chordRestAt(measureAt(score, 2), 4), Fraction(3, 5), eighth), measureAt(score, 2)->endTick());

    delete score;
}

TEST_F(Engraving_TupletTests, tupletEndTickDotted)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);

    TDuration dottedEighth(DurationType::V_EIGHTH);
    dottedEighth.setDots(1);
    TDuration doubleDottedEighth(DurationType::V_EIGHTH);
    doubleDottedEighth.setDots(2);

    // from the start of m1
    ChordRest* first = chordRestAt(measureAt(score, 0), 0);
    // 2 dotted eighths in the time of 3: 9/16
    EXPECT_EQ(score->tupletEndTick(first, Fraction(2, 3), dottedEighth), Fraction(9, 16));
    // 2 double-dotted eighths in the time of 3: 21/32
    EXPECT_EQ(score->tupletEndTick(first, Fraction(2, 3), doubleDottedEighth), Fraction(21, 32));

    delete score;
}

// Checks and adds tuplets that use the selected durations as the subdivision, as the tuplet commands do
static std::vector<Tuplet*> addTupletsWithSelectedDurations(Score* score, const Fraction& ratio)
{
    MScore::setError(MsError::MS_NO_ERROR);
    if (!EditTuplet::checkAddTupletsWithSelectedDurations(score, ratio)) {
        return {};
    }

    std::vector<Tuplet*> tuplets;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving tuplet tests"), [&](Transaction& tx) {
        tuplets = EditTuplet::addTuplets(tx, score, ratio, TupletNumberType::SHOW_NUMBER, TupletBracketType::AUTO_BRACKET,
                                         /*autoBaseLen*/ true, TupletSelectedDuration::SUBDIVISION);
    });
    return tuplets;
}

TEST_F(Engraving_TupletTests, addTupletsWithSelectedDurations)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = measureAt(score, 0);

    // the first 3 eighths of m1: the first tuplet replaces the second eighth, so it gets no tuplet of its own
    score->select(chordRestAt(m1, 0), SelectType::SINGLE);
    score->select(chordRestAt(m1, 1), SelectType::ADD);
    score->select(chordRestAt(m1, 2), SelectType::ADD);
    EXPECT_EQ(addTupletsWithSelectedDurations(score, Fraction(3, 2)).size(), 2);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "3:2[8 r8 r8] 3:2[8 r8 r8] 8 8 8 8");

    // the start of the first tuplet is selected
    EXPECT_EQ(score->selection().element(), toChord(chordRestAt(m1, 0))->upNote());
    EXPECT_EQ(score->inputState().duration(), TDuration(DurationType::V_EIGHTH));

    score->undoRedo(true, nullptr);
    EXPECT_EQ(rhythm(m1), "8 8 8 8 8 8 8 8");

    delete score;
}

TEST_F(Engraving_TupletTests, addTupletsWithSelectedDurationsInRange)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = measureAt(score, 0);

    // a range of 1 eighth
    score->select(chordRestAt(m1, 0), SelectType::RANGE);
    EXPECT_EQ(addTupletsWithSelectedDurations(score, Fraction(3, 2)).size(), 1);
    EXPECT_EQ(MScore::_error, MsError::MS_NO_ERROR);
    EXPECT_EQ(rhythm(m1), "3:2[8 r8 r8] 8 8 8 8 8 8");

    // the range grows to the end of the tuplet
    EXPECT_TRUE(score->selection().isRange());
    EXPECT_EQ(score->selection().tickStart(), Fraction(0, 1));
    EXPECT_EQ(score->selection().tickEnd(), Fraction(1, 4));

    score->undoRedo(true, nullptr);
    EXPECT_EQ(rhythm(m1), "8 8 8 8 8 8 8 8");

    // a range of 3 eighths: the first tuplet replaces the second eighth, and the third gets a tuplet that ends after
    // the range
    score->select(chordRestAt(m1, 0), SelectType::RANGE);
    score->select(chordRestAt(m1, 2), SelectType::RANGE);
    EXPECT_EQ(addTupletsWithSelectedDurations(score, Fraction(3, 2)).size(), 2);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "3:2[8 r8 r8] 3:2[8 r8 r8] 8 8 8 8");
    EXPECT_EQ(score->selection().tickEnd(), Fraction(1, 2));

    score->undoRedo(true, nullptr);
    EXPECT_EQ(rhythm(m1), "8 8 8 8 8 8 8 8");

    // the whole measure: a tuplet on every other eighth
    score->select(chordRestAt(m1, 0), SelectType::RANGE);
    score->select(chordRestAt(m1, 7), SelectType::RANGE);
    EXPECT_EQ(addTupletsWithSelectedDurations(score, Fraction(3, 2)).size(), 4);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "3:2[8 r8 r8] 3:2[8 r8 r8] 3:2[8 r8 r8] 3:2[8 r8 r8]");

    delete score;
}

TEST_F(Engraving_TupletTests, addTupletsWithSelectedDurationsNoSpace)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = measureAt(score, 0);

    // the last eighth's tuplet would cross the barline: nothing fits
    score->select(chordRestAt(m1, 7), SelectType::SINGLE);
    EXPECT_TRUE(addTupletsWithSelectedDurations(score, Fraction(3, 2)).empty());
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
    EXPECT_EQ(rhythm(m1), "8 8 8 8 8 8 8 8");

    // the first one fits, the last one is left out
    score->select(chordRestAt(m1, 0), SelectType::SINGLE);
    score->select(chordRestAt(m1, 7), SelectType::ADD);
    EXPECT_EQ(addTupletsWithSelectedDurations(score, Fraction(3, 2)).size(), 1);
    EXPECT_EQ(MScore::_error, MsError::MS_NO_ERROR);
    EXPECT_EQ(rhythm(m1), "3:2[8 r8 r8] 8 8 8 8 8 8");

    score->undoRedo(true, nullptr);

    // would cross the end of the 9:8 tuplet of m3
    Measure* m3 = measureAt(score, 2);
    score->select(chordRestAt(m3, 8), SelectType::SINGLE);
    EXPECT_TRUE(addTupletsWithSelectedDurations(score, Fraction(3, 2)).empty());
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);

    delete score;
}

TEST_F(Engraving_TupletTests, addTupletsWithSelectedDurationsInRangeNoSpace)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = measureAt(score, 0);

    TDuration dottedHalf(DurationType::V_HALF);
    dottedHalf.setDots(1);
    score->startCmd(TranslatableString::untranslatable("Engraving tuplet tests"));
    score->changeCRlen(chordRestAt(m1, 0), dottedHalf);
    score->endCmd();
    ASSERT_EQ(rhythm(m1), "2. 8 8");

    // 3 dotted halves in the time of 2 don't fit, so they replace nothing: the next eighth gets its tuplet
    score->select(chordRestAt(m1, 0), SelectType::RANGE);
    score->select(chordRestAt(m1, 2), SelectType::RANGE);
    EXPECT_EQ(addTupletsWithSelectedDurations(score, Fraction(3, 2)).size(), 1);
    EXPECT_EQ(MScore::_error, MsError::MS_NO_ERROR);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "2. 3:2[8 r8 r8]");

    score->undoRedo(true, nullptr);
    EXPECT_EQ(rhythm(m1), "2. 8 8");

    // a dotted half and a quarter: neither tuplet fits
    score->startCmd(TranslatableString::untranslatable("Engraving tuplet tests"));
    score->changeCRlen(chordRestAt(m1, 1), TDuration(DurationType::V_QUARTER));
    score->endCmd();
    ASSERT_EQ(rhythm(m1), "2. 4");
    score->select(chordRestAt(m1, 0), SelectType::RANGE);
    score->select(chordRestAt(m1, 1), SelectType::RANGE);
    EXPECT_TRUE(addTupletsWithSelectedDurations(score, Fraction(3, 2)).empty());
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
    EXPECT_EQ(rhythm(m1), "2. 4");

    delete score;
}

TEST_F(Engraving_TupletTests, addTupletsWithSelectedDurationsRangeAcrossStaves)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    addSecondStaff(score, /*linked*/ false);
    Measure* m1 = measureAt(score, 0);

    // m1 on both staves: the eighths of the first staff get a tuplet on every other eighth, and the measure rest of the
    // second staff is left as it is
    score->selection().setRange(chordRestAt(m1, 0)->segment(), chordRestAt(measureAt(score, 1), 0)->segment(), 0, 2);
    score->selection().updateSelectedElements();
    const std::vector<Tuplet*> tuplets = addTupletsWithSelectedDurations(score, Fraction(3, 2));
    EXPECT_EQ(MScore::_error, MsError::MS_NO_ERROR);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "3:2[8 r8 r8] 3:2[8 r8 r8] 3:2[8 r8 r8] 3:2[8 r8 r8]");
    EXPECT_TRUE(m1->first(SegmentType::ChordRest)->cr(VOICES)->durationType().isMeasure());

    ASSERT_EQ(tuplets.size(), 4);
    for (const Tuplet* tuplet : tuplets) {
        EXPECT_EQ(tuplet->staffIdx(), 0);
    }

    delete score;
}

TEST_F(Engraving_TupletTests, addTupletsWithSelectedDurationsMeasureRest)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = measureAt(score, 0);

    // a measure rest has no duration to use as the subdivision, so it gets no tuplet, alone or in a range
    score->select(chordRestAt(m1, 0), SelectType::SINGLE);
    EXPECT_TRUE(addTupletsWithSelectedDurations(score, Fraction(3, 2)).empty());
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);

    score->select(chordRestAt(m1, 0), SelectType::RANGE);
    score->select(chordRestAt(measureAt(score, 1), 0), SelectType::RANGE);
    EXPECT_TRUE(addTupletsWithSelectedDurations(score, Fraction(3, 2)).empty());
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
    EXPECT_EQ(rhythm(m1), "R");
    EXPECT_EQ(rhythm(measureAt(score, 1)), "R");

    // with the selected duration as the tuplet length, each is divided
    EXPECT_EQ(addTuplets(score, Fraction(3, 2)).size(), 2);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "3:2[r2 r2 r2]");
    EXPECT_EQ(rhythm(measureAt(score, 1)), "3:2[r2 r2 r2]");

    delete score;
}

TEST_F(Engraving_TupletTests, addTupletsWithSelectedDurationsLinkedStaff)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    addSecondStaff(score, /*linked*/ true);
    Measure* m1 = measureAt(score, 0);

    // the first eighth of m1 on both staves: the tuplet on the first staff replaces the linked one
    Segment* first = m1->first(SegmentType::ChordRest);
    score->select(first->cr(0), SelectType::RANGE);
    score->select(first->cr(VOICES), SelectType::RANGE);
    ASSERT_EQ(score->getSelectedChordRests().size(), 2);
    EXPECT_EQ(addTupletsWithSelectedDurations(score, Fraction(3, 2)).size(), 1);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "3:2[8 r8 r8] 8 8 8 8 8 8");

    const Tuplet* linkedTuplet = first->cr(VOICES)->tuplet();
    ASSERT_TRUE(linkedTuplet);
    EXPECT_EQ(linkedTuplet->ratio(), Fraction(3, 2));
    EXPECT_FALSE(linkedTuplet->tuplet());
    EXPECT_EQ(linkedTuplet->elements().size(), 3);

    delete score;
}

TEST_F(Engraving_TupletTests, addTupletsWithSelectedDurationsDotted)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = measureAt(score, 0);

    TDuration dottedQuarter(DurationType::V_QUARTER);
    dottedQuarter.setDots(1);
    score->startCmd(TranslatableString::untranslatable("Engraving tuplet tests"));
    score->changeCRlen(chordRestAt(m1, 0), dottedQuarter);
    score->endCmd();
    ASSERT_EQ(rhythm(m1), "4. 8 8 8 8 8");

    // 3 dotted quarters in the time of 2 end at 3/4: the eighth at 3/8 gets no tuplet of its own
    score->select(chordRestAt(m1, 0), SelectType::SINGLE);
    score->select(chordRestAt(m1, 1), SelectType::ADD);
    score->select(chordRestAt(m1, 4), SelectType::ADD);
    EXPECT_EQ(addTupletsWithSelectedDurations(score, Fraction(3, 2)).size(), 2);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "3:2[4. r4. r4.] 3:2[8 r8 r8]");

    score->undoRedo(true, nullptr);
    EXPECT_EQ(rhythm(m1), "4. 8 8 8 8 8");

    // 2 dotted quarters in the time of 3: 9/8 would cross the barline
    score->select(chordRestAt(m1, 0), SelectType::SINGLE);
    EXPECT_TRUE(addTupletsWithSelectedDurations(score, Fraction(2, 3)).empty());
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);

    delete score;
}

TEST_F(Engraving_TupletTests, addTupletsWithSelectedDurationsDoubleDotted)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"addTuplet.mscx");
    ASSERT_TRUE(score);
    Measure* m2 = measureAt(score, 1);

    TDuration doubleDottedQuarter(DurationType::V_QUARTER);
    doubleDottedQuarter.setDots(2);
    score->startCmd(TranslatableString::untranslatable("Engraving tuplet tests"));
    score->changeCRlen(chordRestAt(m2, 0), doubleDottedQuarter);
    score->endCmd();
    ASSERT_EQ(rhythm(m2), "4.. r16 r2");

    // 3 double-dotted quarters in the time of 2 end at 7/8: the 16th rest at 7/16 gets no tuplet of its own
    score->select(chordRestAt(m2, 0), SelectType::SINGLE);
    score->select(chordRestAt(m2, 1), SelectType::ADD);
    EXPECT_EQ(addTupletsWithSelectedDurations(score, Fraction(3, 2)).size(), 1);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m2), "3:2[4.. r4.. r4..] r8");

    score->undoRedo(true, nullptr);
    EXPECT_EQ(rhythm(m2), "4.. r16 r2");

    // 2 double-dotted quarters in the time of 3: 21/16 would cross the barline
    score->select(chordRestAt(m2, 0), SelectType::SINGLE);
    EXPECT_TRUE(addTupletsWithSelectedDurations(score, Fraction(2, 3)).empty());
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);

    delete score;
}

TEST_F(Engraving_TupletTests, conventionalTupletDenominator)
{
    // duplet to nonuplet: 2:3, 3:2, 4:3, 5:4, 6:4, 7:4, 8:6, 9:8
    const std::vector<std::pair<int, int> > ratios = { { 2, 3 }, { 3, 2 }, { 4, 3 }, { 5, 4 }, { 6, 4 }, { 7, 4 }, { 8, 6 }, { 9, 8 } };
    for (const auto& [numerator, denominator] : ratios) {
        EXPECT_EQ(Tuplet::conventionalTupletDenominator(numerator), denominator) << numerator;
    }
    EXPECT_EQ(Tuplet::conventionalTupletDenominator(0), 0);
}

//-----------------------------------------
//    deleteTupletSelection
//     deleting a tuplet selects the rest that replaces it
//-----------------------------------------
TEST_F(Engraving_TupletTests, deleteTupletSelection)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"tuplet1.mscx");
    ASSERT_TRUE(score);
    Measure* m2 = score->firstMeasure()->nextMeasure();
    ASSERT_TRUE(m2 && m2 != score->lastMeasure());

    // a triplet from the first chord of bar 2
    ChordRest* first = m2->first(SegmentType::ChordRest)->cr(0);
    ASSERT_TRUE(first && first->isChord());
    ASSERT_TRUE(createTuplet(3, first));
    Tuplet* tuplet = m2->first(SegmentType::ChordRest)->cr(0)->tuplet();
    ASSERT_TRUE(tuplet);
    Fraction tupletTick = tuplet->tick();

    score->select(tuplet);
    score->startCmd(TranslatableString::untranslatable("Engraving tuplet tests"));
    score->cmdDeleteSelection();
    score->endCmd();

    ChordRest* selected = score->selection().firstChordRest();
    ASSERT_TRUE(selected);
    EXPECT_TRUE(selected->isRest());
    EXPECT_EQ(selected->measure(), m2);
    EXPECT_EQ(selected->tick(), tupletTick);
    EXPECT_FALSE(selected->tuplet());
    delete score;
}

//-----------------------------------------
//    deleteNestedTupletSelection
//     deleting a nested tuplet selects the rest that replaces it
//     in the outer tuplet
//-----------------------------------------
TEST_F(Engraving_TupletTests, deleteNestedTupletSelection)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"nestedTuplets_lengthen.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure();
    ASSERT_EQ(rhythm(m1), "9:8[8 8 8 3:2[8 8 8] 8 8 8 8]");

    // the nested triplet starts on the 4th chord
    Segment* s = m1->first(SegmentType::ChordRest);
    for (int i = 0; i < 3; ++i) {
        s = s->next(SegmentType::ChordRest);
    }
    Tuplet* nested = s->cr(0)->tuplet();
    ASSERT_TRUE(nested && nested->tuplet());
    Tuplet* outer = nested->tuplet();
    Fraction nestedTick = nested->tick();

    score->select(nested);
    score->startCmd(TranslatableString::untranslatable("Engraving tuplet tests"));
    score->cmdDeleteSelection();
    score->endCmd();

    EXPECT_EQ(rhythm(m1), "9:8[8 8 8 r4 8 8 8 8]");
    ChordRest* selected = score->selection().firstChordRest();
    ASSERT_TRUE(selected);
    EXPECT_TRUE(selected->isRest());
    EXPECT_EQ(selected->tick(), nestedTick);
    EXPECT_EQ(selected->tuplet(), outer);
    delete score;
}

TEST_F(Engraving_TupletTests, removeTimeMovesLaterTuplets)
{
    // four quarters, the second made into an eighth triplet
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    is.setDuration(DurationType::V_QUARTER);
    is.setNoteEntryMode(true);
    is.setNoteEntryMethod(NoteEntryMethod::BY_DURATION);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        for (int pitch : { 60, 62, 64, 65 }) {
            NoteVal nval(pitch);
            NoteInput::addPitch(tx, score, nval, false);
        }
    });
    Measure* m1 = score->firstMeasure();
    auto crAt = [&](const Fraction& tick) {
        Segment* s = m1->findSegment(SegmentType::ChordRest, tick);
        return s ? toChordRest(s->element(0)) : nullptr;
    };
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        score->addTuplet(crAt(Fraction(1, 4)), Fraction(3, 2), TupletNumberType::SHOW_NUMBER, TupletBracketType::AUTO_BRACKET);
    });
    Tuplet* tuplet = crAt(Fraction(1, 4))->tuplet();
    ASSERT_TRUE(tuplet);

    // removing the first quarter moves the triplet to beat 1
    score->select(toChord(crAt(Fraction(0, 1)))->upNote());
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        score->cmdTimeDelete();
    });
    EXPECT_TRUE(score->sanityCheck());
    ASSERT_EQ(rhythm(m1), "3:2[8 r8 r8] 4 4");
    EXPECT_EQ(tuplet->tick(), Fraction(0, 1));

    score->undoRedo(true, nullptr);
    EXPECT_EQ(tuplet->tick(), Fraction(1, 4));
    score->undoRedo(false, nullptr);
    EXPECT_EQ(tuplet->tick(), Fraction(0, 1));

    // deleting the triplet leaves a rest in its place
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        score->cmdDeleteTuplet(tuplet, true);
    });
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "r4 4 4");

    delete score;
}

TEST_F(Engraving_TupletTests, removeInsertedTimeMovesNestedTupletsBack)
{
    // quarter, a quarter triplet starting with an eighth triplet, quarter
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    is.setNoteEntryMode(true);
    is.setNoteEntryMethod(NoteEntryMethod::BY_DURATION);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        for (DurationType duration : { DurationType::V_QUARTER, DurationType::V_HALF, DurationType::V_QUARTER }) {
            is.setDuration(duration);
            NoteVal nval(60);
            NoteInput::addPitch(tx, score, nval, false);
        }
    });
    Measure* m1 = score->firstMeasure();
    auto crAt = [&](const Fraction& tick) {
        Segment* s = m1->findSegment(SegmentType::ChordRest, tick);
        return s ? toChordRest(s->element(0)) : nullptr;
    };
    for (int i = 0; i < 2; ++i) {
        score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
            score->addTuplet(crAt(Fraction(1, 4)), Fraction(3, 2), TupletNumberType::SHOW_NUMBER, TupletBracketType::AUTO_BRACKET);
        });
    }
    ASSERT_EQ(rhythm(m1), "4 3:2[ 3:2[8 r8 r8] r4 r4] 4");
    Tuplet* inner = crAt(Fraction(1, 4))->tuplet();
    ASSERT_TRUE(inner && inner->tuplet());
    Tuplet* outer = inner->tuplet();

    // Insert adds a quarter on beat 1, then removing it moves both tuplets back to beat 2
    is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    is.setSegment(m1->first(SegmentType::ChordRest));
    score->select(toChord(crAt(Fraction(0, 1)))->upNote());
    is.setTrack(0);
    is.setDuration(DurationType::V_QUARTER);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        NoteInput::addPitch(tx, score, NoteInputParams { 0 }, false, false);
    });
    ASSERT_EQ(rhythm(m1), "4 4 3:2[ 3:2[8 r8 r8] r4 r4] 4");
    ASSERT_EQ(outer->tick(), Fraction(2, 4));
    is.setNoteEntryMode(false);
    score->select(toChord(crAt(Fraction(0, 1)))->upNote());
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        score->cmdTimeDelete();
    });
    EXPECT_TRUE(score->sanityCheck());
    ASSERT_EQ(rhythm(m1), "4 3:2[ 3:2[8 r8 r8] r4 r4] 4");
    EXPECT_EQ(outer->tick(), Fraction(1, 4));
    EXPECT_EQ(inner->tick(), Fraction(1, 4));

    // deleting the outer triplet leaves a rest in its place
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        score->cmdDeleteTuplet(outer, true);
    });
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(m1), "4 r4 r4 4");

    delete score;
}

//-----------------------------------------
//    addNestedTriplets
//     fills a half note's length at the start of a bar with a quarter triplet
//     of three eighth triplets, the given pitches in each,
//     and returns the outer triplet
//-----------------------------------------

static Tuplet* addNestedTriplets(Measure* measure, const std::vector<int>& pitches)
{
    Score* score = measure->score();
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        score->setNoteRest(measure->first(SegmentType::ChordRest), 0, NoteVal(pitches[0]), Fraction(1, 2));
    });
    if (!addTuplet(chordRestAt(measure, 0), Fraction(3, 2), DurationType::V_QUARTER)) {
        ADD_FAILURE() << "quarter triplet not added";
        return nullptr;
    }
    for (size_t i = 0; i < 9; i += 3) {
        if (!addTuplet(chordRestAt(measure, i), Fraction(3, 2), DurationType::V_EIGHTH)) {
            ADD_FAILURE() << "eighth triplet not added at element " << i;
            return nullptr;
        }
    }
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        for (size_t i = 0; i < 9; ++i) {
            score->setNoteRest(chordRestAt(measure, i)->segment(), 0, NoteVal(pitches[i % 3]), Fraction(1, 8));
        }
    });
    const std::string expected = "3:2[ 3:2[8 8 8] 3:2[8 8 8] 3:2[8 8 8]]";
    const std::string result = rhythm(measure);
    if (result.substr(0, expected.size()) != expected) {
        ADD_FAILURE() << "unexpected rhythm: " << result;
        return nullptr;
    }
    return chordRestAt(measure, 0)->tuplet()->tuplet();
}

static void changeProperty(EngravingItem* item, Pid pid, const PropertyValue& value)
{
    item->score()->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        item->undoChangeProperty(pid, value);
    });
}

//-----------------------------------------
//    nestedTupletsAvoidEachOther
//     an outer tuplet's bracket or lone number clears
//     the brackets of the tuplets nested inside it
//-----------------------------------------

TEST_F(Engraving_TupletTests, nestedTupletsAvoidEachOther)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    // E4 F4 G4: stems up, all tuplets above
    Measure* m1 = score->firstMeasure();
    Tuplet* outer = addNestedTriplets(m1, { 64, 65, 67 });
    ASSERT_TRUE(outer);

    // nested tuplets with a bracket and no number
    for (DurationElement* e : outer->elements()) {
        changeProperty(toTuplet(e), Pid::NUMBER_TYPE, int(TupletNumberType::NO_TEXT));
        changeProperty(toTuplet(e), Pid::BRACKET_TYPE, int(TupletBracketType::SHOW_BRACKET));
    }
    auto expectOuterClear = [&]() {
        const Shape outerShape = outer->shape().translated(outer->pagePos());
        for (const DurationElement* e : outer->elements()) {
            const Tuplet* inner = toTuplet(e);
            EXPECT_TRUE(inner->hasBracket());
            EXPECT_FALSE(outerShape.intersects(inner->shape().translated(inner->pagePos())));
        }
    };

    // the outer tuplet with a bracket and no number
    changeProperty(outer, Pid::NUMBER_TYPE, int(TupletNumberType::NO_TEXT));
    changeProperty(outer, Pid::BRACKET_TYPE, int(TupletBracketType::SHOW_BRACKET));
    ASSERT_TRUE(outer->hasBracket());
    expectOuterClear();

    // the outer tuplet with a number and no bracket
    changeProperty(outer, Pid::NUMBER_TYPE, int(TupletNumberType::SHOW_NUMBER));
    changeProperty(outer, Pid::BRACKET_TYPE, int(TupletBracketType::SHOW_NO_BRACKET));
    ASSERT_FALSE(outer->hasBracket());
    ASSERT_TRUE(outer->number());
    expectOuterClear();

    delete score;
}

// a second staff in the first part, for cross-staff notes
static void addStaffBelow(Score* score)
{
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        Staff* staff = Factory::createStaff(score->staff(0)->part());
        staff->setPart(score->staff(0)->part());
        score->undoInsertStaff(staff, 1);
    });
}

// moves to the staff below the chords from index `from` up to, but not including, index `to`
static void moveToStaffBelow(Measure* measure, size_t from, size_t to)
{
    Score* score = measure->score();
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        for (size_t i = from; i < to; ++i) {
            EditCrossStaff::moveDown(tx, score, chordRestAt(measure, i));
        }
    });
}

// writes an eighth note over the rest at `index` and joins its beam to the previous chord's
static void addEighthJoinedToBeam(Measure* measure, size_t index, int pitch)
{
    Score* score = measure->score();
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        score->setNoteRest(chordRestAt(measure, index)->segment(), 0, NoteVal(pitch), Fraction(1, 8));
    });
    score->select(chordRestAt(measure, index), SelectType::SINGLE);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        EditBeam::setBeamMode(tx, score, BeamMode::MID);
    });
}

//-----------------------------------------
//    nestedTupletsOuterBracket
//     with Automatic brackets, an outer tuplet that starts and ends its beam group
//     shows a bracket when a printed nested tuplet is on the same side
//-----------------------------------------

TEST_F(Engraving_TupletTests, nestedTupletsOuterBracket)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    // E4 F4 G4: stems up, all nine notes under one beam
    Measure* m1 = score->firstMeasure();
    Tuplet* outer = addNestedTriplets(m1, { 64, 65, 67 });
    ASSERT_TRUE(outer);
    ASSERT_TRUE(outer->isUp());
    ASSERT_TRUE(toChord(chordRestAt(m1, 0))->beam());
    ASSERT_EQ(toChord(chordRestAt(m1, 0))->beam(), toChord(chordRestAt(m1, 8))->beam());
    ASSERT_EQ(outer->elements().size(), 3u);
    Tuplet* middle = toTuplet(outer->elements().at(1));

    // all nested tuplets above, like the outer tuplet
    EXPECT_TRUE(outer->hasBracket());

    // all nested tuplets below: the beam shows the outer tuplet's span
    for (DurationElement* e : outer->elements()) {
        changeProperty(toTuplet(e), Pid::DIRECTION, PropertyValue::fromValue(DirectionV::DOWN));
    }
    EXPECT_FALSE(outer->hasBracket());

    // the middle one above again
    changeProperty(middle, Pid::DIRECTION, PropertyValue::fromValue(DirectionV::AUTO));
    ASSERT_TRUE(middle->isUp());
    EXPECT_TRUE(outer->hasBracket());

    // ... but invisible
    changeProperty(middle, Pid::VISIBLE, false);
    EXPECT_FALSE(outer->hasBracket());

    // ... visible, with neither a number nor a bracket
    changeProperty(middle, Pid::VISIBLE, true);
    changeProperty(middle, Pid::NUMBER_TYPE, int(TupletNumberType::NO_TEXT));
    changeProperty(middle, Pid::BRACKET_TYPE, int(TupletBracketType::SHOW_NO_BRACKET));
    EXPECT_FALSE(outer->hasBracket());

    // ... with a bracket only
    changeProperty(middle, Pid::BRACKET_TYPE, int(TupletBracketType::SHOW_BRACKET));
    EXPECT_TRUE(outer->hasBracket());

    delete score;
}

//-----------------------------------------
//    nestedTupletsOuterBracketCrossStaff
//     where a nested note is cross-staff, the nested tuplets
//     don't give the outer tuplet a bracket
//-----------------------------------------

TEST_F(Engraving_TupletTests, nestedTupletsOuterBracketCrossStaff)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    addStaffBelow(score);
    // D4 E4 F4: stems up, all nine notes under one beam
    Measure* m1 = score->firstMeasure();
    Tuplet* outer = addNestedTriplets(m1, { 62, 64, 65 });
    ASSERT_TRUE(outer);

    // the second and third nested triplets moved to the staff below, still above their notes
    moveToStaffBelow(m1, 3, 9);
    ASSERT_TRUE(outer->cross());
    ASSERT_EQ(toChord(chordRestAt(m1, 0))->beam(), toChord(chordRestAt(m1, 8))->beam());
    ASSERT_TRUE(outer->isUp());
    ASSERT_TRUE(toTuplet(outer->elements().at(2))->isUp());
    EXPECT_FALSE(outer->hasBracket());

    delete score;
}

//-----------------------------------------
//    nestedTupletsOuterDirection
//     an outer tuplet made only of nested tuplets
//     is placed on the stem side of their chords
//-----------------------------------------

TEST_F(Engraving_TupletTests, nestedTupletsOuterDirection)
{
    for (bool up : { false, true }) {
        MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
        ASSERT_TRUE(score);
        // D5 E5 F5 with stems down, or E4 F4 G4 with stems up
        Measure* m1 = score->firstMeasure();
        Tuplet* outer = addNestedTriplets(m1, up ? std::vector<int> { 64, 65, 67 } : std::vector<int> { 74, 76, 77 });
        ASSERT_TRUE(outer);

        for (size_t i = 0; i < 9; ++i) {
            ASSERT_EQ(toChord(chordRestAt(m1, i))->up(), up);
        }
        for (const DurationElement* e : outer->elements()) {
            EXPECT_EQ(toTuplet(e)->isUp(), up);
        }
        EXPECT_EQ(outer->isUp(), up);

        delete score;
    }
}

TEST_F(Engraving_TupletTests, nestedTupletsOuterDirectionCrossStaff)
{
    // the middle nested triplet moved to the staff below: the cross-staff beam sets the stems, so they don't
    // vote, and the outer tuplet stays above
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    addStaffBelow(score);
    Measure* m1 = score->firstMeasure();
    Tuplet* outer = addNestedTriplets(m1, { 62, 64, 65 });
    ASSERT_TRUE(outer);
    moveToStaffBelow(m1, 3, 6);
    ASSERT_EQ(chordRestAt(m1, 4)->staffMove(), 1);
    ASSERT_TRUE(outer->cross());

    EXPECT_TRUE(outer->isUp());

    delete score;
}

//-----------------------------------------
//    nestedTupletsOuterDirectionCrossStaffGroup
//     where any note of a nested group is cross-staff, a tuplet in it without cross-staff notes
//     doesn't take its side from its nested chords either
//-----------------------------------------

TEST_F(Engraving_TupletTests, nestedTupletsOuterDirectionCrossStaffGroup)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    addStaffBelow(score);
    // three levels: a half-note triplet whose first two notes are quarter triplets of eighth triplets, D4 E4 F4
    Measure* m1 = score->firstMeasure();
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        score->setNoteRest(m1->first(SegmentType::ChordRest), 0, NoteVal(62), Fraction(1, 1));
    });
    ASSERT_TRUE(addTuplet(chordRestAt(m1, 0), Fraction(3, 2), DurationType::V_HALF));
    for (size_t start : { size_t(0), size_t(9) }) {
        ASSERT_TRUE(addTuplet(chordRestAt(m1, start), Fraction(3, 2), DurationType::V_QUARTER));
        for (size_t i = start; i < start + 9; i += 3) {
            ASSERT_TRUE(addTuplet(chordRestAt(m1, i), Fraction(3, 2), DurationType::V_EIGHTH));
        }
    }
    const int pitches[] = { 62, 64, 65 };
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        for (size_t i = 0; i < 18; ++i) {
            score->setNoteRest(chordRestAt(m1, i)->segment(), 0, NoteVal(pitches[i % 3]), Fraction(1, 8));
        }
    });
    ASSERT_EQ(rhythm(m1), "3:2[ 3:2[ 3:2[8 8 8] 3:2[8 8 8] 3:2[8 8 8]] 3:2[ 3:2[8 8 8] 3:2[8 8 8] 3:2[8 8 8]] r2]");
    Tuplet* second = toTuplet(chordRestAt(m1, 0)->topTuplet()->elements().at(1));
    ASSERT_TRUE(second->isUp());

    // only the first note moved to the staff below: the cross-staff beam turns the second quarter triplet's
    // stems down, but it stays above
    moveToStaffBelow(m1, 0, 1);
    ASSERT_FALSE(second->cross());
    ASSERT_FALSE(toChord(chordRestAt(m1, 9))->up());
    EXPECT_TRUE(second->isUp());

    delete score;
}

//-----------------------------------------
//    nestedTupletsOuterDirectionCrossStaffBeam
//     where a nested group is under a cross-staff beam, the outer tuplet
//     doesn't take its side from its nested chords
//-----------------------------------------

TEST_F(Engraving_TupletTests, nestedTupletsOuterDirectionCrossStaffBeam)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    addStaffBelow(score);
    // D4 E4 F4: stems up
    Measure* m1 = score->firstMeasure();
    Tuplet* outer = addNestedTriplets(m1, { 62, 64, 65 });
    ASSERT_TRUE(outer);
    ASSERT_TRUE(outer->isUp());

    // a G4 beamed to the group and moved to the staff below: no note of the group is cross-staff, but the
    // cross-staff beam turns its stems down, and the outer tuplet stays above
    addEighthJoinedToBeam(m1, 9, 67);
    moveToStaffBelow(m1, 9, 10);
    ASSERT_FALSE(outer->cross());
    const Beam* beam = toChord(chordRestAt(m1, 0))->beam();
    ASSERT_TRUE(beam);
    ASSERT_EQ(beam, toChord(chordRestAt(m1, 9))->beam());
    ASSERT_TRUE(beam->cross());
    ASSERT_FALSE(toChord(chordRestAt(m1, 0))->up());
    EXPECT_TRUE(outer->isUp());

    delete score;
}
