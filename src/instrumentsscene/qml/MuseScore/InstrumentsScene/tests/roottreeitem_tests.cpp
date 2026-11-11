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

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/staff.h"

#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/roottreeitem.h"
#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/parttreeitem.h"
#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/systemobjectslayertreeitem.h"

#include "notation/tests/mocks/masternotationmock.h"
#include "notation/tests/mocks/notationmock.h"
#include "notation/tests/mocks/notationpartsmock.h"

using namespace ::testing;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;

using Moves = std::vector<INotationParts::SystemObjectsMove>;

namespace mu::instrumentsscene {
class InstrumentsScene_RootTreeItemTests : public ::testing::Test
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

        ON_CALL(*m_parts, staff(_)).WillByDefault(Invoke([this](const ID& staffId) {
            return m_score->staffById(staffId);
        }));

        ON_CALL(*m_parts, partList()).WillByDefault(Invoke([this]() {
            const std::vector<const Part*> parts(m_score->parts().begin(), m_score->parts().end());
            return async::NotifyList<const Part*>(parts, nullptr);
        }));
    }

    void TearDown() override
    {
        delete m_root;
        delete m_score;
    }

    const Part* addPart(const String& name, size_t staffCount = 1)
    {
        Part* part = new Part(m_score);
        part->setLongNameAll(name);
        m_score->appendPart(part);

        for (size_t i = 0; i < staffCount; ++i) {
            m_score->appendStaff(Factory::createStaff(part));
        }

        return part;
    }

    void addSystemObjects(Staff* staff, bool belowBottomStaff = false)
    {
        m_score->addSystemObjectStaff(staff);
        if (belowBottomStaff) {
            m_score->style().set(Sid::systemObjectsBelowBottomStaff, true);
        }
    }

    //! Shown as a shared-staves group row
    void markAsSharedPart(const Part* part)
    {
        m_sharedParts.push_back(part);
    }

    //! Same rows as LayoutPanelTreeModel::load() builds; part scores have no system markings rows
    void buildTree(bool withSystemObjects = true)
    {
        m_root = new RootTreeItem(m_masterNotation, m_notation);

        std::vector<Staff*> systemObjectStaves;
        if (withSystemObjects) {
            systemObjectStaves = m_score->systemObjectStavesWithTopStaff();
        }

        for (Part* part : m_score->parts()) {
            for (Staff* staff : part->staves()) {
                if (!staff->hasSystemObjectsBelowBottomStaff() && muse::contains(systemObjectStaves, staff)) {
                    appendSystemObjectsRow(staff);
                }
            }

            const LayoutPanelItemType::ItemType type = muse::contains(m_sharedParts, static_cast<const Part*>(part))
                                                       ? LayoutPanelItemType::SHARED_PART : LayoutPanelItemType::PART;
            PartTreeItem* partItem = new PartTreeItem(m_masterNotation, m_notation, m_root, type);
            partItem->init(part);
            m_root->appendChild(partItem);

            for (Staff* staff : part->staves()) {
                if (staff->hasSystemObjectsBelowBottomStaff() && muse::contains(systemObjectStaves, staff)) {
                    appendSystemObjectsRow(staff);
                }
            }
        }
    }

    //! Same as the up/down buttons: LayoutPanelTreeModel::moveRows()
    void moveRows(int row, int count, int toRow)
    {
        const int destinationRow = row + count - 1 > toRow ? toRow : toRow + 1;
        m_root->moveChildren(row, count, m_root, destinationRow, true);
    }

    void moveRow(int row, int toRow)
    {
        moveRows(row, 1, toRow);
    }

    //! Same as LayoutPanelTreeModel during a drag: moveRows() for each one-row step, endActiveDrag() on drop
    void dragRow(int row, int toRow)
    {
        const int step = toRow > row ? 1 : -1;
        for (int r = row; r != toRow; r += step) {
            const int destinationRow = step > 0 ? r + 2 : r - 1;
            m_root->moveChildren(r, 1, m_root, destinationRow, false);
        }

        m_root->applyToScore();
    }

    //! E.g. "[A:0] A B [C:0] C": instrument titles, and "[instrument:staff index]" for system markings rows
    std::string childNames() const
    {
        StringList names;
        for (const AbstractLayoutPanelTreeItem* item : m_root->childItems()) {
            if (item->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
                const Staff* staff = static_cast<const SystemObjectsLayerTreeItem*>(item)->staff();
                names << u"[" + staff->part()->partName() + u":" + String::number(staff->rstaff()) + u"]";
            } else {
                names << String::fromQString(item->title());
            }
        }

        return names.join(u" ").toStdString();
    }

    MasterScore* m_score = nullptr;
    RootTreeItem* m_root = nullptr;
    std::shared_ptr<NotationPartsMock> m_parts;

