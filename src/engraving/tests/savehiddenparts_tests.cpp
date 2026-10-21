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

#include <memory>

#include "global/io/buffer.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mmrest.h"
#include "engraving/dom/part.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/editing/editpart.h"
#include "engraving/editing/transaction/undostack.h"
#include "engraving/infrastructure/localfileinfoprovider.h"
#include "engraving/infrastructure/mscreader.h"
#include "engraving/infrastructure/mscwriter.h"
#include "engraving/rw/mscloader.h"
#include "engraving/rw/mscsaver.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::io;

class Engraving_SaveHiddenPartsTests : public ::testing::Test
{
public:
    static constexpr int NUM_MEASURES = 10;

    /* Two single-staff instruments:
     * - The upper one only has notes in the first and last measures, so measures 2-9 form
     *   a multimeasure rest while the lower one is hidden.
     * - The lower one has a single note in `hiddenNoteMeasure`.
     * - The lower instrument is hidden at instrument level only (as e.g. the Timeline does),
     *   so its staff is still visible on its own (unlike hiding it from the Layout panel,
     *   which also hides its staves).
    */
    std::unique_ptr<MasterScore> createScoreWithHiddenInstrument(int hiddenNoteMeasure)
    {
        MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
        for (int i = 0; i < 2; ++i) {
            Part* part = new Part(score);
            score->appendPart(part);
            score->appendStaff(Factory::createStaff(part));
        }
        score->appendMeasures(NUM_MEASURES);

        score->startCmd(TranslatableString::untranslatable("Save hidden parts tests"));
        auto addNote = [score](int measureNumber, staff_idx_t staffIdx) {
            Segment* segment = score->crMeasure(measureNumber - 1)->first(SegmentType::ChordRest);
            score->setNoteRest(segment, staffIdx * VOICES, NoteVal(60), Fraction(1, 4));
        };
        addNote(1, 0);
        addNote(NUM_MEASURES, 0);
        addNote(hiddenNoteMeasure, 1);
        score->undoChangeStyleVal(Sid::createMultiMeasureRests, true);
        EditPart::setPartVisible(score, score->parts().back(), false);
        score->endCmd();

        return std::unique_ptr<MasterScore>(score);
    }

    ByteArray saveScore(MasterScore* score)
    {
        ByteArray data;
        Buffer buffer(&data);

        MscWriter::Params params;
        params.device = &buffer;

        MscWriter writer(params);
        EXPECT_TRUE(writer.open());
        EXPECT_TRUE(MscSaver(score->iocContext()).writeMscz(score, writer, /*createThumbnail*/ false));
        writer.close();

        return data;
    }

    std::unique_ptr<MasterScore> loadScore(ByteArray data)
    {
        Buffer buffer(&data);

        MscReader::Params params;
        params.device = &buffer;

        MscReader reader(params);
        EXPECT_TRUE(reader.open());

        MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
        score->setFileInfoProvider(std::make_shared<LocalFileInfoProvider>(path_t()));
        EXPECT_TRUE(MscLoader().loadMscz(score, reader, nullptr, /*ignoreVersionError*/ true));
        score->doLayout();

        return std::unique_ptr<MasterScore>(score);
    }

    // Pairs of (first measure number, number of measures) for every multimeasure rest
    std::vector<std::pair<int, int> > mmRests(const Score* score)
    {
        std::vector<std::pair<int, int> > result;
        for (const Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            if (const Measure* mmRest = m->mmRest()) {
                result.emplace_back(static_cast<int>(m->measureIndex()) + 1, mmRest->mmRestCount());
            }
        }
        return result;
    }

    bool hasNote(const Score* score, int measureNumber, staff_idx_t staffIdx)
    {
        const Segment* segment = score->crMeasure(measureNumber - 1)->first(SegmentType::ChordRest);
        const EngravingItem* item = segment->element(staffIdx * VOICES);
        return item && item->isChord();
    }

    void showAllParts(MasterScore* score)
    {
        score->startCmd(TranslatableString::untranslatable("Save hidden parts tests"));
        for (Part* part : score->parts()) {
            EditPart::setPartVisible(score, part, true);
        }
        score->endCmd();
    }
};

