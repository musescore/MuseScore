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

#include "containers.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"

#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/parttreeitem.h"
#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/stafftreeitem.h"

#include "notation/tests/mocks/masternotationmock.h"
#include "notation/tests/mocks/notationmock.h"
#include "notation/tests/mocks/notationpartsmock.h"

using namespace ::testing;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;

namespace mu::instrumentsscene {
class InstrumentsScene_StaffTreeItemTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_score = compat::ScoreAccess::createMasterScore(nullptr);

        m_parts = std::make_shared<NiceMock<NotationPartsMock> >();
        m_notation = std::make_shared<NiceMock<NotationMock> >();
        m_masterNotation = std::make_shared<NiceMock<MasterNotationMock> >();

        ON_CALL(*m_notation, parts()).WillByDefault(Return(m_parts));
        ON_CALL(*m_masterNotation, notation()).WillByDefault(Return(m_notation));
        ON_CALL(*m_masterNotation, parts()).WillByDefault(Return(m_parts));

        ON_CALL(*m_parts, part(_)).WillByDefault(Invoke([this](const ID& partId) {
            return m_score->partById(partId);
        }));

        ON_CALL(*m_parts, partExists(_)).WillByDefault(Invoke([this](const ID& partId) {
            return m_score->partById(partId) != nullptr;
        }));

        ON_CALL(*m_parts, staff(_)).WillByDefault(Invoke([this](const ID& staffId) -> const Staff* {
            return muse::contains(m_stavesMissingFromPartScore, staffId) ? nullptr : m_score->staffById(staffId);
        }));

        m_itemsOwner = new QObject();
    }

    void TearDown() override
    {
        delete m_itemsOwner;
        delete m_score;
    }

    Part* addPart(size_t staffCount = 1)
    {
        Part* part = new Part(m_score);
        m_score->appendPart(part);

        for (size_t i = 0; i < staffCount; ++i) {
            m_score->appendStaff(Factory::createStaff(part));
        }

        return part;
    }

    //! Not in the part score the panel shows
    void removeFromPartScore(const Staff* staff)
    {
        m_stavesMissingFromPartScore.push_back(staff->id());
    }

    //! Without an instrument row
    StaffTreeItem* buildStaffTreeItem(const Staff* staff)
    {
        StaffTreeItem* item = new StaffTreeItem(m_masterNotation, m_notation, m_itemsOwner);
        item->init(staff);

        return item;
    }

    //! Same rows as LayoutPanelTreeModel::buildMasterPartItem(), without the "add staff" row
    PartTreeItem* buildPartTreeItem(const Part* part)
    {
        PartTreeItem* partItem = new PartTreeItem(m_masterNotation, m_notation, m_itemsOwner);
        partItem->init(part);

        for (const Staff* staff : part->staves()) {
            StaffTreeItem* staffItem = new StaffTreeItem(m_masterNotation, m_notation, partItem);
            staffItem->init(staff);
            partItem->appendChild(staffItem);
        }

        return partItem;
    }

    MasterScore* m_score = nullptr;

    std::shared_ptr<NotationPartsMock> m_parts;

private:
    IDList m_stavesMissingFromPartScore;
    QObject* m_itemsOwner = nullptr;

    std::shared_ptr<NotationMock> m_notation;
    std::shared_ptr<MasterNotationMock> m_masterNotation;
};
}

TEST_F(InstrumentsScene_StaffTreeItemTests, Init_StaffInScore)
{
    // [GIVEN] A staff
    const Staff* staff = addPart()->staff(0);

    // [WHEN] Its row is built
    const StaffTreeItem* item = buildStaffTreeItem(staff);

    // [THEN] The row shows the staff, named after its clef
    EXPECT_EQ(item->id(), staff->id());
    EXPECT_EQ(item->type(), LayoutPanelItemType::STAFF);
    EXPECT_EQ(item->title(), staff->staffName().toQString());

    // [THEN] It's visible, has its settings, and can be selected and removed
    EXPECT_TRUE(item->isVisible());
    EXPECT_TRUE(item->settingsAvailable());
    EXPECT_TRUE(item->settingsEnabled());
    EXPECT_TRUE(item->isSelectable());
    EXPECT_TRUE(item->isRemovable());
}

TEST_F(InstrumentsScene_StaffTreeItemTests, Init_StaffTypeName)
{
    // [GIVEN] A staff whose staff type has a name
    Staff* staff = addPart()->staff(0);
    staff->staffType(Fraction(0, 1))->setLongName(u"Percussion");

    // [WHEN] Its row is built
    const StaffTreeItem* item = buildStaffTreeItem(staff);

    // [THEN] The row shows the staff type's name
    EXPECT_EQ(item->title(), QString("Percussion"));
}

