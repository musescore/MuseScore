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

#include "engraving/dom/chord.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/input.h"
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
#include "engraving/editing/editexcerpt.h"
#include "engraving/editing/editmeasurerepeat.h"
#include "engraving/editing/edittie.h"
#include "engraving/editing/edittimesig.h"
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

// the rhythm of a track in a measure, e.g. "3:5[8~ 8 r8] 8. 4": durations as note values
// (r = rest, . = dot, ~ = tie, R = measure rest), tuplets as actual:normal[...]
static std::string rhythm(Measure* measure, track_idx_t track = 0)
{
    std::string result;
    std::vector<const Tuplet*> open;
    for (Segment* s = measure->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        const ChordRest* cr = toChordRest(s->element(track));
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
    const size_t start = result.find_first_not_of(' ');
    return start == std::string::npos ? std::string() : result.substr(start);
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

//! A measure of four quarters
static Measure* measureWithQuarters(MasterScore* score)
{
    InputState& is = score->inputState();
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    is.setNoteEntryMode(true);
    is.setNoteEntryMethod(NoteEntryMethod::BY_DURATION);
    is.setDuration(DurationType::V_QUARTER);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction& tx) {
        for (int i = 0; i < 4; ++i) {
            NoteVal nval(60);
            NoteInput::addPitch(tx, score, nval, false);
        }
    });
    return score->firstMeasure();
}

//! A tuplet that cannot be created is reported through the error, for the caller to show it
TEST_F(Engraving_TupletTests, addTuplet_RatioNotPossible_ReportsTheError)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = measureWithQuarters(score);
    ChordRest* quarter = toChordRest(m1->first(SegmentType::ChordRest)->element(0));
    MScore::setError(MsError::MS_NO_ERROR);

    //! [WHEN] A quarter is divided by 3, which gives no note value (1/12)...
    Tuplet* tuplet = nullptr;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        tuplet = score->addTuplet(quarter, Fraction(7, 3), TupletNumberType::SHOW_NUMBER, TupletBracketType::AUTO_BRACKET);
    });

    //! [THEN] ...there is no tuplet, and the error tells why
    EXPECT_FALSE(tuplet);
    EXPECT_EQ(MScore::_error, MsError::TUPLET_RATIO_NOT_POSSIBLE);
    EXPECT_EQ(rhythm(m1), "4 4 4 4");

    MScore::setError(MsError::MS_NO_ERROR);
    delete score;
}

TEST_F(Engraving_TupletTests, addTuplet_NoSpace_ReportsTheError)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = measureWithQuarters(score);
    ChordRest* lastQuarter = toChordRest(m1->last(SegmentType::ChordRest)->element(0));
    MScore::setError(MsError::MS_NO_ERROR);

    //! [WHEN] A half note triplet starts on the last quarter of the measure...
    Tuplet* tuplet = nullptr;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Tuplet tests"), [&](Transaction&) {
        tuplet = score->addTuplet(lastQuarter, Fraction(3, 2), TDuration(DurationType::V_QUARTER),
                                  TupletNumberType::SHOW_NUMBER, TupletBracketType::AUTO_BRACKET);
    });

    //! [THEN] ...there is no tuplet, and the error tells why
    EXPECT_FALSE(tuplet);
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
    EXPECT_EQ(rhythm(m1), "4 4 4 4");

    MScore::setError(MsError::MS_NO_ERROR);
    delete score;
}

//-----------------------------------------
//    pending tuplets
//     in By duration and Rhythm input, a tuplet waits for the
//     next duration, in an empty 4/4 score
//-----------------------------------------

static MasterScore* pendingTupletScore(NoteEntryMethod method = NoteEntryMethod::BY_DURATION)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    if (!score) {
        return nullptr;
    }
    InputState& is = score->inputState();
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    is.setDuration(DurationType::V_QUARTER);
    is.setNotes({ NoteVal(60) });
    is.setNoteEntryMode(true);
    is.setNoteEntryMethod(method);
    return score;
}

// like a triplet command in By duration or Rhythm input
static void setPendingTriplet(Score* score, bool durationAsSubdivision)
{
    InputState& is = score->inputState();
    PendingTuplet pending;
    pending.ratio = Fraction(3, 2);
    pending.numberType = TupletNumberType::SHOW_NUMBER;
    pending.bracketType = TupletBracketType::AUTO_BRACKET;
    pending.autoBaseLen = true;
    pending.durationAsSubdivision = durationAsSubdivision;
    pending.tick = is.tick();
    pending.track = is.track();
    pending.beyondScore = is.beyondScore();
    is.setPendingTuplet(pending);
}

static void pressDuration(Score* score, DurationType duration)
{
    // as after each command in the app; a refused duration leaves an error, which would roll back the next one
    MScore::setError(MsError::MS_NO_ERROR);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
        NoteInput::setDuration(tx, score, duration);
    });
}

// a click on the staff in note input, at line `line` of staff 0
static void clickNote(Score* score, const Fraction& tick, int line, bool beyondScore = false)
{
    // as after each command in the app
    MScore::setError(MsError::MS_NO_ERROR);
    Position pos;
    pos.segment = score->tick2segment(tick, false, SegmentType::ChordRest);
    pos.staffIdx = 0;
    pos.line = line;
    pos.beyondScore = beyondScore;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
        NoteInput::putNote(tx, score, pos, false);
    });
}

TEST_F(Engraving_TupletTests, pendingTupletNoteValue)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // triplet, eighth: an eighth triplet with the note first
        setPendingTriplet(score, true);
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 r8 r8] r4 r2");
        EXPECT_FALSE(is.pendingTuplet());
        EXPECT_EQ(is.tick(), Fraction(1, 12));
        EXPECT_EQ(is.duration().type(), DurationType::V_EIGHTH);

        pressDuration(score, DurationType::V_EIGHTH);
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 8 8] r4 r2");

        // undoing the first note takes the tuplet away too, and the tuplet waits again
        score->undoRedo(true, nullptr);
        score->undoRedo(true, nullptr);
        score->undoRedo(true, nullptr);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
        EXPECT_TRUE(is.pendingTupletHere());

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletUndoAfterEscape)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // after Escape, undoing the first note doesn't bring the tuplet back
        setPendingTriplet(score, true);
        for (int i = 0; i < 3; ++i) {
            pressDuration(score, DurationType::V_EIGHTH);
        }
        is.forgetPendingTuplets();
        for (int i = 0; i < 3; ++i) {
            score->undoRedo(true, nullptr);
        }
        EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
        EXPECT_FALSE(is.pendingTuplet());

        // a tuplet that waits after Escape comes back
        setPendingTriplet(score, true);
        pressDuration(score, DurationType::V_EIGHTH);
        score->undoRedo(true, nullptr);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
        EXPECT_TRUE(is.pendingTupletHere());

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletUndoAfterCancelAndRedo)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        setPendingTriplet(score, true);
        pressDuration(score, DurationType::V_EIGHTH);
        score->undoRedo(true, nullptr);
        ASSERT_TRUE(is.pendingTupletHere());

        // the next undo only cancels the wait
        is.setPendingTuplet(std::nullopt);

        // after a redo, undo brings the wait back
        score->undoRedo(false, nullptr);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 r8 r8] r4 r2");
        score->undoRedo(true, nullptr);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
        EXPECT_TRUE(is.pendingTupletHere());

        // but not after Escape
        is.forgetPendingTuplets();
        is.setPendingTuplet(std::nullopt);
        score->undoRedo(false, nullptr);
        score->undoRedo(true, nullptr);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
        EXPECT_FALSE(is.pendingTuplet());

        delete score;
    }
}

// like a tuplet command in By duration or Rhythm input, for any ratio
static void setPendingTuplet(Score* score, const Fraction& ratio, bool durationAsSubdivision)
{
    setPendingTriplet(score, durationAsSubdivision);
    InputState& is = score->inputState();
    PendingTuplet pending = *is.pendingTuplet();
    pending.ratio = ratio;
    is.setPendingTuplet(pending);
}

TEST_F(Engraving_TupletTests, pendingTupletRatios)
{
    struct Case {
        Fraction ratio;
        bool durationAsSubdivision;
        DurationType duration;
        int dots;
        std::string expected;
        DurationType inputDuration;
    };
    const std::vector<Case> cases {
        // triplet, half: three quarters in the time of a half
        { Fraction(3, 2), false, DurationType::V_HALF, 0, "3:2[4 r4 r4] r2", DurationType::V_QUARTER },
        // quintuplet, sixteenth: five sixteenths in the time of four
        { Fraction(5, 4), true, DurationType::V_16TH, 0, "5:4[16 r16 r16 r16 r16] r4 r2", DurationType::V_16TH },
        // quintuplet, quarter: a quarter divided into five
        { Fraction(5, 4), false, DurationType::V_QUARTER, 0, "5:4[16 r16 r16 r16 r16] r4 r2", DurationType::V_16TH },
        // duplet, eighth: two eighths in the time of three
        { Fraction(2, 3), true, DurationType::V_EIGHTH, 0, "2:3[8 r8] r8 r2", DurationType::V_EIGHTH },
        // duplet, dotted quarter: a dotted quarter divided into two
        { Fraction(2, 3), false, DurationType::V_QUARTER, 1, "2:3[8 r8] r8 r2", DurationType::V_EIGHTH },
    };
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        for (const Case& c : cases) {
            MasterScore* score = pendingTupletScore(method);
            ASSERT_TRUE(score);
            InputState& is = score->inputState();

            is.setDots(c.dots);
            setPendingTuplet(score, c.ratio, c.durationAsSubdivision);
            pressDuration(score, c.duration);
            EXPECT_TRUE(score->sanityCheck());
            EXPECT_EQ(rhythm(measureAt(score, 0)), c.expected);
            EXPECT_FALSE(is.pendingTuplet());
            EXPECT_EQ(is.duration().type(), c.inputDuration);
            EXPECT_EQ(is.duration().dots(), 0);

            delete score;
        }
    }
}

TEST_F(Engraving_TupletTests, pendingTupletRest)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        setPendingTriplet(score, true);
        is.setRest(true);
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[r8 r8 r8] r4 r2");
        EXPECT_FALSE(is.pendingTuplet());
        EXPECT_EQ(is.tick(), Fraction(1, 12));

        // notes after the rest go into the tuplet
        is.setRest(false);
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[r8 8 r8] r4 r2");

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletMixedDurations)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);

        // triplet, eighth, then a quarter fills the tuplet, and the next eighth comes after it
        setPendingTriplet(score, true);
        pressDuration(score, DurationType::V_EIGHTH);
        pressDuration(score, DurationType::V_QUARTER);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 4] r4 r2");
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 4] 8 r8 r2");

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletDoesNotFit)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // a quarter on beat 4, then a quarter triplet would cross the barline
        pressDuration(score, DurationType::V_HALF);
        pressDuration(score, DurationType::V_QUARTER);
        setPendingTriplet(score, true);
        pressDuration(score, DurationType::V_QUARTER);
        EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "2 4 r4");
        EXPECT_TRUE(is.pendingTupletHere());
        EXPECT_EQ(is.duration().type(), DurationType::V_QUARTER);

        // a rest neither
        is.setRest(true);
        pressDuration(score, DurationType::V_QUARTER);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "2 4 r4");
        EXPECT_TRUE(is.pendingTupletHere());
        is.setRest(false);

        // an eighth triplet fits
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "2 4 3:2[8 r8 r8]");

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletCannotDivide)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // 7:6 dividing a quarter: a quarter can't be divided into six, so nothing is entered and it keeps waiting
        setPendingTuplet(score, Fraction(7, 6), false);
        PendingTuplet pending = *is.pendingTuplet();
        pending.autoBaseLen = false;
        is.setPendingTuplet(pending);
        pressDuration(score, DurationType::V_QUARTER);
        EXPECT_EQ(MScore::_error, MsError::TUPLET_RATIO_NOT_POSSIBLE);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
        EXPECT_TRUE(is.pendingTupletHere());

        // the same for a click
        clickNote(score, Fraction(0, 1), 1);
        EXPECT_EQ(MScore::_error, MsError::TUPLET_RATIO_NOT_POSSIBLE);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
        EXPECT_TRUE(is.pendingTupletHere());

        // a dotted quarter can: seven sixteenths in the time of six
        is.setDots(1);
        pressDuration(score, DurationType::V_QUARTER);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "7:6[16 r16 r16 r16 r16 r16 r16] r8 r2");

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletPitchAlreadyInChord)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // a triplet waiting on the second of three eighths, whose pitch is the input pitch: the eighth is the
        // triplet's first note, and it's selected, so that the status bar describes it
        for (int i = 0; i < 3; ++i) {
            pressDuration(score, DurationType::V_EIGHTH);
        }
        is.setSegment(score->tick2segment(Fraction(1, 8), false, SegmentType::ChordRest));
        setPendingTriplet(score, true);
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "8 3:2[8 r8 r8] r8 r2");
        const ChordRest* first = chordRestAt(measureAt(score, 0), 1);
        ASSERT_TRUE(first && first->isChord() && first->tuplet());
        const EngravingItem* selected = score->selection().element();
        ASSERT_TRUE(selected && selected->isNote());
        EXPECT_EQ(toNote(selected)->chord(), first);
        EXPECT_EQ(is.tick(), Fraction(5, 24));
        // Alt+letter and T act on the triplet's first chord
        EXPECT_EQ(is.lastSegment(), first->segment());

        // a click with the pitch that's already there goes the same way
        is.setDuration(DurationType::V_EIGHTH);
        clickNote(score, Fraction(1, 1), 1);
        is.setSegment(score->tick2segment(Fraction(1, 1), false, SegmentType::ChordRest));
        setPendingTriplet(score, true);
        clickNote(score, Fraction(1, 1), 1);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 1)), "3:2[8 r8 r8] r4 r2");
        const ChordRest* clicked = chordRestAt(measureAt(score, 1), 0);
        ASSERT_TRUE(clicked && clicked->isChord() && clicked->tuplet());
        EXPECT_EQ(score->selection().element(), toChord(clicked)->upNote());

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletMovedAway)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // moving the input position cancels the tuplet: the duration enters a plain note
        setPendingTriplet(score, true);
        is.setSegment(score->tick2segment(Fraction(1, 1), false, SegmentType::ChordRest));
        EXPECT_FALSE(is.pendingTupletHere());
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_EQ(rhythm(measureAt(score, 1)), "8 r8 r4 r2");
        EXPECT_FALSE(is.pendingTuplet());

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletFollowsDottedPreviousNote)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // dotting the previous note moves the input position: the tuplet waits there instead
        pressDuration(score, DurationType::V_QUARTER);
        pressDuration(score, DurationType::V_QUARTER);
        setPendingTriplet(score, true);
        const EngravingItem* secondNote = toChord(chordRestAt(measureAt(score, 0), 1))->upNote();
        ASSERT_EQ(score->selection().element(), secondNote);
        score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
            NoteInput::toggleDots(tx, score, 1, true /*toggleForSelectionOnly*/);
        });
        EXPECT_EQ(rhythm(measureAt(score, 0)), "4 4. r8 r4");
        EXPECT_EQ(is.tick(), Fraction(5, 8));
        EXPECT_TRUE(is.pendingTupletHere());

        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "4 4. 3:2[8 r8 r8] r8");

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletBeyondScore)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // past the last measure, the tuplet goes in a new measure
        const int measures = score->nmeasures();
        is.setSegment(score->lastMeasure()->first(SegmentType::ChordRest));
        is.setBeyondScore(true);
        setPendingTriplet(score, true);
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_TRUE(score->sanityCheck());
        ASSERT_EQ(score->nmeasures(), measures + 1);
        EXPECT_EQ(rhythm(score->lastMeasure()), "3:2[8 r8 r8] r4 r2");

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletBeyondShortLastMeasure)
{
    MasterScore* score = pendingTupletScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    // a last measure of 1/4 in 4/4: the new measure is a whole 4/4 measure
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving tuplet tests"), [&](Transaction&) {
        score->lastMeasure()->adjustToLen(Fraction(1, 4));
    });
    ASSERT_EQ(score->lastMeasure()->ticks(), Fraction(1, 4));

    // a quarter-note triplet doesn't fit in the last measure, but fits in the new one
    const int measures = score->nmeasures();
    is.setSegment(score->lastMeasure()->first(SegmentType::ChordRest));
    is.setBeyondScore(true);
    setPendingTriplet(score, true);
    pressDuration(score, DurationType::V_QUARTER);
    EXPECT_TRUE(score->sanityCheck());
    ASSERT_EQ(score->nmeasures(), measures + 1);
    EXPECT_EQ(rhythm(score->lastMeasure()), "3:2[4 r4 r4] r2");

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletBeyondLocalTimeSig)
{
    MasterScore* score = pendingTupletScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    // a local 3/4 in the last measure: the new measure is 3/4 on this staff too
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving tuplet tests"), [&](Transaction& tx) {
        TimeSig* ts = Factory::createTimeSig(score->dummy());
        ts->setSig(Fraction(3, 4));
        EditTimeSig::addTimeSig(tx, score, score->lastMeasure(), 0, ts, true);
    });

    // a half-note triplet doesn't fit in 3/4: refused, no measure added, and the tuplet still waits
    const int measures = score->nmeasures();
    is.setSegment(score->lastMeasure()->first(SegmentType::ChordRest));
    is.setBeyondScore(true);
    setPendingTriplet(score, true);
    pressDuration(score, DurationType::V_HALF);
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(score->nmeasures(), measures);
    EXPECT_TRUE(is.pendingTupletHere());

    // a quarter-note triplet fits
    pressDuration(score, DurationType::V_QUARTER);
    EXPECT_TRUE(score->sanityCheck());
    ASSERT_EQ(score->nmeasures(), measures + 1);
    EXPECT_EQ(rhythm(score->lastMeasure()), "3:2[4 r4 r4] r4");

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNested)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // a quarter triplet on beats 1 and 2
        setPendingTriplet(score, true);
        for (int i = 0; i < 3; ++i) {
            pressDuration(score, DurationType::V_QUARTER);
        }
        ASSERT_EQ(rhythm(measureAt(score, 0)), "3:2[4 4 4] r2");

        // on its last quarter, another quarter triplet doesn't fit: nothing is entered, it keeps waiting
        is.setSegment(score->tick2segment(Fraction(1, 3), false, SegmentType::ChordRest));
        setPendingTriplet(score, true);
        pressDuration(score, DurationType::V_QUARTER);
        EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[4 4 4] r2");
        EXPECT_TRUE(is.pendingTupletHere());

        // on its second quarter, an eighth triplet replaces it
        is.setSegment(score->tick2segment(Fraction(1, 6), false, SegmentType::ChordRest));
        setPendingTriplet(score, true);
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[4 3:2[8 r8 r8] 4] r2");
        EXPECT_EQ(is.tick(), Fraction(2, 9));

        delete score;
    }
}

