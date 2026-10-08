/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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
#include <gmock/gmock.h>

#include <QMimeData>

#include "engraving/dom/chord.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/internal/qmimedataadapter.h"

#include "engraving/editing/edittie.h"
#include "engraving/editing/noteinput.h"
#include "engraving/editing/paste.h"
#include "engraving/editing/transaction/transaction.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu::engraving;
static const String PARTIALTIE_DATA_DIR(u"partialtie_data/");

class Engraving_PartialTieTests : public ::testing::Test
{
protected:
    void TearDown() override
    {
        delete m_masterScore;
    }

    Note* getNoteAtTick(const Fraction& tick)
    {
        Segment* seg = m_masterScore->tick2segment(tick, false, SegmentType::ChordRest);
        if (!seg) {
            ADD_FAILURE() << "No ChordRest segment at tick " << tick.toString().toStdString();
            return nullptr;
        }

        EngravingItem* item = seg->element(0);
        if (!item || !item->isChord()) {
            ADD_FAILURE() << "No chord at tick " << tick.toString().toStdString();
            return nullptr;
        }

        Note* note = toChord(item)->upNote();
        EXPECT_TRUE(note);

        return note;
    }

    void testPartialTies(const String& score, const Fraction& startPointLocation, const std::vector<Fraction>& jumpPointLocations)
    {
        ASSERT_NO_FATAL_FAILURE(openScore(score, startPointLocation, jumpPointLocations));

        addTie();

        ASSERT_NO_FATAL_FAILURE(saveAndLoad(score, startPointLocation, jumpPointLocations));

        toggleJumpPoint();

        deleteJumpTie();

        deleteJumpNote();

        toggleFirstJumpPoint();

        deleteStartTie();
    }

    void openScore(const String& score, const Fraction& startPointLocation, const std::vector<Fraction>& jumpPointLocations)
    {
        m_masterScore = ScoreRW::readScore(PARTIALTIE_DATA_DIR + score + u".mscx");

        ASSERT_TRUE(m_masterScore);

        // Find start note
        m_startNote = getNoteAtTick(startPointLocation);
        ASSERT_TRUE(m_startNote);
        // Find jump points
        for (const Fraction& jumpPointTick : jumpPointLocations) {
            Note* jumpPoint = getNoteAtTick(jumpPointTick);
            ASSERT_TRUE(jumpPoint);
            m_jumpPoints.push_back(jumpPoint);
        }
    }

    Tie* addTie()
    {
        // Add tie to start note
        // Expect tie to be added successfully and all jump points to have an incoming tie
        m_masterScore->select(m_startNote);
        Tie* t = EditTie::cmdToggleTie(m_masterScore); // calls startCmd/endCmd internally
        EXPECT_TRUE(t);

        for (const Note* note : m_jumpPoints) {
            EXPECT_TRUE(note->tieBack());
        }

        return t;
    }

    void toggleJumpPoint()
    {
        // Toggle the second jump point
        // Expect the second jump point to not have an incoming tie and all other jump points to have incoming ties
        TieJumpPointList* jumpPointList = m_startNote->tieJumpPoints();
        EXPECT_TRUE(jumpPointList->size() > 1);

        jumpPointList->toggleJumpPoint(u"jumpPoint1");

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            if (jumpPoint->id() == u"jumpPoint1") {
                EXPECT_FALSE(jumpPoint->endTie());
            } else {
                ASSERT_TRUE(jumpPoint->endTie());
            }
        }

        m_masterScore->undoRedo(true, 0);

