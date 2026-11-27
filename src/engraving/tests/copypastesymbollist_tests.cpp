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

#include <QMimeData>

#include "engraving/internal/qmimedataadapter.h"

#include "engraving/dom/breath.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"

#include "engraving/editing/paste.h"
#include "engraving/editing/transaction/transaction.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

#include "log.h"

using namespace mu::engraving;

static const String CPSYMBOLLIST_DATA_DIR(u"copypastesymbollist_data/");

class Engraving_CopyPasteSymbolListTests : public ::testing::Test
{
protected:
    void copypastecommon(MasterScore*, const char16_t*);
    void copypaste(const char16_t*, ElementType);
    void copypastepart(const char16_t*, ElementType);
    void copypastedifferentvoice(const char16_t*, ElementType);
    void copypasteLyricsAcrossSymbol(const std::function<EngravingItem* (MasterScore*, Chord*)>& addSymbol);
};

//---------------------------------------------------------
//   copy and paste to first chord in measure 4
//---------------------------------------------------------
void Engraving_CopyPasteSymbolListTests::copypastecommon(MasterScore* score, const char16_t* name)
{
    // copy selection to clipboard
    EXPECT_TRUE(score->selection().canCopy());
    String mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData().toQByteArray());

    // select first chord in 5th measure
    Measure* m = score->firstMeasure();
    for (int i=0; i < 4; i++) {
        m = m->nextMeasure();
    }
    score->select(m->first()->element(0));

    score->startCmd(TranslatableString::untranslatable("Copy/paste symbol tests"));
    if (!mimeData->hasFormat(mimeSymbolListFormat)) {
        LOGD("wrong type mime data");
        return;
    }

    QMimeDataAdapter ma(mimeData);
    Paste::paste(score->transactionManager()->currentOrDummyTransaction(), score, &ma, 0);
    score->endCmd();
    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("copypastesymbollist-%1.mscx").arg(name),
                                            CPSYMBOLLIST_DATA_DIR + String("copypastesymbollist-%1-ref.mscx").arg(name)));
    delete score;
}

//---------------------------------------------------------
//    select all elements of type and copy paste
//---------------------------------------------------------
void Engraving_CopyPasteSymbolListTests::copypaste(const char16_t* name, ElementType type)
{
    MasterScore* score = ScoreRW::readScore(CPSYMBOLLIST_DATA_DIR + String("copypastesymbollist-%1.mscx").arg(name));
    ASSERT_TRUE(score);

    EngravingItem* el = Factory::createItem(type, score->dummy());
    score->selectSimilar(el, false);
    delete el;

    copypastecommon(score, name);
}

TEST_F(Engraving_CopyPasteSymbolListTests, copypasteArticulation)
{
    copypaste(u"articulation", ElementType::ARTICULATION);
}

TEST_F(Engraving_CopyPasteSymbolListTests, copypasteOrnament)
{
    copypaste(u"ornament", ElementType::ORNAMENT);
}

TEST_F(Engraving_CopyPasteSymbolListTests, copypasteChordNames)
{
    copypaste(u"chordnames", ElementType::HARMONY);
}

TEST_F(Engraving_CopyPasteSymbolListTests, copypasteChordNames1)
{
    copypaste(u"chordnames-01", ElementType::HARMONY);
}

TEST_F(Engraving_CopyPasteSymbolListTests, DISABLED_copypasteFiguredBass)
{
    copypaste(u"figuredbass", ElementType::FIGURED_BASS);
}

TEST_F(Engraving_CopyPasteSymbolListTests, copypasteLyrics)
{
    copypaste(u"lyrics", ElementType::LYRICS);
}

TEST_F(Engraving_CopyPasteSymbolListTests, copypasteStaffText)
{
    copypaste(u"stafftext", ElementType::STAFF_TEXT);
}

TEST_F(Engraving_CopyPasteSymbolListTests, copypasteSticking)
{
    copypaste(u"sticking", ElementType::STICKING);
}

TEST_F(Engraving_CopyPasteSymbolListTests, copypasteTremoloSingleChord)
{
    copypaste(u"tremolo-single-chord", ElementType::TREMOLO_SINGLECHORD);
}

TEST_F(Engraving_CopyPasteSymbolListTests, copypasteArticulationRest)
{
    copypaste(u"articulation-rest", ElementType::ARTICULATION);
}

TEST_F(Engraving_CopyPasteSymbolListTests, DISABLED_copypasteFermataRest)
{
    copypaste(u"fermata-rest", ElementType::ARTICULATION);
}

//---------------------------------------------------------
//    select all elements of type in 2 first measures
//    in the first staff and copy paste
//---------------------------------------------------------
void Engraving_CopyPasteSymbolListTests::copypastepart(const char16_t* name, ElementType type)
{
    MasterScore* score = ScoreRW::readScore(CPSYMBOLLIST_DATA_DIR + String("copypastesymbollist-%1.mscx").arg(name));
    ASSERT_TRUE(score);
    score->doLayout();

    //select all
    score->select(score->firstMeasure());
    score->select(score->firstMeasure()->nextMeasure(), SelectType::RANGE);

    EngravingItem* el = Factory::createItem(type, score->dummy());
    score->selectSimilarInRange(el);
    delete el;

    copypastecommon(score, name);
}

