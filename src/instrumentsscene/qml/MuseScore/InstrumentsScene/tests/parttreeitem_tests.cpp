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

#include "async/notifylist.h"
#include "async/processevents.h"
#include "containers.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/sharedpart.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/text.h"

#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/parttreeitem.h"
#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/roottreeitem.h"
#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/sharedparttreeitem.h"
#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/staffcontroltreeitem.h"
#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/stafftreeitem.h"
#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/systemobjectslayertreeitem.h"

#include "interactive/tests/mocks/interactivemock.h"
#include "notation/tests/mocks/excerptnotationmock.h"
#include "notation/tests/mocks/masternotationmock.h"
#include "notation/tests/mocks/notationmock.h"
#include "notation/tests/mocks/notationpartsmock.h"
#include "notationscene/tests/mocks/selectinstrumentsscenariomock.h"

using namespace ::testing;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;

namespace mu::instrumentsscene {
class InstrumentsScene_PartTreeItemTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_score = compat::ScoreAccess::createMasterScore(nullptr);

        m_parts = std::make_shared<NiceMock<NotationPartsMock> >();
        m_masterParts = std::make_shared<NiceMock<NotationPartsMock> >();
        m_notation = std::make_shared<NiceMock<NotationMock> >();
        m_masterNotation = std::make_shared<NiceMock<MasterNotationMock> >();

        ON_CALL(*m_notation, parts()).WillByDefault(Return(m_parts));
        ON_CALL(*m_masterNotation, notation()).WillByDefault(Return(m_notation));
        ON_CALL(*m_masterNotation, parts()).WillByDefault(Return(m_masterParts));
        ON_CALL(*m_masterNotation, excerpts()).WillByDefault(ReturnRef(m_excerpts));

        ON_CALL(*m_parts, part(_)).WillByDefault(Invoke([this](const ID& partId) -> const Part* {
            return isInPartScore(partId) ? m_score->partById(partId) : nullptr;
        }));

        ON_CALL(*m_parts, partExists(_)).WillByDefault(Invoke([this](const ID& partId) {
            return isInPartScore(partId);
        }));

        ON_CALL(*m_parts, partList()).WillByDefault(Invoke([this]() {
            std::vector<const Part*> parts;
            for (const Part* part : m_score->parts()) {
                if (isInPartScore(part->id())) {
                    parts.push_back(part);
                }
            }

            return async::NotifyList<const Part*>(parts, nullptr);
        }));

        ON_CALL(*m_parts, staff(_)).WillByDefault(Invoke([this](const ID& staffId) {
            return m_score->staffById(staffId);
        }));

        ON_CALL(*m_masterParts, part(_)).WillByDefault(Invoke([this](const ID& partId) {
            return m_score->partById(partId);
        }));

        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        m_selectInstrumentsScenario = std::make_shared<NiceMock<SelectInstrumentsScenarioMock> >();

