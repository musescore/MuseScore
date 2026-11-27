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
#include <gmock/gmock.h>

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rehearsalmark.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/systemtext.h"
#include "engraving/dom/tempotext.h"
#include "engraving/editing/transaction/undoablecommand.h"

#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/systemobjectslayertreeitem.h"

#include "notation/tests/mocks/masternotationmock.h"
#include "notation/tests/mocks/notationmock.h"

using namespace ::testing;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;

namespace mu::instrumentsscene {
class InstrumentsScene_SystemObjectsLayerTreeItemTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_score = compat::ScoreAccess::createMasterScore(nullptr);

        m_notation = std::make_shared<NiceMock<NotationMock> >();
        m_masterNotation = std::make_shared<NiceMock<MasterNotationMock> >();

        ON_CALL(*m_masterNotation, notation()).WillByDefault(Return(m_notation));

        Part* part = new Part(m_score);
        m_score->appendPart(part);
        m_score->appendStaff(Factory::createStaff(part));
        m_score->appendStaff(Factory::createStaff(part));

        m_item = new SystemObjectsLayerTreeItem(m_masterNotation, m_notation, nullptr);
    }

    void TearDown() override
    {
        delete m_item;

        for (EngravingItem* element : m_elements) {
            delete element;
        }

        delete m_score;
    }

    Staff* staff(staff_idx_t idx) const
    {
        return m_score->staff(idx);
    }

    //! A system marking on the staff
    template<typename T>
    T* addElement(T* element, const Staff* staff)
    {
        element->setXmlText(u"text");
        element->setTrack(staff->idx() * VOICES);
        element->setSystemFlag(true);
        m_elements.push_back(element);

        return element;
    }

    TempoText* addTempo(const Staff* staff)
    {
        return addElement(Factory::createTempoText(m_score->dummy()), staff);
    }

    SystemText* addSystemText(const Staff* staff)
    {
        return addElement(Factory::createSystemText(m_score->dummy()), staff);
    }

    RehearsalMark* addRehearsalMark(const Staff* staff)
    {
        return addElement(Factory::createRehearsalMark(m_score->dummy()), staff);
    }

    Marker* addMarker(const Staff* staff)
    {
        return addElement(Factory::createMarker(m_score->dummy()), staff);
    }

    static SystemObjectsGroup group(std::vector<EngravingItem*> items)
    {
        return SystemObjectsGroup { items.front()->type(), items, nullptr };
    }

    //! A change to all staves, as sent by the score after an edit
    ScoreChanges changes(EngravingItem* element, CommandType command) const
    {
        ScoreChanges result;
        result.staffIdxFrom = 0;
        result.staffIdxTo = m_score->nstaves() - 1;
        result.changedObjects[element] = { command };

        return result;
    }

    void sendChanges(const ScoreChanges& changes)
    {
        static_cast<AbstractLayoutPanelTreeItem*>(m_item)->onScoreChanged(changes);
    }

    MasterScore* m_score = nullptr;
    SystemObjectsLayerTreeItem* m_item = nullptr;