// quarters in voice 2, as after a switch to voice 1: the selection is in voice 2, and voice 1 has an
// input position on every beat
static void enterVoice2Quarters(Score* score)
{
    InputState& is = score->inputState();
    is.setTrack(1);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    for (int i = 0; i < 4; ++i) {
        pressDuration(score, DurationType::V_QUARTER);
    }
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
}

TEST_F(Engraving_TupletTests, pendingTupletOtherVoiceSelected)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // the tuplet and its first note go in the input voice
        enterVoice2Quarters(score);
        setPendingTriplet(score, true);
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 r8 r8] r4 r2");
        EXPECT_EQ(is.track(), 0);
        EXPECT_EQ(is.tick(), Fraction(1, 12));

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletInsideLongerChordRest)
{
    // the tuplet waits on beat 2, inside a half note or rest: it shortens it, as a plain note does
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        for (bool rest : { false, true }) {
            for (DurationType duration : { DurationType::V_EIGHTH, DurationType::V_QUARTER }) {
                MasterScore* score = pendingTupletScore(method);
                ASSERT_TRUE(score);
                InputState& is = score->inputState();

                is.setRest(rest);
                pressDuration(score, DurationType::V_HALF);
                is.setRest(false);
                enterVoice2Quarters(score);
                is.setSegment(score->tick2segment(Fraction(1, 4), false, SegmentType::ChordRest));
                ASSERT_FALSE(is.cr());

                setPendingTriplet(score, true);
                pressDuration(score, duration);
                EXPECT_TRUE(score->sanityCheck());
                const std::string first = rest ? "r4" : "4";
                if (duration == DurationType::V_EIGHTH) {
                    EXPECT_EQ(rhythm(measureAt(score, 0)), first + " 3:2[8 r8 r8] r2");
                    EXPECT_EQ(is.tick(), Fraction(1, 3));
                } else {
                    EXPECT_EQ(rhythm(measureAt(score, 0)), first + " 3:2[4 r4 r4] r4");
                    EXPECT_EQ(is.tick(), Fraction(5, 12));
                }
                EXPECT_FALSE(is.pendingTuplet());
                EXPECT_EQ(is.track(), 0);

                delete score;
            }
        }
    }
}

TEST_F(Engraving_TupletTests, pendingTupletInsideLongerChordRestInTuplet)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // a quarter triplet on beats 1 and 2
        setPendingTriplet(score, true);
        for (int i = 0; i < 3; ++i) {
            pressDuration(score, DurationType::V_QUARTER);
        }
        enterVoice2Quarters(score);
        ASSERT_EQ(rhythm(measureAt(score, 0)), "3:2[4 4 4] r2");

        // the tuplet waits on beat 2, inside the second quarter: it fits up to the end of the quarter triplet
        is.setSegment(score->tick2segment(Fraction(1, 4), false, SegmentType::ChordRest));
        ASSERT_FALSE(is.cr());
        setPendingTriplet(score, true);
        pressDuration(score, DurationType::V_QUARTER);
        EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[4 4 4] r2");
        EXPECT_TRUE(is.pendingTupletHere());
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[4 8 3:2[8 r8 r8] 8] r2");
        EXPECT_FALSE(is.pendingTuplet());

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletDrums)
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"tupletDrums.mscx");
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    is.setDuration(DurationType::V_QUARTER);
    is.setNoteEntryMode(true);
    is.setNoteEntryMethod(NoteEntryMethod::BY_DURATION);

    // a snare (voice 1) takes the tuplet
    is.setNotes({ NoteVal(38) });
    setPendingTriplet(score, true);
    pressDuration(score, DurationType::V_EIGHTH);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 r8 r8] r4 r2");

    // a bass drum (voice 2) takes it to its own voice: no empty tuplet in voice 1
    auto crAt = [score](const Fraction& tick, track_idx_t track) -> const ChordRest* {
        const Segment* s = score->tick2segment(tick, false, SegmentType::ChordRest);
        return s ? toChordRest(s->element(track)) : nullptr;
    };
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(1, 2), false, SegmentType::ChordRest));
    is.setNotes({ NoteVal(36) });
    setPendingTriplet(score, true);
    pressDuration(score, DurationType::V_EIGHTH);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_FALSE(is.pendingTuplet());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 r8 r8] r4 r2");
    const ChordRest* bassDrum = crAt(Fraction(1, 2), 1);
    ASSERT_TRUE(bassDrum && bassDrum->isChord() && bassDrum->tuplet());
    EXPECT_EQ(bassDrum->tuplet()->ratio(), Fraction(3, 2));
    EXPECT_EQ(toChord(bassDrum)->upNote()->pitch(), 36);
    EXPECT_EQ(is.track(), 1);
    EXPECT_EQ(is.tick(), Fraction(7, 12));

    // where the tuplet doesn't fit, a snare is refused, and so is a bass drum in its own voice: it keeps waiting there
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(1, 2), false, SegmentType::ChordRest));
    setPendingTriplet(score, true);
    is.setNotes({ NoteVal(38) });
    pressDuration(score, DurationType::V_HALF);
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 r8 r8] r4 r2");
    EXPECT_TRUE(is.pendingTupletHere());
    is.setNotes({ NoteVal(36) });
    pressDuration(score, DurationType::V_HALF);
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
    EXPECT_TRUE(is.pendingTupletHere());
    EXPECT_EQ(is.track(), 1);

    // and back: a snare takes it to voice 1
    is.setNotes({ NoteVal(38) });
    pressDuration(score, DurationType::V_EIGHTH);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_FALSE(is.pendingTuplet());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 r8 r8] r4 3:2[8 r8 r8] r4");
    EXPECT_EQ(is.track(), 0);

    // picking a drum while the tuplet waits, as in By duration input: the tuplet goes with it, at the same tick
    is.setSegment(score->tick2segment(Fraction(1, 1), false, SegmentType::ChordRest));
    setPendingTriplet(score, true);
    is.moveToDrumVoice(1);
    EXPECT_TRUE(is.pendingTupletHere());
    EXPECT_EQ(is.track(), 1);
    EXPECT_EQ(is.tick(), Fraction(1, 1));
    is.moveToDrumVoice(0);
    EXPECT_TRUE(is.pendingTupletHere());
    EXPECT_EQ(is.track(), 0);

    // the voice a tuplet command starts the wait in: the next drum's, if it's another voice
    is.setNotes({ NoteVal(36) });
    EXPECT_EQ(is.otherDrumVoice(), std::optional<voice_idx_t>(1));
    is.setNotes({ NoteVal(38) });
    EXPECT_FALSE(is.otherDrumVoice());

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletDrumPad)
{
    // in Rhythm input, a drum pad key enters a drum: a bass drum takes the tuplet to voice 2
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"tupletDrums.mscx");
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    is.setDuration(DurationType::V_QUARTER);
    is.setNoteEntryMode(true);
    is.setNoteEntryMethod(NoteEntryMethod::RHYTHM);

    setPendingTriplet(score, true);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
        NoteInput::addPitch(tx, score, NoteInputParams { 0, 36 }, false, false);
    });
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_FALSE(is.pendingTuplet());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
    const Segment* s = score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest);
    ASSERT_TRUE(s && s->element(1) && s->element(1)->isChord());
    const Chord* bassDrum = toChord(s->element(1));
    ASSERT_TRUE(bassDrum->tuplet());
    EXPECT_EQ(bassDrum->tuplet()->ratio(), Fraction(3, 2));
    EXPECT_EQ(bassDrum->actualTicks(), Fraction(1, 6));
    EXPECT_EQ(bassDrum->upNote()->pitch(), 36);
    EXPECT_EQ(is.track(), 1);
    EXPECT_EQ(is.tick(), Fraction(1, 6));

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletRhythmSelectedDrum)
{
    // in Rhythm input, the selected note's drum is entered, in its own voice; the tuplet goes with it there
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"tupletDrums.mscx");
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setDuration(DurationType::V_QUARTER);
    is.setNoteEntryMode(true);
    is.setNoteEntryMethod(NoteEntryMethod::RHYTHM);

    auto crAt = [score](const Fraction& tick, track_idx_t track) -> const ChordRest* {
        const Segment* s = score->tick2segment(tick, false, SegmentType::ChordRest);
        return s ? toChordRest(s->element(track)) : nullptr;
    };

    // a bass drum (voice 2) entered at `tick`, then changed to a snare (voice 1) and selected; the bass drum is still the input note
    auto selectSnareInVoice2 = [&](const Fraction& tick) {
        is.setTrack(1);
        is.setSegment(score->tick2segment(tick, false, SegmentType::ChordRest));
        is.setDuration(DurationType::V_QUARTER);
        is.setNotes({ NoteVal(36) });
        score->deselectAll();
        pressDuration(score, DurationType::V_QUARTER);
        const ChordRest* cr = crAt(tick, 1);
        ASSERT_TRUE(cr && cr->isChord());
        Note* note = toChord(cr)->upNote();
        score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction&) {
            note->undoChangeProperty(Pid::PITCH, 38);
        });
        score->select(note, SelectType::SINGLE, 0);
        is.setTrack(1);
        is.setSegment(score->tick2segment(tick, false, SegmentType::ChordRest));
        ASSERT_EQ(is.notes().front().pitch, 36);
    };

    // with no tuplet waiting, the snare goes into voice 1
    ASSERT_NO_FATAL_FAILURE(selectSnareInVoice2(Fraction(0, 1)));
    pressDuration(score, DurationType::V_EIGHTH);
    EXPECT_TRUE(score->sanityCheck());
    const ChordRest* control = crAt(Fraction(0, 1), 0);
    ASSERT_TRUE(control && control->isChord());
    EXPECT_EQ(toChord(control)->upNote()->pitch(), 38);

    // a tuplet command waits in the snare's voice, as it takes the selected note as the next one
    ASSERT_NO_FATAL_FAILURE(selectSnareInVoice2(Fraction(1, 1)));
    NoteInput::useSelectedNoteForRhythm(score);
    EXPECT_EQ(is.otherDrumVoice(), std::optional<voice_idx_t>(0));

    // with a tuplet waiting, the snare takes it to voice 1 too, also from another voice with the bass drum as the input note
    is.setNotes({ NoteVal(36) });
    setPendingTriplet(score, true);
    pressDuration(score, DurationType::V_EIGHTH);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_FALSE(is.pendingTuplet());
    const ChordRest* snare = crAt(Fraction(1, 1), 0);
    ASSERT_TRUE(snare && snare->isChord());
    EXPECT_EQ(toChord(snare)->upNote()->pitch(), 38);
    ASSERT_TRUE(snare->tuplet());
    EXPECT_EQ(snare->tuplet()->ratio(), Fraction(3, 2));
    const ChordRest* voice2 = crAt(Fraction(1, 1), 1);
    EXPECT_TRUE(voice2 && !voice2->tuplet());

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletDrumVoiceInsideTuplet)
{
    // the drum's voice has a tuplet around the input position: the waiting tuplet goes to that tuplet's start,
    // as a voice change does, whether the drum is picked first or comes with the duration
    for (bool pickDrumFirst : { true, false }) {
        SCOPED_TRACE(pickDrumFirst ? "drum picked first" : "drum with the duration");
        MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"tupletDrums.mscx");
        ASSERT_TRUE(score);
        InputState& is = score->inputState();
        is.setTrack(0);
        is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
        is.setNoteEntryMode(true);
        is.setNoteEntryMethod(NoteEntryMethod::BY_DURATION);
        auto crAt = [score](const Fraction& tick, track_idx_t track) -> const ChordRest* {
            const Segment* s = score->tick2segment(tick, false, SegmentType::ChordRest);
            return s ? toChordRest(s->element(track)) : nullptr;
        };

        // a quarter triplet of bass drums in voice 2, then a snare eighth in voice 1
        is.setNotes({ NoteVal(36) });
        setPendingTriplet(score, true);
        for (int i = 0; i < 3; ++i) {
            pressDuration(score, DurationType::V_QUARTER);
        }
        is.setTrack(0);
        is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
        is.setNotes({ NoteVal(38) });
        pressDuration(score, DurationType::V_EIGHTH);
        ASSERT_EQ(is.tick(), Fraction(1, 8));

        // a triplet waits at beat 1.5 in voice 1; a bass drum takes it to the voice 2 triplet's start
        setPendingTriplet(score, true);
        is.setNotes({ NoteVal(36) });
        if (pickDrumFirst) {
            is.moveToDrumVoice(1);
            EXPECT_TRUE(is.pendingTupletHere());
            EXPECT_EQ(is.track(), 1);
            EXPECT_EQ(is.tick(), Fraction(0, 1));
        }
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_FALSE(is.pendingTuplet());

        // nested in the first note; the other two notes stay as they were
        const ChordRest* first = crAt(Fraction(0, 1), 1);
        ASSERT_TRUE(first && first->isChord() && first->tuplet());
        EXPECT_EQ(first->tuplet()->ratio(), Fraction(3, 2));
        EXPECT_TRUE(first->tuplet()->tuplet());
        for (const Fraction& tick : { Fraction(1, 6), Fraction(1, 3) }) {
            const ChordRest* cr = crAt(tick, 1);
            ASSERT_TRUE(cr && cr->isChord() && cr->tuplet());
            EXPECT_EQ(cr->durationType(), TDuration(DurationType::V_QUARTER));
            EXPECT_FALSE(cr->tuplet()->tuplet());
        }

        delete score;
    }
}

//-----------------------------------------
//    pending tuplets taken up by a note entered with its pitch
//     a click, a TAB fret or a drum pad at the input position
//     where the tuplet waits: the input duration sizes it
//-----------------------------------------