        m_root = new RootTreeItem(m_masterNotation, m_notation);
    }

    void TearDown() override
    {
        for (EngravingItem* element : m_elements) {
            delete element;
        }

        delete m_root;
        delete m_score;
    }

    //! A score element on the staff; without a staff, it has no instrument
    EngravingItem* addElement(const Staff* staff)
    {
        Text* text = Factory::createText(m_score->dummy(), TextStyleType::DEFAULT, false);
        text->setTrack(staff ? staff->idx() * VOICES : muse::nidx);
        m_elements.push_back(text);

        return text;
    }

    const Part* addPart(size_t staffCount = 1, const String& name = u"")
    {
        Part* part = new Part(m_score);
        part->setLongNameAll(name);
        m_score->appendPart(part);

        for (size_t i = 0; i < staffCount; ++i) {
            m_score->appendStaff(Factory::createStaff(part));
        }

        return part;
    }

    void addExcerpt()
    {
        m_excerpts.push_back(std::make_shared<NiceMock<ExcerptNotationMock> >());
    }

    //! Not in the part score the panel shows
    void removeFromPartScore(const Part* part)
    {
        m_partsMissingFromPartScore.push_back(part->id());
    }

    //! Same rows as LayoutPanelTreeModel::buildMasterPartItem(): the staves, then the "add staff" row
    PartTreeItem* buildPartTreeItem(const Part* part)
    {
        m_partItem = new PartTreeItem(m_masterNotation, m_notation, m_root);
        m_partItem->interactive.set(m_interactive);
        m_partItem->selectInstrumentsScenario.set(m_selectInstrumentsScenario);
        m_partItem->init(part);
        m_root->appendChild(m_partItem);

        for (const Staff* staff : part->staves()) {
            StaffTreeItem* staffItem = new StaffTreeItem(m_masterNotation, m_notation, m_partItem);
            staffItem->init(staff);
            m_partItem->appendChild(staffItem);
        }

        StaffControlTreeItem* controlItem = new StaffControlTreeItem(m_masterNotation, m_notation, m_partItem);
        controlItem->init(part->id());
        m_partItem->appendChild(controlItem);

        return m_partItem;
    }

    SharedPart* addSharedPart(const std::vector<const Part*>& members)
    {
        SharedPart* sharedPart = new SharedPart(m_score);
        m_score->appendPart(sharedPart);
        m_score->appendStaff(Factory::createStaff(sharedPart));

        for (const Part* member : members) {
            sharedPart->addOriginPart(const_cast<Part*>(member));
        }

        return sharedPart;
    }

    //! The root works out the group roles from the score's shared parts
    SharedPartTreeItem* buildSharedPartTreeItem(const SharedPart* sharedPart)
    {
        SharedPartTreeItem* item = new SharedPartTreeItem(m_masterNotation, m_notation, m_root);
        item->init(sharedPart);
        m_root->appendChild(item);

        return item;
    }

    //! Same as the up/down buttons: LayoutPanelTreeModel::moveRows()
    void moveRow(int row, int toRow)
    {
        const int destinationRow = row > toRow ? toRow : toRow + 1;
        m_partItem->moveChildren(row, 1, m_partItem, destinationRow, true);
    }

    //! Same as LayoutPanelTreeModel during a drag: moveRows() for each one-row step, endActiveDrag() on drop
    void dragRow(int row, int toRow)
    {
        const int step = toRow > row ? 1 : -1;
        for (int r = row; r != toRow; r += step) {
            const int destinationRow = step > 0 ? r + 2 : r - 1;
            m_partItem->moveChildren(r, 1, m_partItem, destinationRow, false);
        }

        m_partItem->applyToScore();
    }

    IDList childIds() const
    {
        IDList ids;
        ids.reserve(m_partItem->childCount());
        for (const AbstractLayoutPanelTreeItem* item : m_partItem->childItems()) {
            ids.push_back(item->id());
        }

        return ids;
    }

    MasterScore* m_score = nullptr;
    RootTreeItem* m_root = nullptr;
    PartTreeItem* m_partItem = nullptr;

    //! The parts of the score the panel shows, and of the main score
    std::shared_ptr<NotationPartsMock> m_parts;
    std::shared_ptr<NotationPartsMock> m_masterParts;

    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<SelectInstrumentsScenarioMock> m_selectInstrumentsScenario;

    std::shared_ptr<NotationMock> m_notation;
    std::shared_ptr<MasterNotationMock> m_masterNotation;
    ExcerptNotationList m_excerpts;

private:
    std::vector<EngravingItem*> m_elements;

    bool isInPartScore(const ID& partId) const
    {
        return !muse::contains(m_partsMissingFromPartScore, partId);
    }

    IDList m_partsMissingFromPartScore;
};
}

TEST_F(InstrumentsScene_PartTreeItemTests, Init_PartInScore)
{
    // [GIVEN] An instrument whose name has extra whitespace
    const Part* part = addPart(1, u"  Flute   1 ");

    // [WHEN] Its row is built
    buildPartTreeItem(part);

    // [THEN] The row shows the instrument, with the whitespace collapsed
    EXPECT_EQ(m_partItem->id(), part->id());
    EXPECT_EQ(m_partItem->part(), part);
    EXPECT_EQ(m_partItem->title(), QString("Flute 1"));

    // [THEN] It's visible, enabled and has its settings
    EXPECT_TRUE(m_partItem->existsInScore());
    EXPECT_TRUE(m_partItem->isVisible());
    EXPECT_TRUE(m_partItem->isEnabled());
    EXPECT_TRUE(m_partItem->settingsAvailable());
    EXPECT_TRUE(m_partItem->settingsEnabled());
    EXPECT_TRUE(m_partItem->isExpandable());
    EXPECT_TRUE(m_partItem->isRemovable());
}