private:
    void appendSystemObjectsRow(Staff* staff)
    {
        SystemObjectsLayerTreeItem* item = new SystemObjectsLayerTreeItem(m_masterNotation, m_notation, m_root);
        item->init(staff, {});
        m_root->appendChild(item);
    }

    std::vector<const Part*> m_sharedParts;

    std::shared_ptr<NotationMock> m_notation;
    std::shared_ptr<MasterNotationMock> m_masterNotation;
};
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_InstrumentDown)
{
    // [GIVEN] Three instruments, system markings only on the top staff
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    buildTree(); // [A:0] A B C

    // [THEN] A goes between B and C
    EXPECT_CALL(*m_parts, reorderParts(IDList { b->id(), a->id(), c->id() }, Moves {}));

    // [WHEN] A is moved down by one row
    moveRow(1, 2);

    // [THEN] A is below B
    EXPECT_EQ(m_root->childCount(), 4);
    EXPECT_EQ(childNames(), "[A:0] B A C");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_InstrumentUp)
{
    // [GIVEN] Three instruments, system markings only on the top staff
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    buildTree(); // [A:0] A B C

    // [THEN] C goes before B
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), c->id(), b->id() }, Moves {}));

    // [WHEN] C is moved up by one row
    moveRow(3, 2);

    // [THEN] C is above B
    EXPECT_EQ(m_root->childCount(), 4);
    EXPECT_EQ(childNames(), "[A:0] A C B");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_InstrumentPastMarkings)
{
    // [GIVEN] Three instruments, system markings above A and C
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    addSystemObjects(c->staff(0));
    buildTree(); // [A:0] A B [C:0] C

    // [THEN] The instruments keep their order, and the markings move onto B
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), b->id(), c->id() },
                                       Moves { { c->staff(0)->id(), b->staff(0)->id() } }));

    // [WHEN] B is moved down past the markings row above C
    moveRow(2, 3);

    // [THEN] The markings row is above B, and shows B's staff
    EXPECT_EQ(m_root->childCount(), 5);
    EXPECT_EQ(childNames(), "[A:0] A [B:0] B C");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_MarkingsUp)
{
    // [GIVEN] Three instruments, system markings above A and C
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    addSystemObjects(c->staff(0));
    buildTree(); // [A:0] A B [C:0] C

    // [THEN] The markings move onto B
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), b->id(), c->id() },
                                       Moves { { c->staff(0)->id(), b->staff(0)->id() } }));

    // [WHEN] The markings row above C is moved up past B
    moveRow(3, 2);

    // [THEN] The markings row is above B, and shows B's staff
    EXPECT_EQ(m_root->childCount(), 5);
    EXPECT_EQ(childNames(), "[A:0] A [B:0] B C");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_MarkingsToEnd)
{
    // [GIVEN] Three instruments, C with two staves, system markings above A and C
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C", 2);
    addSystemObjects(c->staff(0));
    buildTree(); // [A:0] A B [C:0] C

    // [THEN] The markings move below the bottom staff
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), b->id(), c->id() },
                                       Moves { { c->staff(0)->id(), c->staff(1)->id(), true } }));

    // [WHEN] The markings row above C is moved to the end
    moveRow(3, 4);

    // [THEN] The markings row is last, and shows C's bottom staff
    EXPECT_EQ(m_root->childCount(), 5);
    EXPECT_EQ(childNames(), "[A:0] A B C [C:1]");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_MarkingsUpFromBottom)
{
    // [GIVEN] Three instruments, C with two staves, system markings on the top staff and below the bottom staff
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C", 2);
    addSystemObjects(c->staff(1), true);
    buildTree(); // [A:0] A B C [C:1]

    // [THEN] The markings move onto C's top staff, above it
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), b->id(), c->id() },
                                       Moves { { c->staff(1)->id(), c->staff(0)->id() } }));

    // [WHEN] The markings row below C is moved up past C
    moveRow(4, 3);

    // [THEN] The markings row is above C, and shows C's top staff
    EXPECT_EQ(m_root->childCount(), 5);
    EXPECT_EQ(childNames(), "[A:0] A B [C:0] C");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_LastInstrumentPastMarkings)
{
    // [GIVEN] Three instruments, system markings above A and C
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    addSystemObjects(c->staff(0));
    buildTree(); // [A:0] A B [C:0] C

    // [THEN] The markings move below the bottom staff
    EXPECT_CALL(*m_parts, removeSystemObjects(_)).Times(0);
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), b->id(), c->id() },
                                       Moves { { c->staff(0)->id(), c->staff(0)->id(), true } }));

    // [WHEN] C is moved up past its markings row
    moveRow(4, 3);

    // [THEN] The markings row is last
    EXPECT_EQ(m_root->childCount(), 5);
    EXPECT_EQ(childNames(), "[A:0] A B C [C:0]");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_LastInstrumentWithTwoStavesPastMarkings)
{
    // [GIVEN] Three instruments, C with two staves, system markings above A and C
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C", 2);
    addSystemObjects(c->staff(0));
    buildTree(); // [A:0] A B [C:0] C

    // [THEN] The markings move below the bottom staff
    EXPECT_CALL(*m_parts, removeSystemObjects(_)).Times(0);
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), b->id(), c->id() },
                                       Moves { { c->staff(0)->id(), c->staff(1)->id(), true } }));

    // [WHEN] C is moved up past its markings row
    moveRow(4, 3);

    // [THEN] The markings row is last, and shows C's bottom staff
    EXPECT_EQ(m_root->childCount(), 5);
    EXPECT_EQ(childNames(), "[A:0] A B C [C:1]");
}

