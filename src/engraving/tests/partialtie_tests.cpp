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
#include <memory>

#include "dom/note.h"
#include "dom/chord.h"

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

    void testPartialTies(const String& score, const Fraction& startPointLocation, const std::vector<Fraction>& endPointLocations)
    {
        openScore(score, startPointLocation, endPointLocations);

        addTie();

        saveAndLoad(score, startPointLocation, endPointLocations);

        toggleEndPoint();

        deleteEndTie();

        deleteEndNote();

        toggleFirstEndPoint();

        deleteStartTie();
    }

    void openScore(const String& score, const Fraction& startPointLocation, const std::vector<Fraction>& endPointLocations)
    {
        m_masterScore = ScoreRW::readScore(PARTIALTIE_DATA_DIR + score + u".mscx");

        EXPECT_TRUE(m_masterScore);

        // Find start note
        m_startNote = getNoteAtTick(startPointLocation);
        // Find endpoints
        for (const Fraction& endPointTick : endPointLocations) {
            m_endPoints.push_back(getNoteAtTick(endPointTick));
        }
    }

    Tie* addTie()
    {
        // Add tie to start note
        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->select(m_startNote);
        Tie* t = m_masterScore->cmdToggleTie();
        EXPECT_TRUE(t);
        m_masterScore->endCmd();

        for (const Note* note : m_endPoints) {
            EXPECT_TRUE(note->tieBack());
        }

        return t;
    }

    void toggleEndPoint()
    {
        TieEndPointList* endPointList = m_startNote->tieEndPoints();
        EXPECT_TRUE(endPointList->size() > 1);

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        endPointList->toggleEndPoint(u"endPoint1");
        m_masterScore->endCmd();

        for (TieEndPoint* endPoint : *endPointList) {
            if (endPoint->id() == u"endPoint1") {
                EXPECT_FALSE(endPoint->endTie());
            } else {
                EXPECT_TRUE(endPoint->endTie());
            }
        }

        m_masterScore->undoRedo(true, 0);

        for (TieEndPoint* endPoint : *endPointList) {
            EXPECT_TRUE(endPoint->endTie());
        }
    }

    void deleteEndTie()
    {
        TieEndPointList* endPointList = m_startNote->tieEndPoints();
        EXPECT_TRUE(endPointList->size() > 1);
        EXPECT_TRUE(m_endPoints.at(1)->tieBack()->frontSegment());

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->deleteItem(m_endPoints.at(1)->tieBack()->frontSegment());
        m_masterScore->endCmd();

        for (TieEndPoint* endPoint : *endPointList) {
            if (endPoint->id() == u"endPoint1") {
                EXPECT_FALSE(endPoint->endTie());
            } else {
                EXPECT_TRUE(endPoint->endTie());
            }
        }

        m_masterScore->undoRedo(true, 0);

        for (TieEndPoint* endPoint : *endPointList) {
            EXPECT_TRUE(endPoint->endTie());
        }
    }

    void deleteEndNote()
    {
        TieEndPointList* endPointList = m_startNote->tieEndPoints();
        EXPECT_TRUE(endPointList->size() > 1);
        EXPECT_TRUE(m_endPoints.at(1)->chord());

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->deleteItem(m_endPoints.at(1)->chord());
        m_masterScore->endCmd();

        for (TieEndPoint* endPoint : *endPointList) {
            if (endPoint->id() == u"endPoint1") {
                EXPECT_FALSE(endPoint->endTie());
            } else {
                EXPECT_TRUE(endPoint->endTie());
            }
        }

        m_masterScore->undoRedo(true, 0);

        for (TieEndPoint* endPoint : *endPointList) {
            EXPECT_TRUE(endPoint->endTie());
        }
    }

    void toggleFirstEndPoint()
    {
        TieEndPointList* endPointList = m_startNote->tieEndPoints();

        Tie* startTie = endPointList->startTie();

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        endPointList->toggleEndPoint(u"endPoint0");
        m_masterScore->endCmd();

        for (TieEndPoint* endPoint : *endPointList) {
            if (endPoint->id() == u"endPoint0") {
                EXPECT_FALSE(endPoint->endTie());
            } else {
                EXPECT_TRUE(endPoint->endTie());
            }
        }
        Tie* newStartTie = endPointList->startTie();

        EXPECT_NE(startTie, newStartTie);
        EXPECT_TRUE(newStartTie->isPartialTie());

        m_masterScore->undoRedo(true, 0);

        for (TieEndPoint* endPoint : *endPointList) {
            EXPECT_TRUE(endPoint->endTie());
        }

        startTie = endPointList->startTie();
        EXPECT_NE(startTie, newStartTie);
        EXPECT_FALSE(startTie->isPartialTie());
    }

    void deleteStartTie()
    {
        TieEndPointList* endPointList = m_startNote->tieEndPoints();
        EXPECT_TRUE(m_startNote->tieFor()->frontSegment());

        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->deleteItem(m_startNote->tieFor()->frontSegment());
        m_masterScore->endCmd();

        for (TieEndPoint* endPoint : *endPointList) {
            EXPECT_FALSE(endPoint->endTie());
            EXPECT_FALSE(endPoint->active());
        }
    }

    void testSegnoPartialTieFirst(Fraction tickBeforeSegno, Fraction tickAfterSegno)
    {
        // Add a partial tie to the note following a segno, then add a full tie to the note preceding the segno
        addTie();

        TieEndPointList* endPointList = m_startNote->tieEndPoints();

        Note* noteAfterSegno = getNoteAtTick(tickAfterSegno);
        Tie* initialTie = noteAfterSegno->tieBack();
        EXPECT_TRUE(initialTie && initialTie->isPartialTie());

        Note* noteBeforeSegno = getNoteAtTick(tickBeforeSegno);
        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->select(noteBeforeSegno);
        Tie* newTie = m_masterScore->cmdToggleTie();
        m_masterScore->endCmd();

        bool newTieFound = false;
        for (TieEndPoint* endPoint : *endPointList) {
            EXPECT_NE(endPoint->endTie(), initialTie);
            newTieFound |= endPoint->endTie() == newTie;
        }

        EXPECT_TRUE(newTieFound);

        EXPECT_TRUE(newTie);
        EXPECT_FALSE(newTie->isPartialTie());
        EXPECT_NE(newTie, initialTie);

        EXPECT_EQ(newTie->tieEndPoints()->size(), 1);
    }

    void testSegnoPartialTieAfter(Fraction tickBeforeSegno)
    {
        // Add a full tie to the note preceding a segno, then add a tie to the D.S which should add the previous tie to the list of endpoints
        Note* noteBeforeSegno = getNoteAtTick(tickBeforeSegno);
        m_masterScore->startCmd(TranslatableString::untranslatable("Partial tie tests"));
        m_masterScore->select(noteBeforeSegno);
        Tie* t = m_masterScore->cmdToggleTie();
        EXPECT_TRUE(t);
        m_masterScore->endCmd();

        Tie* startTie = addTie();

        EXPECT_EQ(startTie->tieEndPoints()->size(), 1);
    }

    void saveAndLoad(const String& score, const Fraction& startPointLocation, const std::vector<Fraction>& endPointLocations)
    {
        // Save score
        const String savePath = score + u".mscx";
        EXPECT_TRUE(ScoreComp::saveCompareScore(m_masterScore, savePath, PARTIALTIE_DATA_DIR + score + u"-ref.mscx"));
        delete m_masterScore;
        m_masterScore = nullptr;
        m_startNote = nullptr;
        m_endPoints.clear();

        // Load
        openScore(score + u"-ref", startPointLocation, endPointLocations);

        // Check partial tie has endpoints & ties are present
        TieEndPointList* endPointList = m_startNote->tieEndPoints();
        EXPECT_TRUE(endPointList);

        EXPECT_EQ(endPointLocations.size(), endPointList->size());

        for (TieEndPoint* endPoint : *endPointList) {
            EXPECT_EQ(endPoint->endTie()->startTie(), m_startNote->tieFor());
        }

        for (const Note* note : m_endPoints) {
            EXPECT_TRUE(note->tieBack());
        }
    }

