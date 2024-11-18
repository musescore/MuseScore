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

#include <gtest/gtest.h>

#include <QMimeData>

#include "internal/qmimedataadapter.h"

#include "dom/chord.h"
#include "dom/chordrest.h"
#include "dom/durationtype.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/segment.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"
#include "utils/testutils.h"

using namespace mu;
using namespace mu::engraving;

static const String COPYPASTE_DATA_DIR(u"copypaste_data/");

class Engraving_CopyPasteTests : public ::testing::Test
{
public:
    void copypaste(const char*);
    void copypastestaff(const char*);
    void copypastevoice(const char*, int);
    void copypastetuplet(const char*);
    void copypastenote(const String&, Fraction = Fraction(1, 1));
};

//---------------------------------------------------------
//    copy measure 2, paste into measure 4
//---------------------------------------------------------

void Engraving_CopyPasteTests::copypaste(const char* idx)
{
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + String(u"copypaste%1.mscx").arg(String::fromUtf8(idx)));
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();      // src
    Measure* m3 = m2->nextMeasure();
    Measure* m4 = m3->nextMeasure();      // dst

    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);
    EXPECT_TRUE(m3);
    EXPECT_TRUE(m4);

    score->select(m2);
    if (score->nstaves() > 1) {
        score->select(m2, SelectType::RANGE, score->nstaves() - 1);
    }
    EXPECT_TRUE(score->selection().canCopy());
    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    QByteArray ba = score->selection().mimeData().toQByteArray();
    mimeData->setData(mimeType, ba);

    EXPECT_TRUE(m4->first()->element(0));
    score->select(m4->first()->element(0));

    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"copypaste%1.mscx").arg(String::fromUtf8(idx)),
                                            COPYPASTE_DATA_DIR + String(u"copypaste%1-ref.mscx").arg(String::fromUtf8(idx))));
    delete score;
}

TEST_F(Engraving_CopyPasteTests, copypaste01)
{
    copypaste("01");    // start slur
}

TEST_F(Engraving_CopyPasteTests, copypaste02)
{
    copypaste("02");    // end slur
}

TEST_F(Engraving_CopyPasteTests, copypaste03)
{
    copypaste("03");    // slur
}

TEST_F(Engraving_CopyPasteTests, copypaste04)
{
    copypaste("04");    // start tie
}

TEST_F(Engraving_CopyPasteTests, copypaste05)
{
    copypaste("05");    // end tie
}

TEST_F(Engraving_CopyPasteTests, copypaste06)
{
    copypaste("06");    // tie
}

TEST_F(Engraving_CopyPasteTests, DISABLED_copypaste07)
{
    copypaste("07");    // start ottava
}

TEST_F(Engraving_CopyPasteTests, copypaste08)
{
    copypaste("08");    // end ottava
}

TEST_F(Engraving_CopyPasteTests, copypaste09)
{
    copypaste("09");    // ottava
}

TEST_F(Engraving_CopyPasteTests, copypaste10)
{
    copypaste("10");    // two slurs
}

TEST_F(Engraving_CopyPasteTests, copypaste11)
{
    copypaste("11");    // grace notes
}

TEST_F(Engraving_CopyPasteTests, copypaste12)
{
    copypaste("12");    // voices
}

TEST_F(Engraving_CopyPasteTests, copypaste19)
{
    copypaste("19");    // chord symbols
}

TEST_F(Engraving_CopyPasteTests, copypaste22)
{
    copypaste("22");    // cross-staff slur
}

TEST_F(Engraving_CopyPasteTests, copypaste23)
{
    copypaste("23");    // full measure tuplet 10/8
}

TEST_F(Engraving_CopyPasteTests, copypaste24)
{
    copypaste("24");    // more complex non reduced tuplet
}

TEST_F(Engraving_CopyPasteTests, copypaste25)
{
    copypaste("25");    // copy full measure rest
}

TEST_F(Engraving_CopyPasteTests, copypaste26)
{
    copypaste("26");    // Copy chords (#298541)
}

TEST_F(Engraving_CopyPasteTests, copypaste27)
{
    copypaste("27");    // Paste after local time signature (#18940)
}

