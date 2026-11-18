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

#include <algorithm>
#include <functional>

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/instrchange.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/scoreorder.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/sharedpart.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stringdata.h"
#include "engraving/dom/systemtext.h"
#include "engraving/editing/editpart.h"
#include "engraving/editing/editstavesharing.h"
#include "engraving/editing/transaction/transaction.h"

using namespace mu::engraving;
using namespace muse;

using InstrumentIds = std::vector<String>;

class Engraving_EditPartTests : public ::testing::Test
{
protected:
    MasterScore* createMasterScore(const InstrumentIds& ids, int measureCount = 4)
    {
        MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

        edit(score, [score, &ids](Transaction& tx) {
            for (const String& id : ids) {
                EditPart::insertPart(tx, score, searchTemplate(id), score->parts().size());
            }
        });

        score->appendMeasures(measureCount);

        // ASSERT_* needs a void function, the tests assert the part count themselves
        const std::vector<Part*>& parts = score->parts();
        EXPECT_EQ(instrumentIds(score), ids);
        if (parts.size() == ids.size()) {
            for (size_t i = 0; i < ids.size(); ++i) {
                EXPECT_EQ(parts.at(i)->nstaves(), searchTemplate(ids.at(i))->staffCount);
            }
        }

        return score;
    }

    InstrumentIds instrumentIds(const Score* score) const
    {
        InstrumentIds result;
        for (const Part* part : score->parts()) {
            result.push_back(part->instrumentId());
        }

        return result;
    }

    //! The parts in the given order, each staying in its shared part
    static EditPart::PartsLayout layout(const std::vector<Part*>& parts, const std::vector<EditPart::SystemObjectsMove>& moves = {})
    {
        EditPart::PartsLayout result;
        result.parts.reserve(parts.size());

        for (Part* part : parts) {
            result.parts.push_back({ part, part->sharedPart() });
        }

        result.systemObjects = moves;

        return result;
    }

    Excerpt* addExcerpt(MasterScore* score, Excerpt* excerpt)
    {
        edit(score, [score, excerpt](Transaction&) {
            score->initAndAddExcerpt(excerpt, false);
        });

        return excerpt;
    }

    Excerpt* addExcerptOfPart(MasterScore* score, Part* part)
    {
        return addExcerpt(score, Excerpt::createExcerptsFromParts({ part }, score).front());
    }

    Excerpt* addExcerptOfAllParts(MasterScore* score)
    {
        Excerpt* excerpt = new Excerpt(score);
        excerpt->parts() = score->parts();
        excerpt->setName(u"All parts");

        return addExcerpt(score, excerpt);
    }

    void edit(MasterScore* score, const std::function<void(Transaction&)>& func)
    {
        score->startCmd(TranslatableString::untranslatable("Edit part tests"));
        func(score->transactionManager()->currentOrDummyTransaction());
        score->endCmd();
    }

    void undo(MasterScore* score)
    {
        score->undoRedo(true, nullptr);
    }

    Segment* firstSegment(const MasterScore* score) const
    {
        return score->firstMeasure()->first(SegmentType::ChordRest);
    }

    void addSystemText(MasterScore* score)
    {
        SystemText* text = Factory::createSystemText(firstSegment(score));
        text->setTrack(0);

        edit(score, [score, text](Transaction&) {
            score->undoAddElement(text);
        });
    }

    std::vector<staff_idx_t> systemTextStaves(const MasterScore* score) const
    {
        std::vector<staff_idx_t> result;
        for (const EngravingItem* annotation : firstSegment(score)->annotations()) {
            if (annotation->isSystemText()) {
                result.push_back(annotation->staffIdx());
            }
        }

        std::sort(result.begin(), result.end());
        return result;
    }

    const Note* addNote(MasterScore* score, int pitch)
    {
        edit(score, [this, score, pitch](Transaction&) {
            score->setNoteRest(firstSegment(score), 0, NoteVal(pitch), Fraction(1, 4));
        });

        const EngravingItem* item = firstSegment(score)->element(0);
        return item && item->isChord() ? toChord(item)->notes().front() : nullptr;
    }
};

TEST_F(Engraving_EditPartTests, InsertPart_Template_InsertsPartWithTemplateStaves)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    // [WHEN] A violin is inserted between them
    edit(score, [score](Transaction& tx) {
        EditPart::insertPart(tx, score, searchTemplate(u"violin"), 1);
    });

    // [THEN] The violin is the second part, with one staff
    ASSERT_EQ(instrumentIds(score), InstrumentIds({ u"flute", u"violin", u"piano" }));
    EXPECT_EQ(parts.at(0)->nstaves(), 1u);
    EXPECT_EQ(parts.at(1)->nstaves(), 1u);
    EXPECT_EQ(parts.at(2)->nstaves(), 2u);
    EXPECT_EQ(score->nstaves(), 4u);

    // [WHEN] Undo
    undo(score);

    // [THEN] The violin is gone
    ASSERT_EQ(instrumentIds(score), InstrumentIds({ u"flute", u"piano" }));
    EXPECT_EQ(parts.at(0)->nstaves(), 1u);
    EXPECT_EQ(parts.at(1)->nstaves(), 2u);

    delete score;
}

TEST_F(Engraving_EditPartTests, InsertPart_PartCopyIntoExcerpt_LinksStavesToMaster)
{
    // [GIVEN] Flute and piano, with a part score of the flute
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    const Part* piano = parts.at(1);
    Score* excerptScore = addExcerptOfPart(score, parts.at(0))->excerptScore();
    const std::vector<Part*>& excerptParts = excerptScore->parts();
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute" }));

    Part* pianoCopy = piano->clone();
    pianoCopy->setSharedPart(nullptr);

    // [WHEN] A copy of the piano is inserted into the part score
    edit(score, [excerptScore, pianoCopy](Transaction& tx) {
        EditPart::insertPart(tx, excerptScore, pianoCopy, 1);
    });

    // [THEN] The part score has the piano after the flute
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute", u"piano" }));
    EXPECT_EQ(excerptParts.at(0)->nstaves(), 1u);
    ASSERT_EQ(excerptParts.at(1)->nstaves(), 2u);

    // [THEN] The piano has the main score's id, and staves linked to the main score
    const Part* excerptPiano = excerptParts.at(1);
    EXPECT_EQ(excerptPiano->id(), piano->id());
    EXPECT_EQ(piano->staff(0)->findLinkedInScore(excerptScore), excerptPiano->staff(0));
    EXPECT_EQ(piano->staff(1)->findLinkedInScore(excerptScore), excerptPiano->staff(1));

    // [THEN] The main score is unchanged
    ASSERT_EQ(instrumentIds(score), InstrumentIds({ u"flute", u"piano" }));
    EXPECT_EQ(parts.at(0)->nstaves(), 1u);
    EXPECT_EQ(parts.at(1)->nstaves(), 2u);

    delete score;
}

TEST_F(Engraving_EditPartTests, ReplacePart_Template_ReplacesPartAtSameIndex)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    // [WHEN] The flute is replaced with a violin
    edit(score, [score, &parts](Transaction& tx) {
        EditPart::replacePart(tx, score, parts.at(0), searchTemplate(u"violin"));
    });

    // [THEN] The violin takes the flute's place
    ASSERT_EQ(instrumentIds(score), InstrumentIds({ u"violin", u"piano" }));
    EXPECT_EQ(parts.at(0)->nstaves(), 1u);
    EXPECT_EQ(parts.at(1)->nstaves(), 2u);

    delete score;
}

