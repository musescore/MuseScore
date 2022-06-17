/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "libmscore/masterscore.h"
#include "libmscore/undo.h"
#include "libmscore/chord.h"
#include "libmscore/segment.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String EXCHVOICES_DATA_DIR("exchangevoices_data/");

class ExchangevoicesTests : public ::testing::Test
{
};

TEST_F(ExchangevoicesTests, slurs)
{
    Score* score = ScoreRW::readScore(EXCHVOICES_DATA_DIR + "exchangevoices-slurs.mscx");
    EXPECT_TRUE(score);
    score->doLayout();

    // select all
    score->startCmd();
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd();
    score->cmdExchangeVoice(0, 1);
    score->endCmd();

    // compare
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"exchangevoices-slurs.mscx", EXCHVOICES_DATA_DIR + u"exchangevoices-slurs-ref.mscx"));
}

TEST_F(ExchangevoicesTests, glissandi)
{
    Score* score = ScoreRW::readScore(EXCHVOICES_DATA_DIR + u"exchangevoices-gliss.mscx");
    EXPECT_TRUE(score);
    score->doLayout();

    // select all
    score->startCmd();
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd();
    score->cmdExchangeVoice(0, 1);
    score->endCmd();

    // compare
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"exchangevoices-gliss.mscx", EXCHVOICES_DATA_DIR + u"exchangevoices-gliss-ref.mscx"));
}

TEST_F(ExchangevoicesTests, undoChangeVoice)
{
    String readFile(EXCHVOICES_DATA_DIR + u"undoChangeVoice.mscx");
    String writeFile1(u"undoChangeVoice01-test.mscx");
    String reference1(EXCHVOICES_DATA_DIR + u"undoChangeVoice01-ref.mscx");
    String writeFile2(u"undoChangeVoice02-test.mscx");
    String reference2(EXCHVOICES_DATA_DIR + u"undoChangeVoice02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);
    score->doLayout();

    // do
    score->deselectAll();
    // select bottom note of all voice 1 chords
    for (Segment* s = score->firstSegment(SegmentType::ChordRest); s; s = s->next1()) {
        ChordRest* cr = static_cast<ChordRest*>(s->element(0));
        if (cr && cr->type() == ElementType::CHORD) {
            Chord* c = toChord(cr);
            score->select(c->downNote(), SelectType::ADD);
        }
    }
    // change voice
    score->startCmd();
    score->changeSelectedNotesVoice(1);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}
