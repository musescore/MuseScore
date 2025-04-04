/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "dom/note.h"
#include "dom/chord.h"

#include "internal/qmimedataadapter.h"
#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu::engraving;
static const String PARTIALTIE_DATA_DIR(u"partialtie_data/");

class Engraving_PartialTieTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_useRead302 = MScore::useRead302InTestMode;
        MScore::useRead302InTestMode = false;
    }

    void TearDown() override
    {
        MScore::useRead302InTestMode = m_useRead302;
        delete m_masterScore;
    }

    Note* getNoteAtTick(const Fraction& tick)
    {
        Segment* seg = m_masterScore->tick2segment(tick, false, SegmentType::ChordRest);
        EXPECT_TRUE(seg);
        EXPECT_TRUE(seg->element(0) && seg->element(0)->isChord());

        Chord* chord = toChord(seg->element(0));
        Note* note = chord->upNote();
        EXPECT_TRUE(note);

        return note;
    }

    void testPartialTies(const String& score, const Fraction& startPointLocation, const std::vector<Fraction>& jumpPointLocations)
    {
        openScore(score, startPointLocation, jumpPointLocations);

        addTie();

        saveAndLoad(score, startPointLocation, jumpPointLocations);

        toggleJumpPoint();

        deleteJumpTie();

        deleteJumpNote();

        toggleFirstJumpPoint();

        deleteStartTie();
    }

    void openScore(const String& score, const Fraction& startPointLocation, const std::vector<Fraction>& jumpPointLocations)
    {
        m_masterScore = ScoreRW::readScore(PARTIALTIE_DATA_DIR + score + u".mscx");

        EXPECT_TRUE(m_masterScore);

        // Find start note
        m_startNote = getNoteAtTick(startPointLocation);
        // Find jump points
        for (const Fraction& jumpPointTick : jumpPointLocations) {
            m_jumpPoints.push_back(getNoteAtTick(jumpPointTick));
        }
    }

    Tie* addTie()
    {
        // Add tie to start note
        // Expect tie to be added successfully and all jump points to have an incoming tie
        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->select(m_startNote);
        Tie* t = m_masterScore->cmdToggleTie();
        EXPECT_TRUE(t);
        m_masterScore->endCmd();

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

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        jumpPointList->toggleJumpPoint(u"jumpPoint1");
        m_masterScore->endCmd();

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            if (jumpPoint->id() == u"jumpPoint1") {
                EXPECT_FALSE(jumpPoint->endTie());
            } else {
                EXPECT_TRUE(jumpPoint->endTie());
            }
        }

        m_masterScore->undoRedo(true, 0);

        // Expect all jump points to have incoming ties

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            EXPECT_TRUE(jumpPoint->endTie());
        }
    }

    void deleteJumpTie()
    {
        // Delete the second jump tie
        // Expect the second jump point to not have an incoming tie and all other jump points to have incoming ties
        TieJumpPointList* jumpPointList = m_startNote->tieJumpPoints();
        EXPECT_TRUE(jumpPointList->size() > 1);
        EXPECT_TRUE(m_jumpPoints.at(1)->tieBack()->frontSegment());

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->deleteItem(m_jumpPoints.at(1)->tieBack()->frontSegment());
        m_masterScore->endCmd();

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            if (jumpPoint->id() == u"jumpPoint1") {
                EXPECT_FALSE(jumpPoint->endTie());
            } else {
                EXPECT_TRUE(jumpPoint->endTie());
            }
        }

        m_masterScore->undoRedo(true, 0);

        // Expect all jumpPoints to have incoming ties

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            EXPECT_TRUE(jumpPoint->endTie());
        }
    }

    void deleteJumpNote()
    {
        // Delete the second jump point note
        // Expect the second jump point to note have in incoming tie and all other jump points to have incoming ties
        TieJumpPointList* jumpPointList = m_startNote->tieJumpPoints();
        EXPECT_TRUE(jumpPointList->size() > 1);
        EXPECT_TRUE(m_jumpPoints.at(1)->chord());

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->deleteItem(m_jumpPoints.at(1)->chord());
        m_masterScore->endCmd();

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            if (jumpPoint->id() == u"jumpPoint1") {
                EXPECT_FALSE(jumpPoint->endTie());
            } else {
                EXPECT_TRUE(jumpPoint->endTie());
            }
        }

        m_masterScore->undoRedo(true, 0);

        // Expect all jump points to have incoming ties

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            EXPECT_TRUE(jumpPoint->endTie());
        }
    }

    void toggleFirstJumpPoint()
    {
        // Toggle the first jump point
        // Expect the first jump point to not have an incoming tie
        TieJumpPointList* jumpPointList = m_startNote->tieJumpPoints();

        Tie* startTie = jumpPointList->startTie();

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        jumpPointList->toggleJumpPoint(u"jumpPoint0");
        m_masterScore->endCmd();

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            if (jumpPoint->id() == u"jumpPoint0") {
                EXPECT_FALSE(jumpPoint->endTie());
            } else {
                EXPECT_TRUE(jumpPoint->endTie());
            }
        }

        // Expect the start (full) tie to be replaced with a partial tie
        Tie* newStartTie = jumpPointList->startTie();

        EXPECT_NE(startTie, newStartTie);
        EXPECT_TRUE(newStartTie->isPartialTie());

        // Expect all jump points to have incoming ties

        m_masterScore->undoRedo(true, 0);

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            EXPECT_TRUE(jumpPoint->endTie());
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
        EXPECT_TRUE(m_startNote->tieFor()->frontSegment());

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
        Tie* initialTie = noteAfterSegno->tieBack();
        EXPECT_TRUE(initialTie && initialTie->isPartialTie());

        Note* noteBeforeSegno = getNoteAtTick(tickBeforeSegno);
        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->select(noteBeforeSegno);
        Tie* tieBeforeSegno = m_masterScore->cmdToggleTie();
        m_masterScore->endCmd();

        bool newTieFound = false;
        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            EXPECT_NE(jumpPoint->endTie(), initialTie);
            newTieFound |= jumpPoint->endTie() == tieBeforeSegno;
        }

        EXPECT_TRUE(newTieFound);

        EXPECT_TRUE(tieBeforeSegno);
        EXPECT_FALSE(tieBeforeSegno->isPartialTie());
        EXPECT_NE(tieBeforeSegno, initialTie);

        EXPECT_EQ(tieBeforeSegno->tieJumpPoints()->size(), 0);
        EXPECT_EQ(jumpPointList->size(), 1);

        // Delete the start tie
        // Expect segno tie to still have a tie but no jump point
        EXPECT_TRUE(m_startNote->tieFor()->frontSegment());

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->deleteItem(m_startNote->tieFor()->frontSegment());
        m_masterScore->endCmd();

        EXPECT_TRUE(tieBeforeSegno);
        EXPECT_FALSE(tieBeforeSegno->jumpPoint());
    }

    void testSegnoPartialTieAfter(Fraction tickBeforeSegno)
    {
        // Add a full tie to the note preceding a segno, then add a tie to the D.S which should add the previous tie to the list of jump points
        Note* noteBeforeSegno = getNoteAtTick(tickBeforeSegno);
        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->select(noteBeforeSegno);
        Tie* tieBeforeSegno = m_masterScore->cmdToggleTie();
        EXPECT_TRUE(tieBeforeSegno);
        m_masterScore->endCmd();

        Tie* startTie = addTie();

        EXPECT_EQ(startTie->tieJumpPoints()->size(), 1);

        // Delete the start tie
        // Expect segno tie to still have a tie but no jump point
        EXPECT_TRUE(m_startNote->tieFor()->frontSegment());

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->deleteItem(m_startNote->tieFor()->frontSegment());
        m_masterScore->endCmd();

        EXPECT_TRUE(tieBeforeSegno);
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
        openScore(score + u"-ref", startPointLocation, jumpPointLocations);

        // Expect start tie has jumpPoints
        // Expect each jumpPoint to have an incoming tie
        TieJumpPointList* jumpPointList = m_startNote->tieJumpPoints();
        EXPECT_TRUE(jumpPointList);

        EXPECT_EQ(jumpPointLocations.size(), jumpPointList->size());

        for (TieJumpPoint* jumpPoint : *jumpPointList) {
            EXPECT_EQ(jumpPoint->endTie()->startTie(), m_startNote->tieFor());
        }

        for (const Note* note : m_jumpPoints) {
            EXPECT_TRUE(note->tieBack());
        }
    }

    void testPartialTieListSelection(const String& score, const Fraction& startPointLocation, const Fraction& secondNoteLocation,
                                     const std::vector<Fraction>& jumpPointLocations)
    {
        openScore(score, startPointLocation, jumpPointLocations);

        Note* secondTieNote = getNoteAtTick(secondNoteLocation);
        EXPECT_TRUE(secondTieNote);

        // Add tie to start note
        // Expect tie to be added successfully and all jump points to have an incoming tie
        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->select(m_startNote);
        m_masterScore->select(secondTieNote, SelectType::ADD);
        Tie* t = m_masterScore->cmdToggleTie();
        EXPECT_TRUE(t);
        m_masterScore->endCmd();

        for (const Note* note : m_jumpPoints) {
            EXPECT_TRUE(note->tieBack());
        }

        saveAndLoad(score, startPointLocation, jumpPointLocations);

        toggleJumpPoint();

        deleteJumpTie();

        deleteJumpNote();

        toggleFirstJumpPoint();

        deleteStartTie();
    }

