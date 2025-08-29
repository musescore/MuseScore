/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "dom/box.h"
#include "dom/masterscore.h"
#include "dom/fret.h"
#include "dom/harmony.h"
#include "dom/factory.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String FRET_BOX_DATA_DIR(u"fretbox_data/");

class Engraving_FretBoxTests : public ::testing::Test
{
public:
    MasterScore* createEmptyScore()
    {
        String readFile(FRET_BOX_DATA_DIR + u"empty.mscx");
        MasterScore* score = ScoreRW::readScore(readFile);
        EXPECT_TRUE(score);

        score->doLayout();
        return score;
    }

    void addChords(MasterScore* score, const StringList& chords)
    {
        Measure* measure = score->firstMeasure();
        measure = toMeasure(measure->next()); // ignore first one with clef

        for (const String& chord : chords) {
            Segment* segment = measure->first(SegmentType::ChordRest);

            engraving::FretDiagram* diagram = engraving::Factory::createFretDiagram(score->dummy()->segment());

            diagram->setHarmony(chord);
            diagram->updateDiagram(chord);

            diagram->setTrack(0);
            diagram->setParent(segment);

            diagram->assignNewEID();

            score->startCmd(TranslatableString());
            score->undoAddElement(diagram);
            score->endCmd();

            // +2 - save place for upcoming operations
            measure = measure->nextMeasure();
            measure = measure->nextMeasure();
        }
    }

    void insertChord(MasterScore* score, const String& chord, int measureIndex)
    {
        Measure* measure = score->firstMeasure();
        measure = toMeasure(measure->next()); // ignore first one with clef
        int i = 0;
        while (i++ < measureIndex) {
            measure = toMeasure(measure->next());
        }

        Segment* segment = measure->first(SegmentType::ChordRest);

        engraving::FretDiagram* diagram = engraving::Factory::createFretDiagram(score->dummy()->segment());

        diagram->setHarmony(chord);
        diagram->updateDiagram(chord);

        diagram->setTrack(0);
        diagram->setParent(segment);

        diagram->assignNewEID();

        score->startCmd(TranslatableString());
        score->undoAddElement(diagram);
        score->endCmd();
    }

    void removeChord(MasterScore* score, int measureIndex)
    {
        Measure* measure = score->firstMeasure();
        measure = toMeasure(measure->next()); // ignore first one with clef
        int i = 0;
        while (i++ < measureIndex) {
            measure = toMeasure(measure->next());
        }

        Segment* segment = measure->first(SegmentType::ChordRest);

        score->startCmd(TranslatableString());
        score->undoRemoveElement(toFretDiagram(segment->annotations().front()), true);
        score->endCmd();
    }

    void renameChord(MasterScore* score, int measureIndex, const String& newChordName)
    {
        Measure* measure = score->firstMeasure();
        measure = toMeasure(measure->next()); // ignore first one with clef
        int i = 0;
        while (i++ < measureIndex) {
            measure = toMeasure(measure->next());
        }

        Segment* segment = measure->first(SegmentType::ChordRest);
        FretDiagram* diagram = toFretDiagram(segment->annotations().front());

        EditData ed;
        diagram->harmony()->startEdit(ed);

        score->startCmd(TranslatableString());
        diagram->harmony()->undoChangeProperty(Pid::TEXT, newChordName);
        score->endCmd();

        diagram->harmony()->endEdit(ed);
    }

    std::vector<EID> chordsEIDs(MasterScore* score)
    {
        std::vector<EID> eids;
        FBox* fretBox = toFBox(score->measure(0));

        for (EngravingItem* item : fretBox->el()) {
            eids.push_back(item->eid());
        }

        return eids;
    }