//---------------------------------------------------------
//    copy measure 2 from first staff, paste into staff 2
//---------------------------------------------------------

void Engraving_CopyPasteTests::copypastevoice(const char* idx, int voice)
{
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + String(u"copypaste%1.mscx").arg(String::fromUtf8(idx)));
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);

    // create a range selection on 2 and 3 beat of first measure
    SegmentType segTypeCR = SegmentType::ChordRest;
    Segment* s = m1->first(segTypeCR)->next1(segTypeCR);
    score->select(toChord(s->element(voice))->notes().at(0));
    s = s->next(SegmentType::ChordRest);
    score->select(s->element(voice), SelectType::RANGE);

    EXPECT_TRUE(score->selection().canCopy());
    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData().toQByteArray());

    //paste to second measure
    score->select(m2->first()->element(0));

    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"copypaste%1.mscx").arg(String::fromUtf8(idx)),
                                            COPYPASTE_DATA_DIR + String(u"copypaste%1-ref.mscx").arg(String::fromUtf8(idx))));
    delete score;
}

TEST_F(Engraving_CopyPasteTests, copypaste2Voice)
{
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + String(u"copypaste13.mscx"));
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);

    // select 2 chord rests at the start of the first measure
    Segment* s = m1->first(SegmentType::ChordRest);
    score->select(toChord(s->element(0))->notes().at(0));
    s = s->next(SegmentType::ChordRest);
    score->select(s->element(0), SelectType::RANGE);

    EXPECT_TRUE(score->selection().canCopy());
    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData().toQByteArray());

    // paste into the second CR of second measure
    Segment* secondCRSeg = m2->first()->next1(SegmentType::ChordRest);
    score->select(secondCRSeg->element(0));

    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"copypaste13.mscx"),
                                            COPYPASTE_DATA_DIR + String(u"copypaste13-ref.mscx")));
    delete score;
}

TEST_F(Engraving_CopyPasteTests, copypaste2Voice5)
{
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + String(u"copypaste17.mscx"));
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    EXPECT_TRUE(m1);

    // create a range selection from 2 eighth note to the end of first measure
    SegmentType segTypeCR = SegmentType::ChordRest;
    Segment* s = m1->first(segTypeCR)->next1(segTypeCR);
    score->select(toChord(s->element(0))->notes().at(0));

    s = m1->last()->prev(SegmentType::ChordRest);
    score->select(s->element(0), SelectType::RANGE);

    EXPECT_TRUE(score->selection().canCopy());
    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData().toQByteArray());

    score->cmdDeleteSelection();   //cut

    //paste to quarter rest
    EngravingItem* dest = m1->first()->next(segTypeCR)->next(segTypeCR)->next(segTypeCR)->element(0);
    EXPECT_TRUE(dest->isRest());
    EXPECT_EQ(static_cast<ChordRest*>(dest)->durationType(), DurationType::V_QUARTER);
    score->select(dest);

    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"copypaste17.mscx"),
                                            COPYPASTE_DATA_DIR + String(u"copypaste17-ref.mscx")));
    delete score;
}

TEST_F(Engraving_CopyPasteTests, copypaste2Voice6)
{
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + String(u"copypaste20.mscx"));
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    EXPECT_TRUE(m1);

    // create a range selection from 2nd eighth note to the end of first measure
    SegmentType segTypeCR = SegmentType::ChordRest;
    Segment* s = m1->first(segTypeCR)->next1(segTypeCR);
    score->select(toChord(s->element(0))->notes().at(0));

    s = m1->last()->prev(SegmentType::ChordRest);
    score->select(s->element(1), SelectType::RANGE);

    EXPECT_TRUE(score->selection().canCopy());
    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData().toQByteArray());

    //paste to 16th rest
    EngravingItem* dest = m1->first(segTypeCR)->next(segTypeCR)->next(segTypeCR)->next(segTypeCR)->next(segTypeCR)->element(0);

    EXPECT_TRUE(dest->isRest());
    EXPECT_EQ(static_cast<ChordRest*>(dest)->durationType(), DurationType::V_16TH);
    score->select(dest);

    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("copypaste20.mscx"),
                                            COPYPASTE_DATA_DIR + String("copypaste20-ref.mscx")));
    delete score;
}

