/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "types/translatablestring.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/jianpuoctavedot.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String JIANPU_OCTAVE_DOTS_DATA_DIR("beam_data/");

namespace {
struct NoteLocator {
    Fraction tick;
    track_idx_t track = 0;
    int pitch = -1;
};

Note* findJianpuNoteInChord(MasterScore* score)
{
    for (Segment* seg = score->firstSegment(SegmentType::ChordRest); seg; seg = seg->next1(SegmentType::ChordRest)) {
        for (track_idx_t track = 0; track < score->ntracks(); ++track) {
            EngravingItem* item = seg->element(track);
            if (!item || !item->isChord()) {
                continue;
            }

            Chord* chord = toChord(item);
            if (!chord->onJianpuStaff() || chord->notes().empty()) {
                continue;
            }

            return chord->notes().front();
        }
    }

    return nullptr;
}

Note* findNoteByLocator(MasterScore* score, const NoteLocator& locator)
{
    for (Segment* seg = score->firstSegment(SegmentType::ChordRest); seg; seg = seg->next1(SegmentType::ChordRest)) {
        EngravingItem* item = seg->element(locator.track);
        if (!item || !item->isChord()) {
            continue;
        }

        Chord* chord = toChord(item);
        if (chord->tick() != locator.tick) {
            continue;
        }

        for (Note* note : chord->notes()) {
            if (locator.pitch == -1 || note->pitch() == locator.pitch) {
                return note;
            }
        }
    }

    return nullptr;
}

void expectJianpuOctaveDotsOwnedByNote(const Note* note)
{
    ASSERT_FALSE(note->jianpuOctaveDots().empty());

    for (JianpuOctaveDot* octaveDot : note->jianpuOctaveDots()) {
        ASSERT_NE(octaveDot, nullptr);
        EXPECT_EQ(octaveDot->ownershipParent(), note);
        EXPECT_EQ(octaveDot->track(), note->track());
    }
}
}

class Engraving_JianpuOctaveDotTests : public ::testing::Test
{
};

TEST_F(Engraving_JianpuOctaveDotTests, ledgerLineOctaveDependsOnClef)
{
    MasterScore* score = ScoreRW::readScore(JIANPU_OCTAVE_DOTS_DATA_DIR + u"jianpuBeam.mscx");
    ASSERT_TRUE(score);
    Note* note = findJianpuNoteInChord(score);
    ASSERT_TRUE(note);
    Staff* staff = score->staff(note->staffIdx());

    struct TestCase {
        ClefType clef;
        int pitch;
    };
    for (const TestCase& test : { TestCase{ ClefType::G, 60 }, TestCase{ ClefType::G8_VB, 48 },
                                  TestCase{ ClefType::G8_VA, 72 }, TestCase{ ClefType::G8_VB_P, 60 } }) {
        SCOPED_TRACE(test.pitch);
        staff->setDefaultClefType(ClefTypeList(test.clef));
        ASSERT_EQ(staff->clef(note->tick()), test.clef);

        note->setPitch(test.pitch);
        score->doLayout();
        EXPECT_TRUE(note->jianpuOctaveDots().empty());

        note->setPitch(test.pitch + 12);
        score->doLayout();
        ASSERT_EQ(note->jianpuOctaveDots().size(), 1);
        EXPECT_TRUE(note->jianpuOctaveDots().front()->ldata()->above);
    }

    delete score;
}

TEST_F(Engraving_JianpuOctaveDotTests, layoutDataStoresGeometry)
{
    MasterScore* score = ScoreRW::readScore(JIANPU_OCTAVE_DOTS_DATA_DIR + u"jianpuBeam.mscx");
    ASSERT_TRUE(score);

    Note* note = findJianpuNoteInChord(score);
    ASSERT_TRUE(note);

    note->setJianpuOctaveDotCount(1);
    ASSERT_FALSE(note->jianpuOctaveDots().empty());

    JianpuOctaveDot* octaveDot = note->jianpuOctaveDots().front();
    EXPECT_EQ(octaveDot->note(), note);

    JianpuOctaveDot::LayoutData* ldata = octaveDot->mutldata();
    ldata->len = 2.0;
    ldata->above = true;
    EXPECT_DOUBLE_EQ(ldata->len, 2.0);
    EXPECT_TRUE(ldata->above);

    delete score;
}