TEST_F(InstrumentsScene_PartTreeItemTests, Init_HiddenPart)
{
    // [GIVEN] A hidden instrument
    Part* part = const_cast<Part*>(addPart());
    part->setShow(false);

    // [THEN] Building the row doesn't change the score
    EXPECT_CALL(*m_parts, setPartVisible(_, _)).Times(0);

    // [WHEN] Its row is built
    buildPartTreeItem(part);

    // [THEN] The row is hidden, but still has its settings
    EXPECT_FALSE(m_partItem->isVisible());
    EXPECT_TRUE(m_partItem->settingsAvailable());
}

TEST_F(InstrumentsScene_PartTreeItemTests, Init_PartMissingFromPartScore)
{
    // [GIVEN] An instrument of the main score that isn't in the part score
    const Part* part = addPart(1, u"Flute");
    removeFromPartScore(part);

    // [WHEN] Its row is built
    buildPartTreeItem(part);

    // [THEN] The row shows the main score's instrument
    EXPECT_EQ(m_partItem->id(), part->id());
    EXPECT_EQ(m_partItem->part(), part);
    EXPECT_EQ(m_partItem->title(), QString("Flute"));

    // [THEN] It's hidden and can't be removed, but can be expanded and has its settings
    EXPECT_FALSE(m_partItem->existsInScore());
    EXPECT_FALSE(m_partItem->isVisible());
    EXPECT_FALSE(m_partItem->isRemovable());
    EXPECT_TRUE(m_partItem->settingsAvailable());
    EXPECT_TRUE(m_partItem->settingsEnabled());
    EXPECT_TRUE(m_partItem->isExpandable());
}

TEST_F(InstrumentsScene_PartTreeItemTests, MoveChildren_StaffDown)
{
    // [GIVEN] An instrument with three staves
    const Part* part = addPart(3);
    const ID s0 = part->staff(0)->id();
    const ID s1 = part->staff(1)->id();
    const ID s2 = part->staff(2)->id();
    buildPartTreeItem(part);

    // [THEN] The first staff goes between the other two
    EXPECT_CALL(*m_parts, reorderStaves(IDList { s1, s0, s2 }));

    // [WHEN] The first staff is moved down by one row
    moveRow(0, 1);

    // [THEN] The "add staff" row stays last
    EXPECT_EQ(childIds(), (IDList { s1, s0, s2, ID() }));
}

TEST_F(InstrumentsScene_PartTreeItemTests, MoveChildren_StaffUp)
{
    // [GIVEN] An instrument with three staves
    const Part* part = addPart(3);
    const ID s0 = part->staff(0)->id();
    const ID s1 = part->staff(1)->id();
    const ID s2 = part->staff(2)->id();
    buildPartTreeItem(part);

    // [THEN] The last staff goes between the other two
    EXPECT_CALL(*m_parts, reorderStaves(IDList { s0, s2, s1 }));

    // [WHEN] The last staff is moved up by one row
    moveRow(2, 1);

    // [THEN] The "add staff" row stays last
    EXPECT_EQ(childIds(), (IDList { s0, s2, s1, ID() }));
}

TEST_F(InstrumentsScene_PartTreeItemTests, MoveChildren_StaffToEnd)
{
    // [GIVEN] An instrument with three staves
    const Part* part = addPart(3);
    const ID s0 = part->staff(0)->id();
    const ID s1 = part->staff(1)->id();
    const ID s2 = part->staff(2)->id();
    buildPartTreeItem(part);

    // [THEN] The first staff goes last
    EXPECT_CALL(*m_parts, reorderStaves(IDList { s1, s2, s0 }));

    // [WHEN] The first staff is dragged below the last one
    dragRow(0, 2);

    // [THEN] The "add staff" row stays last
    EXPECT_EQ(childIds(), (IDList { s1, s2, s0, ID() }));
}