TEST_F(Engraving_CopyPasteTests, copypasteOnlySecondVoice)
{
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + String("copypaste18.mscx"));
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);

    score->select(m1, SelectType::RANGE, 0);

    score->selectionFilter().setFiltered(SelectionFilterType::FIRST_VOICE, false);

    EXPECT_TRUE(score->selection().canCopy());
    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData().toQByteArray());

    //paste to second measure
    score->deselectAll();
    score->selectionFilter().setFiltered(SelectionFilterType::FIRST_VOICE, true);
    score->select(m2, SelectType::RANGE);

    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("copypaste18.mscx"),
                                            COPYPASTE_DATA_DIR + String("copypaste18-ref.mscx")));
    delete score;
}

//---------------------------------------------------------
//    copy measure 2 from first staff, paste into staff 2
//---------------------------------------------------------

void Engraving_CopyPasteTests::copypastestaff(const char* idx)
{
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + String("copypaste%1.mscx").arg(String::fromUtf8(idx)));
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();      // src

    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);

    score->select(m2, SelectType::RANGE, 0);
    EXPECT_TRUE(score->selection().canCopy());
    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData().toQByteArray());

    score->deselectAll();

    score->select(m2, SelectType::RANGE, 1);

    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("copypaste%1.mscx").arg(String::fromUtf8(idx)),
                                            COPYPASTE_DATA_DIR + String("copypaste%1-ref.mscx").arg(String::fromUtf8(idx))));
    delete score;
}

TEST_F(Engraving_CopyPasteTests, copypastestaff50)
{
    copypastestaff("50");    // staff & slurs
}

TEST_F(Engraving_CopyPasteTests, copypastePartial)
{
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + String("copypaste_partial_01.mscx"));
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    EXPECT_TRUE(m1);

    Segment* s = m1->first(SegmentType::ChordRest);
    s = s->next(SegmentType::ChordRest);
    score->select(s->element(0));
    s = s->next(SegmentType::ChordRest);
    score->select(s->element(4), SelectType::RANGE);

    EXPECT_TRUE(score->selection().canCopy());
    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData().toQByteArray());

    score->select(m1->first(SegmentType::ChordRest)->element(0));

    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("copypaste_partial_01.mscx"),
                                            COPYPASTE_DATA_DIR + String("copypaste_partial_01-ref.mscx")));
    delete score;
}

void Engraving_CopyPasteTests::copypastetuplet(const char* idx)
{
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + String("copypaste_tuplet_%1.mscx").arg(String::fromUtf8(idx)));
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);

    Segment* s = m1->first(SegmentType::ChordRest);
    score->select(toChord(s->element(0))->notes().at(0));
    s = s->next(SegmentType::ChordRest);
    score->select(s->element(0), SelectType::RANGE);

    EXPECT_TRUE(score->selection().canCopy());
    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData().toQByteArray());

    EngravingItem* dest = m2->first(SegmentType::ChordRest)->element(0);
    score->select(dest);
    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("copypaste_tuplet_%1.mscx").arg(String::fromUtf8(idx)),
                                            COPYPASTE_DATA_DIR + String("copypaste_tuplet_%1-ref.mscx").arg(String::fromUtf8(idx))));
    delete score;
}

TEST_F(Engraving_CopyPasteTests, copypasteTuplet01)
{
    copypastetuplet("01");
}

TEST_F(Engraving_CopyPasteTests, copypasteTuplet02)
{
    copypastetuplet("02");
}

void Engraving_CopyPasteTests::copypastenote(const String& idx, Fraction scale)
{
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + "copypasteNote" + idx + ".mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);

    Segment* s = m2->first(SegmentType::ChordRest);
    score->select(toChord(s->element(0))->notes().at(0));
    QMimeData mimeData;
    mimeData.setData(score->selection().mimeType(), score->selection().mimeData().toQByteArray());
    ChordRest* cr = m1->first(SegmentType::ChordRest)->nextChordRest(0);
    score->select(cr->isChord() ? toChord(cr)->upNote() : static_cast<EngravingItem*>(cr));
    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(&mimeData);
    score->cmdPaste(&ma, 0, scale);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, "copypasteNote" + idx + ".mscx",
                                            COPYPASTE_DATA_DIR + "copypasteNote" + idx + "-ref.mscx"));
}