        // Expect all jump points to have incoming ties

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            ASSERT_TRUE(jumpPoint->endTie());
        }
    }

    void deleteJumpTie()
    {
        // Delete the second jump tie
        // Expect the second jump point to not have an incoming tie and all other jump points to have incoming ties
        TieJumpPointList* jumpPointList = m_startNote->tieJumpPoints();
        EXPECT_TRUE(jumpPointList->size() > 1);
        ASSERT_TRUE(m_jumpPoints.at(1)->tieBack()->frontSegment());

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->deleteItem(m_jumpPoints.at(1)->tieBack()->frontSegment());
        m_masterScore->endCmd();

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            if (jumpPoint->id() == u"jumpPoint1") {
                EXPECT_FALSE(jumpPoint->endTie());
            } else {
                ASSERT_TRUE(jumpPoint->endTie());
            }
        }

        m_masterScore->undoRedo(true, 0);

        // Expect all jumpPoints to have incoming ties

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            ASSERT_TRUE(jumpPoint->endTie());
        }
    }

    void deleteJumpNote()
    {
        // Delete the second jump point note
        // Expect the second jump point to note have in incoming tie and all other jump points to have incoming ties
        TieJumpPointList* jumpPointList = m_startNote->tieJumpPoints();
        EXPECT_TRUE(jumpPointList->size() > 1);
        ASSERT_TRUE(m_jumpPoints.at(1)->chord());

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->deleteItem(m_jumpPoints.at(1)->chord());
        m_masterScore->endCmd();

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            if (jumpPoint->id() == u"jumpPoint1") {
                EXPECT_FALSE(jumpPoint->endTie());
            } else {
                ASSERT_TRUE(jumpPoint->endTie());
            }
        }

        m_masterScore->undoRedo(true, 0);

        // Expect all jump points to have incoming ties

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            ASSERT_TRUE(jumpPoint->endTie());
        }
    }

    void toggleFirstJumpPoint()
    {
        // Toggle the first jump point
        // Expect the first jump point to not have an incoming tie
        TieJumpPointList* jumpPointList = m_startNote->tieJumpPoints();

        Tie* startTie = jumpPointList->startTie();

        jumpPointList->toggleJumpPoint(u"jumpPoint0"); // calls startCmd/endCmd internally

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            if (jumpPoint->id() == u"jumpPoint0") {
                EXPECT_FALSE(jumpPoint->endTie());
            } else {
                ASSERT_TRUE(jumpPoint->endTie());
            }
        }

        // Expect the start (full) tie to be replaced with a partial tie
        Tie* newStartTie = jumpPointList->startTie();

        EXPECT_NE(startTie, newStartTie);
        EXPECT_TRUE(newStartTie->isPartialTie());

        // Expect all jump points to have incoming ties

        m_masterScore->undoRedo(true, 0);

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            ASSERT_TRUE(jumpPoint->endTie());
        }

        // Expect the start (partial) tie to be replaced with a full tie

        startTie = jumpPointList->startTie();
        EXPECT_NE(startTie, newStartTie);
        EXPECT_FALSE(startTie->isPartialTie());
    }

    void deleteStartTie()
    {
        // Delete the start tie
        // Expect no jump points to have incoming ties
        TieJumpPointList* jumpPointList = m_startNote->tieJumpPoints();
        ASSERT_TRUE(m_startNote->tieFor()->frontSegment());

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->deleteItem(m_startNote->tieFor()->frontSegment());
        m_masterScore->endCmd();

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            EXPECT_FALSE(jumpPoint->endTie());
            EXPECT_FALSE(jumpPoint->active());
        }
    }

    void testSegnoPartialTieFirst(Fraction tickBeforeSegno, Fraction tickAfterSegno)
    {
        // Add a partial tie to the note following a segno, then add a full tie to the note preceding the segno
        addTie();

        TieJumpPointList* jumpPointList = m_startNote->tieJumpPoints();

        Note* noteAfterSegno = getNoteAtTick(tickAfterSegno);
        ASSERT_TRUE(noteAfterSegno);
        Tie* initialTie = noteAfterSegno->tieBack();
        ASSERT_TRUE(initialTie);
        EXPECT_TRUE(initialTie->isPartialTie());

        Note* noteBeforeSegno = getNoteAtTick(tickBeforeSegno);
        ASSERT_TRUE(noteBeforeSegno);
        m_masterScore->select(noteBeforeSegno);
        Tie* tieBeforeSegno = EditTie::cmdToggleTie(m_masterScore); // calls startCmd/endCmd internally

        bool newTieFound = false;
        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            EXPECT_NE(jumpPoint->endTie(), initialTie);
            newTieFound |= jumpPoint->endTie() == tieBeforeSegno;
        }

        EXPECT_TRUE(newTieFound);

        ASSERT_TRUE(tieBeforeSegno);
        EXPECT_FALSE(tieBeforeSegno->isPartialTie());
        EXPECT_NE(tieBeforeSegno, initialTie);

        EXPECT_EQ(tieBeforeSegno->tieJumpPoints()->size(), 0);
        EXPECT_EQ(jumpPointList->size(), 1);

        // Delete the start tie
        // Expect segno tie to still have a tie but no jump point
        ASSERT_TRUE(m_startNote->tieFor()->frontSegment());

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->deleteItem(m_startNote->tieFor()->frontSegment());
        m_masterScore->endCmd();

        ASSERT_TRUE(tieBeforeSegno);
        EXPECT_FALSE(tieBeforeSegno->jumpPoint());
    }

    void testSegnoPartialTieAfter(Fraction tickBeforeSegno)
    {
        // Add a full tie to the note preceding a segno, then add a tie to the D.S which should add the previous tie to the list of jump points
        Note* noteBeforeSegno = getNoteAtTick(tickBeforeSegno);
        ASSERT_TRUE(noteBeforeSegno);
        m_masterScore->select(noteBeforeSegno);
        Tie* tieBeforeSegno = EditTie::cmdToggleTie(m_masterScore); // calls startCmd/endCmd internally
        ASSERT_TRUE(tieBeforeSegno);

        Tie* startTie = addTie();
        ASSERT_TRUE(startTie);

        EXPECT_EQ(startTie->tieJumpPoints()->size(), 1);

        // Delete the start tie
        // Expect segno tie to still have a tie but no jump point
        ASSERT_TRUE(m_startNote->tieFor()->frontSegment());

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->deleteItem(m_startNote->tieFor()->frontSegment());
        m_masterScore->endCmd();

        ASSERT_TRUE(tieBeforeSegno);
        EXPECT_FALSE(tieBeforeSegno->jumpPoint());
    }

    void saveAndLoad(const String& score, const Fraction& startPointLocation, const std::vector<Fraction>& jumpPointLocations)
    {
        // Save score
        const String savePath = score + u".mscx";
        EXPECT_TRUE(ScoreComp::saveCompareScore(m_masterScore, savePath, PARTIALTIE_DATA_DIR + score + u"-ref.mscx"));
        delete m_masterScore;
        m_masterScore = nullptr;
        m_startNote = nullptr;
        m_jumpPoints.clear();

        // Load
        ASSERT_NO_FATAL_FAILURE(openScore(score + u"-ref", startPointLocation, jumpPointLocations));

        // Expect start tie has jumpPoints
        // Expect each jumpPoint to have an incoming tie
        TieJumpPointList* jumpPointList = m_startNote->tieJumpPoints();
        ASSERT_TRUE(jumpPointList);

        EXPECT_EQ(jumpPointLocations.size(), jumpPointList->size());

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            EXPECT_EQ(jumpPoint->endTie()->startTie(), m_startNote->tieFor());
        }

        for (const Note* note : m_jumpPoints) {
            ASSERT_TRUE(note->tieBack());
        }
    }

    void testPartialTieListSelection(const String& score, const Fraction& startPointLocation, const Fraction& secondNoteLocation,
                                     const std::vector<Fraction>& jumpPointLocations)
    {
        ASSERT_NO_FATAL_FAILURE(openScore(score, startPointLocation, jumpPointLocations));

        Note* secondTieNote = getNoteAtTick(secondNoteLocation);
        ASSERT_TRUE(secondTieNote);

        // Add tie to start note
        // Expect tie to be added successfully and all jump points to have an incoming tie
        m_masterScore->select(m_startNote);
        m_masterScore->select(secondTieNote, SelectType::ADD);
        Tie* t = EditTie::cmdToggleTie(m_masterScore); // calls startCmd/endCmd internally
        ASSERT_TRUE(t);

        for (const Note* note : m_jumpPoints) {
            ASSERT_TRUE(note->tieBack());
        }

        ASSERT_NO_FATAL_FAILURE(saveAndLoad(score, startPointLocation, jumpPointLocations));

        toggleJumpPoint();

        deleteJumpTie();

        deleteJumpNote();

        toggleFirstJumpPoint();

        deleteStartTie();
    }