TEST_F(Engraving_TupletTests, pendingTupletTakenUpByNote)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // a click where the tuplet waits: a quarter triplet, with the clicked E5 first
        setPendingTriplet(score, true);
        clickNote(score, Fraction(0, 1), 1);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[4 r4 r4] r2");
        EXPECT_EQ(toChord(chordRestAt(measureAt(score, 0), 0))->upNote()->pitch(), 76);
        EXPECT_FALSE(is.pendingTuplet());
        EXPECT_EQ(is.tick(), Fraction(1, 6));

        // a click elsewhere enters a plain note: the tuplet no longer waits at the input position
        is.setSegment(score->tick2segment(Fraction(1, 2), false, SegmentType::ChordRest));
        setPendingTriplet(score, true);
        clickNote(score, Fraction(1, 1), 1);
        EXPECT_EQ(rhythm(measureAt(score, 1)), "4 r4 r2");
        EXPECT_FALSE(is.pendingTupletHere());

        // where it doesn't fit, nothing is entered and it keeps waiting
        is.setSegment(score->tick2segment(Fraction(3, 2), false, SegmentType::ChordRest));
        setPendingTriplet(score, true);
        is.setDuration(DurationType::V_HALF);
        clickNote(score, Fraction(3, 2), 1);
        EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
        EXPECT_EQ(rhythm(measureAt(score, 1)), "4 r4 r2");
        EXPECT_TRUE(is.pendingTupletHere());

        // a note name key in Rhythm input goes the same way
        is.setDuration(DurationType::V_EIGHTH);
        score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
            NoteInput::addPitch(tx, score, NoteInputParams { 35 }, false, false);
        });
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 1)), "4 r4 3:2[8 r8 r8] r4");
        EXPECT_FALSE(is.pendingTuplet());

        // an inserted note doesn't take it up: the inserted time is the input duration
        is.setSegment(score->tick2segment(Fraction(5, 4), false, SegmentType::ChordRest));
        setPendingTriplet(score, true);
        score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
            NoteInput::addPitch(tx, score, NoteInputParams { 35 }, false, true);
        });
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 1)), "4 8 r4 3:2[8 r8 r8] r4");
        EXPECT_FALSE(is.pendingTupletHere());

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletTakenUpByNoteBeyondScore)
{
    MasterScore* score = pendingTupletScore(NoteEntryMethod::RHYTHM);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    const int measures = score->nmeasures();
    is.setSegment(score->lastMeasure()->first(SegmentType::ChordRest));
    is.setBeyondScore(true);

    // a whole note triplet doesn't fit in a 4/4 measure: no measure is added
    setPendingTriplet(score, true);
    is.setDuration(DurationType::V_WHOLE);
    clickNote(score, is.tick(), 1, true);
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
    EXPECT_EQ(score->nmeasures(), measures);
    EXPECT_TRUE(is.pendingTupletHere());

    // an eighth triplet goes in one new measure
    is.setDuration(DurationType::V_EIGHTH);
    clickNote(score, is.tick(), 1, true);
    EXPECT_TRUE(score->sanityCheck());
    ASSERT_EQ(score->nmeasures(), measures + 1);
    EXPECT_EQ(rhythm(score->lastMeasure()), "3:2[8 r8 r8] r4 r2");
    EXPECT_FALSE(is.pendingTuplet());

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletMeasureRepeat)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // 1-measure repeats in measures 2 and 3
        Measure* m2 = measureAt(score, 1);
        Measure* m3 = measureAt(score, 2);
        score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
            EditMeasureRepeat::addMeasureRepeat(tx, score, m2, 1, 0);
            EditMeasureRepeat::addMeasureRepeat(tx, score, m3, 1, 0);
        });
        ASSERT_TRUE(m2->isMeasureRepeatGroup(0));
        ASSERT_TRUE(m3->isMeasureRepeatGroup(0));

        // a triplet that fills the measure removes the repeat first, as a plain note does
        is.setSegment(m2->first(SegmentType::ChordRest));
        setPendingTriplet(score, true);
        pressDuration(score, DurationType::V_HALF);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_FALSE(m2->isMeasureRepeatGroup(0));
        EXPECT_EQ(rhythm(m2), "3:2[2 r2 r2]");

        // undo brings back the repeat and the waiting tuplet
        score->undoRedo(true, nullptr);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_TRUE(m2->isMeasureRepeatGroup(0));
        EXPECT_TRUE(is.pendingTupletHere());

        // a shorter one too
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_FALSE(m2->isMeasureRepeatGroup(0));
        EXPECT_EQ(rhythm(m2), "3:2[8 r8 r8] r4 r2");

        // one that doesn't fit the measure leaves the repeat and keeps waiting
        is.setSegment(m3->first(SegmentType::ChordRest));
        setPendingTriplet(score, true);
        pressDuration(score, DurationType::V_WHOLE);
        EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
        EXPECT_TRUE(m3->isMeasureRepeatGroup(0));
        EXPECT_TRUE(is.pendingTupletHere());

        // a click takes it up the same way
        is.setDuration(DurationType::V_EIGHTH);
        clickNote(score, m3->tick(), 1);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_FALSE(m3->isMeasureRepeatGroup(0));
        EXPECT_EQ(rhythm(m3), "3:2[8 r8 r8] r4 r2");

        delete score;
    }
}

// staff 0 as a guitar TAB staff, in Rhythm input
static MasterScore* pendingTupletTabScore()
{
    MasterScore* score = pendingTupletScore(NoteEntryMethod::RHYTHM);
    if (!score) {
        return nullptr;
    }
    Staff* staff = score->staff(0);
    staff->setStaffType(Fraction(0, 1), *StaffType::preset(StaffTypes::TAB_6COMMON));
    int strings[] = { 40, 45, 50, 55, 59, 64 };
    staff->part()->instrument()->setStringData(StringData(24, 6, strings));
    return score;
}

static void addFret(Score* score, int fret)
{
    score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
        NoteInput::addFret(tx, score, fret);
    });
}

TEST_F(Engraving_TupletTests, pendingTupletTab)
{
    MasterScore* score = pendingTupletTabScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setString(2);

    // the first digit makes a quarter triplet; the input position stays on it, for a second digit
    setPendingTriplet(score, true);
    addFret(score, 1);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[4 r4 r4] r2");
    EXPECT_FALSE(is.pendingTuplet());
    EXPECT_EQ(is.tick(), Fraction(0, 1));
    const Chord* chord = toChord(chordRestAt(measureAt(score, 0), 0));
    ASSERT_EQ(chord->notes().size(), 1u);
    EXPECT_EQ(chord->upNote()->fret(), 1);

    // a second digit makes fret 12, another string joins the chord
    addFret(score, 2);
    EXPECT_EQ(chord->upNote()->fret(), 12);
    is.setString(4);
    addFret(score, 3);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[4 r4 r4] r2");
    EXPECT_EQ(chord->notes().size(), 2u);

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletTabSameString)
{
    // a fret on the string of the note where the tuplet waits changes that note, as with no tuplet waiting
    for (int string = 0; string < 6; ++string) {
        SCOPED_TRACE("string " + std::to_string(string));
        MasterScore* score = pendingTupletTabScore();
        ASSERT_TRUE(score);
        InputState& is = score->inputState();
        is.setString(string);

        addFret(score, 3);
        setPendingTriplet(score, true);
        addFret(score, 2);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[4 r4 r4] r2");
        const Chord* chord = toChord(chordRestAt(measureAt(score, 0), 0));
        ASSERT_EQ(chord->notes().size(), 1u);
        EXPECT_EQ(chord->upNote()->fret(), 2);
        EXPECT_EQ(chord->upNote()->string(), string);
        // selected, so that the status bar describes it
        EXPECT_EQ(score->selection().element(), chord->upNote());

        delete score;
    }
}

//-----------------------------------------
//    pending tuplets with dotted durations
//     single and double dots, before and after the
//     tuplet command
//-----------------------------------------

static void pressDot(Score* score, int dots)
{
    score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
        NoteInput::toggleDots(tx, score, dots);
    });
}

TEST_F(Engraving_TupletTests, pendingTupletDots)
{
    // a dotted input duration stays dotted, as without a tuplet: a dotted eighth triplet, of notes or rests
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        for (int dots : { 1, 2 }) {
            for (bool rest : { false, true }) {
                MasterScore* score = pendingTupletScore(method);
                ASSERT_TRUE(score);
                InputState& is = score->inputState();

                is.setDots(dots);
                is.setRest(rest);
                setPendingTriplet(score, true);
                pressDuration(score, DurationType::V_EIGHTH);
                EXPECT_TRUE(score->sanityCheck());
                const std::string first = rest ? "r8" : "8";
                EXPECT_EQ(rhythm(measureAt(score, 0)),
                          dots == 1 ? "3:2[" + first + ". r8. r8.] r8 r2" : "3:2[" + first + ".. r8.. r8..] r16 r2");
                EXPECT_FALSE(is.pendingTuplet());
                EXPECT_EQ(is.duration().dots(), dots);

                delete score;
            }
        }
    }
}

TEST_F(Engraving_TupletTests, pendingTupletDotWhileWaiting)
{
    // a dot pressed while the tuplet waits: the same as before the tuplet command;
    // pressed again it goes, and the other dot replaces it
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        for (int dots : { 1, 2 }) {
            MasterScore* score = pendingTupletScore(method);
            ASSERT_TRUE(score);
            InputState& is = score->inputState();

            setPendingTriplet(score, true);
            pressDot(score, dots);
            EXPECT_TRUE(is.pendingTupletHere());
            EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
            EXPECT_EQ(is.duration().dots(), dots);
            pressDot(score, dots);
            EXPECT_EQ(is.duration().dots(), 0);
            pressDot(score, 3 - dots);
            pressDot(score, dots);
            EXPECT_TRUE(is.pendingTupletHere());
            EXPECT_EQ(is.duration().dots(), dots);

            pressDuration(score, DurationType::V_EIGHTH);
            EXPECT_TRUE(score->sanityCheck());
            EXPECT_EQ(rhythm(measureAt(score, 0)), dots == 1 ? "3:2[8. r8. r8.] r8 r2" : "3:2[8.. r8.. r8..] r16 r2");

            delete score;
        }
    }
}

TEST_F(Engraving_TupletTests, pendingTupletDottedDivide)
{
    // a dotted length is divided like a selected dotted note: plain notes where they fit
    // triplet, dotted half: three quarters (3:3); triplet, double-dotted half: three eighths in the time of 7
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        for (int dots : { 1, 2 }) {
            MasterScore* score = pendingTupletScore(method);
            ASSERT_TRUE(score);
            InputState& is = score->inputState();

            is.setDots(dots);
            setPendingTriplet(score, false);
            pressDuration(score, DurationType::V_HALF);
            EXPECT_TRUE(score->sanityCheck());
            EXPECT_EQ(rhythm(measureAt(score, 0)), dots == 1 ? "3:3[4 r4 r4] r4" : "3:7[8 r8 r8] r8");
            EXPECT_FALSE(is.pendingTuplet());
            EXPECT_EQ(is.duration().type(), dots == 1 ? DurationType::V_QUARTER : DurationType::V_EIGHTH);
            EXPECT_EQ(is.duration().dots(), 0);

            delete score;
        }
    }
}

TEST_F(Engraving_TupletTests, pendingTupletDottedDoesNotFit)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // a half note, then a dotted quarter triplet would cross the barline: nothing is entered
        pressDuration(score, DurationType::V_HALF);
        setPendingTriplet(score, true);
        is.setDots(1);
        pressDuration(score, DurationType::V_QUARTER);
        EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "2 r2");
        EXPECT_TRUE(is.pendingTupletHere());

        // a double-dotted eighth triplet fits
        is.setDots(2);
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "2 3:2[8.. r8.. r8..] r16");

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletDottedUndo)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        for (int dots : { 1, 2 }) {
            MasterScore* score = pendingTupletScore(method);
            ASSERT_TRUE(score);
            InputState& is = score->inputState();
            const std::string dotted = dots == 1 ? "8." : "8..";
            const std::string after = dots == 1 ? " r8 r2" : " r16 r2";

            is.setDots(dots);
            setPendingTriplet(score, true);
            pressDuration(score, DurationType::V_EIGHTH);
            pressDuration(score, DurationType::V_EIGHTH);
            pressDuration(score, DurationType::V_EIGHTH);
            EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[" + dotted + " " + dotted + " " + dotted + "]" + after);

            // undoing the first note takes the tuplet away too, and the dotted tuplet waits again
            score->undoRedo(true, nullptr);
            score->undoRedo(true, nullptr);
            score->undoRedo(true, nullptr);
            EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
            EXPECT_TRUE(is.pendingTupletHere());
            EXPECT_EQ(is.duration().dots(), dots);
            pressDuration(score, DurationType::V_EIGHTH);
            EXPECT_TRUE(score->sanityCheck());
            EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[" + dotted + " r" + dotted + " r" + dotted + "]" + after);

            delete score;
        }
    }
}

//-----------------------------------------
//    tupletPreview
//     the tuplet that the next note or rest creates, shown while
//     note input waits
//-----------------------------------------

TEST_F(Engraving_TupletTests, tupletPreviewPending)
{
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        // nothing waits
        EXPECT_FALSE(NoteInput::tupletPreview(score));

        // a triplet waits for the next duration
        setPendingTriplet(score, true);
        std::optional<TupletPreview> preview = NoteInput::tupletPreview(score);
        ASSERT_TRUE(preview);
        EXPECT_EQ(preview->levels.back().ratio, Fraction(3, 2));
        EXPECT_EQ(preview->levels.back().numberType, TupletNumberType::SHOW_NUMBER);

        // the next duration adds it
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_FALSE(NoteInput::tupletPreview(score));

        // after the triplet, with the number type it was asked for
        pressDuration(score, DurationType::V_EIGHTH);
        pressDuration(score, DurationType::V_EIGHTH);
        for (TupletNumberType numberType : { TupletNumberType::SHOW_RELATION, TupletNumberType::NO_TEXT }) {
            setPendingTriplet(score, true);
            PendingTuplet pending = *is.pendingTuplet();
            pending.numberType = numberType;
            is.setPendingTuplet(pending);
            preview = NoteInput::tupletPreview(score);
            ASSERT_TRUE(preview);
            EXPECT_EQ(preview->levels.back().numberType, numberType);
        }

        // moving away cancels it
        is.setSegment(score->tick2segment(Fraction(1, 2), false, SegmentType::ChordRest));
        EXPECT_FALSE(NoteInput::tupletPreview(score));

        // divided, the ratio follows the input duration: a dotted half gives 3:3
        TDuration dottedHalf(DurationType::V_HALF);
        dottedHalf.setDots(1);
        is.setDuration(dottedHalf);
        setPendingTriplet(score, false);
        preview = NoteInput::tupletPreview(score);
        ASSERT_TRUE(preview);
        EXPECT_EQ(preview->levels.back().ratio, Fraction(3, 3));

        // a duration it can't divide still shows its number: 7:6 dividing a quarter
        is.setDuration(DurationType::V_QUARTER);
        setPendingTuplet(score, Fraction(7, 6), false);
        PendingTuplet pending = *is.pendingTuplet();
        pending.autoBaseLen = false;
        is.setPendingTuplet(pending);
        preview = NoteInput::tupletPreview(score);
        ASSERT_TRUE(preview);
        EXPECT_EQ(preview->levels.back().ratio, Fraction(7, 6));

        delete score;
    }
}

TEST_F(Engraving_TupletTests, tupletPreviewPendingBeyondScore)
{
    MasterScore* score = pendingTupletScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    // past the last measure, where the next duration appends a measure
    is.setSegment(score->lastMeasure()->first(SegmentType::ChordRest));
    is.setBeyondScore(true);
    setPendingTriplet(score, true);
    std::optional<TupletPreview> preview = NoteInput::tupletPreview(score);
    ASSERT_TRUE(preview);
    EXPECT_EQ(preview->levels.back().ratio, Fraction(3, 2));

    delete score;
}

//-----------------------------------------
//    pending tuplets in By note name input
//     the tuplet waits, sized by the input duration, and
//     the next note or rest adds it, in an empty 4/4 score
//-----------------------------------------

static void enterNote(Score* score, int pitch = 60, bool addToChord = false)
{
    score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
        NoteVal nval(pitch);
        NoteInput::addPitch(tx, score, nval, addToChord);
    });
}

// a letter in note input; with `insert`, Ctrl+Shift+letter (insert) in By note name input
static void enterLetter(Score* score, bool insert = false)
{
    score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
        NoteInput::addPitch(tx, score, NoteInputParams { 0 }, false, insert);
    });
}

static void enterRest(Score* score)
{
    score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction&) {
        score->enterRest(score->inputState().duration());
    });
}

// like R (repeat) in note input: the chord again at the input position, as a new chord
static void repeatChord(Score* score, const Chord* chord)
{
    std::vector<NoteVal> notes;
    for (const Note* note : chord->notes()) {
        notes.push_back(note->noteVal());
    }
    score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
        for (size_t i = 0; i < notes.size(); ++i) {
            NoteVal nval = notes[i];
            if (!NoteInput::addPitch(tx, score, nval, i > 0) && i == 0) {
                break;
            }
        }
    });
}

// the drum set in By note name input, at the start of the score
static MasterScore* drumScore()
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"tupletDrums.mscx");
    if (!score) {
        return nullptr;
    }
    InputState& is = score->inputState();
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    is.setNoteEntryMode(true);
    is.setNoteEntryMethod(NoteEntryMethod::BY_NOTE_NAME);
    return score;
}

// a drum by its shortcut: 38 (snare) goes in voice 1, 36 (bass drum) in voice 2
static void enterDrum(Score* score, int drumPitch)
{
    score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
        NoteInput::addPitch(tx, score, NoteInputParams { 0, drumPitch }, false, false);
    });
}

// like a tuplet command in By note name input
static bool waitForTuplet(Score* score, bool durationAsSubdivision, const Fraction& ratio = Fraction(3, 2))
{
    PendingTuplet pending;
    pending.ratio = ratio;
    pending.numberType = TupletNumberType::SHOW_NUMBER;
    pending.bracketType = TupletBracketType::AUTO_BRACKET;
    pending.autoBaseLen = true;
    pending.durationAsSubdivision = durationAsSubdivision;
    return NoteInput::setPendingTuplet(score, pending);
}