TEST_F(Engraving_CopyPasteTests, copypasteQtrNoteOntoWholeRest)
{
    copypastenote(u"01");
}

TEST_F(Engraving_CopyPasteTests, copypasteQtrNoteOntoWholeNote)
{
    copypastenote(u"02");
}

TEST_F(Engraving_CopyPasteTests, copypasteQtrNoteOntoQtrRest)
{
    copypastenote(u"03");
}

TEST_F(Engraving_CopyPasteTests, copypasteQtrNoteOntoQtrNote)
{
    copypastenote(u"04");
}

TEST_F(Engraving_CopyPasteTests, copypasteWholeNoteOntoQtrNote)
{
    copypastenote(u"05");
}

TEST_F(Engraving_CopyPasteTests, copypasteWholeNoteOntoQtrRest)
{
    copypastenote(u"06");
}

TEST_F(Engraving_CopyPasteTests, copypasteQtrNoteOntoTriplet)
{
    copypastenote(u"07");
}

TEST_F(Engraving_CopyPasteTests, copypasteWholeNoteOntoTriplet)
{
    copypastenote(u"08");
}

TEST_F(Engraving_CopyPasteTests, copypasteQtrNoteIntoChord)
{
    copypastenote(u"09");
}

TEST_F(Engraving_CopyPasteTests, copypasteQtrNoteOntoMMRest)
{
    copypastenote(u"10");
}

TEST_F(Engraving_CopyPasteTests, copypasteQtrNoteDoubleDuration)
{
    copypastenote(u"11", Fraction(2, 1));
}

TEST_F(Engraving_CopyPasteTests, copyPasteNoteDrumStave)
{
    copypastenote(u"12");
}

TEST_F(Engraving_CopyPasteTests, copypasteSplitNoteOverBar)
{
    // Copy first note m2 to last note m1
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + "copypasteSplit01.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);

    Segment* s = m2->first(SegmentType::ChordRest);
    score->select(toChord(s->element(0))->notes().at(0));
    QMimeData mimeData;
    mimeData.setData(score->selection().mimeType(), score->selection().mimeData().toQByteArray());
    ChordRest* cr = m1->findChordRest(Fraction(7, 8), 0);
    score->select(cr->isChord() ? toChord(cr)->upNote() : static_cast<EngravingItem*>(cr));
    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(&mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("copypasteSplit01.mscx"),
                                            COPYPASTE_DATA_DIR + "copypasteSplit01-ref.mscx"));
}

TEST_F(Engraving_CopyPasteTests, copypasteSplitTiedNoteOverBar)
{
    // Copy range first 2 beats of m2 to 2nd to last note m1
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + "copypasteSplit02.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);

    // create a range selection on 1st to 2nd beat (voice 1) of 2nd measure
    Segment* s = m2->first(SegmentType::ChordRest);
    score->select(toChord(s->element(0))->notes().at(0));
    s = s->next1(SegmentType::ChordRest)->next1(SegmentType::ChordRest);
    score->select(s->element(0), SelectType::RANGE);

    EXPECT_TRUE(score->selection().canCopy());

    QMimeData mimeData;
    mimeData.setData(score->selection().mimeType(), score->selection().mimeData().toQByteArray());
    ChordRest* cr = m1->findChordRest(Fraction(6, 8), 0);
    EXPECT_TRUE(cr);
    score->select(cr->isChord() ? toChord(cr)->upNote() : static_cast<EngravingItem*>(cr));
    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(&mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("copypasteSplit02.mscx"),
                                            COPYPASTE_DATA_DIR + "copypasteSplit02-ref.mscx"));
}

