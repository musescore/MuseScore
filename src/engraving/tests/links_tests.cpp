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

#include "dom/excerpt.h"
#include "dom/factory.h"
#include "dom/linkedobjects.h"
#include "dom/masterscore.h"
#include "dom/mcursor.h"
#include "dom/measure.h"
#include "dom/part.h"
#include "dom/segment.h"
#include "dom/tempotext.h"
#include "dom/text.h"
#include "dom/undo.h"
#include "utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;

//---------------------------------------------------------
//   TestLinks
//---------------------------------------------------------

class Engraving_LinksTests : public ::testing::Test
{
};

static const String LINKS_DATA_DIR("links_data/");

//---------------------------------------------------------
//   addTitleText
//---------------------------------------------------------

static void addTitleText(Score* score, const String& title)
{
    MeasureBase* measure = score->first();
    if (!measure->isVBox()) {
        score->insertBox(ElementType::VBOX, measure);
        measure = score->first();
    }

    Text* text = Factory::createText(score->dummy(), TextStyleType::TITLE);
    text->setPlainText(title);
    measure->add(text);
}

//---------------------------------------------------------
//   test3LinkedSameScore
///  Create an empty 1 staff score
///  Add 2 linked staff
///  Delete first staff, undo, redo
//---------------------------------------------------------

TEST_F(Engraving_LinksTests, test3LinkedSameScore_99796)
{
    MCursor c;
    c.setTimeSig(Fraction(4, 4));
    c.createScore(nullptr, u"test");
    c.addPart(u"voice");
    c.move(0, Fraction(0, 1));       // move to track 0 tick 0

    c.addKeySig(Key(1));
    c.addTimeSig(Fraction(4, 4));
    c.addChord(60, TDuration(DurationType::V_WHOLE));

    MasterScore* score = c.score();
    score->doLayout();
    Measure* m = score->firstMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    EngravingItem* e = s->element(0);
    EXPECT_TRUE(e->isChord());

    score->select(e);
    score->cmdDeleteSelection();
    e = s->element(0);
    EXPECT_TRUE(e->isRest());
    EXPECT_TRUE(e->links() == nullptr);

    // add a linked staff
    score->startCmd(TranslatableString::untranslatable("Engraving links tests"));
    Staff* oStaff = score->staff(0);
    Staff* staff  = Factory::createStaff(oStaff->part());
    staff->setPart(oStaff->part());
    score->undoInsertStaff(staff, 1, false);
    Excerpt::cloneStaff(oStaff, staff);

    e = s->element(0);
    EXPECT_TRUE(e->isRest());
    EXPECT_TRUE(e->links()->size() == 2);

    // add a second linked staff
    Staff* staff2 = Factory::createStaff(oStaff->part());
    staff2->setPart(oStaff->part());
    score->undoInsertStaff(staff2, 2, false);
    Excerpt::cloneStaff(oStaff, staff2);
    score->endCmd();

    // we should have now 3 staves and 3 linked rests
    EXPECT_TRUE(score->staves().size() == 3);
    e = s->element(0);
    EXPECT_TRUE(e->isRest());
    EXPECT_TRUE(e->links()->size() == 3);
    e = s->element(4);
    EXPECT_TRUE(e->isRest());
    EXPECT_TRUE(e->links()->size() == 3);
    e = s->element(8);
    EXPECT_TRUE(e->isRest());
    EXPECT_TRUE(e->links()->size() == 3);

    // delete staff
    score->startCmd(TranslatableString::untranslatable("Engraving links tests"));
    score->cmdRemoveStaff(0);
    score->endCmd();

    // we have now 2 staves
    EXPECT_TRUE(score->staves().size() == 2);
    e = s->element(0);
    EXPECT_TRUE(e->isRest());
    EXPECT_TRUE(e->links()->size() == 2);
    e = s->element(4);
    EXPECT_TRUE(e->isRest());
    EXPECT_TRUE(e->links()->size() == 2);

    // undo
    score->undoRedo(true, 0);
    // now 3 staves
    EXPECT_TRUE(score->staves().size() == 3);
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 3);
    e = s->element(4);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 3);
    e = s->element(8);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 3);

    // redo, back to 2 staves
    score->undoRedo(false, 0);
    EXPECT_TRUE(score->staves().size() == 2);
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 2);
    e = s->element(4);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 2);
}

//---------------------------------------------------------
//   test3LinkedSameScore
///  Create an empty 1 staff score
///  Create part
///  Add linked staff
///  Delete part
//---------------------------------------------------------