private:
    MasterScore* m_masterScore = nullptr;
    Note* m_startNote = nullptr;
    std::vector<Note*> m_jumpPoints;
};

TEST_F(Engraving_PartialTieTests, repeatBarlines)
{
    const String test = u"repeat_barlines";

    const Fraction startPointTick = Fraction(7, 4);
    const std::vector<Fraction> jumpPoints = { Fraction(8, 4), Fraction(0, 4) };

    ASSERT_NO_FATAL_FAILURE(testPartialTies(test, startPointTick, jumpPoints));
}

TEST_F(Engraving_PartialTieTests, voltaCoda)
{
    const String test = u"volta_coda";

    const Fraction startPointTick = Fraction(3, 4);
    const std::vector<Fraction> jumpPoints = { Fraction(4, 4), Fraction(8, 4), Fraction(12, 4), Fraction(16, 4) };

    ASSERT_NO_FATAL_FAILURE(testPartialTies(test, startPointTick, jumpPoints));
}

TEST_F(Engraving_PartialTieTests, coda)
{
    const String test = u"coda";

    const Fraction startPointTick = Fraction(3, 4);
    const std::vector<Fraction> jumpPoints = { Fraction(4, 4), Fraction(8, 4) };

    ASSERT_NO_FATAL_FAILURE(testPartialTies(test, startPointTick, jumpPoints));
}