TEST_F(InstrumentsScene_PartTreeItemTests, MoveChildren_DragBackToStart)
{
    // [GIVEN] An instrument with two staves
    const Part* part = addPart(2);
    const ID s0 = part->staff(0)->id();
    const ID s1 = part->staff(1)->id();
    buildPartTreeItem(part);

    // [THEN] The panel order is sent once, on drop
    EXPECT_CALL(*m_parts, reorderStaves(IDList { s0, s1 }));

    // [WHEN] The first staff is dragged down and back up before the drop
    m_partItem->moveChildren(0, 1, m_partItem, 2, false);
    m_partItem->moveChildren(1, 1, m_partItem, 0, false);
    m_partItem->applyToScore();

    // [THEN] The order is unchanged
    EXPECT_EQ(childIds(), (IDList { s0, s1, ID() }));
}

TEST_F(InstrumentsScene_PartTreeItemTests, RemoveChildren_RemovesStaves)
{
    // [GIVEN] An instrument with three staves
    const Part* part = addPart(3);
    const ID s0 = part->staff(0)->id();
    const ID s1 = part->staff(1)->id();
    const ID s2 = part->staff(2)->id();
    buildPartTreeItem(part);

    // [THEN] The middle staff is removed from the main score
    EXPECT_CALL(*m_masterParts, removeStaves(IDList { s1 }));

    // [WHEN] The middle staff's row is deleted
    m_partItem->removeChildren(1, 1, true);

    // [THEN] Its row is gone
    EXPECT_EQ(childIds(), (IDList { s0, s2, ID() }));
}

TEST_F(InstrumentsScene_PartTreeItemTests, RemoveChildren_KeepsStavesWhenRowsOnlyRemoved)
{
    // [GIVEN] An instrument with two staves
    const Part* part = addPart(2);
    const ID s1 = part->staff(1)->id();
    buildPartTreeItem(part);

    // [THEN] The score keeps the staff
    EXPECT_CALL(*m_masterParts, removeStaves(_)).Times(0);

    // [WHEN] The first staff's row is removed without deleting it
    m_partItem->removeChildren(0, 1, false);

    // [THEN] Its row is gone
    EXPECT_EQ(childIds(), (IDList { s1, ID() }));
}

TEST_F(InstrumentsScene_PartTreeItemTests, CanAcceptDrop)
{
    // [GIVEN] Two instruments and a system markings row
    const Part* a = addPart(2);
    const Part* b = addPart();
    PartTreeItem* aItem = buildPartTreeItem(a);
    PartTreeItem* bItem = buildPartTreeItem(b);
    SystemObjectsLayerTreeItem* layerItem = new SystemObjectsLayerTreeItem(m_masterNotation, m_notation, m_root);

    // [THEN] Instruments and system markings rows can be dropped on an instrument
    EXPECT_TRUE(aItem->canAcceptDrop(bItem, LayoutPanelItemType::ON_ITEM));
    EXPECT_TRUE(aItem->canAcceptDrop(layerItem, LayoutPanelItemType::ON_ITEM));

    // [THEN] Staves and other objects can't
    EXPECT_FALSE(aItem->canAcceptDrop(aItem->childAtRow(0), LayoutPanelItemType::ON_ITEM));
    EXPECT_FALSE(aItem->canAcceptDrop(m_root, LayoutPanelItemType::ON_ITEM));

    // [GIVEN] A shared-staves group header
    PartTreeItem* groupItem = new PartTreeItem(m_masterNotation, m_notation, m_root, LayoutPanelItemType::SHARED_PART);
    m_root->appendChild(groupItem);

    // [THEN] Instruments and groups can be dropped on each other
    EXPECT_TRUE(aItem->canAcceptDrop(groupItem, LayoutPanelItemType::ON_ITEM));
    EXPECT_TRUE(groupItem->canAcceptDrop(aItem, LayoutPanelItemType::ON_ITEM));

    // [GIVEN] An instrument in another tree
    RootTreeItem otherRoot(m_masterNotation, m_notation);
    PartTreeItem* otherPartItem = new PartTreeItem(m_masterNotation, m_notation, &otherRoot);
    otherRoot.appendChild(otherPartItem);

    // [THEN] It can't be dropped on the group
    EXPECT_FALSE(groupItem->canAcceptDrop(otherPartItem, LayoutPanelItemType::ON_ITEM));
    EXPECT_EQ(groupItem->dropTarget(otherPartItem, LayoutPanelItemType::ON_ITEM), nullptr);
}