TEST_F(Engraving_LinksTests, test3LinkedParts_99796)
{
    MCursor c;
    c.setTimeSig(Fraction(4, 4));
    c.createScore(nullptr, u"test");
    c.addPart(u"voice");
    c.move(0, Fraction(0, 1));       // move to track 0 tick 0

    c.addKeySig(Key(1));
    c.addTimeSig(Fraction(4, 4));
    c.addChord(60, TDuration(DurationType::V_WHOLE));

    MasterScore* score = c.score();
    addTitleText(score, u"Title");
    score->doLayout();
    // delete chord
    Measure* m = score->firstMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    EngravingItem* e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::CHORD);
    score->select(e);
    score->cmdDeleteSelection();
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links() == nullptr);

    // create parts
    score->startCmd(TranslatableString::untranslatable("Engraving links tests"));
    std::vector<Part*> parts;
    parts.push_back(score->parts().at(0));
    Score* nscore = score->createScore();
    Excerpt ex(score);
    ex.setExcerptScore(nscore);
    ex.setName(u"voice");
    ex.setParts(parts);
    Excerpt::createExcerpt(&ex);
    EXPECT_TRUE(nscore);
    score->undo(new AddExcerpt(&ex));
    score->endCmd();

    // add a linked staff
    score->startCmd(TranslatableString::untranslatable("Engraving links tests"));
    Staff* oStaff = score->staff(0);
    Staff* staff  = Factory::createStaff(oStaff->part());
    staff->setPart(oStaff->part());
    score->undoInsertStaff(staff, 1, false);
    Excerpt::cloneStaff(oStaff, staff);
    score->endCmd();

    // we should have now 2 staves and 3 linked rests
    EXPECT_TRUE(score->staves().size() == 2);
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 3);
    e = s->element(4);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 3);

    // delete part
    score->startCmd(TranslatableString::untranslatable("Engraving links tests"));
    score->deleteExcerpt(&ex);
    score->undo(new RemoveExcerpt(&ex));

    // we should have now 2 staves and *2* linked rests
    EXPECT_TRUE(score->staves().size() == 2);
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 2);
    e = s->element(4);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 2);
}

//---------------------------------------------------------
//   test4LinkedParts_94911
///  Create an empty 1 staff score
///  Add linked staff
///  Create part
///  Delete linked staff, undo, redo
//---------------------------------------------------------

TEST_F(Engraving_LinksTests, DISABLED_test4LinkedParts_94911)
{
    MCursor c;
    c.setTimeSig(Fraction(4, 4));
    c.createScore(nullptr, u"test");
    c.addPart(u"electric-guitar");
    c.move(0, Fraction(0, 1));       // move to track 0 tick 0

    //c.addKeySig(Key(1));
    //c.addTimeSig(Fraction(4,4));
    c.addChord(60, TDuration(DurationType::V_WHOLE));

    MasterScore* score = c.score();
    addTitleText(score, u"Title");
    score->doLayout();
    // delete chord
    Measure* m = score->firstMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    EngravingItem* e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::CHORD);
    score->select(e);
    score->cmdDeleteSelection();
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links() == nullptr);

    // add a linked staff
    score->startCmd(TranslatableString::untranslatable("Engraving links tests"));
    Staff* oStaff = score->staff(0);
    Staff* staff  = Factory::createStaff(oStaff->part());
    staff->setPart(oStaff->part());
    score->undoInsertStaff(staff, 1, false);
    Excerpt::cloneStaff(oStaff, staff);
    score->endCmd();

    // we should have now 2 staves and 2 linked rests
    EXPECT_TRUE(score->staves().size() == 2);
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 2);
    e = s->element(4);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 2);

    // create parts
    score->startCmd(TranslatableString::untranslatable("Engraving links tests"));
    std::vector<Part*> parts;
    parts.push_back(score->parts().at(0));
    Score* nscore = score->createScore();
    Excerpt ex(score);
    ex.setExcerptScore(nscore);
    ex.setName(u"Guitar");
    ex.setParts(parts);
    Excerpt::createExcerpt(&ex);
    EXPECT_TRUE(nscore);
    //nscore->setName(parts.front()->partName());
    score->undo(new AddExcerpt(&ex));
    score->endCmd();

    // we should have now 2 staves and 4 linked rests
    EXPECT_TRUE(score->staves().size() == 2);
    EXPECT_TRUE(nscore->staves().size() == 2);
    EXPECT_TRUE(score->staves()[0]->links()->size() == 4);
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 4);
    e = s->element(4);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 4);
    EXPECT_TRUE(score->excerpts().size() == 1);

    // delete second staff
    score->startCmd(TranslatableString::untranslatable("Engraving links tests"));
    score->cmdRemoveStaff(1);
    for (Excerpt* excerpt : score->excerpts()) {
        std::vector<Staff*> sl = nscore->staves();
        if (sl.size() == 0) {
            score->undo(new RemoveExcerpt(excerpt));
        }
    }
    score->endCmd();

    // we should have now 2 staves and *4* linked rest
    // no excerpt
    EXPECT_TRUE(score->staves().size() == 1);
    //EXPECT_TRUE(score->staves()[0]->links() == nullptr);
    e = s->element(0);
    EXPECT_TRUE(e->isRest());
    EXPECT_TRUE(e->links() == nullptr);
    qDebug() << score->excerpts().size();

    // undo
    score->undoRedo(true, 0);
    // we should have now 2 staves and 4 linked rests
    EXPECT_EQ(nscore->staves().size(), 2);
    EXPECT_EQ(score->staves().size(), 2);
    EXPECT_TRUE(score->staves()[0]->links()->size() == 4);
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 4);
    e = s->element(4);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 4);
    EXPECT_TRUE(score->excerpts().size() == 1);

    // redo
    score->undoRedo(false, 0);
    // we should have now 2 staves and *4* linked rest
    // no excerpt
    EXPECT_TRUE(score->staves().size() == 1);
    EXPECT_TRUE(score->staves()[0]->links() == nullptr);
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links() == nullptr);
}