TEST_F(Engraving_CopyPasteTests, copypasteSplitNoteOverManyBars)
{
    // copy first note to last note m2
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + "copypasteSplit03.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);

    Segment* s = m1->first(SegmentType::ChordRest);
    score->select(toChord(s->element(0))->notes().at(0));
    QMimeData mimeData;
    mimeData.setData(score->selection().mimeType(), score->selection().mimeData().toQByteArray());
    ChordRest* cr = m2->findChordRest(Fraction(19, 8), 0);
    EXPECT_TRUE(cr);
    score->select(cr->isChord() ? toChord(cr)->upNote() : static_cast<EngravingItem*>(cr));
    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(&mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("copypasteSplit03.mscx"),
                                            COPYPASTE_DATA_DIR + "copypasteSplit03-ref.mscx"));
}

TEST_F(Engraving_CopyPasteTests, copypasteSplitNoteOverBarDrumStave)
{
    // Copy first note m2 to last note m1
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + "copypasteSplit04.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);

    Segment* s = m2->first(SegmentType::ChordRest);
    score->select(toChord(s->element(0))->notes().at(0));
    QMimeData mimeData;
    mimeData.setData(score->selection().mimeType(), score->selection().mimeData().toQByteArray());
    ChordRest* cr = m1->findChordRest(Fraction(7, 8), 0);
    score->select(cr->isChord() ? toChord(cr)->upNote() : static_cast<EngravingItem*>(cr));
    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(&mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("copypasteSplit04.mscx"),
                                            COPYPASTE_DATA_DIR + "copypasteSplit04-ref.mscx"));
}

//---------------------------------------------------------
//   copy-paste of tremolo between two notes
//---------------------------------------------------------

TEST_F(Engraving_CopyPasteTests, DISABLED_copypastetremolo)
{
    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + String("copypaste_tremolo.mscx"));
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();
    Measure* m3 = m2->nextMeasure();

    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);
    EXPECT_TRUE(m3);

    // create a range selection on 2nd to 3rd beat (voice 1) of first measure
    SegmentType segTypeCR = SegmentType::ChordRest;
    Segment* s = m1->first(segTypeCR)->next1(segTypeCR);
    score->select(toChord(s->element(1))->notes().at(0));
    s = s->next1(segTypeCR);
    score->select(s->element(1), SelectType::RANGE);

    EXPECT_TRUE(score->selection().canCopy());
    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData().toQByteArray());

    //paste to second measure
    score->select(m2->first()->element(0));

    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    // create a range selection on 2nd to 4th beat (voice 0) of first measure
    s = m1->first(segTypeCR)->next1(segTypeCR);
    score->select(toChord(s->element(0))->notes().at(0));
    s = s->next1(segTypeCR)->next1(segTypeCR);
    score->select(s->element(0), SelectType::RANGE);

    EXPECT_TRUE(score->selection().canCopy());
    mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    mimeData->setData(mimeType, score->selection().mimeData().toQByteArray());

    //paste to third measure
    score->select(m3->first()->element(0));

    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("copypaste_tremolo.mscx"),
                                            COPYPASTE_DATA_DIR + String("copypaste_tremolo-ref.mscx")));
    delete score;
}

TEST_F(Engraving_CopyPasteTests, copypasteparts)
{
    bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    MasterScore* score = ScoreRW::readScore(COPYPASTE_DATA_DIR + String("copypaste_parts.mscx"));
    EXPECT_TRUE(score);
    // create part
    TestUtils::createPart(score);

    // select measures 1-3
    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();
    Measure* m3 = m2->nextMeasure();
    Measure* m4 = m3->nextMeasure();

    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);
    EXPECT_TRUE(m3);
    EXPECT_TRUE(m4);

    score->select(m1);
    score->select(m3, SelectType::RANGE);

    EXPECT_TRUE(score->selection().canCopy());

    // copy

    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData().toQByteArray());

    // paste measure 4
    score->select(m4);

    score->startCmd(TranslatableString::untranslatable("Copy/paste tests"));
    QMimeDataAdapter ma(mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("copypaste_parts.mscx"),
                                            COPYPASTE_DATA_DIR + String("copypaste_parts-ref.mscx")));

    MScore::useRead302InTestMode = useRead302;
}