TEST_F(InstrumentsScene_PartTreeItemTests, DropTarget_OnOtherGroup_Group)
{
    // [GIVEN] A, then B in a shared-staves group
    const Part* a = addPart();
    const Part* b = addPart();
    const SharedPart* s = addSharedPart({ b });
    PartTreeItem* aItem = buildPartTreeItem(a);
    PartTreeItem* header = buildSharedPartTreeItem(s);
    PartTreeItem* bItem = buildPartTreeItem(b);

    // [THEN] A dragged onto B or onto the group header joins the group on release
    EXPECT_EQ(bItem->dropTarget(aItem, LayoutPanelItemType::ON_ITEM), header);
    EXPECT_EQ(header->dropTarget(aItem, LayoutPanelItemType::ON_ITEM), header);
}

TEST_F(InstrumentsScene_PartTreeItemTests, CanAcceptDrop_BelowOtherGroupFromAbove_True)
{
    // [GIVEN] A, then B in a shared-staves group
    const Part* a = addPart();
    const Part* b = addPart();
    const SharedPart* s = addSharedPart({ b });
    PartTreeItem* aItem = buildPartTreeItem(a);
    buildSharedPartTreeItem(s);
    PartTreeItem* bItem = buildPartTreeItem(b);

    // [THEN] A dragged into the gap below the group moves onto B's row, without a drop target
    EXPECT_EQ(bItem->dropTarget(aItem, LayoutPanelItemType::BELOW_ITEM), nullptr);
    EXPECT_TRUE(bItem->canAcceptDrop(aItem, LayoutPanelItemType::BELOW_ITEM));
}

TEST_F(InstrumentsScene_PartTreeItemTests, CanAcceptDrop_BelowOtherGroupFromBelow_False)
{
    // [GIVEN] B in a shared-staves group, then C
    const Part* b = addPart();
    const Part* c = addPart();
    const SharedPart* s = addSharedPart({ b });
    buildSharedPartTreeItem(s);
    PartTreeItem* bItem = buildPartTreeItem(b);
    PartTreeItem* cItem = buildPartTreeItem(c);

    // [THEN] C dragged into the gap below the group is already after it: nothing moves, and there is no drop target
    EXPECT_EQ(bItem->dropTarget(cItem, LayoutPanelItemType::BELOW_ITEM), nullptr);
    EXPECT_FALSE(bItem->canAcceptDrop(cItem, LayoutPanelItemType::BELOW_ITEM));
}

TEST_F(InstrumentsScene_PartTreeItemTests, CanAcceptDrop_OwnGroup_True)
{
    // [GIVEN] B and C in the same shared-staves group
    const Part* b = addPart();
    const Part* c = addPart();
    const SharedPart* s = addSharedPart({ b, c });
    buildSharedPartTreeItem(s);
    PartTreeItem* bItem = buildPartTreeItem(b);
    PartTreeItem* cItem = buildPartTreeItem(c);

    // [THEN] B dragged onto C moves onto C's row, without a drop target
    EXPECT_EQ(cItem->dropTarget(bItem, LayoutPanelItemType::ON_ITEM), nullptr);
    EXPECT_TRUE(cItem->canAcceptDrop(bItem, LayoutPanelItemType::ON_ITEM));
}

TEST_F(InstrumentsScene_PartTreeItemTests, CanAcceptDrop_NotInGroup_True)
{
    // [GIVEN] Two instruments
    PartTreeItem* aItem = buildPartTreeItem(addPart());
    PartTreeItem* bItem = buildPartTreeItem(addPart());

    // [THEN] A dragged onto B or below it moves onto B's row, without a drop target
    EXPECT_EQ(bItem->dropTarget(aItem, LayoutPanelItemType::ON_ITEM), nullptr);
    EXPECT_TRUE(bItem->canAcceptDrop(aItem, LayoutPanelItemType::ON_ITEM));
    EXPECT_EQ(bItem->dropTarget(aItem, LayoutPanelItemType::BELOW_ITEM), nullptr);
    EXPECT_TRUE(bItem->canAcceptDrop(aItem, LayoutPanelItemType::BELOW_ITEM));
}