    void checkFretBox(MasterScore* score, const StringList& chords)
    {
        ASSERT_TRUE(!score->measures()->empty());
        ASSERT_TRUE(score->measure(0)->type() == ElementType::FBOX);

        FBox* fretBox = toFBox(score->measure(0));
        ASSERT_TRUE(fretBox);

        const ElementList& elements = fretBox->orderedElements(false /*includeInvisible*/);
        EXPECT_EQ(elements.size(), chords.size());

        for (size_t i = 0; i < chords.size(); ++i) {
            FretDiagram* diagram = toFretDiagram(elements[i]);
            ASSERT_TRUE(diagram);
            LOGD() << "Checking chord: " << chords[i].toStdString()
                   << ", diagram: " << diagram->harmonyText().toStdString();

            EXPECT_EQ(diagram->harmonyText(), chords[i]);
        }
    }

    void reorderElements(MasterScore* score, const StringList& newOrder)
    {
        FBox* fretBox = toFBox(score->measure(0));
        ASSERT_TRUE(fretBox);
        ASSERT_EQ(fretBox->el().size(), newOrder.size());

        score->startCmd(TranslatableString());
        fretBox->undoReorderElements(newOrder);
        score->endCmd();
    }

    void undoLastCommand(MasterScore* score)
    {
        score->undoRedo(true, nullptr);
        score->doLayout();
    }
};

TEST_F(Engraving_FretBoxTests, Init)
{
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A", u"B", u"C", u"D" });

    // [WHEN] Insert fret box
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"B", u"C", u"D" });

    delete score;
}

TEST_F(Engraving_FretBoxTests, ReorderChords)
{
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A", u"B", u"C", u"D" });
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    // [WHEN] Reorder chords in the fret box
    StringList newOrder = { u"A", u"C", u"B", u"D" };
    reorderElements(score, newOrder);

    // [THEN] Check reordered fret box and diagrams
    checkFretBox(score, { u"A", u"C", u"B", u"D" });

    // [WHEN] Undo the reordering
    undoLastCommand(score);

    // [THEN] Check original fret box and chords
    checkFretBox(score, { u"A", u"B", u"C", u"D" });

    delete score;
}

TEST_F(Engraving_FretBoxTests, DISABLED_AddChords)
{
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A" /*0*/, u"B" /*2*/, u"C" /*4*/, u"D" /*6*/, u"E" /*8*/, u"F" /*10*/ });
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    // [WHEN] Add more chords to the fret box
    insertChord(score, u"Dd", 3);

    // -------- Chords in the score: A, B, Dd, C, D, E, F --------

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"B", u"Dd", u"C", u"D", u"E", u"F" });

    // [WHEN] Undo the last command
    undoLastCommand(score);

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"B", u"C", u"D", u"E", u"F" });

    delete score;
}

TEST_F(Engraving_FretBoxTests, AddChords_SameChordAfterPreviousOne) {
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A" /*0*/, u"B" /*2*/, u"C" /*4*/, u"D" /*6*/, u"E" /*8*/, u"F" /*10*/ });
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    // [WHEN] Add the same chord after the previous one
    insertChord(score, u"C", 7);

    // -------- Chords in the score: A, B, C, D, C, E, F --------

    // [THEN] Check fret box and chords
    //        The second "C" should be ignored
    checkFretBox(score, { u"A", u"B", u"C", u"D", u"E", u"F" });

    // [WHEN] Undo the last command
    undoLastCommand(score);

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"B", u"C", u"D", u"E", u"F" });

    delete score;
}

TEST_F(Engraving_FretBoxTests, DISABLED_AddChords_SameChordBeforePreviousOne)
{
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A" /*0*/, u"B" /*2*/, u"C" /*4*/, u"D" /*6*/, u"E" /*8*/, u"F" /*10*/ });
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    // [WHEN] Add the same chord before the first one
    insertChord(score, u"C", 1);

    // -------- Chords in the score: A, C, B, C, D, E, F --------

    // [THEN] Check fret box and chords
    //        The "C" should be moved to the first position
    checkFretBox(score, { u"A", u"C", u"B", u"D", u"E", u"F" });

    // [WHEN] Undo the last command
    undoLastCommand(score);

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"B", u"C", u"D", u"E", u"F" });

    delete score;
}

