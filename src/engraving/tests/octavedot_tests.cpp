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

#include "types/translatablestring.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/octavedot.h"
#include "engraving/dom/segment.h"

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
            if (!chord->isJianpuStaff() || chord->notes().empty()) {
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

void expectOctaveDotsOwnedByNote(const Note* note)
{
    ASSERT_FALSE(note->octaveDots().empty());

    for (OctaveDot* octaveDot : note->octaveDots()) {
        ASSERT_NE(octaveDot, nullptr);
        EXPECT_EQ(octaveDot->explicitParent(), note);
        EXPECT_EQ(octaveDot->track(), note->track());
    }
}
}

class Engraving_OctaveDotTests : public ::testing::Test
{
};

TEST_F(Engraving_OctaveDotTests, propertiesAndSpatiumScaling)
{
    MasterScore* score = ScoreRW::readScore(JIANPU_OCTAVE_DOTS_DATA_DIR + u"jianpuBeam.mscx");
    ASSERT_TRUE(score);

    Note* note = findJianpuNoteInChord(score);
    ASSERT_TRUE(note);

    note->resizeOctaveDotsTo(1);
    ASSERT_FALSE(note->octaveDots().empty());

    OctaveDot* octaveDot = note->octaveDots().front();
    EXPECT_EQ(octaveDot->note(), note);

    octaveDot->setLen(2.0);
    octaveDot->setAbove(true);
    EXPECT_DOUBLE_EQ(octaveDot->len(), 2.0);
    EXPECT_TRUE(octaveDot->above());

    octaveDot->spatiumChanged(2.0, 3.0);
    EXPECT_DOUBLE_EQ(octaveDot->len(), 3.0);

    octaveDot->spatiumChanged(0.0, 3.0);
    EXPECT_DOUBLE_EQ(octaveDot->len(), 3.0);

    delete score;
}

TEST_F(Engraving_OctaveDotTests, jianpuUndoRedoRemoveNoteKeepsOctaveDotsOwnership)
{
    MasterScore* score = ScoreRW::readScore(JIANPU_OCTAVE_DOTS_DATA_DIR + u"jianpuBeam.mscx");
    ASSERT_TRUE(score);

    Note* original = findJianpuNoteInChord(score);
    ASSERT_TRUE(original);

    const NoteLocator locator { original->chord()->tick(), original->track(), original->pitch() };

    // Force octave dots to be allocated
    original->resizeOctaveDotsTo(1);
    expectOctaveDotsOwnedByNote(original);

    Chord* chord = original->chord();
    score->startCmd(muse::TranslatableString::untranslatable("Jianpu octave dots tests"));
    score->undoRemoveElement(chord);
    score->endCmd();

    EXPECT_EQ(findNoteByLocator(score, locator), nullptr);

    score->undoRedo(true, nullptr);

    Note* restored = findNoteByLocator(score, locator);
    ASSERT_TRUE(restored);
    expectOctaveDotsOwnedByNote(restored);

    score->undoRedo(false, nullptr);
    EXPECT_EQ(findNoteByLocator(score, locator), nullptr);

    delete score;
}

TEST_F(Engraving_OctaveDotTests, jianpuCloneNoteKeepsOctaveDotsOwnership)
{
    MasterScore* score = ScoreRW::readScore(JIANPU_OCTAVE_DOTS_DATA_DIR + u"jianpuBeam.mscx");
    ASSERT_TRUE(score);

    Note* source = findJianpuNoteInChord(score);
    ASSERT_TRUE(source);

    // Force octave dots to be allocated on source
    source->resizeOctaveDotsTo(1);
    expectOctaveDotsOwnedByNote(source);

    EngravingItem* clonedItem = source->clone();
    ASSERT_TRUE(clonedItem);
    ASSERT_TRUE(clonedItem->isNote());

    Note* cloned = toNote(clonedItem);
    cloned->resizeOctaveDotsTo(1);
    expectOctaveDotsOwnedByNote(cloned);

    OctaveDot* clonedOctaveDot = cloned->octaveDots().front();
    clonedOctaveDot->setLen(2.5);
    clonedOctaveDot->setAbove(true);
    EXPECT_DOUBLE_EQ(clonedOctaveDot->len(), 2.5);
    EXPECT_TRUE(clonedOctaveDot->above());
    EXPECT_NE(clonedOctaveDot, source->octaveDots().front());

    delete cloned;
    delete score;
}

TEST_F(Engraving_OctaveDotTests, jianpuSaveReloadKeepsOctaveDotsOwnership)
{
    String tempFile(u"jianpu-octavedots-save-reload-test.mscx");

    MasterScore* score = ScoreRW::readScore(JIANPU_OCTAVE_DOTS_DATA_DIR + u"jianpuBeam.mscx");
    ASSERT_TRUE(score);

    Note* beforeSave = findJianpuNoteInChord(score);
    ASSERT_TRUE(beforeSave);

    const NoteLocator locator { beforeSave->chord()->tick(), beforeSave->track(), beforeSave->pitch() };

    // Force octave dots to be allocated
    beforeSave->resizeOctaveDotsTo(1);
    expectOctaveDotsOwnedByNote(beforeSave);

    ASSERT_TRUE(ScoreRW::saveScore(score, tempFile));
    delete score;

    MasterScore* reloaded = ScoreRW::readScore(tempFile, true);
    ASSERT_TRUE(reloaded);

    Note* afterReload = findNoteByLocator(reloaded, locator);
    ASSERT_TRUE(afterReload);
    expectOctaveDotsOwnedByNote(afterReload);

    delete reloaded;
}