TEST_F(Engraving_EditPartTests, ReplacePart_PartCopyInExcerpt_LinksStavesToMaster)
{
    // [GIVEN] A flute with a part score
    MasterScore* score = createMasterScore({ u"flute" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    Part* flute = parts.at(0);
    Score* excerptScore = addExcerptOfPart(score, flute)->excerptScore();
    const std::vector<Part*>& excerptParts = excerptScore->parts();
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute" }));

    Part* fluteCopy = flute->clone();
    fluteCopy->setSharedPart(nullptr);

    // [WHEN] The part score's flute is replaced with a copy of the main score's flute
    edit(score, [excerptScore, &excerptParts, fluteCopy](Transaction& tx) {
        EditPart::replacePart(tx, excerptScore, excerptParts.at(0), fluteCopy);
    });

    // [THEN] The part score has the copy, with the main score's id and its staff linked to the main score
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute" }));
    ASSERT_EQ(excerptParts.at(0)->nstaves(), 1u);

    const Part* excerptFlute = excerptParts.at(0);
    EXPECT_EQ(excerptFlute->id(), flute->id());
    EXPECT_EQ(flute->staff(0)->findLinkedInScore(excerptScore), excerptFlute->staff(0));

    // [THEN] The main score is unchanged
    EXPECT_EQ(instrumentIds(score), InstrumentIds({ u"flute" }));

    delete score;
}

TEST_F(Engraving_EditPartTests, RemoveParts_MasterScore_UpdatesExcerpts)
{
    // [GIVEN] Flute and piano, with a part score of each and one of both
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    const ID fluteId = parts.at(0)->id();

    addExcerptOfPart(score, parts.at(0));
    addExcerptOfPart(score, parts.at(1));
    const Score* allPartsScore = addExcerptOfAllParts(score)->excerptScore();
    const std::vector<Part*>& allParts = allPartsScore->parts();
    ASSERT_EQ(instrumentIds(allPartsScore), InstrumentIds({ u"flute", u"piano" }));

    // [WHEN] The flute is removed from the main score
    edit(score, [score, &parts](Transaction& tx) {
        EditPart::removeParts(tx, score, { parts.at(0) });
    });

    // [THEN] Only the piano is left
    ASSERT_EQ(instrumentIds(score), InstrumentIds({ u"piano" }));
    EXPECT_EQ(parts.at(0)->nstaves(), 2u);

    // [THEN] The flute's part score is deleted, the other part scores lose the flute
    EXPECT_EQ(score->excerpts().size(), 2u);
    for (const Excerpt* excerpt : score->excerpts()) {
        EXPECT_NE(excerpt->initialPartId(), fluteId);
    }
    ASSERT_EQ(instrumentIds(allPartsScore), InstrumentIds({ u"piano" }));
    EXPECT_EQ(allParts.at(0)->nstaves(), 2u);

    // [WHEN] Undo
    undo(score);

    // [THEN] The flute and its part score are back
    EXPECT_EQ(instrumentIds(score), InstrumentIds({ u"flute", u"piano" }));
    EXPECT_EQ(score->excerpts().size(), 3u);
    EXPECT_EQ(instrumentIds(allPartsScore), InstrumentIds({ u"flute", u"piano" }));

    delete score;
}

TEST_F(Engraving_EditPartTests, RemoveParts_ExcerptScore_KeepsMaster)
{
    // [GIVEN] Flute and piano, with a part score of both
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    Score* allPartsScore = addExcerptOfAllParts(score)->excerptScore();
    const std::vector<Part*>& allParts = allPartsScore->parts();
    ASSERT_EQ(instrumentIds(allPartsScore), InstrumentIds({ u"flute", u"piano" }));

    // [WHEN] The flute is removed from the part score
    edit(score, [allPartsScore, &allParts](Transaction& tx) {
        EditPart::removeParts(tx, allPartsScore, { allParts.at(0) });
    });

    // [THEN] Only the part score loses the flute
    ASSERT_EQ(instrumentIds(allPartsScore), InstrumentIds({ u"piano" }));
    EXPECT_EQ(allParts.at(0)->id(), parts.at(1)->id());
    EXPECT_EQ(instrumentIds(score), InstrumentIds({ u"flute", u"piano" }));
    EXPECT_EQ(score->excerpts().size(), 1u);

    delete score;
}

TEST_F(Engraving_EditPartTests, RemoveStaves_MasterScore_UpdatesExcerpts)
{
    // [GIVEN] Flute and piano, with a part score of the piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    Part* piano = parts.at(1);
    const Score* excerptScore = addExcerptOfPart(score, piano)->excerptScore();
    const std::vector<Part*>& excerptParts = excerptScore->parts();
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"piano" }));
    ASSERT_EQ(excerptParts.at(0)->nstaves(), 2u);

    const ID keptStaffId = piano->staff(0)->id();

    // [WHEN] The piano's lower staff is removed from the main score
    edit(score, [score, piano](Transaction& tx) {
        EditPart::removeStaves(tx, score, { piano->staff(1) });
    });

    // [THEN] Both scores keep only the piano's upper staff
    ASSERT_EQ(instrumentIds(score), InstrumentIds({ u"flute", u"piano" }));
    EXPECT_EQ(parts.at(0)->nstaves(), 1u);
    ASSERT_EQ(parts.at(1)->nstaves(), 1u);
    EXPECT_EQ(piano->staff(0)->id(), keptStaffId);

    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"piano" }));
    ASSERT_EQ(excerptParts.at(0)->nstaves(), 1u);
    EXPECT_EQ(excerptScore->staff(0)->id(), keptStaffId);

    // [WHEN] Undo
    undo(score);

    // [THEN] Both scores have the lower staff again
    EXPECT_EQ(parts.at(0)->nstaves(), 1u);
    EXPECT_EQ(parts.at(1)->nstaves(), 2u);
    EXPECT_EQ(excerptParts.at(0)->nstaves(), 2u);

    delete score;
}