TEST_F(Engraving_FretBoxTests, DISABLED_RemoveChords)
{
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A" /*0*/, u"B" /*2*/, u"C" /*4*/, u"D" /*6*/, u"E" /*8*/, u"F" /*10*/ });
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    // [WHEN] Remove chord from the score
    removeChord(score, 2); // Remove "B"

    // -------- Chords in the score: A, C, D, E, F --------

    // [THEN] Check fret box and chords
    //        The "B" chord should be removed
    checkFretBox(score, { u"A", u"C", u"D", u"E", u"F" });

    // [WHEN] Undo the last command
    undoLastCommand(score);

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"B", u"C", u"D", u"E", u"F" });

    delete score;
}

TEST_F(Engraving_FretBoxTests, DISABLED_RemoveChords_SameChord_RemoveFirst)
{
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A" /*0*/, u"B" /*2*/, u"C" /*4*/, u"D" /*6*/, u"E" /*8*/, u"F" /*10*/ });
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    // [GIVEN] There is a few same chords in the score
    insertChord(score, u"B", 5); // Insert new "B" after "C"

    // [WHEN] Remove the first "B" chord
    removeChord(score, 2);

    // -------- Chords in the score: A, C, B, D, E, F --------

    // [THEN] Check fret box and chords
    //        The "B" should be moved to the second position
    checkFretBox(score, { u"A", u"C", u"B", u"D", u"E", u"F" });

    // [WHEN] Undo the last command
    undoLastCommand(score);

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"B", u"C", u"D", u"E", u"F" });

    delete score;
}

TEST_F(Engraving_FretBoxTests, RemoveChords_SameChord_RemoveSecond)
{
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A" /*0*/, u"B" /*2*/, u"C" /*4*/, u"D" /*6*/, u"E" /*8*/, u"F" /*10*/ });
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    // [GIVEN] There is a few same chords in the score
    insertChord(score, u"B", 5); // Insert new "B" after "C"

    // [WHEN] Remove the second "B" chord
    removeChord(score, 5);

    // -------- Chords in the score: A, B, C, D, E, F --------

    // [THEN] Check fret box and chords
    //        The "B" should stay in the first position
    checkFretBox(score, { u"A", u"B", u"C", u"D", u"E", u"F" });

    // [WHEN] Undo the last command
    undoLastCommand(score);

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"B", u"C", u"D", u"E", u"F" });

    delete score;
}

TEST_F(Engraving_FretBoxTests, DISABLED_RenameChords)
{
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A" /*0*/, u"B" /*2*/, u"C" /*4*/, u"D" /*6*/, u"E" /*8*/, u"F" /*10*/ });
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    // [WHEN] Rename chord in the score
    renameChord(score, 2, u"G"); // Rename "B" to "G"

    // -------- Chords in the score: A, G, C, D, E, F --------

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"G", u"C", u"D", u"E", u"F" });

    // [WHEN] Undo the last command
    undoLastCommand(score);

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"B", u"C", u"D", u"E", u"F" });

    delete score;
}

TEST_F(Engraving_FretBoxTests, DISABLED_RenameChords_SameChordBefore)
{
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A" /*0*/, u"B" /*2*/, u"C" /*4*/, u"D" /*6*/, u"E" /*8*/, u"F" /*10*/ });
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    // [WHEN] After renaming, there is a few same chords in the score
    renameChord(score, 6, u"B"); // Rename "D" to "B"

    // -------- Chords in the score: A, B, C, B, E, F --------

    // [THEN] Check fret box and chords
    //        The second "B" should be ignored
    checkFretBox(score, { u"A", u"B", u"C", u"E", u"F" });

    // [WHEN] Undo the last command
    undoLastCommand(score);

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"B", u"C", u"D", u"E", u"F" });

    delete score;
}

TEST_F(Engraving_FretBoxTests, DISABLED_RenameChords_SameChordAfter)
{
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A" /*0*/, u"B" /*2*/, u"C" /*4*/, u"D" /*6*/, u"E" /*8*/, u"F" /*10*/ });
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    // [WHEN] Rename the chord before the same one
    renameChord(score, 2, u"D"); // Rename "B" to "D"

    // -------- Chords in the score: A, D, C, D, E, F --------

    // [THEN] Check fret box and chords
    //        There shouldn't be two "D" chords
    checkFretBox(score, { u"A", u"D", u"C", u"E", u"F" });

    // [WHEN] Undo the last command
    undoLastCommand(score);

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"B", u"C", u"D", u"E", u"F" });

    delete score;
}