private:
    std::vector<EngravingItem*> m_elements;

    std::shared_ptr<NotationMock> m_notation;
    std::shared_ptr<MasterNotationMock> m_masterNotation;
};
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, Init_TopStaff)
{
    // [WHEN] The row of the top staff's markings is built
    m_item->init(staff(0), {});

    // [THEN] The row shows the staff
    EXPECT_EQ(m_item->type(), LayoutPanelItemType::SYSTEM_OBJECTS_LAYER);
    EXPECT_EQ(m_item->staff(), staff(0));
    EXPECT_EQ(m_item->id(), staff(0)->id());
    EXPECT_EQ(m_item->staffId(), staff(0)->id().toQString());

    // [THEN] It has settings, but can't be selected, removed, expanded or dropped on
    EXPECT_TRUE(m_item->settingsAvailable());
    EXPECT_FALSE(m_item->isSelectable());
    EXPECT_FALSE(m_item->isRemovable());
    EXPECT_FALSE(m_item->isExpandable());
    EXPECT_FALSE(m_item->canAcceptDrop(m_item, LayoutPanelItemType::ON_ITEM));
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, Init_OtherStaff)
{
    // [WHEN] The row of the second staff's markings is built
    m_item->init(staff(1), {});

    // [THEN] It can be selected, removed and dropped on
    EXPECT_EQ(m_item->id(), staff(1)->id());
    EXPECT_TRUE(m_item->isSelectable());
    EXPECT_TRUE(m_item->isRemovable());
    EXPECT_TRUE(m_item->canAcceptDrop(m_item, LayoutPanelItemType::ON_ITEM));
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, SetStaff_None)
{
    // [GIVEN] A markings row
    m_item->init(staff(1), {});

    // [WHEN] Its staff is cleared
    m_item->setStaff(nullptr);

    // [THEN] The row has no staff
    EXPECT_EQ(m_item->staff(), nullptr);
    EXPECT_EQ(m_item->id(), ID());
    EXPECT_TRUE(m_item->staffId().isEmpty());
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, Title_NoMarkings)
{
    // [WHEN] The row is built for a staff without markings
    m_item->init(staff(0), {});

    // [THEN] The row has a generic title, is hidden and its settings are disabled
    EXPECT_EQ(m_item->title(), QString("System markings"));
    EXPECT_FALSE(m_item->isVisible());
    EXPECT_FALSE(m_item->settingsEnabled());
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, Title_OneGroup)
{
    // [GIVEN] Two tempo markings on the top staff
    const SystemObjectGroups groups { group({ addTempo(staff(0)), addTempo(staff(0)) }) };

    // [WHEN] The row is built
    m_item->init(staff(0), groups);

    // [THEN] The row is named after the markings, is visible and its settings are enabled
    EXPECT_EQ(m_item->title(), QString("Tempo(s)"));
    EXPECT_TRUE(m_item->isVisible());
    EXPECT_TRUE(m_item->settingsEnabled());
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, Title_SeveralGroups)
{
    // [GIVEN] A hidden tempo marking, then a system text, a rehearsal mark and a marker on the top staff
    TempoText* tempo = addTempo(staff(0));
    tempo->setVisible(false);

    const SystemObjectGroups groups {
        group({ tempo }),
        group({ addSystemText(staff(0)) }),
        group({ addRehearsalMark(staff(0)) }),
        group({ addMarker(staff(0)) }),
    };

    // [WHEN] The row is built
    m_item->init(staff(0), groups);

    // [THEN] The row lists the visible kinds of markings, in order
    EXPECT_EQ(m_item->title(), QString("System text(s), rehearsal mark(s) & marker(s)"));
    EXPECT_TRUE(m_item->isVisible());
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, Title_AllHidden)
{
    // [GIVEN] A hidden tempo marking on the top staff
    TempoText* tempo = addTempo(staff(0));
    tempo->setVisible(false);

    // [WHEN] The row is built
    m_item->init(staff(0), { group({ tempo }) });

    // [THEN] The row says the markings are hidden, is hidden, and its settings stay enabled
    EXPECT_EQ(m_item->title(), QString("System markings hidden"));
    EXPECT_FALSE(m_item->isVisible());
    EXPECT_TRUE(m_item->settingsEnabled());
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, OnScoreChanged_AddElement)
{
    // [GIVEN] A row of the top staff with a tempo marking
    m_item->init(staff(0), { group({ addTempo(staff(0)) }) });

    // [WHEN] A system text is added to the top staff
    sendChanges(changes(addSystemText(staff(0)), CommandType::AddElement));

    // [THEN] The row lists it
    EXPECT_EQ(m_item->title(), QString("Tempo(s) & system text(s)"));
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, OnScoreChanged_AddElementToEmptyRow)
{
    // [GIVEN] A row of the top staff without markings
    m_item->init(staff(0), {});

    // [WHEN] A tempo marking is added to the top staff
    sendChanges(changes(addTempo(staff(0)), CommandType::AddElement));

    // [THEN] The row is named after it, and is shown
    EXPECT_EQ(m_item->title(), QString("Tempo(s)"));
    EXPECT_TRUE(m_item->isVisible());
    EXPECT_TRUE(m_item->settingsEnabled());
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, OnScoreChanged_AddElementToOtherStaff)
{
    // [GIVEN] A row of the top staff with a tempo marking
    m_item->init(staff(0), { group({ addTempo(staff(0)) }) });

    // [WHEN] A system text is added to the second staff
    sendChanges(changes(addSystemText(staff(1)), CommandType::AddElement));

    // [THEN] The row doesn't change
    EXPECT_EQ(m_item->title(), QString("Tempo(s)"));
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, OnScoreChanged_AddEmptyText)
{
    // [GIVEN] A row of the top staff with a tempo marking
    m_item->init(staff(0), { group({ addTempo(staff(0)) }) });

    // [WHEN] An empty system text is added to the top staff
    SystemText* text = addSystemText(staff(0));
    text->setXmlText(u"");
    sendChanges(changes(text, CommandType::AddElement));

    // [THEN] The row doesn't change
    EXPECT_EQ(m_item->title(), QString("Tempo(s)"));
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, OnScoreChanged_AddNonSystemElement)
{
    // [GIVEN] A row of the top staff with a tempo marking
    m_item->init(staff(0), { group({ addTempo(staff(0)) }) });

    // [WHEN] A text that isn't a system marking is added to the top staff
    SystemText* text = addSystemText(staff(0));
    text->setSystemFlag(false);
    sendChanges(changes(text, CommandType::AddElement));

    // [THEN] The row doesn't change
    EXPECT_EQ(m_item->title(), QString("Tempo(s)"));
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, OnScoreChanged_OutsideChangedStaves)
{
    // [GIVEN] A row of the second staff with a tempo marking
    m_item->init(staff(1), { group({ addTempo(staff(1)) }) });

    // [WHEN] A system text is added to the second staff, in a change that covers only the top staff
    ScoreChanges topStaffChanges = changes(addSystemText(staff(1)), CommandType::AddElement);
    topStaffChanges.staffIdxTo = 0;
    sendChanges(topStaffChanges);

    // [THEN] The row doesn't change
    EXPECT_EQ(m_item->title(), QString("Tempo(s)"));
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, OnScoreChanged_RemoveElement)
{
    // [GIVEN] A row of the top staff with a tempo marking and a system text
    SystemText* text = addSystemText(staff(0));
    m_item->init(staff(0), { group({ addTempo(staff(0)) }), group({ text }) });

    // [WHEN] The system text is removed
    sendChanges(changes(text, CommandType::RemoveElement));

    // [THEN] The row lists only the tempo marking
    EXPECT_EQ(m_item->title(), QString("Tempo(s)"));
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, OnScoreChanged_RemoveLastElement)
{
    // [GIVEN] A row of the top staff with a tempo marking
    TempoText* tempo = addTempo(staff(0));
    m_item->init(staff(0), { group({ tempo }) });

    // [WHEN] The tempo marking is removed
    sendChanges(changes(tempo, CommandType::RemoveElement));

    // [THEN] The row has no markings left
    EXPECT_EQ(m_item->title(), QString("System markings"));
    EXPECT_FALSE(m_item->isVisible());
    EXPECT_FALSE(m_item->settingsEnabled());
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, OnScoreChanged_HideElement)
{
    // [GIVEN] A row of the top staff with a tempo marking
    TempoText* tempo = addTempo(staff(0));
    m_item->init(staff(0), { group({ tempo }) });

    // [WHEN] The tempo marking is hidden
    tempo->setVisible(false);
    ScoreChanges visibilityChanges = changes(tempo, CommandType::ChangeProperty);
    visibilityChanges.changedPropertyIdSet.insert(Pid::VISIBLE);
    sendChanges(visibilityChanges);

    // [THEN] The row is hidden
    EXPECT_EQ(m_item->title(), QString("System markings hidden"));
    EXPECT_FALSE(m_item->isVisible());
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, OnScoreChanged_OtherPropertyChanged)
{
    // [GIVEN] A row of the top staff with a tempo marking
    TempoText* tempo = addTempo(staff(0));
    m_item->init(staff(0), { group({ tempo }) });

    // [WHEN] The tempo marking is hidden, but only another property is reported as changed
    tempo->setVisible(false);
    ScoreChanges otherChanges = changes(tempo, CommandType::ChangeProperty);
    otherChanges.changedPropertyIdSet.insert(Pid::PLACEMENT);
    sendChanges(otherChanges);

    // [THEN] The row isn't updated
    EXPECT_EQ(m_item->title(), QString("Tempo(s)"));
    EXPECT_TRUE(m_item->isVisible());
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, OnScoreChanged_MeasureNumbersOnSystemObjectStaves)
{
    // [GIVEN] A row of the top staff without markings
    m_item->init(staff(0), {});

    // [WHEN] Measure numbers are placed on system object staves
    m_score->style().set(Sid::measureNumberPlacementMode, MeasureNumberPlacement::ON_SYSTEM_OBJECT_STAVES);

    ScoreChanges styleChanges;
    styleChanges.changedStyleIdSet.insert(Sid::measureNumberPlacementMode);
    sendChanges(styleChanges);

    // [THEN] The row lists the measure numbers, and is shown
    EXPECT_EQ(m_item->title(), QString("Measure number(s)"));
    EXPECT_TRUE(m_item->isVisible());
    EXPECT_TRUE(m_item->settingsEnabled());
}

TEST_F(InstrumentsScene_SystemObjectsLayerTreeItemTests, OnScoreChanged_MeasureNumbersHiddenOnStaff)
{
    // [GIVEN] A row of the second staff, which isn't a system object staff
    m_item->init(staff(1), {});

    // [WHEN] Measure numbers are placed on system object staves
    m_score->style().set(Sid::measureNumberPlacementMode, MeasureNumberPlacement::ON_SYSTEM_OBJECT_STAVES);

    ScoreChanges styleChanges;
    styleChanges.changedStyleIdSet.insert(Sid::measureNumberPlacementMode);
    sendChanges(styleChanges);

    // [THEN] The row lists the measure numbers as hidden
    EXPECT_EQ(m_item->title(), QString("System markings hidden"));
    EXPECT_FALSE(m_item->isVisible());
    EXPECT_TRUE(m_item->settingsEnabled());
}