TEST_F(Engraving_TupletTests, pendingTupletNoteName)
{
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    // eighth, triplet: nothing is added until the next note
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
    EXPECT_TRUE(is.pendingTupletHere());
    EXPECT_EQ(is.duration().type(), DurationType::V_EIGHTH);

    enterNote(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 r8 r8] r4 r2");
    EXPECT_FALSE(is.pendingTuplet());
    EXPECT_EQ(is.tick(), Fraction(1, 12));
    enterNote(score);
    enterNote(score);
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 8 8] r4 r2");

    // undoing the first note takes the tuplet away too, and the tuplet waits again
    for (int i = 0; i < 3; ++i) {
        score->undoRedo(true, nullptr);
    }
    EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
    EXPECT_TRUE(is.pendingTupletHere());

    // after Escape, it doesn't
    for (int i = 0; i < 3; ++i) {
        enterNote(score);
    }
    is.forgetPendingTuplets();
    for (int i = 0; i < 3; ++i) {
        score->undoRedo(true, nullptr);
    }
    EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
    EXPECT_FALSE(is.pendingTuplet());

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameCannotDivide)
{
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    // 7:6 dividing a quarter: a quarter can't be divided into six, so no tuplet waits
    is.setDuration(DurationType::V_QUARTER);
    PendingTuplet pending;
    pending.ratio = Fraction(7, 6);
    EXPECT_FALSE(NoteInput::setPendingTuplet(score, pending));
    EXPECT_EQ(MScore::_error, MsError::TUPLET_RATIO_NOT_POSSIBLE);
    EXPECT_FALSE(is.pendingTuplet());
    MScore::setError(MsError::MS_NO_ERROR);

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameCancel)
{
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    // Tuplet length: a quarter sizes a triplet of eighths; cancelled, also after it was asked for again, the quarter
    // comes back
    is.setDuration(DurationType::V_QUARTER);
    ASSERT_TRUE(waitForTuplet(score, false));
    EXPECT_EQ(is.duration().type(), DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, false));
    EXPECT_EQ(is.duration().type(), DurationType::V_EIGHTH);
    NoteInput::cancelPendingTuplet(score);
    EXPECT_FALSE(is.pendingTuplet());
    EXPECT_EQ(is.duration().type(), DurationType::V_QUARTER);

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameNoteValues)
{
    // the input duration sizes the tuplet, as its note value or divided, with its dots; a duration chosen while it
    // waits is for the first note; asked for again while one waits, the same duration keeps what sized it
    struct Case {
        DurationTypeWithDots duration;
        bool durationAsSubdivision;
        DurationType changeTo;
        std::optional<Fraction> again;
        DurationTypeWithDots inputDuration;
        std::string expected;
    };
    const DurationType none = DurationType::V_INVALID;
    const std::vector<Case> cases {
        // quarter, triplet: the quarter divided into three eighths
        { DurationType::V_QUARTER, false, none, std::nullopt, DurationType::V_EIGHTH, "3:2[8 r8 r8] r4 r2" },
        // dotted half divided: three quarters (3:3); double-dotted half: three eighths in the time of 7
        { { DurationType::V_HALF, 1 }, false, none, std::nullopt, DurationType::V_QUARTER, "3:3[4 r4 r4] r4" },
        { { DurationType::V_HALF, 2 }, false, none, std::nullopt, DurationType::V_EIGHTH, "3:7[8 r8 r8] r8" },
        // a dotted or double-dotted eighth triplet
        { { DurationType::V_EIGHTH, 1 }, true, none, std::nullopt, { DurationType::V_EIGHTH, 1 }, "3:2[8. r8. r8.] r8 r2" },
        { { DurationType::V_EIGHTH, 2 }, true, none, std::nullopt, { DurationType::V_EIGHTH, 2 }, "3:2[8.. r8.. r8..] r16 r2" },
        // eighth, triplet, quarter: the first note of the eighth triplet, not a quarter triplet
        { DurationType::V_EIGHTH, true, DurationType::V_QUARTER, std::nullopt, DurationType::V_QUARTER, "3:2[4 r8] r4 r2" },
        // eighth (dotted or not), triplet, quintuplet: an eighth quintuplet
        { DurationType::V_EIGHTH, true, none, Fraction(5, 4), DurationType::V_EIGHTH, "5:4[8 r8 r8 r8 r8] r2" },
        { { DurationType::V_EIGHTH, 1 }, true, none, Fraction(5, 4), { DurationType::V_EIGHTH, 1 }, "5:4[8. r8. r8. r8. r8.] r4" },
        { { DurationType::V_EIGHTH, 2 }, true, none, Fraction(5, 4), { DurationType::V_EIGHTH, 2 }, "5:4[8.. r8.. r8.. r8.. r8..] r8" },
        // eighth, triplet, quarter, triplet: a quarter triplet
        { DurationType::V_EIGHTH, true, DurationType::V_QUARTER, Fraction(3, 2), DurationType::V_QUARTER, "3:2[4 r4 r4] r2" },
        // quarter divided into a triplet, then a quintuplet: the quarter divided into five
        { DurationType::V_QUARTER, false, none, Fraction(5, 4), DurationType::V_16TH, "5:4[16 r16 r16 r16 r16] r4 r2" },
        // quarter divided into a triplet, then half, triplet: the half divided into three
        { DurationType::V_QUARTER, false, DurationType::V_HALF, Fraction(3, 2), DurationType::V_QUARTER, "3:2[4 r4 r4] r2" },
    };
    for (const Case& c : cases) {
        for (bool rest : { false, true }) {
            SCOPED_TRACE(c.expected);
            MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
            ASSERT_TRUE(score);
            InputState& is = score->inputState();

            is.setDuration(c.duration);
            ASSERT_TRUE(waitForTuplet(score, c.durationAsSubdivision));
            if (c.changeTo != none) {
                pressDuration(score, c.changeTo);
            }
            if (c.again) {
                ASSERT_TRUE(waitForTuplet(score, c.durationAsSubdivision, *c.again));
            }
            EXPECT_TRUE(is.pendingTupletHere());
            EXPECT_EQ(is.duration(), TDuration(c.inputDuration));
            if (rest) {
                enterRest(score);
            } else {
                enterNote(score);
            }
            EXPECT_TRUE(score->sanityCheck());
            std::string expected = c.expected;
            if (rest) {
                expected.insert(expected.find('[') + 1, "r");
            }
            EXPECT_EQ(rhythm(measureAt(score, 0)), expected);
            EXPECT_EQ(is.duration(), TDuration(c.inputDuration));

            delete score;
        }
    }
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameDoesNotFit)
{
    // a half and a quarter, then a quarter triplet on beat 4, dotted or not, doesn't fit: it waits all the same
    const std::string fitting[] = { "2 4 3:2[8 r8 r8]", "2 4 3:2[16. r16. r16.] r16", "2 4 3:2[16.. r16.. r16..] r32" };
    for (int dots : { 0, 1, 2 }) {
        MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        is.setDuration(DurationType::V_HALF);
        enterNote(score);
        is.setDuration(DurationType::V_QUARTER);
        enterNote(score);
        is.setDuration(DurationTypeWithDots(DurationType::V_QUARTER, dots));
        ASSERT_TRUE(waitForTuplet(score, true));
        EXPECT_TRUE(is.pendingTupletHere());
        EXPECT_FALSE(NoteInput::pendingTupletFitsHere(score));

        // a note is refused, and the tuplet keeps waiting
        enterNote(score);
        EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "2 4 r4");
        EXPECT_TRUE(is.pendingTupletHere());

        // a duration key doesn't change it
        pressDuration(score, DurationType::V_EIGHTH);
        EXPECT_FALSE(NoteInput::pendingTupletFitsHere(score));
        enterNote(score);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "2 4 r4");

        // the tuplet command again, with an eighth (dotted: a 16th, with the same dots): a triplet that fits replaces it
        MScore::setError(MsError::MS_NO_ERROR);
        is.setDuration(DurationTypeWithDots(dots ? DurationType::V_16TH : DurationType::V_EIGHTH, dots));
        ASSERT_TRUE(waitForTuplet(score, true));
        EXPECT_TRUE(NoteInput::pendingTupletFitsHere(score));
        enterNote(score);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), fitting[dots]);

        // undoing that note brings back the triplet that fits, not the one that didn't
        score->undoRedo(true, nullptr);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "2 4 r4");
        EXPECT_TRUE(is.pendingTupletHere());
        EXPECT_TRUE(NoteInput::pendingTupletFitsHere(score));

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameEntryPaths)
{
    // after a quarter, an eighth triplet on beat 2 takes a note or rest entered each way; a half triplet, which
    // doesn't fit, is refused each way and keeps waiting
    const std::vector<std::pair<std::function<void(Score*)>, std::string> > paths {
        // letter
        { [](Score* score) { enterLetter(score); }, "4 3:2[8 r8 r8] r2" },
        // MIDI
        { [](Score* score) { enterNote(score); }, "4 3:2[8 r8 r8] r2" },
        // mouse, as for a tablature fret
        { [](Score* score) { clickNote(score, Fraction(1, 4), 4); }, "4 3:2[8 r8 r8] r2" },
        // rest
        { [](Score* score) { enterRest(score); }, "4 3:2[r8 r8 r8] r2" },
        // R
        { [](Score* score) { repeatChord(score, toChord(chordRestAt(measureAt(score, 0), 0))); }, "4 3:2[8 r8 r8] r2" },
        // tie
        { [](Score* score) { EditTie::cmdAddTie(score); }, "4~ 3:2[8 r8 r8] r2" },
        // Ctrl+Shift+letter: the tuplet's time is inserted
        { [](Score* score) { enterLetter(score, true); }, "4 3:2[8 r8 r8] r4 r2" },
    };
    for (size_t i = 0; i < paths.size(); ++i) {
        for (bool fits : { true, false }) {
            SCOPED_TRACE(testing::Message() << "path " << i << (fits ? ", fits" : ", doesn't fit"));
            MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
            ASSERT_TRUE(score);
            InputState& is = score->inputState();

            enterNote(score);
            is.setDuration(fits ? DurationType::V_EIGHTH : DurationType::V_HALF);
            ASSERT_TRUE(waitForTuplet(score, true));
            paths[i].first(score);
            EXPECT_EQ(MScore::_error, fits ? MsError::MS_NO_ERROR : MsError::NO_SPACE_FOR_TUPLET);
            MScore::setError(MsError::MS_NO_ERROR);
            EXPECT_TRUE(score->sanityCheck());
            EXPECT_EQ(rhythm(measureAt(score, 0)), fits ? paths[i].second : "4 r4 r2");
            EXPECT_EQ(bool(is.pendingTuplet()), !fits);
            EXPECT_EQ(is.pendingTupletHere(), !fits);

            delete score;
        }
    }
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameRest)
{
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    enterRest(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[r8 r8 r8] r4 r2");
    EXPECT_FALSE(is.pendingTuplet());
    EXPECT_EQ(is.tick(), Fraction(1, 12));

    enterNote(score);
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[r8 8 r8] r4 r2");

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameMovedAway)
{
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    // the input position moves to bar 2: an ordinary note there
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    is.setSegment(score->tick2segment(Fraction(1, 1), false, SegmentType::ChordRest));
    enterNote(score);
    EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
    EXPECT_EQ(rhythm(measureAt(score, 1)), "8 r8 r4 r2");
    EXPECT_FALSE(is.pendingTuplet());

    // a note entered with the mouse elsewhere: an ordinary note too (the notation layer cancels the tuplet)
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    ASSERT_TRUE(waitForTuplet(score, true));
    clickNote(score, Fraction(2, 1), 4);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
    EXPECT_EQ(rhythm(measureAt(score, 2)), "8 r8 r4 r2");
    EXPECT_FALSE(is.pendingTupletHere());

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameBeyondScore)
{
    // past the last measure, by MIDI or by letter: the tuplet goes in a new measure
    for (bool byLetter : { false, true }) {
        MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        const int measures = score->nmeasures();
        is.setSegment(score->lastMeasure()->first(SegmentType::ChordRest));
        is.setBeyondScore(true);
        is.setDuration(DurationType::V_EIGHTH);
        ASSERT_TRUE(waitForTuplet(score, true));
        if (byLetter) {
            enterLetter(score);
        } else {
            enterNote(score);
        }
        EXPECT_TRUE(score->sanityCheck());
        ASSERT_EQ(score->nmeasures(), measures + 1);
        EXPECT_EQ(rhythm(score->lastMeasure()), "3:2[8 r8 r8] r4 r2");

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameOtherVoiceSelected)
{
    MasterScore* score = pendingTupletScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    // the tuplet and its first note go in the input voice
    enterVoice2Quarters(score);
    is.setNoteEntryMethod(NoteEntryMethod::BY_NOTE_NAME);
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    enterNote(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 r8 r8] r4 r2");
    EXPECT_EQ(is.track(), 0);
    EXPECT_EQ(is.tick(), Fraction(1, 12));

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameInsideLongerChordRest)
{
    // the tuplet waits on beat 2, inside a half note or rest: it shortens it, as a plain note does
    for (bool rest : { false, true }) {
        MasterScore* score = pendingTupletScore();
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        is.setRest(rest);
        pressDuration(score, DurationType::V_HALF);
        is.setRest(false);
        enterVoice2Quarters(score);
        is.setNoteEntryMethod(NoteEntryMethod::BY_NOTE_NAME);
        is.setSegment(score->tick2segment(Fraction(1, 4), false, SegmentType::ChordRest));
        ASSERT_FALSE(is.cr());

        is.setDuration(DurationType::V_EIGHTH);
        ASSERT_TRUE(waitForTuplet(score, true));
        enterNote(score);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), std::string(rest ? "r4" : "4") + " 3:2[8 r8 r8] r2");
        EXPECT_EQ(is.track(), 0);

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletRepeat)
{
    // By note name: R after a chord takes the waiting tuplet, as any next note does
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setDuration(DurationType::V_EIGHTH);
    enterNote(score, 60);
    enterNote(score, 64, true);
    ASSERT_TRUE(waitForTuplet(score, true));
    repeatChord(score, toChord(chordRestAt(measureAt(score, 0), 0)));
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "8 3:2[8 r8 r8] r8 r2");
    EXPECT_FALSE(is.pendingTuplet());
    const ChordRest* repeated = chordRestAt(measureAt(score, 0), 1);
    ASSERT_TRUE(repeated && repeated->isChord());
    EXPECT_EQ(toChord(repeated)->notes().size(), 2u);
    EXPECT_TRUE(repeated->tuplet());
    delete score;

    // By duration: the chord entered where the tuplet waits takes it up, sized by the input duration
    score = pendingTupletScore(NoteEntryMethod::BY_DURATION);
    ASSERT_TRUE(score);
    InputState& is2 = score->inputState();
    is2.setNotes({ NoteVal(60), NoteVal(64) });
    pressDuration(score, DurationType::V_EIGHTH);
    ASSERT_EQ(rhythm(measureAt(score, 0)), "8 r8 r4 r2");
    setPendingTriplet(score, true);
    repeatChord(score, toChord(chordRestAt(measureAt(score, 0), 0)));
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "8 3:2[8 r8 r8] r8 r2");
    EXPECT_FALSE(is2.pendingTuplet());
    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameAddToChord)
{
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    // a note added to the chord just entered leaves the tuplet waiting after it
    is.setDuration(DurationType::V_EIGHTH);
    enterNote(score, 60);
    ASSERT_TRUE(waitForTuplet(score, true));
    enterNote(score, 64, true);
    EXPECT_TRUE(is.pendingTupletHere());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "8 r8 r4 r2");
    EXPECT_EQ(toChord(chordRestAt(measureAt(score, 0), 0))->notes().size(), 2u);

    enterNote(score, 67);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "8 3:2[8 r8 r8] r8 r2");

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameTie)
{
    // after a chord: the tie key ties both notes into the waiting triplet
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setDuration(DurationType::V_EIGHTH);
    enterNote(score, 60);
    enterNote(score, 64, true);
    ASSERT_TRUE(waitForTuplet(score, true));
    EditTie::cmdAddTie(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "8~ 3:2[8 r8 r8] r8 r2");
    EXPECT_FALSE(is.pendingTuplet());
    const ChordRest* tied = chordRestAt(measureAt(score, 0), 1);
    ASSERT_TRUE(tied && tied->isChord());
    EXPECT_EQ(toChord(tied)->notes().size(), 2u);
    for (const Note* note : toChord(tied)->notes()) {
        EXPECT_TRUE(note->tieBack());
    }
    delete score;

    // with the input position moved back onto a note (D, after the chord), the triplet waits there and takes a note
    // tied from the chord before it, rather than the selected D being tied into the next position
    score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is2 = score->inputState();
    is2.setDuration(DurationType::V_EIGHTH);
    enterNote(score, 60);
    enterNote(score, 64, true);
    enterNote(score, 62);
    enterNote(score, 65);
    is2.setSegment(score->tick2segment(Fraction(1, 8), false, SegmentType::ChordRest));
    score->select(toChord(is2.cr())->upNote());
    ASSERT_TRUE(waitForTuplet(score, true));
    EditTie::cmdAddTie(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "8~ 3:2[8 r8 r8] r8 r2");
    EXPECT_FALSE(is2.pendingTuplet());
    const ChordRest* chord = chordRestAt(measureAt(score, 0), 0);
    tied = chordRestAt(measureAt(score, 0), 1);
    ASSERT_TRUE(chord && chord->isChord() && tied && tied->isChord());
    EXPECT_TRUE(tied->tuplet());
    ASSERT_EQ(toChord(tied)->notes().size(), 2u);
    ASSERT_TRUE(toChord(tied)->downNote()->tieBack() && toChord(tied)->upNote()->tieBack());
    EXPECT_EQ(toChord(tied)->downNote()->tieBack()->startNote(), toChord(chord)->downNote());
    EXPECT_EQ(toChord(tied)->upNote()->tieBack()->startNote(), toChord(chord)->upNote());
    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletTieBeyondScore)
{
    // past the end of the score, after F: the tie key appends one measure, which the waiting eighth triplet starts with
    // an F tied into it
    const NoteEntryMethod methods[] = { NoteEntryMethod::BY_DURATION, NoteEntryMethod::BY_NOTE_NAME, NoteEntryMethod::TIMEWISE };
    for (NoteEntryMethod method : methods) {
        SCOPED_TRACE(static_cast<int>(method));
        MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();
        is.setSegment(score->lastMeasure()->first(SegmentType::ChordRest));
        for (int pitch : { 60, 62, 64, 65 }) {
            enterNote(score, pitch);
        }
        ASSERT_TRUE(is.beyondScore());
        is.setNoteEntryMethod(method);
        const int measures = score->nmeasures();

        is.setDuration(DurationType::V_EIGHTH);
        if (method == NoteEntryMethod::BY_DURATION) {
            setPendingTriplet(score, true);
        } else {
            ASSERT_TRUE(waitForTuplet(score, true));
        }
        EditTie::cmdAddTie(score);
        EXPECT_TRUE(score->sanityCheck());
        ASSERT_EQ(score->nmeasures(), measures + 1);
        EXPECT_EQ(rhythm(measureAt(score, measures - 1)), "4 4 4 4~");
        EXPECT_EQ(rhythm(score->lastMeasure()), "3:2[8 r8 r8] r4 r2");
        EXPECT_FALSE(is.pendingTuplet());

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameDrums)
{
    MasterScore* score = drumScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    // a snare (voice 1) takes the tuplet
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    enterDrum(score, 38);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 r8 r8] r4 r2");

    // a bass drum (voice 2) takes it to its own voice: no empty tuplet in voice 1
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(1, 2), false, SegmentType::ChordRest));
    ASSERT_TRUE(waitForTuplet(score, true));
    enterDrum(score, 36);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_FALSE(is.pendingTuplet());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 r8 r8] r4 r2");
    const Segment* s = score->tick2segment(Fraction(1, 2), false, SegmentType::ChordRest);
    ASSERT_TRUE(s && s->element(1));
    const ChordRest* bassDrum = toChordRest(s->element(1));
    ASSERT_TRUE(bassDrum->isChord() && bassDrum->tuplet());
    EXPECT_EQ(bassDrum->tuplet()->ratio(), Fraction(3, 2));
    EXPECT_EQ(is.track(), 1);
    delete score;

    // two snares, then a quarter triplet on beat 4: a snare is refused, and the tuplet keeps waiting
    score = drumScore();
    ASSERT_TRUE(score);
    InputState& is2 = score->inputState();
    is2.setDuration(DurationType::V_HALF);
    enterDrum(score, 38);
    is2.setDuration(DurationType::V_QUARTER);
    enterDrum(score, 38);
    ASSERT_TRUE(waitForTuplet(score, true));
    enterDrum(score, 38);
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
    MScore::setError(MsError::MS_NO_ERROR);
    EXPECT_TRUE(is2.pendingTupletHere());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "2 4 r4");

    // a bass drum takes it to its own voice, where it doesn't fit either
    enterDrum(score, 36);
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
    MScore::setError(MsError::MS_NO_ERROR);
    EXPECT_TRUE(is2.pendingTupletHere());
    EXPECT_EQ(is2.track(), 1);
    EXPECT_EQ(rhythm(measureAt(score, 0)), "2 4 r4");
    delete score;
}