TEST_F(InstrumentsScene_PartTreeItemTests, CanAcceptDrop_GroupHeaderOnOtherGroup_True)
{
    // [GIVEN] Two shared-staves groups, B in the second
    const Part* b = addPart();
    const SharedPart* first = addSharedPart({});
    const SharedPart* second = addSharedPart({ b });
    PartTreeItem* firstHeader = buildSharedPartTreeItem(first);
    buildSharedPartTreeItem(second);
    PartTreeItem* bItem = buildPartTreeItem(b);

    // [THEN] Only instruments join groups: the first group moves onto B's row, without a drop target
    EXPECT_EQ(bItem->dropTarget(firstHeader, LayoutPanelItemType::ON_ITEM), nullptr);
    EXPECT_TRUE(bItem->canAcceptDrop(firstHeader, LayoutPanelItemType::ON_ITEM));
}

TEST_F(InstrumentsScene_PartTreeItemTests, ResolveMove_StavesAboveAddStaffRow)
{
    // [GIVEN] An instrument with two staves, then the "add staff" row
    buildPartTreeItem(addPart(2));

    // [THEN] The staves swap places
    const std::optional<AbstractLayoutPanelTreeItem::RowMove> move = m_partItem->resolveMove(0, 0, 1);
    ASSERT_TRUE(move);
    EXPECT_EQ(move->sourceRow, 0);
    EXPECT_EQ(move->count, 1);
    EXPECT_EQ(move->destinationRow, 1);

    // [THEN] Nothing moves onto the "add staff" row, above the first staff, or onto itself
    EXPECT_FALSE(m_partItem->resolveMove(1, 1, 2));
    EXPECT_FALSE(m_partItem->resolveMove(0, 0, -1));
    EXPECT_FALSE(m_partItem->resolveMove(0, 0, 0));
}

TEST_F(InstrumentsScene_PartTreeItemTests, NeedsRemovalConfirmation_OnlyWithPartScores)
{
    // [GIVEN] An instrument, and no part scores
    buildPartTreeItem(addPart());

    // [THEN] It's removed without asking
    EXPECT_FALSE(m_partItem->needsRemovalConfirmation());

    // [WHEN] There is a part score
    addExcerpt();

    // [THEN] Removing it asks first, as it's also removed from the part score
    EXPECT_TRUE(m_partItem->needsRemovalConfirmation());
}

TEST_F(InstrumentsScene_PartTreeItemTests, IsSelectedInScore)
{
    // [GIVEN] Two instruments
    const Part* a = addPart();
    const Part* b = addPart();
    PartTreeItem* aItem = buildPartTreeItem(a);

    // [THEN] A's row follows a selection with an element of A
    EXPECT_TRUE(aItem->isSelectedInScore({ addElement(b->staff(0)), addElement(a->staff(0)) }));

    // [THEN] But not one with only other elements, or an empty one
    EXPECT_FALSE(aItem->isSelectedInScore({ addElement(b->staff(0)), addElement(nullptr) }));
    EXPECT_FALSE(aItem->isSelectedInScore({}));
}

TEST_F(InstrumentsScene_PartTreeItemTests, IsHiddenByCollapse)
{
    // [GIVEN] A collapsed, then B with two staves in a shared-staves group
    const Part* a = addPart();
    const Part* b = addPart(2);
    const SharedPart* s = addSharedPart({ b });
    PartTreeItem* aItem = buildPartTreeItem(a);
    PartTreeItem* header = buildSharedPartTreeItem(s);
    PartTreeItem* bItem = buildPartTreeItem(b);
    aItem->setIsExpanded(false);

    // [WHEN] The group is expanded
    header->setIsExpanded(true);

    // [THEN] Nothing is hidden
    EXPECT_FALSE(bItem->isHiddenByCollapse());
    EXPECT_FALSE(bItem->childAtRow(0)->isHiddenByCollapse());

    // [WHEN] The group is collapsed
    header->setIsExpanded(false);

    // [THEN] B and its staves are hidden, the header and A aren't
    EXPECT_TRUE(bItem->isHiddenByCollapse());
    EXPECT_TRUE(bItem->childAtRow(0)->isHiddenByCollapse());
    EXPECT_TRUE(bItem->childAtRow(1)->isHiddenByCollapse());
    EXPECT_FALSE(header->isHiddenByCollapse());

    // [THEN] Neither are A's rows: the view hides the children of a collapsed row itself
    EXPECT_FALSE(aItem->isHiddenByCollapse());
    EXPECT_FALSE(aItem->childAtRow(0)->isHiddenByCollapse());
}

