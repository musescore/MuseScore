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
#include <string>

#include "modularity/ioc.h"
#include "types/translatablestring.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/segment.h"
#include "engraving/editing/transaction/undostack.h"
#include "engraving/playback/practiceemphasisdata.h"
#include "engraving/tests/utils/scorerw.h"

#include "practiceemphasispanelmodel.h"

using namespace mu::engraving;
using namespace mu::playback;

namespace {
const muse::String SCORE_FILE(u"playback/playbackmodel_data/practice_emphasis/ssatb-one-bar.mscx");

Note* findPitch(Score* score, int pitch)
{
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
            for (track_idx_t track = 0; track < score->ntracks(); ++track) {
                EngravingItem* element = segment->element(track);
                if (!element || !element->isChord()) {
                    continue;
                }
                for (Note* note : toChord(element)->notes()) {
                    if (note->pitch() == pitch) {
                        return note;
                    }
                }
            }
        }
    }
    return nullptr;
}

const PracticeEmphasisData::PartRule* findPart(const PracticeEmphasisData& emphasis, const std::string& id)
{
    for (const PracticeEmphasisData::PartRule& part : emphasis.document().parts) {
        if (part.id == id) {
            return &part;
        }
    }
    return nullptr;
}

bool partSelectedInModel(const PracticeEmphasisPanelModel& model, const QString& id)
{
    for (const QVariant& item : model.parts()) {
        const QVariantMap map = item.toMap();
        if (map.value(QStringLiteral("id")).toString() == id) {
            return map.value(QStringLiteral("selected")).toBool();
        }
    }
    return false;
}
}

class Playback_PracticeEmphasisPanelModelTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const muse::modularity::ContextPtr ctx = std::make_shared<muse::modularity::Context>(42);
        m_score = ScoreRW::readScore(SCORE_FILE, false, nullptr, ctx);
        ASSERT_TRUE(m_score);
        m_model.attachScoreForTesting(m_score);
        ASSERT_TRUE(m_model.hasScore());
    }

    void TearDown() override
    {
        delete m_score;
        m_score = nullptr;
    }

    MasterScore* m_score = nullptr;
    PracticeEmphasisPanelModel m_model;
};

TEST_F(Playback_PracticeEmphasisPanelModelTests, EditingPartSwitchKeepsNameAndParentAfterLoad)
{
    m_model.setEditingPartId(QStringLiteral("S1"));
    m_model.addCustomPart();
    const QString customId = m_model.editingPartId();
    ASSERT_FALSE(customId.isEmpty());
    ASSERT_NE(customId, QStringLiteral("S1"));
    EXPECT_FALSE(m_model.editingPartBuiltin());
    EXPECT_TRUE(m_model.canDeleteEditingPart());

    m_model.applyEditingPartName(customId, QStringLiteral("sops"));
    m_model.applyEditingPartParentId(customId, QStringLiteral("S1"));
    EXPECT_EQ(m_model.editingPartName(), QStringLiteral("sops"));
    EXPECT_EQ(m_model.editingPartParentId(), QStringLiteral("S1"));
    const int s1ParentIndex = m_model.editingPartParentIndex();
    EXPECT_GT(s1ParentIndex, 0);
    EXPECT_EQ(m_model.parentValueAt(s1ParentIndex), QStringLiteral("S1"));
    EXPECT_EQ(m_model.parentValueAt(0), QString());

    m_model.setEditingPartId(QStringLiteral("S1"));
    EXPECT_TRUE(m_model.editingPartBuiltin());
    EXPECT_EQ(m_model.editingPartName(), QStringLiteral("1st soprano"));
    EXPECT_TRUE(m_model.editingPartParentId().isEmpty());
    EXPECT_EQ(m_model.editingPartParentIndex(), 0);
    EXPECT_EQ(m_model.editingValueAt(m_model.editingPartIndex()), QStringLiteral("S1"));

    m_model.setEditingPartId(customId);
    EXPECT_EQ(m_model.editingPartName(), QStringLiteral("sops"));
    EXPECT_EQ(m_model.editingPartParentId(), QStringLiteral("S1"));
    EXPECT_EQ(m_model.editingPartParentIndex(), s1ParentIndex);

    PracticeEmphasisData parsed;
    ASSERT_TRUE(parsed.fromJson(m_score->practiceEmphasis().toJson()));
    m_score->practiceEmphasis().setDocument(parsed.document());
    EXPECT_EQ(m_model.editingPartId(), customId);
    EXPECT_EQ(m_model.editingPartName(), QStringLiteral("sops"));
    EXPECT_EQ(m_model.editingPartParentId(), QStringLiteral("S1"));
    EXPECT_EQ(m_model.editingPartParentIndex(), s1ParentIndex);
}