static int unmappedDrumPitch(const Score* score)
{
    int pitch = 0;
    while (score->inputState().drumset()->isValid(pitch)) {
        ++pitch;
    }
    return pitch;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameUnmappedDrum)
{
    // a MIDI note for a drum that the drum set doesn't map is refused, and the triplet keeps waiting
    MasterScore* score = drumScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    enterNote(score, unmappedDrumPitch(score));
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
    EXPECT_TRUE(is.pendingTupletHere());

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameDotPreviousNote)
{
    // a half and two eighths, then an eighth triplet waits on beat 4; dotting or double-dotting the second eighth
    // moves the input position to 13/16 or 27/32, where the triplet doesn't fit: it keeps waiting there
    for (int dots : { 1, 2 }) {
        MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        is.setDuration(DurationType::V_HALF);
        enterNote(score);
        is.setDuration(DurationType::V_EIGHTH);
        enterNote(score);
        enterNote(score);
        ASSERT_TRUE(waitForTuplet(score, true));

        score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
            NoteInput::toggleDots(tx, score, dots, true /*toggleForSelectionOnly*/);
        });
        if (dots == 1) {
            EXPECT_EQ(rhythm(measureAt(score, 0)), "2 8 8. r16 r8");
        }
        EXPECT_TRUE(is.pendingTupletHere());
        EXPECT_EQ(is.pendingTuplet()->tick, dots == 1 ? Fraction(13, 16) : Fraction(27, 32));
        EXPECT_FALSE(NoteInput::pendingTupletFitsHere(score));

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameDotAfterCommand)
{
    // eighth, triplet, then a dot: the first note is dotted, in the eighth triplet
    for (int dots : { 1, 2 }) {
        MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        is.setDuration(DurationType::V_EIGHTH);
        ASSERT_TRUE(waitForTuplet(score, true));
        pressDot(score, dots);
        EXPECT_TRUE(is.pendingTupletHere());
        enterNote(score);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), dots == 1 ? "3:2[8. r16 r8] r4 r2" : "3:2[8.. r32 r8] r4 r2");
        EXPECT_EQ(is.tick(), dots == 1 ? Fraction(1, 8) : Fraction(7, 48));
        const ChordRest* first = chordRestAt(measureAt(score, 0), 0);
        ASSERT_TRUE(first && first->tuplet());
        EXPECT_EQ(first->tuplet()->baseLen().type(), DurationType::V_EIGHTH);
        EXPECT_EQ(first->tuplet()->baseLen().dots(), 0);

        delete score;
    }
}

//-----------------------------------------
//    pending tuplets in Insert input
//     the tuplet waits as in By note name input, and the
//     next note or rest inserts its whole time: the bar
//     grows and the following notes move right. Four
//     quarters in 4/4, input position on `tick`.
//-----------------------------------------

// the input position and the selection, which Insert needs, on the note or rest at `tick` in the input voice
static void selectForInsert(Score* score, const Fraction& tick)
{
    InputState& is = score->inputState();
    is.setSegment(score->tick2segment(tick, false, SegmentType::ChordRest));
    const ChordRest* cr = is.cr();
    score->select(cr->isChord() ? static_cast<EngravingItem*>(toChord(cr)->upNote()) : const_cast<ChordRest*>(cr));
}

static MasterScore* pendingTupletInsertScore(const Fraction& tick = Fraction(1, 4))
{
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    if (!score) {
        return nullptr;
    }
    for (int pitch : { 60, 62, 64, 65 }) {
        enterNote(score, pitch);
    }
    score->inputState().setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    selectForInsert(score, tick);
    return score;
}

// the same past the end of the score
static MasterScore* pendingTupletInsertBeyondScoreScore()
{
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    if (!score) {
        return nullptr;
    }
    InputState& is = score->inputState();
    is.setSegment(score->lastMeasure()->first(SegmentType::ChordRest));
    for (int pitch : { 60, 62, 64, 65 }) {
        enterNote(score, pitch);
    }
    is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    score->select(toChord(chordRestAt(score->lastMeasure(), 3))->upNote());
    return score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsert)
{
    MasterScore* score = pendingTupletInsertScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    // eighth, triplet on beat 2: nothing changes until the next note
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 4 4 4");
    EXPECT_TRUE(is.pendingTupletHere());

    // the next note inserts the triplet's quarter, and goes into it
    enterLetter(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 3:2[8 r8 r8] 4 4 4");
    EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(5, 4));
    EXPECT_FALSE(is.pendingTuplet());
    EXPECT_EQ(is.tick(), Fraction(1, 4) + Fraction(1, 12));

    // the next notes fill the triplet, as in By note name input; after it, Insert inserts again
    MScore::setError(MsError::MS_NO_ERROR);
    enterLetter(score);
    enterLetter(score);
    EXPECT_EQ(MScore::_error, MsError::MS_NO_ERROR);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 3:2[8 8 8] 4 4 4");
    EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(5, 4));
    enterLetter(score);
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 3:2[8 8 8] 8 4 4 4");
    EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(11, 8));

    // undoing the first note takes the inserted time away too, and the tuplet waits again
    for (int i = 0; i < 4; ++i) {
        score->undoRedo(true, nullptr);
    }
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 4 4 4");
    EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(1, 1));
    EXPECT_TRUE(is.pendingTupletHere());

    // redone, the next note fills the triplet
    score->undoRedo(false, nullptr);
    MScore::setError(MsError::MS_NO_ERROR);
    enterLetter(score);
    EXPECT_EQ(MScore::_error, MsError::MS_NO_ERROR);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 3:2[8 8 r8] 4 4 4");

    // after Escape, undoing the first note doesn't bring the tuplet back
    is.forgetPendingTuplets();
    score->undoRedo(true, nullptr);
    score->undoRedo(true, nullptr);
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 4 4 4");
    EXPECT_FALSE(is.pendingTuplet());

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsertNoteValues)
{
    // the next note or rest inserts the waiting tuplet's time, sized as in By note name input
    struct Case {
        Fraction tick;
        DurationTypeWithDots duration;
        bool durationAsSubdivision;
        std::optional<Fraction> again;
        DurationTypeWithDots inputDuration;
        std::string expected;
        Fraction measureTicks;
    };
    const Fraction beat2(1, 4);
    const std::vector<Case> cases {
        // a quarter triplet on beat 4, which wouldn't fit in By note name input: the bar grows by a half
        { Fraction(3, 4), DurationType::V_QUARTER, true, std::nullopt, DurationType::V_QUARTER, "4 4 4 3:2[4 r4 r4] 4", Fraction(6, 4) },
        // quarter, triplet: the quarter divided into three eighths
        { beat2, DurationType::V_QUARTER, false, std::nullopt, DurationType::V_EIGHTH, "4 3:2[8 r8 r8] 4 4 4", Fraction(5, 4) },
        // dotted half divided: three quarters (3:3); double-dotted half: three eighths in the time of 7
        { beat2, { DurationType::V_HALF, 1 }, false, std::nullopt, DurationType::V_QUARTER, "4 3:3[4 r4 r4] 4 4 4", Fraction(7, 4) },
        { beat2, { DurationType::V_HALF, 2 }, false, std::nullopt, DurationType::V_EIGHTH, "4 3:7[8 r8 r8] 4 4 4", Fraction(15, 8) },
        // a dotted or double-dotted eighth triplet
        { beat2, { DurationType::V_EIGHTH, 1 }, true, std::nullopt, { DurationType::V_EIGHTH, 1 }, "4 3:2[8. r8. r8.] 4 4 4",
          Fraction(11, 8) },
        { beat2, { DurationType::V_EIGHTH, 2 }, true, std::nullopt, { DurationType::V_EIGHTH, 2 }, "4 3:2[8.. r8.. r8..] 4 4 4",
          Fraction(23, 16) },
        // eighth, triplet, quintuplet: an eighth quintuplet's half
        { beat2, DurationType::V_EIGHTH, true, Fraction(5, 4), DurationType::V_EIGHTH, "4 5:4[8 r8 r8 r8 r8] 4 4 4", Fraction(6, 4) },
    };
    for (const Case& c : cases) {
        for (bool rest : { false, true }) {
            SCOPED_TRACE(c.expected);
            MasterScore* score = pendingTupletInsertScore(c.tick);
            ASSERT_TRUE(score);
            InputState& is = score->inputState();

            is.setDuration(c.duration);
            ASSERT_TRUE(waitForTuplet(score, c.durationAsSubdivision));
            if (c.again) {
                ASSERT_TRUE(waitForTuplet(score, c.durationAsSubdivision, *c.again));
            }
            EXPECT_EQ(is.duration(), TDuration(c.inputDuration));
            if (rest) {
                enterRest(score);
            } else {
                enterLetter(score);
            }
            EXPECT_TRUE(score->sanityCheck());
            std::string expected = c.expected;
            if (rest) {
                expected.insert(expected.find('[') + 1, "r");
            }
            EXPECT_EQ(rhythm(measureAt(score, 0)), expected);
            EXPECT_EQ(measureAt(score, 0)->ticks(), c.measureTicks);
            EXPECT_EQ(is.duration(), TDuration(c.inputDuration));

            delete score;
        }
    }
}

TEST_F(Engraving_TupletTests, pendingTupletInsertTie)
{
    // the input position, and the selection, on D
    MasterScore* score = pendingTupletInsertScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    // eighth, triplet, then the tie key: the triplet's time is inserted before D, and C is tied into it
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    EditTie::cmdAddTie(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4~ 3:2[8 r8 r8] 4 4 4");
    EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(5, 4));
    EXPECT_FALSE(is.pendingTuplet());

    Measure* measure = measureAt(score, 0);
    const ChordRest* c = chordRestAt(measure, 0);
    const ChordRest* tied = chordRestAt(measure, 1);
    ASSERT_TRUE(c && c->isChord() && tied && tied->isChord());
    const Note* tiedNote = toChord(tied)->upNote();
    EXPECT_EQ(tiedNote->pitch(), 60);
    ASSERT_TRUE(tiedNote->tieBack());
    EXPECT_EQ(tiedNote->tieBack()->startNote(), toChord(c)->upNote());

    // D, E, F are unchanged
    for (int i = 0; i < 3; ++i) {
        const ChordRest* cr = chordRestAt(measure, 4 + i);
        ASSERT_TRUE(cr && cr->isChord());
        EXPECT_EQ(toChord(cr)->upNote()->pitch(), (std::vector<int> { 62, 64, 65 })[i]);
        EXPECT_FALSE(toChord(cr)->upNote()->tieBack());
    }
    delete score;

    // on the next bar's measure rest: tied from F, across the barline
    score = pendingTupletInsertScore(Fraction(1, 1));
    ASSERT_TRUE(score);
    score->inputState().setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    EditTie::cmdAddTie(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 4 4 4~");
    EXPECT_EQ(rhythm(measureAt(score, 1)), "3:2[8 r8 r8] r4 r2 r4");
    EXPECT_EQ(measureAt(score, 1)->ticks(), Fraction(5, 4));
    const ChordRest* first = chordRestAt(measureAt(score, 1), 0);
    ASSERT_TRUE(first && first->isChord());
    ASSERT_TRUE(toChord(first)->upNote()->tieBack());
    EXPECT_EQ(toChord(first)->upNote()->tieBack()->startNote(), toChord(chordRestAt(measureAt(score, 0), 3))->upNote());

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsertEntryPaths)
{
    // a note entered by letter, by MIDI, with the mouse, through putNote (the path of a TAB fret) or a rest: each inserts
    // the tuplet's time
    const std::vector<std::pair<std::function<void(Score*)>, std::string> > paths {
        { [](Score* score) { enterLetter(score); }, "4 3:2[8 r8 r8] 4 4 4" },
        { [](Score* score) { enterNote(score); }, "4 3:2[8 r8 r8] 4 4 4" },
        { [](Score* score) {
                Position pos;
                pos.segment = score->inputState().segment();
                pos.staffIdx = 0;
                pos.line = 4;
                score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
                    NoteInput::insertChordByInsertingTime(tx, score, pos);
                });
            }, "4 3:2[8 r8 r8] 4 4 4" },
        { [](Score* score) {
                Position pos;
                pos.segment = score->inputState().segment();
                pos.staffIdx = 0;
                pos.line = 4;
                score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
                    NoteInput::putNote(tx, score, pos, true);
                });
            }, "4 3:2[8 r8 r8] 4 4 4" },
        { [](Score* score) { enterRest(score); }, "4 3:2[r8 r8 r8] 4 4 4" },
    };
    for (const auto& [enter, expected] : paths) {
        MasterScore* score = pendingTupletInsertScore();
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        is.setDuration(DurationType::V_EIGHTH);
        ASSERT_TRUE(waitForTuplet(score, true));
        enter(score);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), expected);
        EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(5, 4));
        EXPECT_FALSE(is.pendingTuplet());

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletInsertInsideTuplet)
{
    // a quarter triplet on beats 1-2, then Insert on its second note: no tuplet can wait there
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    ASSERT_TRUE(waitForTuplet(score, true));
    for (int i = 0; i < 5; ++i) {
        enterNote(score);
    }
    ASSERT_EQ(rhythm(measureAt(score, 0)), "3:2[4 4 4] 4 4");
    is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    selectForInsert(score, chordRestAt(measureAt(score, 0), 1)->tick());
    is.setDuration(DurationType::V_EIGHTH);
    EXPECT_FALSE(waitForTuplet(score, true));
    EXPECT_FALSE(is.pendingTuplet());
    EXPECT_EQ(MScore::_error, MsError::CANNOT_INSERT_TUPLET_IN_TUPLET);

    // and notes inside a tuplet that was already there are still refused
    MScore::setError(MsError::MS_NO_ERROR);
    enterLetter(score);
    EXPECT_EQ(MScore::_error, MsError::CANNOT_INSERT_TUPLET);
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[4 4 4] 4 4");
    MScore::setError(MsError::MS_NO_ERROR);

    // at the triplet's start it can: the new tuplet goes before it
    selectForInsert(score, Fraction(0, 1));
    ASSERT_TRUE(waitForTuplet(score, true));
    enterLetter(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 r8 r8] 3:2[4 4 4] 4 4");

    delete score;
}

// D, which followed the inserted time: still there, and not tied to a note before it
static void expectD(const ChordRest* cr)
{
    ASSERT_TRUE(cr && cr->isChord());
    EXPECT_EQ(toChord(cr)->upNote()->pitch(), 62);
    EXPECT_FALSE(toChord(cr)->upNote()->tieBack());
}

