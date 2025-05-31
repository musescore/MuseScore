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

#include "dom/factory.h"
#include "dom/masterscore.h"
#include "dom/measure.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

static const String CPSYMBOLLIST_DATA_DIR(u"copypastesymbollist_data/");

class Engraving_CopyPasteSymbolListTests : public ::testing::Test
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
    }

    void copypastecommon(MasterScore*, const char16_t*);
    void copypaste(const char16_t*, ElementType);
    void copypastepart(const char16_t*, ElementType);
    void copypastedifferentvoice(const char16_t*, ElementType);

private:
    bool m_useRead302 = false;
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
    score->cmdPaste(&ma, 0);
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
    EXPECT_TRUE(score);

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

TEST_F(Engraving_CopyPasteSymbolListTests, copypasteClef)
{
    copypaste(u"clef", ElementType::CLEF);
}

//---------------------------------------------------------
//    select all elements of type in 2 first measures
//    in the first staff and copy paste
//---------------------------------------------------------
void Engraving_CopyPasteSymbolListTests::copypastepart(const char16_t* name, ElementType type)
{
    MasterScore* score = ScoreRW::readScore(CPSYMBOLLIST_DATA_DIR + String("copypastesymbollist-%1.mscx").arg(name));
    EXPECT_TRUE(score);
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
    EXPECT_TRUE(score);
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