private:
    bool m_useRead302 = false;

    MasterScore* m_masterScore = nullptr;
    Note* m_startNote = nullptr;
    std::vector<Note*> m_jumpPoints;
};

TEST_F(Engraving_PartialTieTests, repeatBarlines)
{
    const String test = u"repeat_barlines";

    const Fraction startPointTick = Fraction(7, 4);
    const std::vector<Fraction> jumpPoints = { Fraction(8, 4), Fraction(0, 4) };

    testPartialTies(test, startPointTick, jumpPoints);
}

TEST_F(Engraving_PartialTieTests, voltaCoda)
{
    const String test = u"volta_coda";

    const Fraction startPointTick = Fraction(3, 4);
    const std::vector<Fraction> jumpPoints = { Fraction(4, 4), Fraction(8, 4), Fraction(12, 4), Fraction(16, 4) };

    testPartialTies(test, startPointTick, jumpPoints);
}

TEST_F(Engraving_PartialTieTests, coda)
{
    const String test = u"coda";

    const Fraction startPointTick = Fraction(3, 4);
    const std::vector<Fraction> jumpPoints = { Fraction(4, 4), Fraction(8, 4) };

    testPartialTies(test, startPointTick, jumpPoints);
}

TEST_F(Engraving_PartialTieTests, segnoBefore)
{
    const String test = u"segno";

    const Fraction startPointTick = Fraction(11, 4);
    const std::vector<Fraction> jumpPoints = { Fraction(4, 4) };

    openScore(test, startPointTick, jumpPoints);

    // Add tie to 3,4.  Replace incoming partial tie at 4,4 with full tie
    testSegnoPartialTieFirst(Fraction(3, 4), Fraction(4, 4));
}