TEST_F(Engraving_TupletTests, pendingTupletInsertLongerNote)
{
    // eighth, triplet, then a half, dotted or not: the note ends with the triplet, and the notes after it stay;
    // a letter in Insert input or Ctrl+Shift+letter in By note name input, a note from a MIDI keyboard, or R (repeating C)
    enum class Path {
        Letter, InsertFlag, Midi, Repeat
    };
    for (Path path : { Path::Letter, Path::InsertFlag, Path::Midi, Path::Repeat }) {
        for (int dots : { 0, 1, 2 }) {
            SCOPED_TRACE(testing::Message() << "path " << int(path) << ", dots " << dots);
            MasterScore* score = pendingTupletInsertScore();
            ASSERT_TRUE(score);
            InputState& is = score->inputState();
            if (path == Path::InsertFlag) {
                is.setNoteEntryMethod(NoteEntryMethod::BY_NOTE_NAME);
            }

            is.setDuration(DurationType::V_EIGHTH);
            ASSERT_TRUE(waitForTuplet(score, true));
            pressDuration(score, DurationType::V_HALF);
            if (dots) {
                pressDot(score, dots);
            }
            ASSERT_TRUE(is.pendingTupletHere());
            if (path == Path::Midi) {
                enterNote(score, 67);
            } else if (path == Path::Repeat) {
                repeatChord(score, toChord(chordRestAt(measureAt(score, 0), 0)));
            } else {
                enterLetter(score, path == Path::InsertFlag);
            }
            EXPECT_TRUE(score->sanityCheck());
            EXPECT_EQ(rhythm(measureAt(score, 0)), "4 3:2[4.] 4 4 4");
            EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(5, 4));
            EXPECT_EQ(is.tick(), Fraction(1, 2));
            ASSERT_NO_FATAL_FAILURE(expectD(chordRestAt(measureAt(score, 0), 2)));

            delete score;
        }
    }
}

TEST_F(Engraving_TupletTests, pendingTupletInsertLongerFillingNote)
{
    // a later note longer than what is left of the inserted triplet ends with it too: a half after the first note of
    // an eighth triplet, a dotted half or double-dotted quarter after the first of a dotted eighth triplet
    struct Case {
        int dots;
        DurationTypeWithDots later;
        std::string expected;
        Fraction measureTicks;
    };
    const std::vector<Case> cases {
        { 0, DurationType::V_HALF, "4 3:2[8 4] 4 4 4", Fraction(5, 4) },
        { 1, { DurationType::V_HALF, 1 }, "4 3:2[8. 4.] 4 4 4", Fraction(11, 8) },
        { 1, { DurationType::V_QUARTER, 2 }, "4 3:2[8. 4.] 4 4 4", Fraction(11, 8) },
    };
    for (const Case& c : cases) {
        // in Insert input, and with Ctrl+Shift+letter in By note name input
        for (bool insertFlag : { false, true }) {
            SCOPED_TRACE(testing::Message() << c.expected << (insertFlag ? ", Ctrl+Shift" : ""));
            MasterScore* score = pendingTupletInsertScore();
            ASSERT_TRUE(score);
            InputState& is = score->inputState();
            if (insertFlag) {
                is.setNoteEntryMethod(NoteEntryMethod::BY_NOTE_NAME);
            }

            is.setDuration(DurationTypeWithDots(DurationType::V_EIGHTH, c.dots));
            ASSERT_TRUE(waitForTuplet(score, true));
            enterLetter(score, insertFlag);
            ASSERT_EQ(rhythm(measureAt(score, 0)), c.dots ? "4 3:2[8. r8. r8.] 4 4 4" : "4 3:2[8 r8 r8] 4 4 4");
            pressDuration(score, c.later.type);
            if (c.later.dots) {
                pressDot(score, c.later.dots);
            }
            enterLetter(score, insertFlag);
            EXPECT_TRUE(score->sanityCheck());
            EXPECT_EQ(rhythm(measureAt(score, 0)), c.expected);
            EXPECT_EQ(measureAt(score, 0)->ticks(), c.measureTicks);
            ASSERT_NO_FATAL_FAILURE(expectD(chordRestAt(measureAt(score, 0), 3)));

            delete score;
        }
    }
}

TEST_F(Engraving_TupletTests, pendingTupletInsertFillsOnlyRests)
{
    MasterScore* score = pendingTupletInsertScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    enterLetter(score);
    enterLetter(score);
    ASSERT_EQ(rhythm(measureAt(score, 0)), "4 3:2[8 8 r8] 4 4 4");

    // back onto one of its notes: refused, as in any other tuplet
    selectForInsert(score, chordRestAt(measureAt(score, 0), 2)->tick());
    MScore::setError(MsError::MS_NO_ERROR);
    enterLetter(score);
    EXPECT_EQ(MScore::_error, MsError::CANNOT_INSERT_TUPLET);
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 3:2[8 8 r8] 4 4 4");
    MScore::setError(MsError::MS_NO_ERROR);

    // its rest is still filled
    selectForInsert(score, chordRestAt(measureAt(score, 0), 3)->tick());
    enterLetter(score);
    EXPECT_EQ(MScore::_error, MsError::MS_NO_ERROR);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 3:2[8 8 8] 4 4 4");
    EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(5, 4));

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsertLeftAndBack)
{
    // after leaving note input and coming back, or changing the method and back, the inserted triplet is like any other
    for (bool leave : { true, false }) {
        MasterScore* score = pendingTupletInsertScore();
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        is.setDuration(DurationType::V_EIGHTH);
        ASSERT_TRUE(waitForTuplet(score, true));
        enterLetter(score);
        ASSERT_EQ(rhythm(measureAt(score, 0)), "4 3:2[8 r8 r8] 4 4 4");
        if (leave) {
            is.setNoteEntryMode(false);
            is.setNoteEntryMode(true);
        } else {
            is.setNoteEntryMethod(NoteEntryMethod::BY_NOTE_NAME);
            is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
        }

        selectForInsert(score, chordRestAt(measureAt(score, 0), 2)->tick());
        MScore::setError(MsError::MS_NO_ERROR);
        enterLetter(score);
        EXPECT_EQ(MScore::_error, MsError::CANNOT_INSERT_TUPLET);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "4 3:2[8 r8 r8] 4 4 4");
        MScore::setError(MsError::MS_NO_ERROR);

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletInsertBeyondScore)
{
    // past the end of the score, a half quintuplet (a quarter one when dotted) doesn't fit in the measure the next note
    // appends: it waits, the note is refused, and nothing is appended; an eighth triplet, with the same dots, then
    // replaces it, and the note appends one measure, which the tuplet starts
    struct Case {
        DurationType tooLong;
        std::string expected;
        Fraction firstNote;
    };
    const Case cases[] = {
        { DurationType::V_HALF, "3:2[8 r8 r8] r4 r2", Fraction(1, 12) },
        { DurationType::V_QUARTER, "3:2[8. r8. r8.] r8 r2", Fraction(1, 8) },
        { DurationType::V_QUARTER, "3:2[8.. r8.. r8..] r16 r2", Fraction(7, 48) },
    };
    for (int dots : { 0, 1, 2 }) {
        const Case& c = cases[dots];
        MasterScore* score = pendingTupletInsertBeyondScoreScore();
        ASSERT_TRUE(score);
        InputState& is = score->inputState();
        ASSERT_TRUE(is.beyondScore());
        const int measures = score->nmeasures();
        const Fraction endTick = score->endTick();

        is.setDuration(DurationTypeWithDots(c.tooLong, dots));
        ASSERT_TRUE(waitForTuplet(score, true, Fraction(5, 4)));
        EXPECT_FALSE(NoteInput::pendingTupletFitsHere(score));
        enterLetter(score);
        EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
        MScore::setError(MsError::MS_NO_ERROR);
        EXPECT_EQ(score->nmeasures(), measures);
        EXPECT_TRUE(is.pendingTupletHere());

        is.setDuration(DurationTypeWithDots(DurationType::V_EIGHTH, dots));
        ASSERT_TRUE(waitForTuplet(score, true));
        EXPECT_TRUE(NoteInput::pendingTupletFitsHere(score));
        enterLetter(score);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(score->nmeasures(), measures + 1);
        EXPECT_EQ(rhythm(score->lastMeasure()), c.expected);
        EXPECT_EQ(is.tick(), endTick + c.firstNote);

        delete score;
    }

    // when the last measure ends with an eighth triplet, that triplet doesn't stop it
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setSegment(score->lastMeasure()->first(SegmentType::ChordRest));
    for (int i = 0; i < 3; ++i) {
        enterNote(score);
    }
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    for (int i = 0; i < 3; ++i) {
        enterNote(score, 62);
    }
    ASSERT_TRUE(is.beyondScore());
    is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    score->select(toChord(chordRestAt(score->lastMeasure(), 5))->upNote());
    const int measures = score->nmeasures();

    ASSERT_TRUE(waitForTuplet(score, true));
    enterLetter(score);
    EXPECT_EQ(MScore::_error, MsError::MS_NO_ERROR);
    MScore::setError(MsError::MS_NO_ERROR);
    EXPECT_TRUE(score->sanityCheck());
    ASSERT_EQ(score->nmeasures(), measures + 1);
    EXPECT_EQ(rhythm(measureAt(score, measures - 1)), "4 4 4 3:2[8 8 8]");
    EXPECT_EQ(rhythm(score->lastMeasure()), "3:2[8 r8 r8] r4 r2");
    delete score;

    // a half triplet inserted past the end fills the measure it appends; past the end again, a whole rest isn't capped
    // at that triplet's end
    score = pendingTupletInsertBeyondScoreScore();
    ASSERT_TRUE(score);
    InputState& is2 = score->inputState();
    is2.setDuration(DurationType::V_HALF);
    ASSERT_TRUE(waitForTuplet(score, true));
    for (int i = 0; i < 3; ++i) {
        enterLetter(score);
    }
    ASSERT_EQ(rhythm(score->lastMeasure()), "3:2[2 2 2]");
    ASSERT_TRUE(is2.beyondScore());
    is2.setDuration(DurationType::V_WHOLE);
    enterRest(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(score->lastMeasure()), "r1");

    delete score;
}

// four quarters, and a quarter triplet in voice 2 from `tick`; then Insert input in voice 1
static MasterScore* voice2TripletInsertScore(const Fraction& tick)
{
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    if (!score) {
        return nullptr;
    }
    InputState& is = score->inputState();
    for (int pitch : { 60, 62, 64, 65 }) {
        enterNote(score, pitch);
    }
    is.setTrack(1);
    is.setSegment(score->tick2segment(tick, false, SegmentType::ChordRest));
    if (!waitForTuplet(score, true)) {
        delete score;
        return nullptr;
    }
    for (int i = 0; i < 3; ++i) {
        enterNote(score, 55);
    }
    is.setTrack(0);
    is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    return score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsertOtherVoiceTuplet)
{
    // a quarter triplet on beats 1-2 in voice 2: time inserted on beat 2 would split it
    MasterScore* score = voice2TripletInsertScore(Fraction(0, 1));
    ASSERT_TRUE(score);
    ASSERT_TRUE(score->sanityCheck());
    InputState& is = score->inputState();
    selectForInsert(score, Fraction(1, 4));
    is.setDuration(DurationType::V_EIGHTH);
    EXPECT_FALSE(waitForTuplet(score, true));
    EXPECT_FALSE(is.pendingTuplet());
    EXPECT_EQ(MScore::_error, MsError::CANNOT_INSERT_TUPLET_IN_TUPLET);
    MScore::setError(MsError::MS_NO_ERROR);

    // on beat 1, where the triplet starts, the new tuplet goes before it
    selectForInsert(score, Fraction(0, 1));
    ASSERT_TRUE(waitForTuplet(score, true));
    enterLetter(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 r8 r8] 4 4 4 4");
    EXPECT_EQ(rhythm(measureAt(score, 0), 1), "r4 3:2[4 4 4] r2");

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsertWhereOtherVoiceTupletStarts)
{
    // a quarter triplet on beats 2-3 in voice 2; a triplet inserted on beat 2 moves it to start at the input position,
    // where the next waiting tuplet is inserted before it too
    MasterScore* score = voice2TripletInsertScore(Fraction(1, 4));
    ASSERT_TRUE(score);
    ASSERT_TRUE(score->sanityCheck());
    InputState& is = score->inputState();
    selectForInsert(score, Fraction(1, 4));
    is.setDuration(DurationType::V_EIGHTH);
    for (int round = 0; round < 2; ++round) {
        ASSERT_TRUE(waitForTuplet(score, true));
        for (int i = 0; i < 3; ++i) {
            enterLetter(score);
        }
    }
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 3:2[8 8 8] 3:2[8 8 8] 4 4 4");
    EXPECT_EQ(rhythm(measureAt(score, 0), 1), "r4 r4 r4 3:2[4 4 4] r4");
    const Segment* segment = score->tick2segment(Fraction(3, 4), false, SegmentType::ChordRest);
    const ChordRest* first = segment ? toChordRest(segment->element(1)) : nullptr;
    ASSERT_TRUE(first && first->tuplet());
    EXPECT_EQ(first->tuplet()->tick(), Fraction(3, 4));

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameInsertOtherVoiceTuplet)
{
    // a quarter triplet on beats 1-2 in voice 2: Ctrl+Shift+letter on beat 2 in By note name input would insert the
    // waiting triplet's time inside it, so the note is refused, and the triplet keeps waiting
    MasterScore* score = voice2TripletInsertScore(Fraction(0, 1));
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setNoteEntryMethod(NoteEntryMethod::BY_NOTE_NAME);
    selectForInsert(score, Fraction(1, 4));
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    enterLetter(score, true);
    EXPECT_EQ(MScore::_error, MsError::CANNOT_INSERT_TUPLET_IN_TUPLET);
    MScore::setError(MsError::MS_NO_ERROR);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 4 4 4");
    EXPECT_EQ(rhythm(measureAt(score, 0), 1), "3:2[4 4 4] r2");
    EXPECT_TRUE(is.pendingTupletHere());

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsertClickElsewhere)
{
    // a click elsewhere inserts a note there, as without a waiting tuplet, and cancels the wait: before D, with a
    // quarter triplet waiting before E (the input position comes back to beat 3, now before D); on F, with an eighth
    // triplet waiting past the end of the score (no measure is appended)
    for (bool beyondScore : { false, true }) {
        SCOPED_TRACE(beyondScore);
        MasterScore* score = beyondScore ? pendingTupletInsertBeyondScoreScore() : pendingTupletInsertScore(Fraction(1, 2));
        ASSERT_TRUE(score);
        InputState& is = score->inputState();
        const int measures = score->nmeasures();
        Measure* measure = beyondScore ? score->lastMeasure() : measureAt(score, 0);
        is.setDuration(beyondScore ? DurationType::V_EIGHTH : DurationType::V_QUARTER);
        ASSERT_TRUE(waitForTuplet(score, true));
        Position pos;
        pos.segment = chordRestAt(measure, beyondScore ? 3 : 1)->segment();
        pos.staffIdx = 0;
        pos.line = 4;
        score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
            NoteInput::insertChordByInsertingTime(tx, score, pos);
        });
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(score->nmeasures(), measures);
        EXPECT_EQ(rhythm(measure), beyondScore ? "4 4 4 8 4" : "4 4 4 4 4");
        if (!beyondScore) {
            EXPECT_EQ(is.tick(), Fraction(1, 2));
        }
        EXPECT_FALSE(is.pendingTuplet());

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletInsertInPart)
{
    // entered in the score or in a part: the tuplet's time is inserted in both
    for (bool inPart : { false, true }) {
        MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
        ASSERT_TRUE(score);
        score->startCmd(TranslatableString::untranslatable("Pending tuplet tests"));
        Score* part = score->createScore();
        Excerpt* excerpt = new Excerpt(score);
        excerpt->setExcerptScore(part);
        excerpt->setName(u"Piano");
        excerpt->setParts({ score->parts().at(0) });
        Excerpt::createExcerpt(excerpt);
        score->undo(new AddExcerpt(excerpt));
        score->endCmd();

        Score* s = inPart ? part : static_cast<Score*>(score);
        InputState& is = s->inputState();
        is.setTrack(0);
        is.setSegment(s->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
        is.setDuration(DurationType::V_QUARTER);
        is.setNotes({ NoteVal(60) });
        is.setNoteEntryMode(true);
        is.setNoteEntryMethod(NoteEntryMethod::BY_NOTE_NAME);
        for (int pitch : { 60, 62, 64, 65 }) {
            enterNote(s, pitch);
        }
        is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
        selectForInsert(s, Fraction(1, 4));

        is.setDuration(DurationType::V_EIGHTH);
        ASSERT_TRUE(waitForTuplet(s, true));
        for (int i = 0; i < 3; ++i) {
            enterLetter(s);
        }
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(score->firstMeasure()), "4 3:2[8 8 8] 4 4 4");
        EXPECT_EQ(rhythm(part->firstMeasure()), "4 3:2[8 8 8] 4 4 4");

        delete score;
    }
}