TEST_F(Engraving_JianpuOctaveDotTests, jianpuUndoRedoRemoveNoteKeepsOctaveDotsOwnership)
{
    MasterScore* score = ScoreRW::readScore(JIANPU_OCTAVE_DOTS_DATA_DIR + u"jianpuBeam.mscx");
    ASSERT_TRUE(score);

    Note* original = findJianpuNoteInChord(score);
    ASSERT_TRUE(original);

    original->setPitch(72); // C5 has a dot with the default treble clef.
    score->doLayout();
    const NoteLocator locator { original->chord()->tick(), original->track(), original->pitch() };
    expectJianpuOctaveDotsOwnedByNote(original);

    Chord* chord = original->chord();
    score->startCmd(muse::TranslatableString::untranslatable("Jianpu octave dots tests"));
    score->undoRemoveElement(chord);
    score->endCmd();

    EXPECT_EQ(findNoteByLocator(score, locator), nullptr);

    score->undoRedo(true, nullptr);

    Note* restored = findNoteByLocator(score, locator);
    ASSERT_TRUE(restored);
    expectJianpuOctaveDotsOwnedByNote(restored);

    score->undoRedo(false, nullptr);
    EXPECT_EQ(findNoteByLocator(score, locator), nullptr);

    delete score;
}

TEST_F(Engraving_JianpuOctaveDotTests, jianpuCloneNoteKeepsOctaveDotsOwnership)
{
    MasterScore* score = ScoreRW::readScore(JIANPU_OCTAVE_DOTS_DATA_DIR + u"jianpuBeam.mscx");
    ASSERT_TRUE(score);

    Note* source = findJianpuNoteInChord(score);
    ASSERT_TRUE(source);

    // Force octave dots to be allocated on source
    source->setJianpuOctaveDotCount(1);
    expectJianpuOctaveDotsOwnedByNote(source);

    EngravingItem* clonedItem = source->clone();
    ASSERT_TRUE(clonedItem);
    ASSERT_TRUE(clonedItem->isNote());

    Note* cloned = toNote(clonedItem);
    cloned->setJianpuOctaveDotCount(1);
    expectJianpuOctaveDotsOwnedByNote(cloned);

    JianpuOctaveDot* clonedOctaveDot = cloned->jianpuOctaveDots().front();
    EXPECT_NE(clonedOctaveDot, source->jianpuOctaveDots().front());

    delete cloned;
    delete score;
}

TEST_F(Engraving_JianpuOctaveDotTests, jianpuSaveReloadKeepsOctaveDotsOwnership)
{
    String tempFile(u"jianpu-octavedots-save-reload-test.mscx");

    MasterScore* score = ScoreRW::readScore(JIANPU_OCTAVE_DOTS_DATA_DIR + u"jianpuBeam.mscx");
    ASSERT_TRUE(score);

    Note* beforeSave = findJianpuNoteInChord(score);
    ASSERT_TRUE(beforeSave);

    beforeSave->setPitch(72); // The dot must also be generated when the score is reloaded.
    score->doLayout();
    const NoteLocator locator { beforeSave->chord()->tick(), beforeSave->track(), beforeSave->pitch() };
    expectJianpuOctaveDotsOwnedByNote(beforeSave);

    ASSERT_TRUE(ScoreRW::saveScore(score, tempFile));
    delete score;

    MasterScore* reloaded = ScoreRW::readScore(tempFile, true);
    ASSERT_TRUE(reloaded);

    Note* afterReload = findNoteByLocator(reloaded, locator);
    ASSERT_TRUE(afterReload);
    expectJianpuOctaveDotsOwnedByNote(afterReload);

    delete reloaded;
}