TEST_F(Playback_PracticeEmphasisPanelModelTests, ParentChangeMapsDropdownIndex)
{
    m_model.setEditingPartId(QStringLiteral("S1"));
    m_model.addCustomPart();
    const QString customId = m_model.editingPartId();
    m_model.applyEditingPartName(customId, QStringLiteral("sops"));
    m_model.applyEditingPartParentId(customId, QStringLiteral("S1"));
    const int s1Index = m_model.editingPartParentIndex();
    EXPECT_GT(s1Index, 0);
    EXPECT_EQ(m_model.parentValueAt(s1Index), QStringLiteral("S1"));

    m_model.applyEditingPartParentId(customId, QStringLiteral("A1"));
    const int a1Index = m_model.editingPartParentIndex();
    EXPECT_GT(a1Index, 0);
    EXPECT_NE(a1Index, s1Index);
    EXPECT_EQ(m_model.parentValueAt(a1Index), QStringLiteral("A1"));
    EXPECT_EQ(m_model.editingPartParentId(), QStringLiteral("A1"));

    Note* e5 = findPitch(m_score, 76);
    Note* alto = findPitch(m_score, 67);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(alto);
    EXPECT_FALSE(m_score->practiceEmphasis().partContains(e5, customId.toStdString()));
    EXPECT_TRUE(m_score->practiceEmphasis().partContains(alto, customId.toStdString()));
}

TEST_F(Playback_PracticeEmphasisPanelModelTests, CopyFromAndResetAreUndoable)
{
    Note* e5 = findPitch(m_score, 76);
    Note* c5 = findPitch(m_score, 72);
    Note* d5 = findPitch(m_score, 74);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(c5);
    ASSERT_TRUE(d5);

    m_model.setEditingPartId(QStringLiteral("S1"));
    m_model.addCustomPart();
    const QString customId = m_model.editingPartId();
    EXPECT_TRUE(m_score->practiceEmphasis().partContains(e5, customId.toStdString()));
    EXPECT_FALSE(m_score->practiceEmphasis().partContains(c5, customId.toStdString()));
    EXPECT_TRUE(m_model.canCopyFromPart());

    m_model.setCopySourceId(QStringLiteral("S2"));
    EXPECT_EQ(m_model.copySourceId(), QStringLiteral("S2"));
    const QVariantList copyOptions = m_model.copySourceOptions();
    ASSERT_GE(m_model.copySourceIndex(), 0);
    ASSERT_LT(m_model.copySourceIndex(), copyOptions.size());
    EXPECT_EQ(copyOptions.at(m_model.copySourceIndex()).toMap().value(QStringLiteral("value")).toString(),
              QStringLiteral("S2"));
    m_model.copyFromPart();
    EXPECT_FALSE(m_score->practiceEmphasis().partContains(e5, customId.toStdString()));
    EXPECT_TRUE(m_score->practiceEmphasis().partContains(c5, customId.toStdString()));
    EXPECT_TRUE(m_score->practiceEmphasis().partContains(d5, customId.toStdString()));
    EXPECT_TRUE(m_model.canUndo());

    m_model.undo();
    EXPECT_TRUE(m_score->practiceEmphasis().partContains(e5, customId.toStdString()));
    EXPECT_FALSE(m_score->practiceEmphasis().partContains(c5, customId.toStdString()));
    EXPECT_TRUE(m_model.canRedo());
    m_model.redo();
    EXPECT_TRUE(m_score->practiceEmphasis().partContains(c5, customId.toStdString()));

    m_model.resetEditingPartEdits();
    EXPECT_TRUE(m_score->practiceEmphasis().partContains(e5, customId.toStdString()));
    EXPECT_FALSE(m_score->practiceEmphasis().partContains(c5, customId.toStdString()));
    m_model.undo();
    EXPECT_TRUE(m_score->practiceEmphasis().partContains(c5, customId.toStdString()));
}