// a guitar staff with a linked TAB staff, in By note name input at the start of the score
static MasterScore* guitarLinkedTabScore()
{
    MasterScore* score = ScoreRW::readScore(TUPLET_DATA_DIR + u"empty.mscx");
    if (!score) {
        return nullptr;
    }
    int strings[] = { 40, 45, 50, 55, 59, 64 };
    score->staff(0)->part()->instrument()->setStringData(StringData(24, 6, strings));
    addSecondStaff(score, /*linked*/ true);
    score->staff(1)->setStaffType(Fraction(0, 1), *StaffType::preset(StaffTypes::TAB_6COMMON));

    InputState& is = score->inputState();
    is.setTrack(0);
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    is.setNoteEntryMode(true);
    is.setNoteEntryMethod(NoteEntryMethod::BY_NOTE_NAME);
    return score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsertLinkedTab)
{
    // a guitar staff with a linked TAB staff: the tuplet is on both
    MasterScore* score = guitarLinkedTabScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setDuration(DurationType::V_QUARTER);
    for (int pitch : { 64, 65, 67, 69 }) {
        enterNote(score, pitch);
    }
    is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    selectForInsert(score, Fraction(1, 4));

    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    for (int i = 0; i < 3; ++i) {
        enterLetter(score);
    }
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(score->firstMeasure()), "4 3:2[8 8 8] 4 4 4");
    EXPECT_EQ(rhythm(score->firstMeasure(), staff2track(1)), "4 3:2[8 8 8] 4 4 4");

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameDoesNotFitTabFret)
{
    // a fret on the linked tablature staff, where the waiting tuplet doesn't fit: refused on both staves
    MasterScore* score = guitarLinkedTabScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setDuration(DurationType::V_WHOLE);
    enterNote(score, 46);
    is.setTrack(staff2track(1));
    is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    is.setString(2);
    // on the whole note in the first measure, a half quintuplet is longer than the measure
    is.setDuration(DurationType::V_HALF);
    ASSERT_TRUE(waitForTuplet(score, true, Fraction(5, 4)));
    EXPECT_FALSE(NoteInput::pendingTupletFitsHere(score));
    addFret(score, 5);
    EXPECT_EQ(MScore::_error, MsError::NO_SPACE_FOR_TUPLET);
    MScore::setError(MsError::MS_NO_ERROR);
    EXPECT_TRUE(score->sanityCheck());
    // the whole note already there
    EXPECT_EQ(rhythm(score->firstMeasure()), "1");
    EXPECT_EQ(rhythm(score->firstMeasure(), staff2track(1)), "1");
    EXPECT_TRUE(is.pendingTupletHere());

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsertEmptyBar)
{
    // in a bar with a measure rest, the bar grows by the tuplet's time
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    selectForInsert(score, Fraction(0, 1));

    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    enterLetter(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[8 r8 r8] r4 r2 r4");
    EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(5, 4));

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsertEmptyVoice)
{
    // in an empty voice 2: voice 1 gets a rest for the inserted time
    MasterScore* score = pendingTupletInsertScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    // after selecting, which moves the input position to the selected voice
    is.setTrack(1);

    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    enterLetter(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 r4 4 4 4");
    EXPECT_EQ(rhythm(measureAt(score, 0), 1), "r4 3:2[8 r8 r8] r2 r4");
    EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(5, 4));

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsertDrums)
{
    // a drum, by its shortcut or with the mouse, takes the tuplet, also to its own voice: there, the other voice gets a
    // rest for the inserted time
    const std::pair<bool, int> cases[] = { { false, 38 }, { false, 36 }, { true, 38 }, { true, 36 } };
    for (const auto& [click, drumPitch] : cases) {
        SCOPED_TRACE((click ? "click, drum " : "shortcut, drum ") + std::to_string(drumPitch));
        MasterScore* score = drumScore();
        ASSERT_TRUE(score);
        InputState& is = score->inputState();
        is.setDuration(DurationType::V_QUARTER);
        for (int i = 0; i < 4; ++i) {
            enterDrum(score, 38);
        }
        is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
        selectForInsert(score, Fraction(1, 4));

        is.setDuration(DurationType::V_EIGHTH);
        ASSERT_TRUE(waitForTuplet(score, true));
        if (click) {
            is.setDrumNote(drumPitch);
            Position pos;
            pos.segment = is.segment();
            pos.staffIdx = 0;
            pos.line = is.drumset()->line(drumPitch);
            score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
                NoteInput::insertChordByInsertingTime(tx, score, pos);
            });
        } else {
            enterDrum(score, drumPitch);
        }
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_FALSE(is.pendingTuplet());
        if (drumPitch == 38) {
            EXPECT_EQ(rhythm(measureAt(score, 0)), "4 3:2[8 r8 r8] 4 4 4");
        } else {
            EXPECT_EQ(rhythm(measureAt(score, 0)), "4 r4 4 4 4");
            EXPECT_EQ(rhythm(measureAt(score, 0), 1), "r4 3:2[8 r8 r8] r2 r4");
        }
        EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(5, 4));

        delete score;
    }
}

// snares in voice 1, bass drums in voice 2, in quarters; in Insert input, an eighth triplet inserted on beat 2 with a
// snare, the input position on its next rest
static MasterScore* insertedSnareTripletScore()
{
    MasterScore* score = drumScore();
    if (!score) {
        return nullptr;
    }
    InputState& is = score->inputState();
    is.setDuration(DurationType::V_QUARTER);
    for (int drumPitch : { 38, 36 }) {
        is.setTrack(0);
        is.setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
        for (int i = 0; i < 4; ++i) {
            enterDrum(score, drumPitch);
        }
    }
    is.setTrack(0);
    is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    selectForInsert(score, Fraction(1, 4));
    is.setDuration(DurationType::V_EIGHTH);
    if (!waitForTuplet(score, true)) {
        delete score;
        return nullptr;
    }
    enterDrum(score, 38);
    return score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsertDrumOtherVoice)
{
    // in voice 2 the inserted time is a plain rest: a bass drum there isn't capped at the voice 1 triplet's end
    // by MIDI, on the triplet's first snare, a half-note bass drum overwrites, as MIDI input does outside an inserted
    // tuplet
    MasterScore* score = insertedSnareTripletScore();
    ASSERT_TRUE(score);
    InputState* is = &score->inputState();
    is->setSegment(score->tick2segment(Fraction(1, 4), false, SegmentType::ChordRest));
    is->setDuration(DurationType::V_HALF);
    enterNote(score, 36);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 3:2[8 r8 r8] 4 4 4");
    EXPECT_EQ(rhythm(measureAt(score, 0), 1), "4 2 4 4");
    delete score;

    // a click on the triplet's next rest: the bass drum would insert time inside the triplet, so it's refused; a snare
    // click fills the rest
    score = insertedSnareTripletScore();
    ASSERT_TRUE(score);
    is = &score->inputState();
    ASSERT_EQ(is->tick(), Fraction(1, 3));
    const auto clickDrum = [&](int drumPitch) {
        is->setDrumNote(drumPitch);
        Position pos;
        pos.segment = is->segment();
        pos.staffIdx = 0;
        pos.line = is->drumset()->line(drumPitch);
        score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
            NoteInput::insertChordByInsertingTime(tx, score, pos);
        });
    };
    is->setDuration(DurationType::V_HALF);
    clickDrum(36);
    EXPECT_EQ(MScore::_error, MsError::CANNOT_INSERT_TUPLET);
    MScore::setError(MsError::MS_NO_ERROR);
    EXPECT_EQ(rhythm(measureAt(score, 0), 1), "4 r4 4 4 4");
    is->setDuration(DurationType::V_EIGHTH);
    clickDrum(38);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 3:2[8 8 r8] 4 4 4");
    EXPECT_EQ(rhythm(measureAt(score, 0), 1), "4 r4 4 4 4");
    delete score;

    // by MIDI, an eighth bass drum on beat 2, then from its end (inside the triplet's rest) a half-note snare: it goes
    // in the triplet and stops at its end, leaving the snares after it
    score = insertedSnareTripletScore();
    ASSERT_TRUE(score);
    is = &score->inputState();
    is->setSegment(score->tick2segment(Fraction(1, 4), false, SegmentType::ChordRest));
    enterNote(score, 36);
    ASSERT_EQ(is->tick(), Fraction(3, 8));
    is->setDuration(DurationType::V_HALF);
    enterNote(score, 38);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(5, 4));
    for (const Fraction& tick : { Fraction(1, 2), Fraction(3, 4), Fraction(1, 1) }) {
        const Segment* segment = score->tick2segment(tick, false, SegmentType::ChordRest);
        const ChordRest* snare = segment ? toChordRest(segment->element(0)) : nullptr;
        ASSERT_TRUE(snare && snare->isChord());
        EXPECT_EQ(snare->ticks(), Fraction(1, 4));
    }
    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsertDotAfterCommand)
{
    // eighth, triplet, then a dot: the first note is dotted, in the eighth triplet, whose quarter is inserted
    for (int dots : { 1, 2 }) {
        MasterScore* score = pendingTupletInsertScore();
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        is.setDuration(DurationType::V_EIGHTH);
        ASSERT_TRUE(waitForTuplet(score, true));
        pressDot(score, dots);
        EXPECT_TRUE(is.pendingTupletHere());
        enterLetter(score);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), dots == 1 ? "4 3:2[8. r16 r8] 4 4 4" : "4 3:2[8.. r32 r8] 4 4 4");
        EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(5, 4));

        // the next note fills the triplet's rest
        pressDuration(score, dots == 1 ? DurationType::V_16TH : DurationType::V_32ND);
        enterLetter(score);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), dots == 1 ? "4 3:2[8. 16 r8] 4 4 4" : "4 3:2[8.. 32 r8] 4 4 4");
        EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(5, 4));

        delete score;
    }
}

//-----------------------------------------
//    tupletPreview in By note name and Insert input
//     the tuplet waiting for a note, whose length is known
//-----------------------------------------

TEST_F(Engraving_TupletTests, tupletPreviewPendingNoteName)
{
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();

    // in By note name input, an eighth triplet: its length is known while it waits
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    std::optional<TupletPreview> preview = NoteInput::tupletPreview(score);
    ASSERT_TRUE(preview);
    ASSERT_EQ(preview->levels.size(), 1u);
    EXPECT_EQ(preview->levels.back().ticks, Fraction(1, 4));
    EXPECT_EQ(preview->space, Fraction(1, 1));
    EXPECT_TRUE(preview->fits);
    EXPECT_FALSE(preview->waiting);

    // another duration is for the first note: the tuplet stays the same
    is.setDuration(DurationType::V_HALF);
    preview = NoteInput::tupletPreview(score);
    ASSERT_TRUE(preview);
    EXPECT_EQ(preview->levels.back().ticks, Fraction(1, 4));

    // after a half and a quarter, a quarter triplet would cross the barline
    is.setPendingTuplet(std::nullopt);
    enterNote(score);
    is.setDuration(DurationType::V_QUARTER);
    enterNote(score);
    ASSERT_TRUE(waitForTuplet(score, true));
    preview = NoteInput::tupletPreview(score);
    ASSERT_TRUE(preview);
    EXPECT_EQ(preview->tick, Fraction(3, 4));
    EXPECT_EQ(preview->levels.back().ticks, Fraction(1, 2));
    EXPECT_EQ(preview->space, Fraction(1, 4));
    EXPECT_FALSE(preview->fits);

    // an eighth triplet fits
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    preview = NoteInput::tupletPreview(score);
    ASSERT_TRUE(preview);
    EXPECT_TRUE(preview->fits);

    delete score;
}

TEST_F(Engraving_TupletTests, tupletPreviewPendingInsert)
{
    // in Insert input, its time is inserted, so it fits although it runs past the barline
    MasterScore* score = pendingTupletInsertScore();
    ASSERT_TRUE(score);
    score->inputState().setDuration(DurationType::V_HALF);
    ASSERT_TRUE(waitForTuplet(score, true));
    std::optional<TupletPreview> preview = NoteInput::tupletPreview(score);
    ASSERT_TRUE(preview);
    EXPECT_TRUE(preview->fits);
    EXPECT_EQ(preview->levels.back().ticks, Fraction(1, 1));
    delete score;

    // past the end of the score, it goes in the measure that the next note appends
    score = pendingTupletInsertBeyondScoreScore();
    ASSERT_TRUE(score);
    score->inputState().setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    preview = NoteInput::tupletPreview(score);
    ASSERT_TRUE(preview);
    EXPECT_EQ(preview->tick, score->endTick());
    delete score;

    // where a quarter triplet starts, its time goes in before the triplet, outside it
    score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    enterNote(score);
    ASSERT_TRUE(waitForTuplet(score, true));
    for (int i = 0; i < 4; ++i) {
        enterNote(score);
    }
    ASSERT_EQ(rhythm(measureAt(score, 0)), "4 3:2[4 4 4] 4");
    is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    selectForInsert(score, Fraction(1, 4));
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    preview = NoteInput::tupletPreview(score);
    ASSERT_TRUE(preview);
    EXPECT_EQ(preview->levels.back().ticks, Fraction(1, 4));
    EXPECT_EQ(preview->space, Fraction(3, 4));
    delete score;
}

//-----------------------------------------
//    tuplets nested in a waiting tuplet in By note name
//     input: a tuplet command with a shorter duration than
//     the waiting tuplet's note value nests a tuplet in its
//     first note
//-----------------------------------------