TEST_F(Engraving_SaveHiddenPartsTests, SavingDoesNotChangeOrRelayoutTheScore)
{
    // [GIVEN] A laid out score with MM rests and a hidden instrument
    std::unique_ptr<MasterScore> score = createScoreWithHiddenInstrument(5);
    ASSERT_EQ(mmRests(score.get()), (std::vector<std::pair<int, int> > { { 2, 8 } }));

    const std::vector<System*> systemsBefore = score->systems();
    const Measure* mmRestBefore = score->crMeasure(1)->mmRest();
    const size_t undoStackSizeBefore = score->undoStack()->size();
    const size_t undoStackIndexBefore = score->undoStack()->currentIndex();

    // [WHEN] The score is saved
    saveScore(score.get());

    // [THEN] Nothing was re-laid out: re-layout frees systems and MM rests that others (e.g. the playback
    // cursor) may still point to, and must not happen while parts are serialised on other threads
    EXPECT_EQ(score->systems(), systemsBefore);
    EXPECT_EQ(score->crMeasure(1)->mmRest(), mmRestBefore);

    // [THEN] The score itself is unchanged
    EXPECT_FALSE(score->parts().back()->show());
    EXPECT_EQ(score->undoStack()->size(), undoStackSizeBefore);
    EXPECT_EQ(score->undoStack()->currentIndex(), undoStackIndexBefore);
}

// see musescore.org/en/node/27861
TEST_F(Engraving_SaveHiddenPartsTests, HiddenInstrumentSurvivesSaveAndReload)
{
    // [GIVEN] A score with MM rests and a hidden instrument that has a note inside the MM rest of the visible one
    std::unique_ptr<MasterScore> score = createScoreWithHiddenInstrument(5);
    ASSERT_EQ(mmRests(score.get()), (std::vector<std::pair<int, int> > { { 2, 8 } }));

    // [WHEN] The score is saved and reloaded
    std::unique_ptr<MasterScore> reloaded = loadScore(saveScore(score.get()));

    // [THEN] The instrument is still hidden, its note is kept, and the MM rests are the same
    ASSERT_EQ(reloaded->parts().size(), 2);
    EXPECT_FALSE(reloaded->parts().back()->show());
    ASSERT_EQ(reloaded->nmeasures(), NUM_MEASURES);
    EXPECT_TRUE(hasNote(reloaded.get(), 5, 1));
    EXPECT_EQ(mmRests(reloaded.get()), mmRests(score.get()));

    // [WHEN] The instrument is shown, in both the reloaded and the original score
    showAllParts(score.get());
    showAllParts(reloaded.get());

    // [THEN] Its note breaks the MM rest in the same way as if it had never been saved
    EXPECT_EQ(mmRests(score.get()), (std::vector<std::pair<int, int> > { { 2, 3 }, { 6, 4 } }));
    EXPECT_EQ(mmRests(reloaded.get()), mmRests(score.get()));
}

TEST_F(Engraving_SaveHiddenPartsTests, EditOnMMRestOfHiddenLayoutSurvivesSaveAndReload)
{
    // [GIVEN] A score where the hidden instrument has a note in the first measure of the MM rest,
    // so that this MM rest only exists while the instrument is hidden
    std::unique_ptr<MasterScore> score = createScoreWithHiddenInstrument(2);
    ASSERT_EQ(mmRests(score.get()), (std::vector<std::pair<int, int> > { { 2, 8 } }));

    // [GIVEN] Its number has been moved by the user
    MMRest* mmRest = toMMRest(score->crMeasure(1)->mmRest()->first(SegmentType::ChordRest)->element(0));
    ASSERT_TRUE(mmRest);
    score->startCmd(TranslatableString::untranslatable("Save hidden parts tests"));
    mmRest->undoChangeProperty(Pid::MMREST_NUMBER_OFFSET, Spatium(2.5));
    score->endCmd();

    // [WHEN] The score is saved and reloaded
    std::unique_ptr<MasterScore> reloaded = loadScore(saveScore(score.get()));

    // [THEN] The edit is kept
    ASSERT_EQ(mmRests(reloaded.get()), (std::vector<std::pair<int, int> > { { 2, 8 } }));
    const MMRest* reloadedMMRest = toMMRest(reloaded->crMeasure(1)->mmRest()->first(SegmentType::ChordRest)->element(0));
    ASSERT_TRUE(reloadedMMRest);
    EXPECT_EQ(reloadedMMRest->getProperty(Pid::MMREST_NUMBER_OFFSET).value<Spatium>(), Spatium(2.5));
}