TEST_F(Playback_PracticeEmphasisPanelModelTests, UndoRecordsClicksParentCopyAndResetOnly)
{
    Note* e5 = findPitch(m_score, 76);
    Note* c5 = findPitch(m_score, 72);
    Note* d5 = findPitch(m_score, 74);
    Note* alto = findPitch(m_score, 67);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(c5);
    ASSERT_TRUE(d5);
    ASSERT_TRUE(alto);

    m_model.setPartSelected(QStringLiteral("S1"), true);
    m_model.setEditingPartId(QStringLiteral("S1"));
    EXPECT_FALSE(m_model.canUndo());
    EXPECT_FALSE(m_model.canRedo());

    m_score->practiceEmphasis().editNote(d5);
    EXPECT_FALSE(m_score->practiceEmphasis().partContains(d5, "S1"));
    EXPECT_TRUE(m_model.canUndo());
    m_model.undo();
    EXPECT_TRUE(m_score->practiceEmphasis().partContains(d5, "S1"));
    EXPECT_TRUE(m_model.canRedo());
    m_model.redo();
    EXPECT_FALSE(m_score->practiceEmphasis().partContains(d5, "S1"));

    m_model.addCustomPart();
    const QString customId = m_model.editingPartId();
    EXPECT_TRUE(m_model.canUndo());
    m_model.applyEditingPartName(customId, QStringLiteral("renamed"));
    m_model.setPlaybackActive(true);
    m_model.setEmphasizedDb(6);
    m_model.setOtherDb(-12);
    m_model.setSolo(true);
    m_model.setPartSelected(customId, true);
    m_model.setFromMeasure(1);
    m_model.setToMeasure(1);
    EXPECT_EQ(m_model.editingPartName(), QStringLiteral("renamed"));
    EXPECT_TRUE(m_model.solo());
    EXPECT_TRUE(partSelectedInModel(m_model, customId));

    m_model.undo();
    EXPECT_EQ(m_model.editingPartName(), QStringLiteral("renamed"));
    EXPECT_TRUE(m_model.solo());
    EXPECT_TRUE(partSelectedInModel(m_model, customId));
    EXPECT_TRUE(m_score->practiceEmphasis().partContains(d5, "S1"));
    EXPECT_EQ(findPart(m_score->practiceEmphasis(), customId.toStdString()) != nullptr, true);

    m_model.applyEditingPartParentId(customId, QStringLiteral("A1"));
    EXPECT_EQ(m_model.editingPartParentId(), QStringLiteral("A1"));
    m_model.undo();
    EXPECT_EQ(m_model.editingPartParentId(), QStringLiteral("S1"));
    EXPECT_EQ(m_model.editingPartName(), QStringLiteral("renamed"));
    EXPECT_TRUE(m_model.solo());

    m_model.deleteEditingPart();
    EXPECT_EQ(findPart(m_score->practiceEmphasis(), customId.toStdString()), nullptr);
    m_model.undo();
    EXPECT_EQ(findPart(m_score->practiceEmphasis(), customId.toStdString()), nullptr);
}

TEST_F(Playback_PracticeEmphasisPanelModelTests, SoloMutesNonEditingNotes)
{
    Note* e5 = findPitch(m_score, 76);
    Note* alto = findPitch(m_score, 67);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(alto);

    m_model.setPartSelected(QStringLiteral("S1"), true);
    m_model.setPartSelected(QStringLiteral("A1"), true);
    m_model.setEditingPartId(QStringLiteral("S1"));
    m_model.setPlaybackActive(true);
    EXPECT_TRUE(m_score->practiceEmphasis().isEmphasized(e5));
    EXPECT_TRUE(m_score->practiceEmphasis().isEmphasized(alto));

    m_model.setSolo(true);
    EXPECT_TRUE(m_model.solo());
    EXPECT_TRUE(m_model.playbackActive());
    EXPECT_TRUE(partSelectedInModel(m_model, QStringLiteral("S1")));
    EXPECT_TRUE(partSelectedInModel(m_model, QStringLiteral("A1")));
    EXPECT_TRUE(m_score->practiceEmphasis().isEmphasized(e5));
    EXPECT_FALSE(m_score->practiceEmphasis().isEmphasized(alto));
    EXPECT_EQ(m_score->practiceEmphasis().otherGainDb(), PracticeEmphasisData::SOLO_MUTE_DB);

    m_model.setEditingPartId(QStringLiteral("A1"));
    EXPECT_FALSE(m_score->practiceEmphasis().isEmphasized(e5));
    EXPECT_TRUE(m_score->practiceEmphasis().isEmphasized(alto));

    m_model.setSolo(false);
    EXPECT_FALSE(m_model.solo());
    EXPECT_TRUE(m_score->practiceEmphasis().isEmphasized(e5));
    EXPECT_TRUE(m_score->practiceEmphasis().isEmphasized(alto));
}