TEST_F(Engraving_FretBoxTests, DISABLED_RenameChords_SameChordBefore_NoDuplicates)
{
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A" /*0*/, u"B" /*2*/, u"C" /*4*/, u"D" /*6*/, u"E" /*8*/, u"F" /*10*/ });
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    // [WHEN] Rename the chord after the same one
    renameChord(score, 6, u"B"); // Rename "D" to "B"
    renameChord(score, 6, u"G"); // Rename "B" to "G"

    // -------- Chords in the score: A, B, C, G, E, F --------

    // [THEN] Check fret box and chords
    //        There shouldn't be two "B" chords
    checkFretBox(score, { u"A", u"B", u"C", u"G", u"E", u"F" });

    // [WHEN] Undo the last command
    undoLastCommand(score);

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"B", u"C", u"E", u"F" });

    delete score;
}

TEST_F(Engraving_FretBoxTests, DISABLED_RenameChords_SameChordAfter_NoDuplicates)
{
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A" /*0*/, u"B" /*2*/, u"C" /*4*/, u"D" /*6*/, u"E" /*8*/, u"F" /*10*/ });
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    // [WHEN] Rename the chord to the new chord that already exists
    renameChord(score, 6, u"E"); // Rename first "D" to "E"
    renameChord(score, 6, u"F"); // Rename first "E" to "F"

    // -------- Chords in the score: A, B, C, F, E, F --------

    // [THEN] Check fret box and chords
    //        The second "F" should be ignored
    checkFretBox(score, { u"A", u"B", u"C", u"F", u"E" });

    // [WHEN] Undo the last command
    undoLastCommand(score);

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"B", u"C", u"E", u"F" });

    delete score;
}

TEST_F(Engraving_FretBoxTests, DISABLED_RenameChords_SameChordAfter_NoDuplicates_2)
{
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A" /*0*/, u"B" /*2*/, u"C" /*4*/, u"D" /*6*/, u"E" /*8*/, u"F" /*10*/ });
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    // [WHEN] Rename the chord to the new chord that already exists
    renameChord(score, 6, u"A"); // Rename "D" to "A"
    renameChord(score, 0, u"C"); // Rename first "A" to "C"

    // -------- Chords in the score: A, B, C, F, E, F --------

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"C", u"B", u"A", u"E", u"F" });

    // [WHEN] Undo the last command
    undoLastCommand(score);

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"A", u"B", u"C", u"E", u"F" });

    delete score;
}

TEST_F(Engraving_FretBoxTests, DISABLED_RenameChords_AfterMoving)
{
    // [GIVEN] Empty score
    MasterScore* score = createEmptyScore();

    // [GIVEN] Add chords to the score
    addChords(score, { u"A" /*0*/, u"B" /*2*/, u"C" /*4*/, u"D" /*6*/, u"E" /*8*/, u"F" /*10*/ });
    score->insertBox(mu::engraving::ElementType::FBOX, score->firstMeasure());

    StringList newOrder = { u"B", u"C", u"D", u"E", u"F", u"A" };
    reorderElements(score, newOrder);

    // [WHEN] Rename the chord to the new chord that already exists
    renameChord(score, 0, u"C"); // Rename "A" to "C"

    // -------- Chords in the score: C, B, C, D, E, F --------

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"B", u"C", u"D", u"E", u"F" });

    // [WHEN] Rename the chord to the new chord that doesn't exist
    renameChord(score, 0, u"C1"); // Rename "C" to "C1"

    // -------- Chords in the score: C1, B, C, D, E, F --------

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"C1", u"B", u"C", u"D", u"E", u"F" });

    // [WHEN] Undo the last command
    undoLastCommand(score);

    // [THEN] Check fret box and chords
    checkFretBox(score, { u"B", u"C", u"D", u"E", u"F" });

    delete score;
}