TEST_F(Engraving_EditPartTests, AppendStaff_MasterScore_AppendsLinkedStaffToExcerpts)
{
    // [GIVEN] A flute with a part score
    MasterScore* score = createMasterScore({ u"flute" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    Part* flute = parts.at(0);
    const Score* excerptScore = addExcerptOfPart(score, flute)->excerptScore();
    const std::vector<Part*>& excerptParts = excerptScore->parts();
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute" }));

    // [WHEN] A staff is appended to the main score's flute
    Staff* staff = nullptr;
    edit(score, [score, flute, &staff](Transaction& tx) {
        staff = EditPart::appendStaff(tx, score, flute);
    });

    // [THEN] The flute has a second staff with its own id
    ASSERT_TRUE(staff);
    EXPECT_EQ(instrumentIds(score), InstrumentIds({ u"flute" }));
    ASSERT_EQ(flute->nstaves(), 2u);
    EXPECT_EQ(flute->staff(1), staff);
    EXPECT_NE(staff->id(), flute->staff(0)->id());

    // [THEN] The part score has a copy of it, linked and with the same id
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute" }));
    ASSERT_EQ(excerptParts.at(0)->nstaves(), 2u);
    const Staff* excerptStaff = excerptScore->staff(1);
    EXPECT_EQ(excerptStaff->id(), staff->id());
    EXPECT_TRUE(excerptStaff->isLinked(staff));
    EXPECT_EQ(staff->findLinkedInScore(excerptScore), excerptStaff);

    // [WHEN] Undo
    undo(score);

    // [THEN] Both scores have one staff again
    EXPECT_EQ(flute->nstaves(), 1u);
    EXPECT_EQ(excerptParts.at(0)->nstaves(), 1u);

    delete score;
}

TEST_F(Engraving_EditPartTests, AppendLinkedStaff_MasterScore_UpdatesExcerpts)
{
    // [GIVEN] A flute with a part score
    MasterScore* score = createMasterScore({ u"flute" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    Part* flute = parts.at(0);
    Staff* sourceStaff = flute->staff(0);
    const Score* excerptScore = addExcerptOfPart(score, flute)->excerptScore();
    const std::vector<Part*>& excerptParts = excerptScore->parts();
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute" }));

    // [WHEN] A staff linked to the flute's staff is appended to the main score
    const Staff* linkedStaff = nullptr;
    edit(score, [score, sourceStaff, flute, &linkedStaff](Transaction& tx) {
        linkedStaff = EditPart::appendLinkedStaff(tx, score, sourceStaff, flute);
    });

    // [THEN] The new staff is linked to the source staff
    ASSERT_TRUE(linkedStaff);
    EXPECT_EQ(instrumentIds(score), InstrumentIds({ u"flute" }));
    EXPECT_EQ(flute->nstaves(), 2u);
    EXPECT_TRUE(linkedStaff->isLinked(sourceStaff));

    // [THEN] The part score gets a linked staff too, with the same id
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute" }));
    ASSERT_EQ(excerptParts.at(0)->nstaves(), 2u);
    const Staff* excerptLinkedStaff = excerptScore->staff(1);
    EXPECT_EQ(excerptLinkedStaff->id(), linkedStaff->id());
    EXPECT_TRUE(excerptLinkedStaff->isLinked(excerptScore->staff(0)));

    delete score;
}

TEST_F(Engraving_EditPartTests, ReplaceInstrument_MainInstrument_UpdatesExcerptsAndRenamesPart)
{
    // [GIVEN] A flute with a part score
    MasterScore* score = createMasterScore({ u"flute" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    Part* flute = parts.at(0);
    const Excerpt* excerpt = addExcerptOfPart(score, flute);
    const Score* excerptScore = excerpt->excerptScore();
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute" }));

    const String oldExcerptName = excerpt->name();
    Instrument violin = Instrument::fromTemplate(searchTemplate(u"violin"));
    violin.setLongName(u"Violin");

    // [WHEN] The flute's main instrument is replaced with a violin
    bool replaced = false;
    edit(score, [score, flute, &violin, &replaced](Transaction& tx) {
        replaced = EditPart::replaceInstrument(tx, score, flute, Part::MAIN_INSTRUMENT_TICK, violin);
    });

    // [THEN] Both scores have the violin, and the part score is renamed after it
    EXPECT_TRUE(replaced);
    EXPECT_EQ(instrumentIds(score), InstrumentIds({ u"violin" }));
    EXPECT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"violin" }));
    EXPECT_EQ(excerpt->name(), u"Violin");

    // [WHEN] Undo
    undo(score);

    // [THEN] Both scores have the flute again, and the old name
    EXPECT_EQ(instrumentIds(score), InstrumentIds({ u"flute" }));
    EXPECT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute" }));
    EXPECT_EQ(excerpt->name(), oldExcerptName);

    delete score;
}

TEST_F(Engraving_EditPartTests, ReplaceInstrument_SameName_KeepsExcerptName)
{
    // [GIVEN] A flute with a part score named after it
    MasterScore* score = createMasterScore({ u"flute" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    Part* flute = parts.at(0);
    const Excerpt* excerpt = addExcerptOfPart(score, flute);
    const String oldExcerptName = excerpt->name();
    ASSERT_EQ(oldExcerptName, flute->partName());

    Instrument newFlute = Instrument::fromTemplate(searchTemplate(u"flute"));
    newFlute.setLongName(flute->instrument()->longName());

    // [WHEN] The main instrument is replaced with one that has the same name
    edit(score, [score, flute, &newFlute](Transaction& tx) {
        EditPart::replaceInstrument(tx, score, flute, Part::MAIN_INSTRUMENT_TICK, newFlute);
    });

    // [THEN] The part score keeps its name
    EXPECT_EQ(excerpt->name(), oldExcerptName);

    delete score;
}

TEST_F(Engraving_EditPartTests, ReplaceInstrument_DifferentClef_ChangesClefAndStaffType)
{
    // [GIVEN] A flute with a part score
    MasterScore* score = createMasterScore({ u"flute" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    Part* flute = parts.at(0);
    const Score* excerptScore = addExcerptOfPart(score, flute)->excerptScore();
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute" }));

    const Instrument cello = Instrument::fromTemplate(searchTemplate(u"violoncello"));
    const StaffType* staffType = StaffType::preset(StaffTypes::PERC_1LINE);

    // [WHEN] The flute is replaced with a cello and a new staff type
    edit(score, [score, flute, &cello, staffType](Transaction& tx) {
        EditPart::replaceInstrument(tx, score, flute, Part::MAIN_INSTRUMENT_TICK, cello, staffType);
    });

    // [THEN] Both scores get the bass clef and the new staff type
    for (const Score* s : { static_cast<const Score*>(score), excerptScore }) {
        ASSERT_EQ(s->nstaves(), 1u);

        const Staff* staff = s->staff(0);
        EXPECT_EQ(staff->defaultClefType(), ClefTypeList(ClefType::F));
        EXPECT_EQ(staff->staffType()->lines(), 1);
    }

    delete score;
}

TEST_F(Engraving_EditPartTests, ReplaceInstrument_TransposingInstrument_TransposesNotes)
{
    // [GIVEN] A flute playing a C
    MasterScore* score = createMasterScore({ u"flute" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    Part* flute = parts.at(0);
    const Note* note = addNote(score, 60);
    ASSERT_TRUE(note);
    ASSERT_EQ(note->tpc2(), Tpc::TPC_C);

    const Instrument clarinet = Instrument::fromTemplate(searchTemplate(u"bb-clarinet"));

    // [WHEN] The flute is replaced with a B♭ clarinet
    edit(score, [score, flute, &clarinet](Transaction& tx) {
        EditPart::replaceInstrument(tx, score, flute, Part::MAIN_INSTRUMENT_TICK, clarinet);
    });

    // [THEN] The note is written a major second higher
    EXPECT_EQ(note->tpc1(), Tpc::TPC_C);
    EXPECT_EQ(note->tpc2(), Tpc::TPC_D);

    delete score;
}

TEST_F(Engraving_EditPartTests, ReplaceInstrument_InstrumentChangeTick_ChangesInstrumentChange)
{
    // [GIVEN] A flute with a part score and an instrument change in the second measure
    MasterScore* score = createMasterScore({ u"flute" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    Part* flute = parts.at(0);
    const Score* excerptScore = addExcerptOfPart(score, flute)->excerptScore();
    const std::vector<Part*>& excerptParts = excerptScore->parts();
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute" }));

    const Fraction tick(1, 1);
    Segment* segment = score->tick2segment(tick, true, SegmentType::ChordRest);
    ASSERT_TRUE(segment);

    InstrumentChange* instrumentChange = Factory::createInstrumentChange(segment, *flute->instrument());
    instrumentChange->setTrack(0);
    edit(score, [score, instrumentChange](Transaction&) {
        score->undoAddElement(instrumentChange);
    });

    const Instrument violin = Instrument::fromTemplate(searchTemplate(u"violin"));

    // [WHEN] The instrument at the instrument change is replaced with a violin
    bool replaced = false;
    edit(score, [score, flute, tick, &violin, &replaced](Transaction& tx) {
        replaced = EditPart::replaceInstrument(tx, score, flute, tick, violin);
    });

    // [THEN] Both scores switch to the violin there, the main instrument stays
    EXPECT_TRUE(replaced);
    EXPECT_EQ(instrumentChange->instrument()->id(), u"violin");
    EXPECT_EQ(flute->instrument(tick)->id(), u"violin");
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute" }));
    EXPECT_EQ(excerptParts.at(0)->instrument(tick)->id(), u"violin");
    EXPECT_EQ(instrumentIds(score), InstrumentIds({ u"flute" }));

    // [WHEN] Undo
    undo(score);

    // [THEN] The instrument change is the flute again
    EXPECT_EQ(flute->instrument(tick)->id(), u"flute");

    delete score;
}

TEST_F(Engraving_EditPartTests, ReplaceInstrument_NoInstrumentChangeAtTick_ReturnsFalse)
{
    // [GIVEN] A flute without instrument changes
    MasterScore* score = createMasterScore({ u"flute" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    const Instrument violin = Instrument::fromTemplate(searchTemplate(u"violin"));

    // [WHEN] The instrument in the second measure is replaced
    bool replaced = true;
    edit(score, [score, &parts, &violin, &replaced](Transaction& tx) {
        replaced = EditPart::replaceInstrument(tx, score, parts.at(0), Fraction(1, 1), violin);
    });

    // [THEN] Nothing is replaced
    EXPECT_FALSE(replaced);
    EXPECT_EQ(instrumentIds(score), InstrumentIds({ u"flute" }));

    delete score;
}

TEST_F(Engraving_EditPartTests, ReplaceDrumset_Undoable_UpdatesExcerpts)
{
    // [GIVEN] A drumset with a part score
    MasterScore* score = createMasterScore({ u"drumset" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    Part* drums = parts.at(0);
    const Score* excerptScore = addExcerptOfPart(score, drums)->excerptScore();
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"drumset" }));

    const Part* excerptDrums = excerptScore->parts().at(0);
    ASSERT_TRUE(drums->instrument()->drumset());
    ASSERT_TRUE(excerptDrums->instrument()->drumset());

    const String oldName = drums->instrument()->drumset()->name(36);
    Drumset drumset = *drums->instrument()->drumset();
    drumset.drum(36).name = u"Test drum";

    // [WHEN] The drumset is replaced
    edit(score, [score, drums, &drumset](Transaction& tx) {
        EditPart::replaceDrumset(tx, score, drums, Part::MAIN_INSTRUMENT_TICK, drumset);
    });

    // [THEN] Both scores have the new drumset
    EXPECT_EQ(drums->instrument()->drumset()->name(36), u"Test drum");
    EXPECT_EQ(excerptDrums->instrument()->drumset()->name(36), u"Test drum");

    // [WHEN] Undo
    undo(score);

    // [THEN] Both scores have the old drumset
    EXPECT_EQ(drums->instrument()->drumset()->name(36), oldName);
    EXPECT_EQ(excerptDrums->instrument()->drumset()->name(36), oldName);

    delete score;
}

TEST_F(Engraving_EditPartTests, ReplaceDrumset_NotUndoable_UpdatesExcerpts)
{
    // [GIVEN] A drumset with a part score
    MasterScore* score = createMasterScore({ u"drumset" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    Part* drums = parts.at(0);
    const Score* excerptScore = addExcerptOfPart(score, drums)->excerptScore();
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"drumset" }));

    const Part* excerptDrums = excerptScore->parts().at(0);
    ASSERT_TRUE(drums->instrument()->drumset());
    ASSERT_TRUE(excerptDrums->instrument()->drumset());

    Drumset drumset = *drums->instrument()->drumset();
    drumset.drum(36).name = u"Test drum";

    // [WHEN] The drumset is replaced without undo
    edit(score, [score, drums, &drumset](Transaction& tx) {
        EditPart::replaceDrumset(tx, score, drums, Part::MAIN_INSTRUMENT_TICK, drumset, false);
    });

    // [THEN] Both scores have the new drumset
    EXPECT_EQ(drums->instrument()->drumset()->name(36), u"Test drum");
    EXPECT_EQ(excerptDrums->instrument()->drumset()->name(36), u"Test drum");

    delete score;
}

TEST_F(Engraving_EditPartTests, SetPartVisible_HidesPart)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    Part* flute = parts.at(0);
    const Part* piano = parts.at(1);

    // [WHEN] The flute is hidden
    edit(score, [score, flute](Transaction& tx) {
        EditPart::setPartVisible(tx, score, flute, false);
    });

    // [THEN] Only the flute is hidden
    EXPECT_FALSE(flute->show());
    EXPECT_TRUE(piano->show());

    // [WHEN] Undo
    undo(score);

    // [THEN] The flute is shown again
    EXPECT_TRUE(flute->show());

    delete score;
}

TEST_F(Engraving_EditPartTests, SetStaffVisible_HidesOnlyThatStaff)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    ASSERT_EQ(score->nstaves(), 3u);

    // [WHEN] The piano's lower staff is hidden
    edit(score, [score](Transaction& tx) {
        EditPart::setStaffVisible(tx, score, score->staff(2), false);
    });

    // [THEN] Only that staff is hidden
    EXPECT_TRUE(score->staff(0)->visible());
    EXPECT_TRUE(score->staff(1)->visible());
    EXPECT_FALSE(score->staff(2)->visible());

    delete score;
}

TEST_F(Engraving_EditPartTests, SetPartSharpFlat_ChangesPreference)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    Part* flute = parts.at(0);
    const Part* piano = parts.at(1);

    // [WHEN] The flute is set to prefer flats
    edit(score, [score, flute](Transaction& tx) {
        EditPart::setPartSharpFlat(tx, score, flute, PreferSharpFlat::FLATS);
    });

    // [THEN] Only the flute prefers flats
    EXPECT_EQ(flute->preferSharpFlat(), PreferSharpFlat::FLATS);
    EXPECT_EQ(piano->preferSharpFlat(), PreferSharpFlat::AUTO);

    delete score;
}

TEST_F(Engraving_EditPartTests, SetInstrumentName_ChangesLongName)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    Part* flute = parts.at(0);
    const Part* piano = parts.at(1);
    const String pianoName = piano->longName();

    // [WHEN] The flute is renamed
    edit(score, [score, flute](Transaction& tx) {
        EditPart::setInstrumentName(tx, score, flute, Part::MAIN_INSTRUMENT_TICK, u"Long name");
    });

    // [THEN] Only the flute has the new name
    EXPECT_EQ(flute->longName(), u"Long name");
    EXPECT_EQ(piano->longName(), pianoName);

    delete score;
}

TEST_F(Engraving_EditPartTests, SetInstrumentAbbreviature_ChangesShortName)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    Part* flute = parts.at(0);
    const Part* piano = parts.at(1);
    const String pianoName = piano->shortName();

    // [WHEN] The flute's abbreviation is changed
    edit(score, [score, flute](Transaction& tx) {
        EditPart::setInstrumentAbbreviature(tx, score, flute, Part::MAIN_INSTRUMENT_TICK, u"Short name");
    });

    // [THEN] Only the flute has the new abbreviation
    EXPECT_EQ(flute->shortName(), u"Short name");
    EXPECT_EQ(piano->shortName(), pianoName);

    delete score;
}

TEST_F(Engraving_EditPartTests, SetInstrumentGroupNameOptions_ChangesOptionsOnce)
{
    // [GIVEN] A flute
    MasterScore* score = createMasterScore({ u"flute" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    Part* flute = parts.at(0);

    // [WHEN] A custom group name is set
    bool changed = false;
    edit(score, [score, flute, &changed](Transaction& tx) {
        changed = EditPart::setInstrumentGroupNameOptions(tx, score, flute, Part::MAIN_INSTRUMENT_TICK, true, u"Group", u"Gr.");
    });

    // [THEN] The instrument uses it
    EXPECT_TRUE(changed);
    const InstrumentLabel& label = flute->instrument()->instrumentLabel();
    EXPECT_TRUE(label.useCustomGroupName());
    EXPECT_EQ(label.customNameLongGroup(), u"Group");
    EXPECT_EQ(label.customNameShortGroup(), u"Gr.");

    // [WHEN] The same options are set again
    edit(score, [score, flute, &changed](Transaction& tx) {
        changed = EditPart::setInstrumentGroupNameOptions(tx, score, flute, Part::MAIN_INSTRUMENT_TICK, true, u"Group", u"Gr.");
    });

    // [THEN] Nothing changes
    EXPECT_FALSE(changed);

    delete score;
}

TEST_F(Engraving_EditPartTests, SetInstrumentNumber_ChangesNumber)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    Part* flute = parts.at(0);
    const Part* piano = parts.at(1);

    // [WHEN] The flute is numbered
    edit(score, [score, flute](Transaction& tx) {
        EditPart::setInstrumentNumber(tx, score, flute, Part::MAIN_INSTRUMENT_TICK, 3);
    });

    // [THEN] Only the flute has the number
    EXPECT_EQ(flute->number(), 3);
    EXPECT_EQ(piano->number(), 0);

    delete score;
}

TEST_F(Engraving_EditPartTests, SetSharedPartEnabled_DisablesSharedPart)
{
    // [GIVEN] Two flutes sharing a staff
    MasterScore* score = createMasterScore({ u"flute", u"flute" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    const Part* flute = parts.at(0);

    edit(score, [score](Transaction& tx) {
        EditStaveSharing::toggleStaveSharing(tx, score, true);
    });

    SharedPart* sharedPart = flute->sharedPart();
    ASSERT_TRUE(sharedPart);
    ASSERT_TRUE(sharedPart->enabled());

    // [WHEN] The shared part is disabled
    edit(score, [score, sharedPart](Transaction& tx) {
        EditPart::setSharedPartEnabled(tx, score, sharedPart, false);
    });

    // [THEN] It's disabled
    EXPECT_FALSE(sharedPart->enabled());

    delete score;
}

TEST_F(Engraving_EditPartTests, SetStaffType_ChangesStaffType)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    ASSERT_EQ(score->nstaves(), 3u);

    Staff* staff = score->staff(0);

    // [WHEN] The flute's staff becomes a one-line percussion staff
    edit(score, [score, staff](Transaction& tx) {
        EditPart::setStaffType(tx, score, staff, StaffTypes::PERC_1LINE);
    });

    // [THEN] Only the flute's staff has the new type
    EXPECT_EQ(staff->staffType()->group(), StaffGroup::PERCUSSION);
    EXPECT_EQ(staff->staffType()->lines(), 1);
    EXPECT_EQ(score->staff(1)->staffType()->group(), StaffGroup::STANDARD);

    delete score;
}

TEST_F(Engraving_EditPartTests, SetStaffConfig_ChangesStaffProperties)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    ASSERT_EQ(score->nstaves(), 3u);

    Staff* staff = score->staff(0);

    StaffConfig config;
    config.visible = true;
    config.userDistance = Spatium(2.0);
    config.cutaway = true;
    config.clefTypeList = ClefTypeList(ClefType::F);
    config.staffType = *staff->staffType();

    // [WHEN] A new config is applied to the flute's staff
    edit(score, [score, staff, &config](Transaction& tx) {
        EditPart::setStaffConfig(tx, score, staff, config);
    });

    // [THEN] The flute's staff has the new properties
    EXPECT_EQ(staff->userDist(), Spatium(2.0));
    EXPECT_TRUE(staff->cutaway());
    EXPECT_EQ(staff->defaultClefType(), ClefTypeList(ClefType::F));

    // [THEN] The piano's staves are unchanged
    EXPECT_FALSE(score->staff(1)->cutaway());
    EXPECT_EQ(score->staff(1)->defaultClefType(), ClefTypeList(ClefType::G));

    delete score;
}

TEST_F(Engraving_EditPartTests, SetStaffConfig_HiddenStaff_ShowsStaff)
{
    // [GIVEN] A flute with a hidden staff
    MasterScore* score = createMasterScore({ u"flute" });
    ASSERT_EQ(score->nstaves(), 1u);

    Staff* staff = score->staff(0);

    edit(score, [score, staff](Transaction& tx) {
        EditPart::setStaffVisible(tx, score, staff, false);
    });
    ASSERT_FALSE(staff->visible());

    StaffConfig config;
    config.visible = true;
    config.clefTypeList = staff->defaultClefType();
    config.staffType = *staff->staffType();

    // [WHEN] A visible config is applied
    edit(score, [score, staff, &config](Transaction& tx) {
        EditPart::setStaffConfig(tx, score, staff, config);
    });

    // [THEN] The staff is shown
    EXPECT_TRUE(staff->visible());

    delete score;
}

TEST_F(Engraving_EditPartTests, SetVoiceVisible_HidesVoicesOnlyInExcerpts)
{
    // [GIVEN] A flute with a part score
    MasterScore* score = createMasterScore({ u"flute" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);
    ASSERT_EQ(score->nstaves(), 1u);

    Staff* masterStaff = score->staff(0);
    Score* excerptScore = addExcerptOfPart(score, parts.at(0))->excerptScore();
    const std::vector<Part*>& excerptParts = excerptScore->parts();
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute" }));
    ASSERT_EQ(excerptScore->nstaves(), 1u);

    // [WHEN] A voice is hidden in the main score
    bool changed = true;
    edit(score, [score, masterStaff, &changed](Transaction& tx) {
        changed = EditPart::setVoiceVisible(tx, score, masterStaff, 1, false);
    });

    // [THEN] Nothing changes
    EXPECT_FALSE(changed);
    EXPECT_TRUE(masterStaff->isVoiceVisible(1));

    // [WHEN] All voices but the first are hidden in the part score
    // [THEN] Each one is hidden (the part score's staff is replaced on each change)
    for (int voice = 1; voice < static_cast<int>(VOICES); ++voice) {
        edit(score, [excerptScore, voice, &changed](Transaction& tx) {
            changed = EditPart::setVoiceVisible(tx, excerptScore, excerptScore->staff(0), voice, false);
        });

        EXPECT_TRUE(changed);
        ASSERT_EQ(excerptScore->nstaves(), 1u);
        EXPECT_FALSE(excerptScore->staff(0)->isVoiceVisible(voice));
    }

    // [WHEN] The first voice is hidden too
    edit(score, [excerptScore, &changed](Transaction& tx) {
        changed = EditPart::setVoiceVisible(tx, excerptScore, excerptScore->staff(0), 0, false);
    });

    // [THEN] It stays visible, as the last visible voice
    EXPECT_FALSE(changed);
    ASSERT_EQ(excerptScore->nstaves(), 1u);
    EXPECT_TRUE(excerptScore->staff(0)->isVoiceVisible(0));

    // [THEN] The part score still has the flute with one staff
    ASSERT_EQ(instrumentIds(excerptScore), InstrumentIds({ u"flute" }));
    EXPECT_EQ(excerptParts.at(0)->nstaves(), 1u);

    delete score;
}

TEST_F(Engraving_EditPartTests, MoveParts_MovesPartAfterDestination)
{
    // [GIVEN] Flute, violin and piano
    MasterScore* score = createMasterScore({ u"flute", u"violin", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 3u);

    Part* flute = parts.at(0);
    Part* piano = parts.at(2);

    // [WHEN] The flute is moved after the piano
    edit(score, [score, flute, piano](Transaction& tx) {
        EditPart::moveParts(tx, score, { flute }, piano, true);
    });

    // [THEN] The flute is last, with its staff
    ASSERT_EQ(instrumentIds(score), InstrumentIds({ u"violin", u"piano", u"flute" }));
    EXPECT_EQ(parts.at(0)->nstaves(), 1u);
    EXPECT_EQ(parts.at(1)->nstaves(), 2u);
    EXPECT_EQ(parts.at(2)->nstaves(), 1u);
    ASSERT_EQ(score->nstaves(), 4u);
    EXPECT_EQ(score->staff(3)->part(), flute);

    delete score;
}

TEST_F(Engraving_EditPartTests, MoveStaves_MovesStaffBeforeDestination)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(score->nstaves(), 3u);

    Staff* upper = score->staff(1);
    Staff* lower = score->staff(2);

    // [WHEN] The piano's lower staff is moved before the upper one
    edit(score, [score, upper, lower](Transaction& tx) {
        EditPart::moveStaves(tx, score, { lower }, upper, false);
    });

    // [THEN] The piano's staves are swapped
    ASSERT_EQ(instrumentIds(score), InstrumentIds({ u"flute", u"piano" }));
    EXPECT_EQ(parts.at(0)->nstaves(), 1u);
    EXPECT_EQ(parts.at(1)->nstaves(), 2u);
    EXPECT_EQ(score->staff(1), lower);
    EXPECT_EQ(score->staff(2), upper);

    delete score;
}

TEST_F(Engraving_EditPartTests, PartsLayoutChangesScore_DetectsOrderAndMarkingChanges)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    Part* flute = parts.at(0);
    Part* piano = parts.at(1);
    Staff* pianoUpper = piano->staff(0);

    // [WHEN] The order is unchanged
    // [THEN] The score doesn't change
    EXPECT_FALSE(EditPart::partsLayoutChangesScore(score, layout({ flute, piano })));

    // [WHEN] The parts are swapped
    // [THEN] The score changes
    EXPECT_TRUE(EditPart::partsLayoutChangesScore(score, layout({ piano, flute })));

    // [WHEN] Markings move below the bottom staff
    // [THEN] The score changes
    EXPECT_TRUE(EditPart::partsLayoutChangesScore(score, layout({ flute, piano }, { { pianoUpper, nullptr, true } })));

    // [WHEN] Markings stay on their staff
    // [THEN] The score doesn't change
    EXPECT_FALSE(EditPart::partsLayoutChangesScore(score, layout({ flute, piano }, { { pianoUpper, pianoUpper, false } })));

    delete score;
}

TEST_F(Engraving_EditPartTests, SetPartsLayout_MasterScore_KeepsExcerptOrder)
{
    // [GIVEN] Flute and piano, with a part score of both
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    Part* flute = parts.at(0);
    Part* piano = parts.at(1);
    const Score* allPartsScore = addExcerptOfAllParts(score)->excerptScore();
    const std::vector<Part*>& allParts = allPartsScore->parts();
    ASSERT_EQ(instrumentIds(allPartsScore), InstrumentIds({ u"flute", u"piano" }));

    // [WHEN] The parts are swapped in the main score
    edit(score, [score, flute, piano](Transaction& tx) {
        EditPart::setPartsLayout(tx, score, layout({ piano, flute }));
    });

    // [THEN] The main score has the new order, with the staves
    ASSERT_EQ(instrumentIds(score), InstrumentIds({ u"piano", u"flute" }));
    EXPECT_EQ(parts.at(0)->nstaves(), 2u);
    EXPECT_EQ(parts.at(1)->nstaves(), 1u);
    ASSERT_EQ(score->nstaves(), 3u);
    EXPECT_EQ(score->staff(0)->part(), piano);
    EXPECT_EQ(score->staff(2)->part(), flute);

    // [THEN] The part score keeps its order
    ASSERT_EQ(instrumentIds(allPartsScore), InstrumentIds({ u"flute", u"piano" }));
    EXPECT_EQ(allParts.at(0)->nstaves(), 1u);
    EXPECT_EQ(allParts.at(1)->nstaves(), 2u);

    // [WHEN] Undo
    undo(score);

    // [THEN] The main score has the old order
    ASSERT_EQ(instrumentIds(score), InstrumentIds({ u"flute", u"piano" }));
    EXPECT_EQ(parts.at(0)->nstaves(), 1u);
    EXPECT_EQ(parts.at(1)->nstaves(), 2u);

    delete score;
}

TEST_F(Engraving_EditPartTests, SetPartsLayout_SystemObjectsBelowBottomStaff_SetsStyle)
{
    // [GIVEN] Flute and piano, with system markings on the piano's upper staff
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    ASSERT_EQ(score->nstaves(), 3u);

    Staff* pianoUpper = score->staff(1);
    const Staff* pianoLower = score->staff(2);

    edit(score, [score, pianoUpper](Transaction& tx) {
        EditPart::addSystemObjects(tx, score, { pianoUpper });
    });
    ASSERT_TRUE(pianoUpper->isSystemObjectStaff());

    // [WHEN] The markings are moved below the bottom staff
    edit(score, [score, pianoUpper](Transaction& tx) {
        EditPart::setPartsLayout(tx, score, layout(score->parts(), { { pianoUpper, nullptr, true } }));
    });

    // [THEN] The bottom staff has the markings, below it
    EXPECT_FALSE(pianoUpper->isSystemObjectStaff());
    EXPECT_TRUE(pianoLower->isSystemObjectStaff());
    EXPECT_TRUE(score->style().styleB(Sid::systemObjectsBelowBottomStaff));

    delete score;
}

TEST_F(Engraving_EditPartTests, SetPartsLayout_SystemObjectsMove_MovesMarkingsAndMeasureNumbers)
{
    // [GIVEN] Flute and piano, with a system text copied to the piano's upper staff, which shows measure numbers
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    ASSERT_EQ(score->nstaves(), 3u);

    Staff* pianoUpper = score->staff(1);
    Staff* pianoLower = score->staff(2);

    addSystemText(score);
    edit(score, [score, pianoUpper](Transaction& tx) {
        EditPart::addSystemObjects(tx, score, { pianoUpper });
        pianoUpper->undoChangeProperty(Pid::SHOW_MEASURE_NUMBERS, AutoOnOff::ON);
    });
    ASSERT_EQ(systemTextStaves(score), std::vector<staff_idx_t>({ 0, 1 }));

    // [WHEN] The markings are moved to the lower staff
    edit(score, [score, pianoUpper, pianoLower](Transaction& tx) {
        EditPart::setPartsLayout(tx, score, layout(score->parts(), { { pianoUpper, pianoLower, false } }));
    });

    // [THEN] The lower staff has the markings, the text copy and the measure numbers, above it
    EXPECT_FALSE(pianoUpper->isSystemObjectStaff());
    EXPECT_TRUE(pianoLower->isSystemObjectStaff());
    EXPECT_FALSE(score->style().styleB(Sid::systemObjectsBelowBottomStaff));
    EXPECT_EQ(systemTextStaves(score), std::vector<staff_idx_t>({ 0, 2 }));
    EXPECT_EQ(pianoLower->getProperty(Pid::SHOW_MEASURE_NUMBERS).value<AutoOnOff>(), AutoOnOff::ON);
    EXPECT_EQ(pianoUpper->getProperty(Pid::SHOW_MEASURE_NUMBERS).value<AutoOnOff>(), AutoOnOff::AUTO);

    delete score;
}

TEST_F(Engraving_EditPartTests, PartsLayoutChangesScore_DetectsSharedPartChanges)
{
    // [GIVEN] Two flutes, the first in a shared part
    MasterScore* score = createMasterScore({ u"flute", u"flute" });
    ASSERT_EQ(score->parts().size(), 2u);

    Part* flute1 = score->parts().at(0);
    Part* flute2 = score->parts().at(1);

    SharedPart* sharedPart = nullptr;
    edit(score, [flute1, &sharedPart](Transaction& tx) {
        sharedPart = EditStaveSharing::createSharedPart(tx, { flute1 });
    });
    ASSERT_TRUE(sharedPart);

    // [WHEN] Every part keeps its shared part
    // [THEN] The score doesn't change
    EXPECT_FALSE(EditPart::partsLayoutChangesScore(score, layout(score->parts())));

    // [WHEN] The second flute joins the shared part
    // [THEN] The score changes
    EXPECT_TRUE(EditPart::partsLayoutChangesScore(score, { { { sharedPart, nullptr }, { flute1, sharedPart },
                                                             { flute2, sharedPart } }, {} }));

    // [WHEN] The first flute leaves the shared part
    // [THEN] The score changes
    EXPECT_TRUE(EditPart::partsLayoutChangesScore(score, { { { sharedPart, nullptr }, { flute1, nullptr },
                                                             { flute2, nullptr } }, {} }));

    delete score;
}

TEST_F(Engraving_EditPartTests, SetPartsLayout_JoinSharedPart_MovesAndJoinsInOneUndoStep)
{
    // [GIVEN] Flute, piano and flute, the first flute in a shared part
    MasterScore* score = createMasterScore({ u"flute", u"piano", u"flute" });
    ASSERT_EQ(score->parts().size(), 3u);

    Part* flute1 = score->parts().at(0);
    Part* piano = score->parts().at(1);
    Part* flute2 = score->parts().at(2);

    SharedPart* sharedPart = nullptr;
    edit(score, [flute1, &sharedPart](Transaction& tx) {
        sharedPart = EditStaveSharing::createSharedPart(tx, { flute1 });
    });
    ASSERT_TRUE(sharedPart);
    ASSERT_EQ(score->parts(), std::vector<Part*>({ sharedPart, flute1, piano, flute2 }));

    // [WHEN] The second flute moves after the first one and joins the shared part
    edit(score, [score, sharedPart, flute1, piano, flute2](Transaction& tx) {
        EditPart::setPartsLayout(tx, score, { { { sharedPart, nullptr }, { flute1, sharedPart }, { flute2, sharedPart },
                                                { piano, nullptr } }, {} });
    });

    // [THEN] Both flutes are next to each other, in the shared part
    EXPECT_EQ(score->parts(), std::vector<Part*>({ sharedPart, flute1, flute2, piano }));
    EXPECT_EQ(flute2->sharedPart(), sharedPart);
    EXPECT_EQ(sharedPart->originParts(), std::vector<Part*>({ flute1, flute2 }));
    EXPECT_EQ(sharedPart->nstaves(), 2u);

    // [WHEN] Undo once
    undo(score);

    // [THEN] Both the order and the shared part are restored
    EXPECT_EQ(score->parts(), std::vector<Part*>({ sharedPart, flute1, piano, flute2 }));
    EXPECT_FALSE(flute2->sharedPart());
    EXPECT_EQ(sharedPart->originParts(), std::vector<Part*>({ flute1 }));
    EXPECT_EQ(sharedPart->nstaves(), 1u);

    delete score;
}

TEST_F(Engraving_EditPartTests, SetPartsLayout_LeaveSharedPart_MovesAndLeavesInOneUndoStep)
{
    // [GIVEN] Two flutes in a shared part, and piano
    MasterScore* score = createMasterScore({ u"flute", u"flute", u"piano" });
    ASSERT_EQ(score->parts().size(), 3u);

    Part* flute1 = score->parts().at(0);
    Part* flute2 = score->parts().at(1);
    Part* piano = score->parts().at(2);

    SharedPart* sharedPart = nullptr;
    edit(score, [flute1, flute2, &sharedPart](Transaction& tx) {
        sharedPart = EditStaveSharing::createSharedPart(tx, { flute1, flute2 });
    });
    ASSERT_TRUE(sharedPart);
    ASSERT_EQ(score->parts(), std::vector<Part*>({ sharedPart, flute1, flute2, piano }));

    // [WHEN] The second flute moves after the piano and leaves the shared part
    edit(score, [score, sharedPart, flute1, flute2, piano](Transaction& tx) {
        EditPart::setPartsLayout(tx, score, { { { sharedPart, nullptr }, { flute1, sharedPart }, { piano, nullptr },
                                                { flute2, nullptr } }, {} });
    });

    // [THEN] The second flute is after the piano, out of the shared part
    EXPECT_EQ(score->parts(), std::vector<Part*>({ sharedPart, flute1, piano, flute2 }));
    EXPECT_FALSE(flute2->sharedPart());
    EXPECT_EQ(sharedPart->originParts(), std::vector<Part*>({ flute1 }));
    EXPECT_EQ(sharedPart->nstaves(), 1u);

    // [WHEN] Undo once
    undo(score);

    // [THEN] Both the order and the shared part are restored
    EXPECT_EQ(score->parts(), std::vector<Part*>({ sharedPart, flute1, flute2, piano }));
    EXPECT_EQ(flute2->sharedPart(), sharedPart);
    EXPECT_EQ(sharedPart->originParts(), std::vector<Part*>({ flute1, flute2 }));
    EXPECT_EQ(sharedPart->nstaves(), 2u);

    delete score;
}

TEST_F(Engraving_EditPartTests, IsValidPartsLayout_DetectsSplitSharedPart)
{
    // [GIVEN] Two flutes in a shared part, and oboe
    MasterScore* score = createMasterScore({ u"flute", u"flute", u"oboe" });
    ASSERT_EQ(score->parts().size(), 3u);

    Part* flute1 = score->parts().at(0);
    Part* flute2 = score->parts().at(1);
    Part* oboe = score->parts().at(2);

    SharedPart* sharedPart = nullptr;
    edit(score, [flute1, flute2, &sharedPart](Transaction& tx) {
        sharedPart = EditStaveSharing::createSharedPart(tx, { flute1, flute2 });
    });
    ASSERT_TRUE(sharedPart);

    // [WHEN] The order is unchanged
    // [THEN] The layout is valid
    EXPECT_TRUE(EditPart::isValidPartsLayout(layout(score->parts())));

    // [WHEN] The oboe goes between the flutes
    // [THEN] The layout isn't valid
    EXPECT_FALSE(EditPart::isValidPartsLayout({ { { sharedPart, nullptr }, { flute1, sharedPart }, { oboe, nullptr },
                                                  { flute2, sharedPart } }, {} }));

    // [WHEN] A flute goes before the shared part
    // [THEN] The layout isn't valid
    EXPECT_FALSE(EditPart::isValidPartsLayout({ { { flute1, sharedPart }, { sharedPart, nullptr }, { flute2, sharedPart },
                                                  { oboe, nullptr } }, {} }));

    // [WHEN] The oboe joins the shared part after the flutes
    // [THEN] The layout is valid
    EXPECT_TRUE(EditPart::isValidPartsLayout({ { { sharedPart, nullptr }, { flute1, sharedPart }, { flute2, sharedPart },
                                                 { oboe, sharedPart } }, {} }));

    delete score;
}

TEST_F(Engraving_EditPartTests, ReorderStavesChangesScore_DetectsOrderChange)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    ASSERT_EQ(score->nstaves(), 3u);

    Staff* upper = score->staff(1);
    Staff* lower = score->staff(2);

    // [WHEN] The piano's staves keep their order
    // [THEN] The score doesn't change
    EXPECT_FALSE(EditPart::reorderStavesChangesScore({ upper, lower }));

    // [WHEN] They're swapped
    // [THEN] The score changes
    EXPECT_TRUE(EditPart::reorderStavesChangesScore({ lower, upper }));

    // [WHEN] Not all of them are given
    // [THEN] The score doesn't change
    EXPECT_FALSE(EditPart::reorderStavesChangesScore({ lower }));

    delete score;
}

TEST_F(Engraving_EditPartTests, ReorderStaves_ReordersPartStaves)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(score->nstaves(), 3u);

    const Staff* fluteStaff = score->staff(0);
    Staff* upper = score->staff(1);
    Staff* lower = score->staff(2);

    // [WHEN] The piano's staves are swapped
    edit(score, [score, upper, lower](Transaction& tx) {
        EditPart::reorderStaves(tx, score, { lower, upper });
    });

    // [THEN] Only the piano's staves move
    ASSERT_EQ(instrumentIds(score), InstrumentIds({ u"flute", u"piano" }));
    EXPECT_EQ(parts.at(0)->nstaves(), 1u);
    EXPECT_EQ(parts.at(1)->nstaves(), 2u);
    EXPECT_EQ(score->staff(0), fluteStaff);
    EXPECT_EQ(score->staff(1), lower);
    EXPECT_EQ(score->staff(2), upper);

    delete score;
}

TEST_F(Engraving_EditPartTests, AddSystemObjects_CopiesTopStaffMarkings)
{
    // [GIVEN] Flute and piano, with a system text on the top staff
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    ASSERT_EQ(score->nstaves(), 3u);

    Staff* staff = score->staff(1);
    addSystemText(score);
    ASSERT_EQ(systemTextStaves(score), std::vector<staff_idx_t>({ 0 }));

    // [WHEN] The piano's upper staff gets system markings
    edit(score, [score, staff](Transaction& tx) {
        EditPart::addSystemObjects(tx, score, { staff });
    });

    // [THEN] Only that staff is a system object staff, with a copy of the text
    EXPECT_FALSE(score->staff(0)->isSystemObjectStaff());
    EXPECT_TRUE(staff->isSystemObjectStaff());
    EXPECT_FALSE(score->staff(2)->isSystemObjectStaff());
    EXPECT_EQ(systemTextStaves(score), std::vector<staff_idx_t>({ 0, 1 }));

    // [WHEN] Undo
    undo(score);

    // [THEN] The staff and the copy are back to how they were
    EXPECT_FALSE(staff->isSystemObjectStaff());
    EXPECT_EQ(systemTextStaves(score), std::vector<staff_idx_t>({ 0 }));

    delete score;
}

TEST_F(Engraving_EditPartTests, RemoveSystemObjects_RemovesMarkingCopies)
{
    // [GIVEN] Flute and piano, with a system text copied to the piano's upper staff
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    ASSERT_EQ(score->nstaves(), 3u);

    Staff* staff = score->staff(1);
    addSystemText(score);

    edit(score, [score, staff](Transaction& tx) {
        EditPart::addSystemObjects(tx, score, { staff });
    });
    ASSERT_EQ(systemTextStaves(score), std::vector<staff_idx_t>({ 0, 1 }));

    // [WHEN] The staff's system markings are removed
    edit(score, [score, staff](Transaction& tx) {
        EditPart::removeSystemObjects(tx, score, { staff });
    });

    // [THEN] It isn't a system object staff anymore, only the top staff has the text
    EXPECT_FALSE(staff->isSystemObjectStaff());
    EXPECT_EQ(systemTextStaves(score), std::vector<staff_idx_t>({ 0 }));

    delete score;
}

TEST_F(Engraving_EditPartTests, RemoveSystemObjects_BelowBottomStaff_ResetsStyle)
{
    // [GIVEN] Flute and piano, with system markings below the bottom staff
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    ASSERT_EQ(score->nstaves(), 3u);

    Staff* pianoUpper = score->staff(1);
    Staff* pianoLower = score->staff(2);

    edit(score, [score, pianoUpper](Transaction& tx) {
        EditPart::addSystemObjects(tx, score, { pianoUpper });
    });
    edit(score, [score, pianoUpper](Transaction& tx) {
        EditPart::setPartsLayout(tx, score, layout(score->parts(), { { pianoUpper, nullptr, true } }));
    });
    ASSERT_TRUE(pianoLower->isSystemObjectStaff());
    ASSERT_TRUE(score->style().styleB(Sid::systemObjectsBelowBottomStaff));

    // [WHEN] The bottom staff's system markings are removed
    edit(score, [score, pianoLower](Transaction& tx) {
        EditPart::removeSystemObjects(tx, score, { pianoLower });
    });

    // [THEN] No markings are below the bottom staff anymore
    EXPECT_FALSE(pianoLower->isSystemObjectStaff());
    EXPECT_FALSE(score->style().styleB(Sid::systemObjectsBelowBottomStaff));

    delete score;
}

TEST_F(Engraving_EditPartTests, SetParts_RemovesAndAddsParts)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    const Part* flute = parts.at(0);
    Part* piano = parts.at(1);

    // [WHEN] The parts are set to a soloist piano and a new violin
    EditPart::SetPartsResult result;
    edit(score, [score, piano, &result](Transaction& tx) {
        const std::vector<EditPart::PartEntry> entries {
            { piano, nullptr, true },
            { nullptr, searchTemplate(u"violin"), false },
        };
        result = EditPart::setParts(tx, score, entries, score->scoreOrder());
    });

    // [THEN] The flute is removed and the violin is added after the piano
    ASSERT_EQ(instrumentIds(score), InstrumentIds({ u"piano", u"violin" }));
    EXPECT_EQ(parts.at(0)->nstaves(), 2u);
    EXPECT_EQ(parts.at(1)->nstaves(), 1u);

    ASSERT_EQ(result.removedParts.size(), 1u);
    EXPECT_EQ(result.removedParts.front(), flute);
    ASSERT_EQ(result.addedParts.size(), 1u);

    const Part* violin = parts.at(1);
    EXPECT_EQ(parts.at(0), piano);
    EXPECT_EQ(violin, result.addedParts.front());
    ASSERT_EQ(score->nstaves(), 3u);
    EXPECT_EQ(score->staff(0)->part(), piano);
    EXPECT_EQ(score->staff(2)->part(), violin);

    // [THEN] The piano is a soloist, the only violin isn't numbered
    EXPECT_TRUE(piano->soloist());
    EXPECT_EQ(violin->number(), 0);

    delete score;
}

TEST_F(Engraving_EditPartTests, SetParts_SameInstruments_NumbersThem)
{
    // [GIVEN] A violin
    MasterScore* score = createMasterScore({ u"violin" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    Part* violin = parts.at(0);

    // [WHEN] A second violin is added
    EditPart::SetPartsResult result;
    edit(score, [score, violin, &result](Transaction& tx) {
        const std::vector<EditPart::PartEntry> entries {
            { violin, nullptr, false },
            { nullptr, searchTemplate(u"violin"), false },
        };
        result = EditPart::setParts(tx, score, entries, score->scoreOrder());
    });

    // [THEN] The violins are numbered 1 and 2
    ASSERT_EQ(instrumentIds(score), InstrumentIds({ u"violin", u"violin" }));
    EXPECT_TRUE(result.removedParts.empty());
    ASSERT_EQ(result.addedParts.size(), 1u);
    EXPECT_EQ(parts.at(0), violin);
    EXPECT_EQ(violin->number(), 1);
    EXPECT_EQ(result.addedParts.front()->number(), 2);

    delete score;
}

TEST_F(Engraving_EditPartTests, SetParts_InstrumentNumberedInName_KeepsNameNumber)
{
    // [GIVEN] A violin named "Violin 1"
    MasterScore* score = createMasterScore({ u"violin" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    Part* violin = parts.at(0);
    violin->instrument()->setLongName(u"Violin 1");

    // [WHEN] A second violin is added
    EditPart::SetPartsResult result;
    edit(score, [score, violin, &result](Transaction& tx) {
        const std::vector<EditPart::PartEntry> entries {
            { violin, nullptr, false },
            { nullptr, searchTemplate(u"violin"), false },
        };
        result = EditPart::setParts(tx, score, entries, score->scoreOrder());
    });

    // [THEN] The first violin isn't numbered, but still counts
    EXPECT_EQ(instrumentIds(score), InstrumentIds({ u"violin", u"violin" }));
    ASSERT_EQ(result.addedParts.size(), 1u);
    EXPECT_EQ(violin->number(), 0);
    EXPECT_EQ(result.addedParts.front()->number(), 2);

    delete score;
}

TEST_F(Engraving_EditPartTests, SetParts_ExistingParts_ReordersThem)
{
    // [GIVEN] Flute and piano
    MasterScore* score = createMasterScore({ u"flute", u"piano" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2u);

    Part* flute = parts.at(0);
    Part* piano = parts.at(1);

    // [WHEN] The parts are set in the opposite order
    EditPart::SetPartsResult result;
    edit(score, [score, flute, piano, &result](Transaction& tx) {
        result = EditPart::setParts(tx, score, { { piano, nullptr, false }, { flute, nullptr, false } }, score->scoreOrder());
    });

    // [THEN] Nothing is added or removed, the parts are swapped
    EXPECT_TRUE(result.removedParts.empty());
    EXPECT_TRUE(result.addedParts.empty());
    ASSERT_EQ(instrumentIds(score), InstrumentIds({ u"piano", u"flute" }));
    EXPECT_EQ(parts.at(0)->nstaves(), 2u);
    EXPECT_EQ(parts.at(1)->nstaves(), 1u);
    ASSERT_EQ(score->nstaves(), 3u);
    EXPECT_EQ(score->staff(2)->part(), flute);

    delete score;
}

TEST_F(Engraving_EditPartTests, ChangePatch_ChangesChannelProgram)
{
    // [GIVEN] A flute
    MasterScore* score = createMasterScore({ u"flute" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    InstrChannel* channel = parts.at(0)->instrument()->channel(0);
    ASSERT_TRUE(channel);

    const int oldProgram = channel->program();

    MidiPatch patch {};
    patch.prog = oldProgram + 1;
    patch.bank = channel->bank();
    patch.synti = channel->synti();

    // [WHEN] The channel's patch is changed
    edit(score, [score, channel, &patch](Transaction&) {
        score->undo(new ChangePatch(score, channel, patch));
    });

    // [THEN] The channel has the new program
    EXPECT_EQ(channel->program(), oldProgram + 1);

    // [WHEN] Undo
    undo(score);

    // [THEN] The channel has the old program
    EXPECT_EQ(channel->program(), oldProgram);

    delete score;
}

TEST_F(Engraving_EditPartTests, SetUserBankController_ChangesChannel)
{
    // [GIVEN] A flute
    MasterScore* score = createMasterScore({ u"flute" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    InstrChannel* channel = parts.at(0)->instrument()->channel(0);
    ASSERT_TRUE(channel);

    const bool oldValue = channel->userBankController();

    // [WHEN] The user bank controller is toggled
    edit(score, [score, channel, oldValue](Transaction&) {
        score->undo(new SetUserBankController(channel, !oldValue));
    });

    // [THEN] The channel has the new value
    EXPECT_EQ(channel->userBankController(), !oldValue);

    // [WHEN] Undo
    undo(score);

    // [THEN] The channel has the old value
    EXPECT_EQ(channel->userBankController(), oldValue);

    delete score;
}

TEST_F(Engraving_EditPartTests, ChangeStringData_ChangesFrets)
{
    // [GIVEN] A guitar
    MasterScore* score = createMasterScore({ u"guitar-steel" });
    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 1u);

    Instrument* instrument = parts.at(0)->instrument();
    ASSERT_TRUE(instrument->stringData());

    const int oldFrets = instrument->stringData()->frets();
    std::vector<instrString> strings = instrument->stringData()->stringList();
    const StringData stringData(oldFrets + 1, strings);

    // [WHEN] The guitar gets one more fret
    edit(score, [score, instrument, &stringData](Transaction&) {
        score->undo(new ChangeStringData(instrument, stringData));
    });

    // [THEN] It has the new number of frets
    EXPECT_EQ(instrument->stringData()->frets(), oldFrets + 1);

    // [WHEN] Undo
    undo(score);

    // [THEN] It has the old number of frets
    EXPECT_EQ(instrument->stringData()->frets(), oldFrets);

    delete score;
}