TEST_F(InstrumentsScene_PartTreeItemTests, InstrumentId)
{
    // [GIVEN] A flute
    Part* part = const_cast<Part*>(addPart());
    part->instrument()->setId(u"flute");
    buildPartTreeItem(part);

    // [THEN] The row gives its instrument
    EXPECT_EQ(m_partItem->instrumentId(), QString("flute"));
}

TEST_F(InstrumentsScene_PartTreeItemTests, ReplaceInstrument)
{
    // [GIVEN] A flute
    Part* part = const_cast<Part*>(addPart());
    part->instrument()->setId(u"flute");
    buildPartTreeItem(part);

    auto isFluteKey = [part](const InstrumentKey& key) {
        return key.partId == part->id() && key.instrumentId == u"flute" && key.tick == Part::MAIN_INSTRUMENT_TICK;
    };

    // [THEN] The instrument dialog opens on the flute, and an oboe is picked
    EXPECT_CALL(*m_selectInstrumentsScenario, selectInstrument(Truly(isFluteKey))).WillOnce(Invoke([](const InstrumentKey&) {
        InstrumentTemplate oboe;
        oboe.id = u"oboe";

        return async::make_promise<InstrumentTemplate>([oboe](auto resolve, auto) {
            return resolve(oboe);
        });
    }));

    // [THEN] The flute is replaced by an oboe with the default staff type, on the main score
    EXPECT_CALL(*m_masterParts, replaceInstrument(Truly(isFluteKey), Property(&Instrument::id, String(u"oboe")),
                                                  StaffType::getDefaultPreset(StaffGroup::STANDARD)));

    // [WHEN] The instrument is replaced
    m_partItem->replaceInstrument();
    async::processMessages();
}

TEST_F(InstrumentsScene_PartTreeItemTests, ReplaceInstrument_Cancelled)
{
    // [GIVEN] An instrument
    const Part* part = addPart();
    buildPartTreeItem(part);

    // [THEN] Nothing is replaced when the dialog is closed
    EXPECT_CALL(*m_selectInstrumentsScenario, selectInstrument(_)).WillOnce(Invoke([](const InstrumentKey&) {
        return async::make_promise<InstrumentTemplate>([](auto, auto reject) {
            return reject(static_cast<int>(Ret::Code::Cancel), "cancel");
        });
    }));
    EXPECT_CALL(*m_masterParts, replaceInstrument(_, _, _)).Times(0);

    // [WHEN] The instrument dialog is closed
    m_partItem->replaceInstrument();
    async::processMessages();
}

TEST_F(InstrumentsScene_PartTreeItemTests, ResetAllFormatting)
{
    // [GIVEN] An instrument
    const Part* part = addPart();
    buildPartTreeItem(part);

    ON_CALL(*m_interactive, buttonData(_)).WillByDefault([](IInteractive::Button button) {
        return IInteractive::ButtonData(button, "");
    });

    // [THEN] The user is asked, and says yes
    EXPECT_CALL(*m_interactive, question(_, _, _, _, _, _)).WillOnce(Invoke([](auto&&...) {
        return async::make_promise<IInteractive::Result>([](auto resolve, auto) {
            return resolve(IInteractive::Result(static_cast<int>(IInteractive::Button::Yes)));
        });
    }));

    // [THEN] The part score's instrument is replaced by a copy of the main score's, without its shared part
    EXPECT_CALL(*m_parts, replacePart(part->id(), NotNull())).WillOnce(Invoke([part](const ID&, Part* newPart) {
        EXPECT_NE(newPart, part);
        EXPECT_EQ(newPart->sharedPart(), nullptr);
        delete newPart;
    }));

    // [WHEN] All formatting is reset
    m_partItem->resetAllFormatting();
    async::processMessages();
}