//---------------------------------------------------------
//   test5LinkedParts_94911
///  Create an empty 1 staff score
///  Create part
///  Add linked staff, undo, redo
//---------------------------------------------------------

TEST_F(Engraving_LinksTests, test5LinkedParts_94911)
{
    MCursor c;
    c.setTimeSig(Fraction(4, 4));
    c.createScore(nullptr, u"test");
    c.addPart(u"electric-guitar");
    c.move(0, Fraction(0, 1));       // move to track 0 tick 0

    //c.addKeySig(Key(1));
    //c.addTimeSig(Fraction(4,4));
    c.addChord(60, TDuration(DurationType::V_WHOLE));

    MasterScore* score = c.score();
    addTitleText(score, u"Title");
    score->doLayout();
    // delete chord
    Measure* m = score->firstMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    EngravingItem* e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::CHORD);
    score->select(e);
    score->cmdDeleteSelection();
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links() == nullptr);

    // create parts//
    score->startCmd(TranslatableString::untranslatable("Engraving links tests"));
    std::vector<Part*> parts;
    parts.push_back(score->parts().at(0));
    Score* nscore = score->createScore();
    Excerpt ex(score);
    ex.setExcerptScore(nscore);
    ex.setName(u"Guitar");
    ex.setParts(parts);
    Excerpt::createExcerpt(&ex);
    EXPECT_TRUE(nscore);
    score->undo(new AddExcerpt(&ex));
    score->endCmd();

    // we should have now 1 staff and 2 linked rests
    EXPECT_TRUE(score->staves().size() == 1);
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 2);

    // add a linked staff
    score->startCmd(TranslatableString::untranslatable("Engraving links tests"));
    Staff* oStaff = score->staff(0);
    Staff* staff  = Factory::createStaff(oStaff->part());
    staff->setPart(oStaff->part());
    score->undoInsertStaff(staff, 1, false);
    Excerpt::cloneStaff(oStaff, staff);
    score->endCmd();

    // we should have now 2 staves and 3 linked rests
    EXPECT_EQ(score->staves().size(), 2);
    EXPECT_EQ(nscore->staves().size(), 1);
    EXPECT_TRUE(score->staves()[0]->links()->size() == 3);
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 3);
    e = s->element(4);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 3);
    EXPECT_TRUE(score->excerpts().size() == 1);

    // undo
    score->undoRedo(true, 0);
    // we should have now 1 staves and 2 linked rests
    EXPECT_TRUE(score->staves().size() == 1);
    EXPECT_TRUE(score->staves()[0]->links()->size() == 2);
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 2);
    EXPECT_TRUE(score->excerpts().size() == 1);

    // redo
    score->undoRedo(false, 0);
    // we should have now 2 staves and 3 linked rests
    EXPECT_TRUE(score->staves().size() == 2);
    EXPECT_TRUE(score->staves()[0]->links()->size() == 3);
    e = s->element(0);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 3);
    e = s->element(4);
    EXPECT_TRUE(e->type() == ElementType::REST);
    EXPECT_TRUE(e->links()->size() == 3);
    EXPECT_TRUE(score->excerpts().size() == 1);
}

TEST_F(Engraving_LinksTests, DISABLED_testMMRestLink)
{
    // NOTE: Temporarily disabling this test because it assumes that the loaded score has multiMeasureRests active,
    // but that is not the case because the mscx file *doesn't contain the style information*, so this
    // score gets actually loaded *without* mmRest, making the test invalid. [M.S.]

    MasterScore* score = ScoreRW::readScore(LINKS_DATA_DIR + u"testMMRestLink.mscx");
    ASSERT_TRUE(score);

    // Get item
    MeasureBase* measureBase2 = score->measure(2);
    Measure* measure2 = measureBase2->isMeasure() ? toMeasure(measureBase2) : nullptr;

    EXPECT_TRUE(measure2);

    Segment* seg = measure2->findSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    EXPECT_TRUE(seg);

    TempoText* tempoText = nullptr;
    for (EngravingItem* el : seg->annotations()) {
        if (el->isTempoText()) {
            tempoText = toTempoText(el);
        }
    }
    EXPECT_TRUE(tempoText);

    // Make element invisible
    if (tempoText) {
        tempoText->undoSetVisible(false);
    }

    // Check we have a linked item in the MMR
    EXPECT_EQ(tempoText->linkList().size(), 2);

    // Make sure all linked items are invisible
    for (EngravingObject* linkedObj : tempoText->linkList()) {
        EngravingItem* linkedItem = toEngravingItem(linkedObj);
        EXPECT_FALSE(linkedItem->visible());
    }
    delete score;
}