TEST_F(InstrumentsScene_StaffTreeItemTests, Init_LinkedStaff)
{
    // [GIVEN] An instrument with a tablature staff linked to its standard staff
    Part* part = addPart(2);
    part->staff(0)->staffType(Fraction(0, 1))->setLongName(u"Standard");
    part->staff(1)->staffType(Fraction(0, 1))->setLongName(u"Tablature");
    part->staff(1)->linkTo(part->staff(0));

    // [WHEN] Their rows are built
    const StaffTreeItem* standardItem = buildStaffTreeItem(part->staff(0));
    const StaffTreeItem* tablatureItem = buildStaffTreeItem(part->staff(1));

    // [THEN] Only the linked copy is marked
    EXPECT_EQ(standardItem->title(), QString("Standard"));
    EXPECT_EQ(tablatureItem->title(), QString("[LINK] Tablature"));
}

TEST_F(InstrumentsScene_StaffTreeItemTests, Init_HiddenStaff)
{
    // [GIVEN] An instrument with a hidden second staff
    Part* part = addPart(2);
    part->staff(1)->setVisible(false);

    // [THEN] Building the rows doesn't change the score
    EXPECT_CALL(*m_parts, setStaffVisible(_, _)).Times(0);

    // [WHEN] Their rows are built
    const StaffTreeItem* firstItem = buildStaffTreeItem(part->staff(0));
    const StaffTreeItem* secondItem = buildStaffTreeItem(part->staff(1));

    // [THEN] Only the second row is hidden
    EXPECT_TRUE(firstItem->isVisible());
    EXPECT_FALSE(secondItem->isVisible());
}

TEST_F(InstrumentsScene_StaffTreeItemTests, Init_HiddenPart)
{
    // [GIVEN] A hidden instrument
    Part* part = addPart();
    part->setShow(false);

    // [THEN] Building the row doesn't change the score
    EXPECT_CALL(*m_parts, setStaffVisible(_, _)).Times(0);

    // [WHEN] Its staff's row is built
    const StaffTreeItem* item = buildStaffTreeItem(part->staff(0));

    // [THEN] The row is hidden
    EXPECT_FALSE(item->isVisible());
}

TEST_F(InstrumentsScene_StaffTreeItemTests, Init_StaffMissingFromPartScore)
{
    // [GIVEN] A staff of the main score that isn't in the part score
    Staff* staff = addPart()->staff(0);
    staff->staffType(Fraction(0, 1))->setLongName(u"Percussion");
    removeFromPartScore(staff);

    // [WHEN] Its row is built
    const StaffTreeItem* item = buildStaffTreeItem(staff);

    // [THEN] The row shows the main score's staff, hidden
    EXPECT_EQ(item->id(), staff->id());
    EXPECT_EQ(item->title(), QString("Percussion"));
    EXPECT_FALSE(item->isVisible());
}

TEST_F(InstrumentsScene_StaffTreeItemTests, SetVisible_HidesStaff)
{
    // [GIVEN] An instrument with two visible staves
    const Part* part = addPart(2);
    PartTreeItem* partItem = buildPartTreeItem(part);

    // [THEN] Only the first staff is hidden on the score
    EXPECT_CALL(*m_parts, setStaffVisible(part->staff(0)->id(), false));
    EXPECT_CALL(*m_parts, setPartVisible(_, _)).Times(0);

    // [WHEN] The first staff's row is hidden
    partItem->childAtRow(0)->setIsVisible(false);

    // [THEN] The instrument and the other staff stay visible
    EXPECT_TRUE(partItem->isVisible());
    EXPECT_TRUE(partItem->childAtRow(1)->isVisible());
}

TEST_F(InstrumentsScene_StaffTreeItemTests, SetVisible_ShowsHiddenPart)
{
    // [GIVEN] A hidden instrument with two staves
    Part* part = addPart(2);
    part->setShow(false);
    PartTreeItem* partItem = buildPartTreeItem(part);

    // [THEN] The instrument and the first staff are shown on the score
    EXPECT_CALL(*m_parts, setPartVisible(part->id(), true));
    EXPECT_CALL(*m_parts, setStaffVisible(part->staff(0)->id(), true));
    EXPECT_CALL(*m_parts, setStaffVisible(part->staff(1)->id(), _)).Times(0);

    // [WHEN] The first staff's row is shown
    partItem->childAtRow(0)->setIsVisible(true);

    // [THEN] The instrument row is shown, the other staff row stays hidden
    EXPECT_TRUE(partItem->isVisible());
    EXPECT_FALSE(partItem->childAtRow(1)->isVisible());
}