TEST_F(InstrumentsScene_PartTreeItemTests, ResetAllFormatting_Declined)
{
    // [GIVEN] An instrument
    const Part* part = addPart();
    buildPartTreeItem(part);

    ON_CALL(*m_interactive, buttonData(_)).WillByDefault([](IInteractive::Button button) {
        return IInteractive::ButtonData(button, "");
    });

    // [THEN] The user is asked, and says no
    EXPECT_CALL(*m_interactive, question(_, _, _, _, _, _)).WillOnce(Invoke([](auto&&...) {
        return async::make_promise<IInteractive::Result>([](auto resolve, auto) {
            return resolve(IInteractive::Result(static_cast<int>(IInteractive::Button::No)));
        });
    }));

    // [THEN] Nothing is reset
    EXPECT_CALL(*m_parts, replacePart(_, _)).Times(0);

    // [WHEN] All formatting is reset
    m_partItem->resetAllFormatting();
    async::processMessages();
}

TEST_F(InstrumentsScene_PartTreeItemTests, SetVisible_HidesPart)
{
    // [GIVEN] A visible instrument
    const Part* part = addPart();
    buildPartTreeItem(part);

    // [THEN] The instrument is hidden on the score
    EXPECT_CALL(*m_parts, setPartVisible(part->id(), false));
    EXPECT_CALL(*m_parts, insertPart(_, _)).Times(0);

    // [WHEN] Its row is hidden
    m_partItem->setIsVisible(false);
}

TEST_F(InstrumentsScene_PartTreeItemTests, SetVisible_AddsPartMissingFromPartScore)
{
    // [GIVEN] Three instruments, B isn't in the part score
    const Part* a = addPart();
    const Part* b = addPart();
    const Part* c = addPart();
    removeFromPartScore(b);

    buildPartTreeItem(a);
    PartTreeItem* bItem = buildPartTreeItem(b);
    buildPartTreeItem(c);

    // [THEN] A copy of B without the main score's shared part goes before C, the next visible instrument
    EXPECT_CALL(*m_parts, setPartVisible(_, _)).Times(0);
    EXPECT_CALL(*m_parts, insertPart(NotNull(), 1)).WillOnce(Invoke([b](Part* part, size_t) {
        EXPECT_NE(part, b);
        EXPECT_EQ(part->sharedPart(), nullptr);
        delete part;
    }));

    // [WHEN] B's row is shown
    bItem->setIsVisible(true);
}

TEST_F(InstrumentsScene_PartTreeItemTests, SetVisible_AddsPartMissingFromPartScoreAtEnd)
{
    // [GIVEN] Two instruments, B isn't in the part score
    const Part* a = addPart();
    const Part* b = addPart();
    removeFromPartScore(b);

    buildPartTreeItem(a);
    PartTreeItem* bItem = buildPartTreeItem(b);

    // [THEN] A copy of B goes last, since no visible instrument follows it
    EXPECT_CALL(*m_parts, insertPart(NotNull(), 1)).WillOnce(Invoke([](Part* part, size_t) {
        delete part;
    }));

    // [WHEN] B's row is shown
    bItem->setIsVisible(true);
}

TEST_F(InstrumentsScene_PartTreeItemTests, OnScoreChanged_UpdatesTitleAndVisibility)
{
    // [GIVEN] A visible instrument
    Part* part = const_cast<Part*>(addPart(1, u"Flute"));
    buildPartTreeItem(part);

    // [THEN] Following the score doesn't change it back
    EXPECT_CALL(*m_parts, setPartVisible(_, _)).Times(0);

    // [WHEN] The instrument is renamed and hidden on the score
    part->setLongNameAll(u"Oboe");
    part->setShow(false);
    static_cast<AbstractLayoutPanelTreeItem*>(m_partItem)->onScoreChanged(ScoreChanges());

    // [THEN] The row follows
    EXPECT_EQ(m_partItem->title(), QString("Oboe"));
    EXPECT_FALSE(m_partItem->isVisible());
}