TEST_F(Engraving_PartialTieTests, segnoBefore)
{
    const String test = u"segno";

    const Fraction startPointTick = Fraction(11, 4);
    const std::vector<Fraction> jumpPoints = { Fraction(4, 4) };

    ASSERT_NO_FATAL_FAILURE(openScore(test, startPointTick, jumpPoints));

    // Add tie to 3,4.  Replace incoming partial tie at 4,4 with full tie
    testSegnoPartialTieFirst(Fraction(3, 4), Fraction(4, 4));
}

TEST_F(Engraving_PartialTieTests, segnoAfter)
{
    const String test = u"segno";

    const Fraction startPointTick = Fraction(11, 4);
    const std::vector<Fraction> jumpPoints = { Fraction(4, 4) };

    ASSERT_NO_FATAL_FAILURE(openScore(test, startPointTick, jumpPoints));

    testSegnoPartialTieAfter(Fraction(3, 4));
}

TEST_F(Engraving_PartialTieTests, copyPartialTiesAndSlurs)
{
    Score* score = ScoreRW::readScore(PARTIALTIE_DATA_DIR + u"copyPastePartials.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure()->nextMeasure();
    ASSERT_TRUE(m1);
    Measure* m2 = m1->nextMeasure();
    ASSERT_TRUE(m2);
    Measure* m3 = m2->nextMeasure();
    ASSERT_TRUE(m3);
    Measure* m4 = m3->nextMeasure();
    ASSERT_TRUE(m4);
    // Copy staff 0, m1

    score->select(m1, SelectType::SINGLE, 0);
    EXPECT_TRUE(score->selection().canCopy());
    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    QByteArray ba = score->selection().mimeData().toQByteArray();
    mimeData->setData(mimeType, ba);

    // Paste staff 1, m1
    ASSERT_TRUE(m1->first(SegmentType::ChordRest)->element(4));
    score->select(m1->first(SegmentType::ChordRest)->element(4));
    score->startCmd(TranslatableString::untranslatable("Partial tie tests"));
    QMimeDataAdapter ma(mimeData);
    Paste::paste(score->transactionManager()->currentOrDummyTransaction(), score, &ma, 0);
    score->endCmd();
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"copyPastePartials01.mscx"),
                                            PARTIALTIE_DATA_DIR + String(u"copyPastePartials01-ref.mscx")));

    // Paste staff 0, m3
    ASSERT_TRUE(m3->first(SegmentType::ChordRest)->element(0));
    score->select(m3->first(SegmentType::ChordRest)->element(0));
    score->startCmd(TranslatableString::untranslatable("Partial tie tests"));
    Paste::paste(score->transactionManager()->currentOrDummyTransaction(), score, &ma, 0);
    score->endCmd();
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"copyPastePartials02.mscx"),
                                            PARTIALTIE_DATA_DIR + String(u"copyPastePartials02-ref.mscx")));

    // Paste staff 0, m4
    ASSERT_TRUE(m4->first(SegmentType::ChordRest)->element(0));
    score->select(m4->first(SegmentType::ChordRest)->element(0));
    score->startCmd(TranslatableString::untranslatable("Partial tie tests"));
    Paste::paste(score->transactionManager()->currentOrDummyTransaction(), score, &ma, 0);
    score->endCmd();
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"copyPastePartials03.mscx"),
                                            PARTIALTIE_DATA_DIR + String(u"copyPastePartials03-ref.mscx")));

    // Paste staff 1, m4
    ASSERT_TRUE(m4->first(SegmentType::ChordRest)->element(4));
    score->select(m4->first(SegmentType::ChordRest)->element(4));
    score->startCmd(TranslatableString::untranslatable("Partial tie tests"));
    Paste::paste(score->transactionManager()->currentOrDummyTransaction(), score, &ma, 0);
    score->endCmd();
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"copyPastePartials04.mscx"),
                                            PARTIALTIE_DATA_DIR + String(u"copyPastePartials04-ref.mscx")));
}