TEST_F(Engraving_PartialTieTests, segnoAfter)
{
    const String test = u"segno";

    const Fraction startPointTick = Fraction(11, 4);
    const std::vector<Fraction> jumpPoints = { Fraction(4, 4) };

    openScore(test, startPointTick, jumpPoints);

    testSegnoPartialTieAfter(Fraction(3, 4));
}

TEST_F(Engraving_PartialTieTests, copyPartialTiesAndSlurs)
{
    Score* score = ScoreRW::readScore(PARTIALTIE_DATA_DIR + u"copyPastePartials.mscx");
    EXPECT_TRUE(score);
    Measure* m1 = score->firstMeasure()->nextMeasure();
    Measure* m2 = m1->nextMeasure();
    Measure* m3 = m2->nextMeasure();
    Measure* m4 = m3->nextMeasure();
    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);
    EXPECT_TRUE(m3);
    EXPECT_TRUE(m4);
    // Copy staff 0, m1

    score->select(m1, SelectType::SINGLE, 0);
    EXPECT_TRUE(score->selection().canCopy());
    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    QByteArray ba = score->selection().mimeData().toQByteArray();
    mimeData->setData(mimeType, ba);

    // Paste staff 1, m1
    EXPECT_TRUE(m1->first(SegmentType::ChordRest)->element(4));
    score->select(m1->first(SegmentType::ChordRest)->element(4));
    score->startCmd(TranslatableString::untranslatable("Partial tie tests"));
    QMimeDataAdapter ma(mimeData);
    score->cmdPaste(&ma, 0);
    score->endCmd();
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"copyPastePartials01.mscx"),
                                            PARTIALTIE_DATA_DIR + String(u"copyPastePartials01-ref.mscx")));

    // Paste staff 0, m3
    EXPECT_TRUE(m3->first(SegmentType::ChordRest)->element(0));
    score->select(m3->first(SegmentType::ChordRest)->element(0));
    score->startCmd(TranslatableString::untranslatable("Partial tie tests"));
    score->cmdPaste(&ma, 0);
    score->endCmd();
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"copyPastePartials02.mscx"),
                                            PARTIALTIE_DATA_DIR + String(u"copyPastePartials02-ref.mscx")));

    // Paste staff 0, m4
    EXPECT_TRUE(m4->first(SegmentType::ChordRest)->element(0));
    score->select(m4->first(SegmentType::ChordRest)->element(0));
    score->startCmd(TranslatableString::untranslatable("Partial tie tests"));
    score->cmdPaste(&ma, 0);
    score->endCmd();
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"copyPastePartials03.mscx"),
                                            PARTIALTIE_DATA_DIR + String(u"copyPastePartials03-ref.mscx")));

    // Paste staff 1, m4
    EXPECT_TRUE(m4->first(SegmentType::ChordRest)->element(4));
    score->select(m4->first(SegmentType::ChordRest)->element(4));
    score->startCmd(TranslatableString::untranslatable("Partial tie tests"));
    score->cmdPaste(&ma, 0);
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