TEST_F(InstrumentsScene_RootTreeItemTests, ApplyPanelOrderToScore_AfterDrag)
{
    // [GIVEN] Three instruments, system markings above A and B
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    addSystemObjects(b->staff(0));
    buildTree(); // [A:0] A [B:0] B C

    // [THEN] C goes before B, and the markings stay on B
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), c->id(), b->id() },
                                       Moves { { b->staff(0)->id(), b->staff(0)->id() } }));

    // [WHEN] C is dragged up past B and the markings row above B
    dragRow(4, 2);

    // [THEN] C is above the markings row and B
    EXPECT_EQ(m_root->childCount(), 5);
    EXPECT_EQ(childNames(), "[A:0] A C [B:0] B");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_TwoInstrumentsUp)
{
    // [GIVEN] Four instruments, system markings only on the top staff
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    const Part* d = addPart(u"D");
    buildTree(); // [A:0] A B C D

    // [THEN] B and C go before A
    EXPECT_CALL(*m_parts, reorderParts(IDList { b->id(), c->id(), a->id(), d->id() }, Moves {}));

    // [WHEN] B and C are moved up by one row
    moveRows(2, 2, 1);

    // [THEN] B and C are above A
    EXPECT_EQ(m_root->childCount(), 5);
    EXPECT_EQ(childNames(), "[A:0] B C A D");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_TwoInstrumentsDown)
{
    // [GIVEN] Four instruments, system markings only on the top staff
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    const Part* d = addPart(u"D");
    buildTree(); // [A:0] A B C D

    // [THEN] A and B go between C and D
    EXPECT_CALL(*m_parts, reorderParts(IDList { c->id(), a->id(), b->id(), d->id() }, Moves {}));

    // [WHEN] A and B are moved down by one row
    moveRows(1, 2, 3);

    // [THEN] A and B are below C
    EXPECT_EQ(m_root->childCount(), 5);
    EXPECT_EQ(childNames(), "[A:0] C A B D");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_InstrumentToEnd)
{
    // [GIVEN] Three instruments, system markings only on the top staff
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    buildTree(); // [A:0] A B C

    // [THEN] B goes after C
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), c->id(), b->id() }, Moves {}));

    // [WHEN] B is moved down to the end
    moveRow(2, 3);

    // [THEN] B is last
    EXPECT_EQ(m_root->childCount(), 4);
    EXPECT_EQ(childNames(), "[A:0] A C B");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_MarkingsDown)
{
    // [GIVEN] Three instruments, system markings above A and B
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    addSystemObjects(b->staff(0));
    buildTree(); // [A:0] A [B:0] B C

    // [THEN] The markings move onto C
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), b->id(), c->id() },
                                       Moves { { b->staff(0)->id(), c->staff(0)->id() } }));

    // [WHEN] The markings row above B is moved down past B
    moveRow(2, 3);

    // [THEN] The markings row is above C, and shows C's staff
    EXPECT_EQ(m_root->childCount(), 5);
    EXPECT_EQ(childNames(), "[A:0] A B [C:0] C");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_MarkingsOntoMarkings)
{
    // [GIVEN] Three instruments, system markings above each of them
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    addSystemObjects(b->staff(0));
    addSystemObjects(c->staff(0));
    buildTree(); // [A:0] A [B:0] B [C:0] C

    // [THEN] Both markings rows ask for C's staff
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), b->id(), c->id() },
                                       Moves { { b->staff(0)->id(), c->staff(0)->id() }, { c->staff(0)->id(), c->staff(0)->id() } }));

    // [WHEN] The markings row above B is moved down past B
    moveRow(2, 3);

    // [THEN] Both markings rows are above C, and show C's staff
    EXPECT_EQ(m_root->childCount(), 6);
    EXPECT_EQ(childNames(), "[A:0] A B [C:0] [C:0] C");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_InstrumentPastBottomMarkings)
{
    // [GIVEN] Three instruments, C with two staves, system markings on the top staff and below the bottom staff
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C", 2);
    addSystemObjects(c->staff(1), true);
    buildTree(); // [A:0] A B C [C:1]

    // [THEN] The markings move onto C's top staff, above it
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), b->id(), c->id() },
                                       Moves { { c->staff(1)->id(), c->staff(0)->id() } }));

    // [WHEN] C is moved down past the markings row below it
    moveRow(3, 4);

    // [THEN] The markings row is above C, and shows C's top staff
    EXPECT_EQ(m_root->childCount(), 5);
    EXPECT_EQ(childNames(), "[A:0] A B [C:0] C");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_MarkingsOntoSharedPart)
{
    // [GIVEN] An instrument, a shared-staves group and an instrument, system markings above A and C
    const Part* a = addPart(u"A");
    const Part* s = addPart(u"S");
    const Part* c = addPart(u"C");
    markAsSharedPart(s);
    addSystemObjects(c->staff(0));
    buildTree(); // [A:0] A S [C:0] C

    // [THEN] The markings move onto S
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), s->id(), c->id() },
                                       Moves { { c->staff(0)->id(), s->staff(0)->id() } }));

    // [WHEN] The markings row above C is moved up past S
    moveRow(3, 2);

    // [THEN] The markings row is above S, and shows S's staff
    EXPECT_EQ(m_root->childCount(), 5);
    EXPECT_EQ(childNames(), "[A:0] A [S:0] S C");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_InstrumentPastSharedPart)
{
    // [GIVEN] An instrument, a shared-staves group and an instrument
    const Part* a = addPart(u"A");
    const Part* s = addPart(u"S");
    const Part* c = addPart(u"C");
    markAsSharedPart(s);
    buildTree(); // [A:0] A S C

    // [THEN] C goes before S
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), c->id(), s->id() }, Moves {}));

    // [WHEN] C is moved up past S
    moveRow(3, 2);

    // [THEN] C is above S
    EXPECT_EQ(m_root->childCount(), 4);
    EXPECT_EQ(childNames(), "[A:0] A C S");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_PartScore_InstrumentPastMissing)
{
    // [GIVEN] A part score with A and C, but not B
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    buildTree(false); // A B C
    ON_CALL(*m_parts, partExists(b->id())).WillByDefault(Return(false));

    // [THEN] A and C keep their order
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), c->id() }, Moves {}));

    // [WHEN] C is moved up past B
    moveRow(2, 1);

    // [THEN] C is above B
    EXPECT_EQ(m_root->childCount(), 3);
    EXPECT_EQ(childNames(), "A C B");
}