TEST_F(Engraving_PartialTieTests, partialTieListSelection)
{
    // Test list selection of non-adjacent notes
    const String test = u"partialTieList";

    const Fraction startPointTick = Fraction(2, 4);
    const Fraction secondTieNoteTick = Fraction(5, 4);
    const std::vector<Fraction> jumpPoints = { Fraction(5, 4), Fraction(2, 1) };

    testPartialTieListSelection(test, startPointTick, secondTieNoteTick, jumpPoints);
}

TEST_F(Engraving_PartialTieTests, toggleTiePartialThenRestore)
{
    const String test = u"toggle_delete";

    Score* score = ScoreRW::readScore(PARTIALTIE_DATA_DIR + test + u".mscx");
    ASSERT_TRUE(score);

    const Fraction tieFromTick = Fraction(3, 4);
    const Fraction tieToTick = Fraction(4, 4);

    Measure* m1 = score->firstMeasure();
    ASSERT_TRUE(m1);
    Measure* m2 = m1->nextMeasure();
    ASSERT_TRUE(m2);
    Segment* tieFromSegment = m1->findSegment(SegmentType::ChordRest, tieFromTick);
    ASSERT_TRUE(tieFromSegment);
    Segment* tieToSegment = m2->findSegment(SegmentType::ChordRest, tieToTick);
    ASSERT_TRUE(tieToSegment);

    const int pitchC = 42;

    score->startCmd(TranslatableString::untranslatable("Partial tie tests"));
    score->inputState().setTrack(0);
    score->inputState().setSegment(tieToSegment);
    score->inputState().setDuration(DurationType::V_WHOLE);
    score->inputState().setNoteEntryMode(true);

    NoteInput::addPitch(score->transactionManager()->currentOrDummyTransaction(), score, pitchC, false, false);
    score->endCmd();

    Chord* tieToChord = m2->findChord(Fraction(4, 4), 0);
    Note* tieToNote = tieToChord ? tieToChord->upNote() : nullptr;
    ASSERT_TRUE(tieToChord);
    ASSERT_TRUE(tieToNote);

    Chord* tieFromChord = toChord(tieFromSegment->element(0));
    Note* tieFromNote = tieFromChord ? tieFromChord->upNote() : nullptr;
    ASSERT_TRUE(tieFromNote);

    // Toggle tie at 4/4
    score->select(tieFromNote);
    EditTie::cmdToggleTie(score); // calls startCmd/endCmd internally

    // Clear the second measure

    score->select(m2, SelectType::SINGLE, 0);
    score->startCmd(TranslatableString::untranslatable("Partial tie tests"));
    score->cmdDeleteSelection();
    score->endCmd();

    // Verify the tie is now partial
    Tie* tie = tieFromNote->tieFor();
    ASSERT_TRUE(tie);
    EXPECT_TRUE(tie->isPartialTie());

    // Undo
    score->undoRedo(true, nullptr); // undo clear
    score->undoRedo(true, nullptr); // undo toggle tie
    score->undoRedo(true, nullptr); // undo add note

    tieToSegment = m2->findSegment(SegmentType::ChordRest, tieToTick);
    ASSERT_TRUE(tieToSegment);

    score->startCmd(TranslatableString::untranslatable("Partial tie tests"));
    score->inputState().setTrack(0);
    score->inputState().setSegment(tieToSegment);
    score->inputState().setDuration(DurationType::V_WHOLE);
    score->inputState().setNoteEntryMode(true);

    NoteInput::addPitch(score->transactionManager()->currentOrDummyTransaction(), score, pitchC, false, false);
    score->endCmd();

    Chord* newTieToChord = m2->findChord(Fraction(4, 4), 0);
    Note* newTieToNote = newTieToChord ? newTieToChord->upNote() : nullptr;
    ASSERT_TRUE(newTieToChord);
    ASSERT_TRUE(newTieToNote);

    // Toggle tie again at 4/4
    score->select(tieFromNote);
    EditTie::cmdToggleTie(score); // calls startCmd/endCmd internally

    // Verify the tie is now full
    Tie* newTie = tieFromNote->tieFor();
    ASSERT_TRUE(newTie);
    EXPECT_FALSE(newTie->isPartialTie());
    EXPECT_EQ(newTie->endNote(), newTieToNote);
}

