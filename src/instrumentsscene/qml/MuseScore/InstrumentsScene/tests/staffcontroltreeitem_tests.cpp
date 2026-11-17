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
#include "engraving/dom/instrument.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"

#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/staffcontroltreeitem.h"

#include "notation/tests/mocks/masternotationmock.h"
#include "notation/tests/mocks/notationmock.h"
#include "notation/tests/mocks/notationpartsmock.h"

using namespace ::testing;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;

namespace mu::instrumentsscene {
class InstrumentsScene_StaffControlTreeItemTests : public ::testing::Test
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

        ON_CALL(*m_masterParts, part(_)).WillByDefault(Invoke([this](const ID& partId) {
            return m_score->partById(partId);
        }));

        m_item = new StaffControlTreeItem(m_masterNotation, m_notation, nullptr);
    }

    void TearDown() override
    {
        delete m_item;
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

    MasterScore* m_score = nullptr;
    StaffControlTreeItem* m_item = nullptr;

    //! The parts of the score the panel shows, and of the main score
    std::shared_ptr<NotationPartsMock> m_parts;
    std::shared_ptr<NotationPartsMock> m_masterParts;

private:
    std::shared_ptr<NotationMock> m_notation;
    std::shared_ptr<MasterNotationMock> m_masterNotation;
};
}

TEST_F(InstrumentsScene_StaffControlTreeItemTests, Init)
{
    // [GIVEN] An instrument
    const Part* part = addPart();

    // [WHEN] Its "add staff" row is built
    m_item->init(part->id());

    // [THEN] The row is an "Add staff" button, not a staff
    EXPECT_EQ(m_item->type(), LayoutPanelItemType::CONTROL_ADD_STAFF);
    EXPECT_EQ(m_item->title(), QString("Add staff"));
    EXPECT_EQ(m_item->id(), ID());

    // [THEN] It has no settings, and can't be selected or removed
    EXPECT_FALSE(m_item->settingsAvailable());
    EXPECT_FALSE(m_item->isSelectable());
    EXPECT_FALSE(m_item->isRemovable());
}

TEST_F(InstrumentsScene_StaffControlTreeItemTests, AppendNewItem)
{
    // [GIVEN] A piano with only its treble staff
    Part* part = addPart();
    part->instrument()->setClefType(0, ClefTypeList(ClefType::G));
    part->instrument()->setClefType(1, ClefTypeList(ClefType::F));
    m_item->init(part->id());

    // [THEN] A staff with the instrument's bass clef is added to the piano, on the main score
    EXPECT_CALL(*m_parts, appendStaff(_, _)).Times(0);
    EXPECT_CALL(*m_masterParts, appendStaff(NotNull(), part->id())).WillOnce(Invoke([part](Staff* staff, const ID&) {
        EXPECT_EQ(staff->part(), part);
        EXPECT_EQ(staff->defaultClefType(), ClefTypeList(ClefType::F));
        delete staff;

        return true;
    }));

    // [WHEN] "Add staff" is clicked
    m_item->appendNewItem();
}

TEST_F(InstrumentsScene_StaffControlTreeItemTests, AppendNewItem_PartRemoved)
{
    // [GIVEN] An "add staff" row for an instrument that's no longer in the score
    m_item->init(ID(uint64_t(12345)));

    // [THEN] No staff is added
    EXPECT_CALL(*m_masterParts, appendStaff(_, _)).Times(0);

    // [WHEN] "Add staff" is clicked
    m_item->appendNewItem();
}
