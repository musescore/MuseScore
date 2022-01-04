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

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

#include "engraving/rw/xml.h"
#include "libmscore/masterscore.h"
#include "libmscore/system.h"
#include "libmscore/measure.h"
#include "libmscore/chord.h"
#include "libmscore/factory.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

static const QString CPSYMBOLLIST_DATA_DIR("copypastesymbollist_data/");

using namespace mu::engraving;
using namespace Ms;

class CopyPasteSymbolListTests : public ::testing::Test
{
public:
    void copypastecommon(MasterScore*, const char*);
    void copypaste(const char*, ElementType);
    void copypastepart(const char*, ElementType);
    void copypastedifferentvoice(const char*, ElementType);
};

//---------------------------------------------------------
//   copy and paste to first chord in measure 4
//---------------------------------------------------------
void CopyPasteSymbolListTests::copypastecommon(MasterScore* score, const char* name)
{
    // copy selection to clipboard
    EXPECT_TRUE(score->selection().canCopy());
    QString mimeType = score->selection().mimeType();
    EXPECT_TRUE(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData());
    QApplication::clipboard()->setMimeData(mimeData);

    // select first chord in 5th measure
    Measure* m = score->firstMeasure();
    for (int i=0; i < 4; i++) {
        m = m->nextMeasure();
    }
    score->select(m->first()->element(0));

    score->startCmd();
    const QMimeData* ms = QApplication::clipboard()->mimeData();
    if (!ms->hasFormat(mimeSymbolListFormat)) {
        qDebug("wrong type mime data");
        return;
    }
    score->cmdPaste(ms, 0);
    score->endCmd();
    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, QString("copypastesymbollist-%1.mscx").arg(name),
                                            CPSYMBOLLIST_DATA_DIR + QString("copypastesymbollist-%1-ref.mscx").arg(name)));
    delete score;
}

//---------------------------------------------------------
//    select all elements of type and copy paste
//---------------------------------------------------------
void CopyPasteSymbolListTests::copypaste(const char* name, ElementType type)
{
    MasterScore* score = ScoreRW::readScore(CPSYMBOLLIST_DATA_DIR + QString("copypastesymbollist-%1.mscx").arg(name));
    EXPECT_TRUE(score);

    EngravingItem* el = Factory::createItem(type, score->dummy());
    score->selectSimilar(el, false);
    delete el;

    copypastecommon(score, name);
}

TEST_F(CopyPasteSymbolListTests, copypasteArticulation)
{
    copypaste("articulation", ElementType::ARTICULATION);
}

TEST_F(CopyPasteSymbolListTests, copypasteChordNames)
{
    copypaste("chordnames", ElementType::HARMONY);
}

TEST_F(CopyPasteSymbolListTests, copypasteChordNames1)
{
    copypaste("chordnames-01", ElementType::HARMONY);
}

TEST_F(CopyPasteSymbolListTests, DISABLED_copypasteFiguredBass)
{
    copypaste("figuredbass", ElementType::FIGURED_BASS);
}

TEST_F(CopyPasteSymbolListTests, copypasteLyrics)
{
    copypaste("lyrics", ElementType::LYRICS);
}

TEST_F(CopyPasteSymbolListTests, copypasteStaffText)
{
    copypaste("stafftext", ElementType::STAFF_TEXT);
}

TEST_F(CopyPasteSymbolListTests, copypasteSticking)
{
    copypaste("sticking", ElementType::STICKING);
}

TEST_F(CopyPasteSymbolListTests, copypasteArticulationRest)
{
    copypaste("articulation-rest", ElementType::ARTICULATION);
}

TEST_F(CopyPasteSymbolListTests, DISABLED_copypasteFermataRest)
{
    copypaste("fermata-rest", ElementType::ARTICULATION);
}

//---------------------------------------------------------
//    select all elements of type in 2 first measures
//    in the first staff and copy paste
//---------------------------------------------------------
void CopyPasteSymbolListTests::copypastepart(const char* name, ElementType type)
{
    MasterScore* score = ScoreRW::readScore(CPSYMBOLLIST_DATA_DIR + QString("copypastesymbollist-%1.mscx").arg(name));
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

TEST_F(CopyPasteSymbolListTests, copypasteRange)
{
    copypastepart("range", ElementType::ARTICULATION);
}

//---------------------------------------------------------
//    select all elements of type in 2 first measures
//    in both staves and copy paste
//---------------------------------------------------------
void CopyPasteSymbolListTests::copypastedifferentvoice(const char* name, ElementType type)
{
    MasterScore* score = ScoreRW::readScore(CPSYMBOLLIST_DATA_DIR + QString("copypastesymbollist-%1.mscx").arg(name));
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

TEST_F(CopyPasteSymbolListTests, copypasteRange1)
{
    copypastedifferentvoice("range-01", ElementType::ARTICULATION);
}