TEST_F(InstrumentsScene_RootTreeItemTests, MoveChildren_PartScore_MissingInstrument)
{
    // [GIVEN] A part score with A and C, but not B
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    buildTree(false); // A B C
    ON_CALL(*m_parts, partExists(b->id())).WillByDefault(Return(false));

    // [THEN] Only A and C are sent, in their old order
    EXPECT_CALL(*m_parts, reorderParts(IDList { a->id(), c->id() }, Moves {}));

    // [WHEN] B is moved up past A
    moveRow(1, 0);

    // [THEN] B is above A
    EXPECT_EQ(m_root->childCount(), 3);
    EXPECT_EQ(childNames(), "B A C");
}

TEST_F(InstrumentsScene_RootTreeItemTests, RemoveChildren_Instrument)
{
    // [GIVEN] Three instruments, system markings only on the top staff
    addPart(u"A");
    const Part* b = addPart(u"B");
    addPart(u"C");
    buildTree(); // [A:0] A B C

    // [THEN] B is removed from the score
    EXPECT_CALL(*m_parts, removeParts(IDList { b->id() }));

    // [WHEN] B is removed
    m_root->removeChildren(2, 1, true);

    // [THEN] B's row is gone
    EXPECT_EQ(m_root->childCount(), 3);
    EXPECT_EQ(childNames(), "[A:0] A C");
}