TEST_F(Playback_PracticeEmphasisPanelModelTests, FromSelectionAndAllMeasureRange)
{
    ASSERT_TRUE(m_score->firstMeasure());
    m_model.setSelectionMeasuresForTesting(m_score->firstMeasure(), m_score->firstMeasure());
    EXPECT_NE(m_model.selectionButtonText().indexOf(QStringLiteral("1")), -1);

    m_model.useSelectionRange();
    EXPECT_EQ(m_model.fromMeasure(), 1);
    EXPECT_EQ(m_model.toMeasure(), 1);

    Note* e5 = findPitch(m_score, 76);
    ASSERT_TRUE(e5);
    m_model.setPartSelected(QStringLiteral("S1"), true);
    m_model.setPlaybackActive(true);
    EXPECT_TRUE(m_score->practiceEmphasis().isEmphasized(e5));

    m_model.setFromMeasure(2);
    m_model.setToMeasure(2);
    EXPECT_FALSE(m_score->practiceEmphasis().isEmphasized(e5));

    m_model.clearMeasureRange();
    EXPECT_EQ(m_model.fromMeasure(), 0);
    EXPECT_EQ(m_model.toMeasure(), 0);
    EXPECT_TRUE(m_score->practiceEmphasis().isEmphasized(e5));
}

TEST_F(Playback_PracticeEmphasisPanelModelTests, CustomPartInheritsNewParentNotes)
{
    Note* e5 = findPitch(m_score, 76);
    Note* c5 = findPitch(m_score, 72);
    ASSERT_TRUE(e5);
    ASSERT_TRUE(c5);

    m_model.setEditingPartId(QStringLiteral("S1"));
    m_model.addCustomPart();
    const QString customId = m_model.editingPartId();
    m_model.setPartSelected(customId, true);
    m_model.setPartSelected(QStringLiteral("S1"), false);

    EXPECT_TRUE(m_score->practiceEmphasis().partContains(e5, customId.toStdString()));
    EXPECT_FALSE(m_score->practiceEmphasis().partContains(c5, customId.toStdString()));
    EXPECT_TRUE(m_score->practiceEmphasis().isEmphasized(e5));
    EXPECT_FALSE(m_score->practiceEmphasis().isEmphasized(c5));

    m_model.setEditingPartId(QStringLiteral("S1"));
    m_score->practiceEmphasis().editNote(c5);
    EXPECT_TRUE(m_score->practiceEmphasis().partContains(c5, "S1"));
    EXPECT_TRUE(m_score->practiceEmphasis().partContains(c5, customId.toStdString()));
    EXPECT_TRUE(m_score->practiceEmphasis().isEmphasized(c5));
}

TEST_F(Playback_PracticeEmphasisPanelModelTests, DormantEditsReapplyWhenEidReturns)
{
    Note* alto = findPitch(m_score, 67);
    ASSERT_TRUE(alto);
    ASSERT_TRUE(alto->eid().isValid());
    const std::string altoEid = alto->eid().toStdString();

    m_model.setEmphasisMode(true);
    m_model.setPartSelected(QStringLiteral("S1"), true);
    m_model.setEditingPartId(QStringLiteral("S1"));
    m_score->practiceEmphasis().editNote(alto);
    EXPECT_TRUE(m_score->practiceEmphasis().partContains(alto, "S1"));

    m_score->startCmd(TranslatableString::untranslatable("Practice emphasis panel tests"));
    m_score->select(alto);
    m_score->cmdDeleteSelection();
    m_score->endCmd();
    EXPECT_EQ(findPitch(m_score, 67), nullptr);
    const PracticeEmphasisData::PartRule* soprano = findPart(m_score->practiceEmphasis(), "S1");
    ASSERT_TRUE(soprano);
    EXPECT_TRUE(std::find(soprano->added.begin(), soprano->added.end(), altoEid) != soprano->added.end());

    m_score->undoRedo(true, nullptr);
    Note* restored = findPitch(m_score, 67);
    ASSERT_TRUE(restored);
    EXPECT_EQ(restored->eid().toStdString(), altoEid);
    EXPECT_TRUE(m_score->practiceEmphasis().partContains(restored, "S1"));
}