TEST_F(Engraving_CopyPasteSymbolListTests, copypasteRange)
{
    copypastepart(u"range", ElementType::ARTICULATION);
}

//---------------------------------------------------------
//    select all elements of type in 2 first measures
//    in both staves and copy paste
//---------------------------------------------------------
void Engraving_CopyPasteSymbolListTests::copypastedifferentvoice(const char16_t* name, ElementType type)
{
    MasterScore* score = ScoreRW::readScore(CPSYMBOLLIST_DATA_DIR + String(u"copypastesymbollist-%1.mscx").arg(name));
    ASSERT_TRUE(score);
    score->doLayout();

    //select all
    score->select(score->firstMeasure());
    score->select(score->firstMeasure()->nextMeasure(), SelectType::RANGE, 1);

    EngravingItem* el = Factory::createItem(type, score->dummy());
    score->selectSimilarInRange(el);
    delete el;

    copypastecommon(score, name);
}

TEST_F(Engraving_CopyPasteSymbolListTests, copypasteRange1)
{
    copypastedifferentvoice(u"range-01", ElementType::ARTICULATION);
}

//---------------------------------------------------------
//    Lyrics are pasted by chord distance (segDelta). Another symbol selected
//    between two lyrics must not shift the second lyric to an earlier chord,
//    nor make it be dropped (e.g. a Breath on a non-ChordRest segment).
//
//    Source (measure 1): half note (lyric "Nel"), quarter (the symbol), quarter (lyric "zo").
//    Destination (measure 5): four quarters; "zo" must land on the 3rd one.
//---------------------------------------------------------
void Engraving_CopyPasteSymbolListTests::copypasteLyricsAcrossSymbol(
    const std::function<EngravingItem* (MasterScore*, Chord*)>& addSymbol)
{
    MasterScore* score = ScoreRW::readScore(CPSYMBOLLIST_DATA_DIR + u"copypastesymbollist-lyrics.mscx");
    ASSERT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Segment* s0 = m1->first(SegmentType::ChordRest);
    Segment* s1 = s0->next(SegmentType::ChordRest);
    Segment* s2 = s1->next(SegmentType::ChordRest);
    ASSERT_TRUE(s0 && s1 && s2);
    Lyrics* lyricA = toChord(s0->element(0))->lyrics().front();
    Lyrics* lyricB = toChord(s2->element(0))->lyrics().front();

    score->startCmd(TranslatableString::untranslatable("Copy/paste symbol tests"));
    EngravingItem* symbol = addSymbol(score, toChord(s1->element(0)));
    score->endCmd();
    ASSERT_TRUE(symbol);

    score->select(lyricA, SelectType::SINGLE);
    score->select(symbol, SelectType::ADD);
    score->select(lyricB, SelectType::ADD);
    ASSERT_EQ(score->selection().mimeType(), mimeSymbolListFormat);

    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeSymbolListFormat, score->selection().mimeData().toQByteArray());

    Measure* m5 = m1;
    for (int i = 0; i < 4; ++i) {
        m5 = m5->nextMeasure();
        ASSERT_TRUE(m5);
    }
    score->select(m5->first(SegmentType::ChordRest)->element(0));

    score->startCmd(TranslatableString::untranslatable("Copy/paste symbol tests"));
    QMimeDataAdapter ma(mimeData);
    Paste::paste(score->transactionManager()->currentOrDummyTransaction(), score, &ma, 0);
    score->endCmd();

    std::vector<std::pair<String, Fraction> > pasted;
    for (Segment* s = m5->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        ChordRest* cr = s->cr(0);
        if (cr && cr->isChord()) {
            for (Lyrics* l : toChord(cr)->lyrics()) {
                pasted.emplace_back(l->plainText(), s->rtick());
            }
        }
    }

    ASSERT_EQ(pasted.size(), 2);
    EXPECT_EQ(pasted[0].first, u"Nel");
    EXPECT_EQ(pasted[0].second, Fraction(0, 1));
    EXPECT_EQ(pasted[1].first, u"zo");
    EXPECT_EQ(pasted[1].second, Fraction(1, 2));

    delete mimeData;
    delete score;
}

TEST_F(Engraving_CopyPasteSymbolListTests, copypasteLyricsAcrossDynamic)
{
    copypasteLyricsAcrossSymbol([](MasterScore* score, Chord* chord) -> EngravingItem* {
        Dynamic* dynamic = Factory::createDynamic(score->dummy());
        dynamic->setDynamicType(DynamicType::F);
        dynamic->setTrack(0);
        dynamic->setOwnershipParent(chord->segment());
        score->undoAddElement(dynamic);
        return dynamic;
    });
}

TEST_F(Engraving_CopyPasteSymbolListTests, copypasteLyricsAcrossBreath)
{
    copypasteLyricsAcrossSymbol([](MasterScore* score, Chord* chord) -> EngravingItem* {
        // Breath segment is not a ChordRest segment
        Segment* breathSeg = chord->measure()->undoGetSegment(SegmentType::Breath, chord->tick());
        Breath* breath = Factory::createBreath(breathSeg);
        breath->setSymId(SymId::breathMarkComma);
        breath->setTrack(0);
        breath->setOwnershipParent(breathSeg);
        score->undoAddElement(breath);
        return breath;
    });
}