static void deleteScoreWithPartialTies(String fileName)
{
    // Load score
    Score* score = ScoreRW::readScore(PARTIALTIE_DATA_DIR + fileName + u".mscx");
    ASSERT_TRUE(score);

    // Make sure there are some measures to delete
    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);
    Measure* lastMeasure = score->lastMeasure();
    ASSERT_TRUE(lastMeasure);
    EXPECT_NE(firstMeasure, lastMeasure);

    // Select all measures
    score->select(firstMeasure, SelectType::RANGE, 0);
    score->select(lastMeasure, SelectType::RANGE, score->nstaves() - 1);

    // Delete selected measures
    score->startCmd(TranslatableString::untranslatable("Partial tie tests"));
    score->cmdDeleteSelection();
    score->endCmd();
}

TEST_F(Engraving_PartialTieTests, deleteAllMeasures)
{
    const String test1 = u"delete_measure_test1";
    const String test2 = u"delete_measure_test2";

    deleteScoreWithPartialTies(test1);
    deleteScoreWithPartialTies(test2);
}

TEST_F(Engraving_PartialTieTests, copyPasteRemoveInvalidPartialTies)
{
    Score* score = ScoreRW::readScore(PARTIALTIE_DATA_DIR + u"copyPastePartials05.mscx");
    ASSERT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ASSERT_TRUE(m1);
    Measure* m2 = m1->nextMeasure();
    ASSERT_TRUE(m2);
    Measure* m3 = m2->nextMeasure();
    ASSERT_TRUE(m3);
    Measure* m4 = m3->nextMeasure();
    ASSERT_TRUE(m4);

    // Select measures 1 and 2
    score->select(m1, SelectType::RANGE, 0);
    score->select(m2, SelectType::RANGE, 0);

    EXPECT_TRUE(score->selection().canCopy());
    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    QByteArray ba = score->selection().mimeData().toQByteArray();
    mimeData->setData(mimeType, ba);

    // Paste at measure 4
    ASSERT_TRUE(m4->first(SegmentType::ChordRest)->element(0));
    score->select(m4->first(SegmentType::ChordRest)->element(0));
    score->transactionManager()->transaction(TranslatableString::untranslatable("Partial tie tests"), [&](Transaction& tx) {
        QMimeDataAdapter ma(mimeData);
        Paste::paste(tx, score, &ma, 0);
    });
    score->doLayout();

    // Measure 4 beat 1 — no ties expected
    const Fraction tick1 = Fraction(3, 1);
    Segment* seg1 = score->tick2segment(tick1, false, SegmentType::ChordRest);
    ASSERT_TRUE(seg1);
    ASSERT_TRUE(seg1->element(0));
    ASSERT_TRUE(seg1->element(0)->isChord());
    Note* note1 = toChord(seg1->element(0))->upNote();
    ASSERT_TRUE(note1);
    EXPECT_FALSE(note1->tieBack());
    EXPECT_FALSE(note1->tieFor());

    // Measure 5 beat 4 — no ties expected
    const Fraction tick2 = Fraction(19, 4);
    Segment* seg2 = score->tick2segment(tick2, false, SegmentType::ChordRest);
    ASSERT_TRUE(seg2);
    ASSERT_TRUE(seg2->element(0));
    ASSERT_TRUE(seg2->element(0)->isChord());
    Note* note2 = toChord(seg2->element(0))->upNote();
    ASSERT_TRUE(note2);
    EXPECT_FALSE(note2->tieBack());
    EXPECT_FALSE(note2->tieFor());

    delete score;
}