TEST_F(InstrumentsScene_RootTreeItemTests, RemoveChildren_Markings)
{
    // [GIVEN] Three instruments, system markings above A and C
    addPart(u"A");
    addPart(u"B");
    const Part* c = addPart(u"C");
    addSystemObjects(c->staff(0));
    buildTree(); // [A:0] A B [C:0] C

    // [THEN] C's markings are removed from the score
    EXPECT_CALL(*m_parts, removeSystemObjects(IDList { c->staff(0)->id() }));

    // [WHEN] The markings row above C is removed
    m_root->removeChildren(3, 1, true);

    // [THEN] The markings row is gone
    EXPECT_EQ(m_root->childCount(), 4);
    EXPECT_EQ(childNames(), "[A:0] A B C");
}

TEST_F(InstrumentsScene_RootTreeItemTests, RemoveChildren_MarkingsAndInstrument)
{
    // [GIVEN] Three instruments, system markings above A and C
    addPart(u"A");
    addPart(u"B");
    const Part* c = addPart(u"C");
    addSystemObjects(c->staff(0));
    buildTree(); // [A:0] A B [C:0] C

    // [THEN] C and its markings are removed from the score
    EXPECT_CALL(*m_parts, removeParts(IDList { c->id() }));
    EXPECT_CALL(*m_parts, removeSystemObjects(IDList { c->staff(0)->id() }));

    // [WHEN] The markings row above C and C are removed together
    m_root->removeChildren(3, 2, true);

    // [THEN] Both rows are gone
    EXPECT_EQ(m_root->childCount(), 3);
    EXPECT_EQ(childNames(), "[A:0] A B");
}

TEST_F(InstrumentsScene_RootTreeItemTests, RemoveChildren_WithoutDeleting)
{
    // [GIVEN] Three instruments, system markings above A and C
    addPart(u"A");
    addPart(u"B");
    const Part* c = addPart(u"C");
    addSystemObjects(c->staff(0));
    buildTree(); // [A:0] A B [C:0] C

    // [THEN] Nothing is removed from the score
    EXPECT_CALL(*m_parts, removeParts(_)).Times(0);
    EXPECT_CALL(*m_parts, removeSystemObjects(_)).Times(0);

    // [WHEN] The markings row above C and C are removed from the panel only
    m_root->removeChildren(3, 2, false);

    // [THEN] The rows are gone
    EXPECT_EQ(m_root->childCount(), 3);
    EXPECT_EQ(childNames(), "[A:0] A B");
}