// quarter, triplet, eighth, triplet: an eighth triplet in the first quarter of a quarter triplet
static MasterScore* nestedPendingTupletScore(const TDuration& outer = TDuration(DurationType::V_QUARTER),
                                             const TDuration& inner = TDuration(DurationType::V_EIGHTH))
{
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    if (!score) {
        return nullptr;
    }
    InputState& is = score->inputState();
    is.setDuration(outer);
    if (!waitForTuplet(score, true)) {
        delete score;
        return nullptr;
    }
    is.setDuration(inner);
    if (!waitForTuplet(score, true)) {
        delete score;
        return nullptr;
    }
    return score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameNested)
{
    // also dotted or double-dotted, both
    const std::string nested[] = {
        "3:2[ 3:2[8 8 8] r4 r4] r2", "3:2[ 3:2[8. 8. 8.] r4. r4.] r4", "3:2[ 3:2[8.. 8.. 8..] r4.. r4..] r8"
    };
    const Fraction afterNested[] = { Fraction(1, 6), Fraction(1, 4), Fraction(7, 24) };
    const std::string outer[] = { "3:2[4 r4 r4] r2", "3:2[4. r4. r4.] r4", "3:2[4.. r4.. r4..] r8" };
    for (int dots : { 0, 1, 2 }) {
        SCOPED_TRACE(dots);
        const TDuration quarter(DurationTypeWithDots(DurationType::V_QUARTER, dots));
        const TDuration eighth(DurationTypeWithDots(DurationType::V_EIGHTH, dots));
        MasterScore* score = nestedPendingTupletScore(quarter, eighth);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();
        ASSERT_TRUE(is.pendingTupletHere());
        EXPECT_EQ(is.pendingTuplet()->nested.size(), 1u);
        EXPECT_TRUE(NoteInput::pendingTupletFitsHere(score));
        EXPECT_EQ(is.duration(), eighth);
        EXPECT_EQ(rhythm(measureAt(score, 0)), "R");

        // C D E: both tuplets are added with the first note
        for (int pitch : { 60, 62, 64 }) {
            enterNote(score, pitch);
        }
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), nested[dots]);
        EXPECT_FALSE(is.pendingTuplet());
        EXPECT_EQ(is.tick(), afterNested[dots]);
        const ChordRest* first = chordRestAt(measureAt(score, 0), 0);
        ASSERT_TRUE(first && first->tuplet() && first->tuplet()->tuplet());
        EXPECT_EQ(first->tuplet()->baseLen().type(), DurationType::V_EIGHTH);
        EXPECT_EQ(first->tuplet()->tuplet()->baseLen().type(), DurationType::V_QUARTER);

        // undoing the first note takes both tuplets away, and both wait again
        for (int i = 0; i < 3; ++i) {
            score->undoRedo(true, nullptr);
        }
        EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
        ASSERT_TRUE(is.pendingTupletHere());
        EXPECT_EQ(is.pendingTuplet()->nested.size(), 1u);

        // cancelling takes only the inner tuplet away; the outer one still waits, with its note value, and is
        // entered as if the inner one never waited
        NoteInput::cancelPendingTuplet(score);
        ASSERT_TRUE(is.pendingTupletHere());
        EXPECT_TRUE(is.pendingTuplet()->nested.empty());
        EXPECT_EQ(is.duration(), quarter);
        std::optional<TupletPreview> preview = NoteInput::tupletPreview(score);
        ASSERT_TRUE(preview);
        EXPECT_EQ(preview->levels.size(), 1u);
        enterNote(score);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), outer[dots]);

        // undone, it waits again; cancelling again takes it away
        score->undoRedo(true, nullptr);
        ASSERT_TRUE(is.pendingTupletHere());
        NoteInput::cancelPendingTuplet(score);
        EXPECT_FALSE(is.pendingTuplet());
        EXPECT_EQ(rhythm(measureAt(score, 0)), "R");

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameNestedCancel)
{
    // Tuplet length: a sixteenth triplet nested in the first eighth of a quarter triplet
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    for (bool allLevels : { false, true }) {
        SCOPED_TRACE(allLevels);
        is.setDuration(DurationType::V_QUARTER);
        ASSERT_TRUE(waitForTuplet(score, false));
        is.setDuration(DurationType::V_16TH);
        ASSERT_TRUE(waitForTuplet(score, false));
        ASSERT_EQ(is.pendingTuplet()->nested.size(), 1u);
        EXPECT_EQ(is.duration().type(), DurationType::V_32ND);

        // one level at a time, the outer tuplet's note value, then the quarter that sized it; or all at once, as when
        // the input position moves away
        if (!allLevels) {
            NoteInput::cancelPendingTuplet(score);
            EXPECT_EQ(is.duration().type(), DurationType::V_EIGHTH);
        }
        NoteInput::cancelPendingTuplet(score, allLevels);
        EXPECT_FALSE(is.pendingTuplet());
        EXPECT_EQ(is.duration().type(), DurationType::V_QUARTER);
    }

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameNestedUndoAfterEscape)
{
    // Escape cancels the nested triplet, and C takes up the outer one: undo brings the outer one back, waiting
    MasterScore* score = nestedPendingTupletScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.forgetPendingTuplets();
    NoteInput::cancelPendingTuplet(score);
    ASSERT_TRUE(is.pendingTupletHere());
    ASSERT_TRUE(is.pendingTuplet()->nested.empty());
    enterNote(score);
    ASSERT_EQ(rhythm(measureAt(score, 0)), "3:2[4 r4 r4] r2");

    score->undoRedo(true, nullptr);
    EXPECT_EQ(rhythm(measureAt(score, 0)), "R");
    EXPECT_TRUE(is.pendingTupletHere());
    EXPECT_TRUE(is.pendingTuplet() && is.pendingTuplet()->nested.empty());

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameInsertNestedLongNote)
{
    // an eighth triplet waiting in the first note of a quarter triplet, on D: Ctrl+Shift+letter with a whole note
    // inserts both and fills them, and stops at the end of the outer one, leaving D
    MasterScore* score = pendingTupletInsertScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setNoteEntryMethod(NoteEntryMethod::BY_NOTE_NAME);
    selectForInsert(score, Fraction(1, 4));
    is.setDuration(DurationType::V_QUARTER);
    ASSERT_TRUE(waitForTuplet(score, true));
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    ASSERT_EQ(is.pendingTuplet()->nested.size(), 1u);
    is.setDuration(DurationType::V_WHOLE);
    enterLetter(score, true);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 3:2[ 3:2[4.~] 2] 4 4 4");
    const ChordRest* d = chordRestAt(measureAt(score, 0), 3);
    ASSERT_TRUE(d && d->isChord());
    EXPECT_EQ(toChord(d)->upNote()->pitch(), 62);

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameNestedAgain)
{
    // with two tuplets waiting, a tuplet command that isn't shorter replaces the inner one only; a shorter one nests a third
    struct Case {
        DurationType changeTo;
        Fraction ratio;
        bool fits;
        size_t nested;
        std::string expected;
    };
    const std::vector<Case> cases {
        // quintuplet straight away: an eighth quintuplet in the quarter triplet
        { DurationType::V_INVALID, Fraction(5, 4), true, 1u, "3:2[ 5:4[8 r8 r8 r8 r8] r4] r2" },
        // quarter, triplet: a quarter triplet in the quarter triplet
        { DurationType::V_QUARTER, Fraction(3, 2), true, 1u, "3:2[ 3:2[4 r4 r4] r4] r2" },
        // half, triplet: a half triplet doesn't fit in the quarter triplet
        { DurationType::V_HALF, Fraction(3, 2), false, 1u, "R" },
        // 16th, triplet: a 16th triplet in the eighth triplet
        { DurationType::V_16TH, Fraction(3, 2), true, 2u, "3:2[ 3:2[ 3:2[16 r16 r16] r8 r8] r4 r4] r2" },
    };
    for (const Case& c : cases) {
        SCOPED_TRACE(c.expected);
        MasterScore* score = nestedPendingTupletScore();
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        if (c.changeTo != DurationType::V_INVALID) {
            pressDuration(score, c.changeTo);
        }
        ASSERT_TRUE(waitForTuplet(score, true, c.ratio));
        ASSERT_TRUE(is.pendingTupletHere());
        EXPECT_EQ(is.pendingTuplet()->nested.size(), c.nested);
        EXPECT_EQ(NoteInput::pendingTupletFitsHere(score), c.fits);
        enterNote(score);
        EXPECT_EQ(MScore::_error, c.fits ? MsError::MS_NO_ERROR : MsError::NO_SPACE_FOR_TUPLET);
        MScore::setError(MsError::MS_NO_ERROR);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), c.expected);

        if (!c.fits) {
            // an eighth and the tuplet command again replace the inner one with one that fits
            pressDuration(score, DurationType::V_EIGHTH);
            ASSERT_TRUE(waitForTuplet(score, true));
            EXPECT_EQ(is.pendingTuplet()->nested.size(), 1u);
            EXPECT_TRUE(NoteInput::pendingTupletFitsHere(score));
            enterNote(score);
            EXPECT_EQ(rhythm(measureAt(score, 0)), "3:2[ 3:2[8 r8 r8] r4 r4] r2");
        }

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameNestedShorterOnly)
{
    // the second tuplet command nests a tuplet only with a shorter duration: divided, quarter, triplet (three eighths),
    // 16th, triplet (three 32nds in the time of a 16th) nest; a dotted eighth isn't shorter than an eighth, nor a
    // double-dotted eighth than a dotted one, so it replaces the first
    struct Case {
        DurationTypeWithDots first;
        DurationTypeWithDots second;
        bool durationAsSubdivision;
        size_t nested;
        DurationTypeWithDots inputDuration;
        std::string expected;
    };
    const std::vector<Case> cases {
        { DurationType::V_QUARTER, DurationType::V_16TH, false, 1u, DurationType::V_32ND, "3:2[ 3:2[32 r32 r32] r16 r8 r8] r4 r2" },
        { DurationType::V_EIGHTH, { DurationType::V_EIGHTH, 1 }, true, 0u, { DurationType::V_EIGHTH, 1 }, "3:2[8. r8. r8.] r8 r2" },
        { { DurationType::V_EIGHTH, 1 }, { DurationType::V_EIGHTH, 2 }, true, 0u, { DurationType::V_EIGHTH, 2 },
            "3:2[8.. r8.. r8..] r16 r2" },
    };
    for (const Case& c : cases) {
        SCOPED_TRACE(c.expected);
        MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();

        is.setDuration(c.first);
        ASSERT_TRUE(waitForTuplet(score, c.durationAsSubdivision));
        is.setDuration(c.second);
        ASSERT_TRUE(waitForTuplet(score, c.durationAsSubdivision));
        EXPECT_EQ(is.pendingTuplet()->nested.size(), c.nested);
        EXPECT_EQ(is.duration(), TDuration(c.inputDuration));
        enterNote(score);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), c.expected);

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameNestedOuterDoesNotFit)
{
    // a quarter triplet on beat 4 doesn't fit: a shorter duration and the tuplet command replace it, rather than nest
    MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setDuration(DurationType::V_HALF);
    enterNote(score);
    is.setDuration(DurationType::V_QUARTER);
    enterNote(score);
    ASSERT_TRUE(waitForTuplet(score, true));
    EXPECT_FALSE(NoteInput::pendingTupletFitsHere(score));
    pressDuration(score, DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    EXPECT_TRUE(is.pendingTuplet()->nested.empty());
    enterNote(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "2 4 3:2[8 r8 r8]");

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameNestedEntryPaths)
{
    // after a quarter C, two tuplets waiting on beat 2 take a note entered each way
    const std::vector<std::pair<std::function<void(Score*)>, std::string> > paths {
        // letter
        { [](Score* score) { enterLetter(score); }, "4 3:2[ 3:2[8 r8 r8] r4 r4] r4" },
        // MIDI
        { [](Score* score) { enterNote(score); }, "4 3:2[ 3:2[8 r8 r8] r4 r4] r4" },
        // mouse, as for a tablature fret
        { [](Score* score) { clickNote(score, Fraction(1, 4), 4); }, "4 3:2[ 3:2[8 r8 r8] r4 r4] r4" },
        // rest
        { [](Score* score) { enterRest(score); }, "4 3:2[ 3:2[r8 r8 r8] r4 r4] r4" },
        // R
        { [](Score* score) { repeatChord(score, toChord(chordRestAt(measureAt(score, 0), 0))); }, "4 3:2[ 3:2[8 r8 r8] r4 r4] r4" },
        // tie
        { [](Score* score) { EditTie::cmdAddTie(score); }, "4~ 3:2[ 3:2[8 r8 r8] r4 r4] r4" },
        // Ctrl+Shift+letter: the outer tuplet's time is inserted
        { [](Score* score) { enterLetter(score, true); }, "4 3:2[ 3:2[8 r8 r8] r4 r4] r4 r2" },
    };
    for (size_t i = 0; i < paths.size(); ++i) {
        SCOPED_TRACE(i);
        MasterScore* score = pendingTupletScore(NoteEntryMethod::BY_NOTE_NAME);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();
        enterNote(score);
        // Ctrl+Shift+letter needs a selected note
        score->select(toChord(chordRestAt(measureAt(score, 0), 0))->upNote());
        is.setSegment(score->tick2segment(Fraction(1, 4), false, SegmentType::ChordRest));
        ASSERT_TRUE(waitForTuplet(score, true));
        is.setDuration(DurationType::V_EIGHTH);
        ASSERT_TRUE(waitForTuplet(score, true));
        ASSERT_EQ(is.pendingTuplet()->nested.size(), 1u);

        paths[i].first(score);
        EXPECT_EQ(MScore::_error, MsError::MS_NO_ERROR);
        MScore::setError(MsError::MS_NO_ERROR);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), paths[i].second);
        EXPECT_FALSE(is.pendingTuplet());

        delete score;
    }
}

// C D E F, and in Insert input on D, quarter, triplet, eighth, triplet: an eighth triplet waits in the first note of a
// quarter triplet
static MasterScore* nestedPendingTupletInsertScore()
{
    MasterScore* score = pendingTupletInsertScore();
    if (!score) {
        return nullptr;
    }
    InputState& is = score->inputState();
    is.setDuration(DurationType::V_QUARTER);
    if (!waitForTuplet(score, true)) {
        delete score;
        return nullptr;
    }
    is.setDuration(DurationType::V_EIGHTH);
    if (!waitForTuplet(score, true)) {
        delete score;
        return nullptr;
    }
    return score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsertNestedEntryPaths)
{
    // a note entered by letter, by MIDI or with the mouse, a rest, the repeat key or the tie key: each inserts the outer
    // triplet's time, and goes in the inner one
    const std::vector<std::pair<std::function<void(Score*)>, std::string> > paths {
        { [](Score* score) { enterLetter(score); }, "4 3:2[ 3:2[8 r8 r8] r4 r4] 4 4 4" },
        { [](Score* score) { enterNote(score); }, "4 3:2[ 3:2[8 r8 r8] r4 r4] 4 4 4" },
        { [](Score* score) {
                Position pos;
                pos.segment = score->inputState().segment();
                pos.staffIdx = 0;
                pos.line = 4;
                score->transactionManager()->transaction(TranslatableString::untranslatable("Pending tuplet tests"), [&](Transaction& tx) {
                    NoteInput::insertChordByInsertingTime(tx, score, pos);
                });
            }, "4 3:2[ 3:2[8 r8 r8] r4 r4] 4 4 4" },
        { [](Score* score) { enterRest(score); }, "4 3:2[ 3:2[r8 r8 r8] r4 r4] 4 4 4" },
        { [](Score* score) { repeatChord(score, toChord(chordRestAt(measureAt(score, 0), 0))); }, "4 3:2[ 3:2[8 r8 r8] r4 r4] 4 4 4" },
        { [](Score* score) { EditTie::cmdAddTie(score); }, "4~ 3:2[ 3:2[8 r8 r8] r4 r4] 4 4 4" },
    };
    for (size_t i = 0; i < paths.size(); ++i) {
        SCOPED_TRACE(i);
        MasterScore* score = nestedPendingTupletInsertScore();
        ASSERT_TRUE(score);
        InputState& is = score->inputState();
        ASSERT_EQ(is.pendingTuplet()->nested.size(), 1u);
        paths[i].first(score);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_EQ(rhythm(measureAt(score, 0)), paths[i].second);
        EXPECT_EQ(measureAt(score, 0)->ticks(), Fraction(6, 4));
        EXPECT_FALSE(is.pendingTuplet());

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletInsertNested)
{
    // the preview shows both levels
    MasterScore* score = nestedPendingTupletInsertScore();
    ASSERT_TRUE(score);
    std::optional<TupletPreview> preview = NoteInput::tupletPreview(score);
    ASSERT_TRUE(preview);
    ASSERT_EQ(preview->levels.size(), 2u);
    EXPECT_EQ(preview->levels[0].ticks, Fraction(1, 2));
    EXPECT_EQ(preview->levels[1].ticks, Fraction(1, 6));
    EXPECT_TRUE(preview->fits);
    delete score;

    // past the end of the score, the next note appends a measure, which both start
    score = pendingTupletInsertBeyondScoreScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    const int measures = score->nmeasures();
    is.setDuration(DurationType::V_QUARTER);
    ASSERT_TRUE(waitForTuplet(score, true));
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    ASSERT_EQ(is.pendingTuplet()->nested.size(), 1u);
    enterLetter(score);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(score->nmeasures(), measures + 1);
    EXPECT_EQ(rhythm(score->lastMeasure()), "3:2[ 3:2[8 r8 r8] r4 r4] r2");
    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletInsertNestedDrums)
{
    // in Insert input, nested triplets wait on the second snare; a bass drum takes both to voice 2
    MasterScore* score = drumScore();
    ASSERT_TRUE(score);
    InputState& is = score->inputState();
    is.setDuration(DurationType::V_QUARTER);
    for (int i = 0; i < 4; ++i) {
        enterDrum(score, 38);
    }
    is.setNoteEntryMethod(NoteEntryMethod::TIMEWISE);
    selectForInsert(score, Fraction(1, 4));
    ASSERT_TRUE(waitForTuplet(score, true));
    is.setDuration(DurationType::V_EIGHTH);
    ASSERT_TRUE(waitForTuplet(score, true));
    ASSERT_EQ(is.pendingTuplet()->nested.size(), 1u);
    enterDrum(score, 36);
    EXPECT_TRUE(score->sanityCheck());
    EXPECT_EQ(rhythm(measureAt(score, 0)), "4 r4 r4 4 4 4");
    EXPECT_EQ(rhythm(measureAt(score, 0), 1), "r4 3:2[ 3:2[8 r8 r8] r4 r4] r4 r2");
    EXPECT_FALSE(is.pendingTuplet());

    delete score;
}

TEST_F(Engraving_TupletTests, pendingTupletDurationNeverNests)
{
    // in By duration and Rhythm input, a tuplet command with a shorter duration replaces the waiting tuplet
    for (NoteEntryMethod method : { NoteEntryMethod::BY_DURATION, NoteEntryMethod::RHYTHM }) {
        SCOPED_TRACE(static_cast<int>(method));
        MasterScore* score = pendingTupletScore(method);
        ASSERT_TRUE(score);
        InputState& is = score->inputState();
        is.setDuration(DurationType::V_QUARTER);
        ASSERT_TRUE(waitForTuplet(score, true));
        is.setDuration(DurationType::V_EIGHTH);
        ASSERT_TRUE(waitForTuplet(score, true));
        EXPECT_TRUE(is.pendingTuplet()->nested.empty());

        delete score;
    }
}

TEST_F(Engraving_TupletTests, pendingTupletNoteNameNestedDrums)
{
    // a snare (voice 1) takes both tuplets; a bass drum takes them to its own voice
    for (int drumPitch : { 38, 36 }) {
        MasterScore* score = drumScore();
        ASSERT_TRUE(score);
        InputState& is = score->inputState();
        is.setDuration(DurationType::V_QUARTER);
        ASSERT_TRUE(waitForTuplet(score, true));
        is.setDuration(DurationType::V_EIGHTH);
        ASSERT_TRUE(waitForTuplet(score, true));
        enterDrum(score, drumPitch);
        EXPECT_TRUE(score->sanityCheck());
        EXPECT_FALSE(is.pendingTuplet());
        const std::string nested = "3:2[ 3:2[8 r8 r8] r4 r4] r2";
        EXPECT_EQ(rhythm(measureAt(score, 0)), drumPitch == 38 ? nested : "R");
        if (drumPitch == 36) {
            EXPECT_EQ(rhythm(measureAt(score, 0), 1), nested);
        }

        delete score;
    }
}

TEST_F(Engraving_TupletTests, tupletPreviewPendingNested)
{
    // the eighth triplet waiting in the quarter triplet: two levels, from the outermost in, in score time
    MasterScore* score = nestedPendingTupletScore();
    ASSERT_TRUE(score);
    std::optional<TupletPreview> preview = NoteInput::tupletPreview(score);
    ASSERT_TRUE(preview);
    ASSERT_EQ(preview->levels.size(), 2u);
    EXPECT_EQ(preview->levels[0].ticks, Fraction(1, 2));
    EXPECT_EQ(preview->levels[1].ticks, Fraction(1, 6));
    EXPECT_EQ(preview->levels[1].ratio, Fraction(3, 2));
    EXPECT_TRUE(preview->fits);

    // a half triplet doesn't fit in it
    pressDuration(score, DurationType::V_HALF);
    ASSERT_TRUE(waitForTuplet(score, true));
    preview = NoteInput::tupletPreview(score);
    ASSERT_TRUE(preview);
    ASSERT_EQ(preview->levels.size(), 2u);
    EXPECT_FALSE(preview->fits);

    delete score;
}