private:
    bool m_useRead302 = false;

    MasterScore* m_masterScore = nullptr;
    Note* m_startNote = nullptr;
    std::vector<Note*> m_endPoints;
};

TEST_F(Engraving_PartialTieTests, repeatBarlines)
{
    const String test = u"repeat_barlines";

    const Fraction startPointTick = Fraction(7, 4);
    const std::vector<Fraction> endPoints = { Fraction(8, 4), Fraction(0, 4) };

    testPartialTies(test, startPointTick, endPoints);
}

TEST_F(Engraving_PartialTieTests, voltaCoda)
{
    const String test = u"volta_coda";

    const Fraction startPointTick = Fraction(3, 4);
    const std::vector<Fraction> endPoints = { Fraction(4, 4), Fraction(8, 4), Fraction(12, 4), Fraction(16, 4) };

    testPartialTies(test, startPointTick, endPoints);
}

TEST_F(Engraving_PartialTieTests, coda)
{
    const String test = u"coda";

    const Fraction startPointTick = Fraction(3, 4);
    const std::vector<Fraction> endPoints = { Fraction(4, 4), Fraction(8, 4) };

    testPartialTies(test, startPointTick, endPoints);
}

TEST_F(Engraving_PartialTieTests, segnoBefore)
{
    const String test = u"segno";

    const Fraction startPointTick = Fraction(11, 4);
    const std::vector<Fraction> endPoints = { Fraction(4, 4) };

    openScore(test, startPointTick, endPoints);

    // Add tie to 3,4.  Replace incoming partial tie at 4,4 with full tie
    testSegnoPartialTieFirst(Fraction(3, 4), Fraction(4, 4));
}

TEST_F(Engraving_PartialTieTests, segnoAfter)
{
    const String test = u"segno";

    const Fraction startPointTick = Fraction(11, 4);
    const std::vector<Fraction> endPoints = { Fraction(4, 4) };

    openScore(test, startPointTick, endPoints);

    testSegnoPartialTieAfter(Fraction(3, 4));
}
