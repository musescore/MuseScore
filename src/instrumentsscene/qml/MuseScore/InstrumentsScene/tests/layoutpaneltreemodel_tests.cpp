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

#include <QItemSelectionModel>

#include "async/changednotify.h"
#include "async/notifylist.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/sharedpart.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/text.h"

#include "instrumentsscene/instrumentscommands.h"
#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/layoutpaneltreemodel.h"
#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/systemobjectslayertreeitem.h"

#include "interactive/tests/mocks/interactivemock.h"
#include "notation/tests/mocks/excerptnotationmock.h"
#include "notation/tests/mocks/masternotationmock.h"
#include "notation/tests/mocks/notationelementsmock.h"
#include "notation/tests/mocks/notationinteractionmock.h"
#include "notation/tests/mocks/notationmock.h"
#include "notation/tests/mocks/notationpartsmock.h"
#include "notation/tests/mocks/notationselectionmock.h"
#include "notation/tests/mocks/notationstylemock.h"
#include "notation/tests/mocks/notationundostackmock.h"
#include "project/tests/mocks/projectsessionmock.h"
#include "rcommand/tests/mocks/commanddispatchermock.h"
#include "shortcuts_v2/tests/mocks/commandshortcutsregistermock.h"

using namespace ::testing;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;

using ItemType = LayoutPanelItemType::ItemType;
using ItemTypes = std::vector<ItemType>;

namespace mu::instrumentsscene {
class InstrumentsScene_LayoutPanelTreeModelTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_score = compat::ScoreAccess::createMasterScore(nullptr);

        m_session = std::make_shared<NiceMock<mu::project::ProjectSessionMock> >();
        m_masterNotation = std::make_shared<NiceMock<MasterNotationMock> >();
        m_notation = std::make_shared<NiceMock<NotationMock> >();
        m_parts = std::make_shared<NiceMock<NotationPartsMock> >();
        m_masterParts = std::make_shared<NiceMock<NotationPartsMock> >();
        m_interaction = std::make_shared<NiceMock<NotationInteractionMock> >();
        m_selection = std::make_shared<NiceMock<NotationSelectionMock> >();
        m_undoStack = std::make_shared<NiceMock<NotationUndoStackMock> >();
        m_style = std::make_shared<NiceMock<NotationStyleMock> >();
        m_elements = std::make_shared<NiceMock<NotationElementsMock> >();
        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        m_dispatcher = std::make_shared<NiceMock<muse::rcommand::CommandDispatcherMock> >();
        m_shortcuts = std::make_shared<NiceMock<muse::shortcuts::CommandShortcutsRegisterMock> >();

        ON_CALL(*m_session, masterNotation()).WillByDefault(Return(m_masterNotation));
        ON_CALL(*m_session, masterNotationChanged()).WillByDefault(Return(m_masterNotationChanged));
        ON_CALL(*m_session, currentNotation()).WillByDefault(Return(m_notation));
        ON_CALL(*m_session, currentNotationChanged()).WillByDefault(Return(m_currentNotationChanged));

        ON_CALL(*m_masterNotation, notation()).WillByDefault(Return(m_notation));
        ON_CALL(*m_masterNotation, parts()).WillByDefault(Return(m_masterParts));
        ON_CALL(*m_masterNotation, excerpts()).WillByDefault(ReturnRef(m_excerpts));

        ON_CALL(*m_notation, isMaster()).WillByDefault(Return(true));
        ON_CALL(*m_notation, name()).WillByDefault(Return(QString("Score")));
        ON_CALL(*m_notation, parts()).WillByDefault(Return(m_parts));
        ON_CALL(*m_notation, interaction()).WillByDefault(Return(m_interaction));
        ON_CALL(*m_notation, undoStack()).WillByDefault(Return(m_undoStack));
        ON_CALL(*m_notation, style()).WillByDefault(Return(m_style));
        ON_CALL(*m_notation, elements()).WillByDefault(Return(m_elements));

        ON_CALL(*m_elements, msScore()).WillByDefault(Return(m_score));

        ON_CALL(*m_interaction, selection()).WillByDefault(Return(m_selection));
        ON_CALL(*m_interaction, selectionChanged()).WillByDefault(Return(m_selectionChanged));
        ON_CALL(*m_selection, elements()).WillByDefault(ReturnRef(m_selectedElements));

        ON_CALL(*m_undoStack, changesChannel()).WillByDefault(Return(m_scoreChanges));

        ON_CALL(*m_style, styleValue(_)).WillByDefault(Invoke([this](const StyleId& styleId) -> const PropertyValue& {
            return m_score->style().styleV(styleId);
        }));
        ON_CALL(*m_style, setStyleValue(_, _)).WillByDefault(Invoke([this](const StyleId& styleId, const PropertyValue& value) {
            m_score->style().set(styleId, value);
        }));

        setUpScoreParts(*m_parts);
        setUpScoreParts(*m_masterParts);

        ON_CALL(*m_parts, systemObjectStaves()).WillByDefault(ReturnRef(m_systemObjectStaves));
        ON_CALL(*m_parts, systemObjectStavesChanged()).WillByDefault(Return(m_systemObjectStavesChanged));
        ON_CALL(*m_parts, sharedPartsChanged()).WillByDefault(Return(m_sharedPartsChanged));

        ON_CALL(*m_shortcuts, shortcutsChanged()).WillByDefault(Return(m_shortcutsChanged));
    }

    void TearDown() override
    {
        delete m_model;

        for (EngravingItem* item : m_textElements) {
            delete item;
        }

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

    //! Shown as a shared-staves group row
    const Part* addSharedPart(const String& name)
    {
        SharedPart* part = new SharedPart(m_score);
        part->setLongNameAll(name);
        m_score->appendPart(part);
        m_score->appendStaff(Factory::createStaff(part));

        return part;
    }

    void addSystemObjects(Staff* staff, bool belowBottomStaff = false)
    {
        m_score->addSystemObjectStaff(staff);
        if (belowBottomStaff) {
            m_score->style().set(Sid::systemObjectsBelowBottomStaff, true);
        }

        syncSystemObjectStaves();
    }

    void removeSystemObjects(Staff* staff)
    {
        m_score->removeSystemObjectStaff(staff);
        syncSystemObjectStaves();
    }

    //! A text on the staff, or without a staff for nullptr
    EngravingItem* addTextElement(const Staff* staff)
    {
        Text* text = Factory::createText(m_score->dummy(), TextStyleType::DEFAULT, false);
        text->setTrack(staff ? staff->idx() * VOICES : muse::nidx);
        m_textElements.push_back(text);

        return text;
    }

    void addExcerpt()
    {
        m_excerpts.push_back(std::make_shared<NiceMock<ExcerptNotationMock> >());
    }

    //! Same as QML: classBegin() runs init(), which loads the current notation
    void createModel()
    {
        syncSystemObjectStaves();

        m_model = new LayoutPanelTreeModel();
        m_model->context.set(m_session);
        m_model->interactive.set(m_interactive);
        m_model->commandDispatcher.set(m_dispatcher);
        m_model->commandShortcutsRegister.set(m_shortcuts);

        static_cast<QQmlParserStatus*>(m_model)->classBegin();
    }

    QModelIndex row(int r) const
    {
        return m_model->index(r, 0, QModelIndex());
    }

    QModelIndex childRow(int parentRow, int r) const
    {
        return m_model->index(r, 0, row(parentRow));
    }

    AbstractLayoutPanelTreeItem* item(int r) const
    {
        return m_model->modelIndexToItem(row(r));
    }

    AbstractLayoutPanelTreeItem* childItem(int parentRow, int r) const
    {
        return m_model->modelIndexToItem(childRow(parentRow, r));
    }

    int rowCount() const
    {
        return m_model->rowCount(QModelIndex());
    }

    //! Same as Shift/Ctrl+click in the panel
    void selectRows(const QModelIndexList& indexes)
    {
        QItemSelection selection;
        selection.reserve(indexes.size());

        for (const QModelIndex& index : indexes) {
            selection.select(index, index);
        }

        m_model->findChild<QItemSelectionModel*>()->select(selection, QItemSelectionModel::ClearAndSelect);
    }

    bool moveRootRows(int sourceRow, int count, int destinationRow)
    {
        return m_model->moveRows(QModelIndex(), sourceRow, count, QModelIndex(), destinationRow);
    }

    template<typename Signal>
    std::shared_ptr<int> countEmits(Signal signal)
    {
        std::shared_ptr<int> count = std::make_shared<int>(0);
        QObject::connect(m_model, signal, m_model, [count]() {
            ++(*count);
        });

        return count;
    }

    //! E.g. "[A:0] A B [C:0] C": instrument titles, and "[instrument:staff index]" for system markings rows
    std::string rowNames() const
    {
        StringList names;
        names.reserve(rowCount());

        for (int r = 0; r < rowCount(); ++r) {
            const AbstractLayoutPanelTreeItem* rowItem = item(r);
            if (rowItem->type() == LayoutPanelItemType::SYSTEM_OBJECTS_LAYER) {
                const Staff* staff = static_cast<const SystemObjectsLayerTreeItem*>(rowItem)->staff();
                names << u"[" + staff->part()->partName() + u":" + String::number(staff->rstaff()) + u"]";
            } else {
                names << String::fromQString(rowItem->title());
            }
        }

        return names.join(u" ").toStdString();
    }

    ItemTypes childTypes(int parentRow) const
    {
        const QList<AbstractLayoutPanelTreeItem*>& children = item(parentRow)->childItems();

        ItemTypes types;
        types.reserve(children.size());

        for (const AbstractLayoutPanelTreeItem* child : children) {
            types.push_back(child->type());
        }

        return types;
    }

    MasterScore* m_score = nullptr;
    LayoutPanelTreeModel* m_model = nullptr;

    //! The parts of the score the panel shows, and of the main score
    std::shared_ptr<NotationPartsMock> m_parts;
    std::shared_ptr<NotationPartsMock> m_masterParts;

    std::shared_ptr<mu::project::ProjectSessionMock> m_session;
    std::shared_ptr<MasterNotationMock> m_masterNotation;
    std::shared_ptr<NotationMock> m_notation;
    std::shared_ptr<NotationInteractionMock> m_interaction;
    std::shared_ptr<NotationSelectionMock> m_selection;
    std::shared_ptr<NotationUndoStackMock> m_undoStack;
    std::shared_ptr<NotationStyleMock> m_style;
    std::shared_ptr<NotationElementsMock> m_elements;
    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<muse::rcommand::CommandDispatcherMock> m_dispatcher;
    std::shared_ptr<muse::shortcuts::CommandShortcutsRegisterMock> m_shortcuts;

    async::ChangedNotifier<const Part*> m_partsNotifier;
    std::map<ID, async::ChangedNotifier<const Staff*> > m_staffNotifiers;

    async::Notification m_masterNotationChanged;
    async::Notification m_currentNotationChanged;
    async::Notification m_selectionChanged;
    async::Notification m_systemObjectStavesChanged;
    async::Notification m_sharedPartsChanged;
    async::Notification m_shortcutsChanged;
    async::Channel<ScoreChanges> m_scoreChanges;

    std::vector<EngravingItem*> m_selectedElements;
    std::vector<Staff*> m_systemObjectStaves;
    ExcerptNotationList m_excerpts;
    Excerpt m_excerpt;

private:
    void setUpScoreParts(NotationPartsMock& parts)
    {
        ON_CALL(parts, partList()).WillByDefault(Invoke([this]() {
            const std::vector<const Part*> list(m_score->parts().begin(), m_score->parts().end());
            return async::NotifyList<const Part*>(list, m_partsNotifier.notify());
        }));

        ON_CALL(parts, staffList(_)).WillByDefault(Invoke([this](const ID& partId) {
            std::vector<const Staff*> staves;
            if (const Part* part = m_score->partById(partId)) {
                staves.assign(part->staves().begin(), part->staves().end());
            }

            return async::NotifyList<const Staff*>(staves, m_staffNotifiers[partId].notify());
        }));

        ON_CALL(parts, part(_)).WillByDefault(Invoke([this](const ID& partId) {
            return m_score->partById(partId);
        }));

        ON_CALL(parts, partExists(_)).WillByDefault(Invoke([this](const ID& partId) {
            return m_score->partById(partId) != nullptr;
        }));

        ON_CALL(parts, staff(_)).WillByDefault(Invoke([this](const ID& staffId) {
            return m_score->staffById(staffId);
        }));
    }

    void syncSystemObjectStaves()
    {
        m_systemObjectStaves = m_score->systemObjectStavesWithTopStaff();
    }

    std::vector<EngravingItem*> m_textElements;
};
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, Load_BuildsRowsInScoreOrder)
{
    // [GIVEN] A with two staves and B, system markings above A and B
    const Part* a = addPart(u"A", 2);
    const Part* b = addPart(u"B");
    addSystemObjects(b->staff(0));

    // [WHEN] The panel is loaded
    createModel();

    // [THEN] Each instrument has a markings row above it
    EXPECT_EQ(rowNames(), "[A:0] A [B:0] B");

    // [THEN] An instrument row has its staves, then the "add staff" row
    EXPECT_EQ(childTypes(1), (ItemTypes { ItemType::STAFF, ItemType::STAFF, ItemType::CONTROL_ADD_STAFF }));
    EXPECT_EQ(childItem(1, 0)->id(), a->staff(0)->id());
    EXPECT_EQ(childItem(1, 1)->id(), a->staff(1)->id());
    EXPECT_FALSE(m_model->isEmpty());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, Load_MarkingsBelowBottomStaff)
{
    // [GIVEN] A, and B with two staves, system markings on the top staff and below the bottom staff
    addPart(u"A");
    const Part* b = addPart(u"B", 2);
    addSystemObjects(b->staff(1), true);

    // [WHEN] The panel is loaded
    createModel();

    // [THEN] The markings row is below B
    EXPECT_EQ(rowNames(), "[A:0] A B [B:1]");
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, Load_SharedPart)
{
    // [GIVEN] An instrument and a shared-staves group
    addPart(u"A");
    const Part* s = addSharedPart(u"S");

    // [WHEN] The panel is loaded
    createModel();

    // [THEN] The group has its own row, without staff rows
    ASSERT_EQ(rowCount(), 3);
    EXPECT_EQ(item(2)->type(), ItemType::SHARED_PART);
    EXPECT_EQ(item(2)->id(), s->id());
    EXPECT_TRUE(item(2)->isEmpty());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, Load_PartScore)
{
    // [GIVEN] A part score with C, then A, and no B; system markings above B
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    addSystemObjects(b->staff(0));

    ON_CALL(*m_notation, isMaster()).WillByDefault(Return(false));
    ON_CALL(*m_parts, partList()).WillByDefault(Invoke([this, a, c]() {
        return async::NotifyList<const Part*>({ c, a }, m_partsNotifier.notify());
    }));

    // [WHEN] The panel is loaded
    createModel();

    // [THEN] The instruments follow the part score's order, B last, and there are no markings rows
    EXPECT_EQ(rowNames(), "C A B");
    EXPECT_FALSE(m_model->isMasterScore());
    EXPECT_FALSE(m_model->isAddingSystemMarkingsAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, OnNotationChanged_NoNotation)
{
    // [GIVEN] No score is open
    addPart(u"A");
    ON_CALL(*m_session, currentNotation()).WillByDefault(Return(INotationPtr()));

    // [WHEN] The panel is loaded
    createModel();

    // [THEN] The panel is empty and nothing can be added
    EXPECT_EQ(rowCount(), 0);
    EXPECT_TRUE(m_model->isEmpty());
    EXPECT_FALSE(m_model->isAddingAvailable());
    EXPECT_FALSE(m_model->isAddingSystemMarkingsAvailable());
    EXPECT_FALSE(m_model->isMasterScore());
    EXPECT_TRUE(m_model->scoreName().isEmpty());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, Load_ReadsScoreSettings)
{
    // [GIVEN] A score with stave sharing on and empty staves hidden
    addPart(u"A");
    m_score->style().set(Sid::enableStaveSharing, true);
    m_score->style().set(Sid::hideEmptyStaves, true);

    // [WHEN] The panel is loaded
    createModel();

    // [THEN] The panel shows the score's name and settings
    EXPECT_EQ(m_model->scoreName(), QString("Score"));
    EXPECT_TRUE(m_model->isMasterScore());
    EXPECT_TRUE(m_model->isAddingAvailable());
    EXPECT_TRUE(m_model->isStaveSharingEnabled());
    EXPECT_TRUE(m_model->isHideEmptyStavesEnabled());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, Index_ParentAndData)
{
    // [GIVEN] An instrument with one staff
    addPart(u"A");
    createModel(); // [A:0] A

    const QModelIndex partIndex = row(1);
    const QModelIndex staffIndex = childRow(1, 0);

    // [THEN] A staff row's parent is its instrument row, and instrument rows have no parent
    EXPECT_EQ(m_model->parent(staffIndex), partIndex);
    EXPECT_FALSE(m_model->parent(partIndex).isValid());
    EXPECT_EQ(m_model->rowCount(partIndex), 2);
    EXPECT_EQ(m_model->columnCount(QModelIndex()), 1);

    // [THEN] There are no rows past the end
    EXPECT_FALSE(m_model->index(2, 0, QModelIndex()).isValid());
    EXPECT_FALSE(m_model->index(2, 0, partIndex).isValid());

    // [THEN] Only the "item" role gives the row's item
    const int itemRole = m_model->roleNames().key("item");
    EXPECT_EQ(m_model->data(staffIndex, itemRole).value<AbstractLayoutPanelTreeItem*>(), m_model->modelIndexToItem(staffIndex));
    EXPECT_FALSE(m_model->data(staffIndex, Qt::DisplayRole).isValid());
    EXPECT_FALSE(m_model->data(QModelIndex(), itemRole).isValid());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, IsMovingUpAndDownAvailable_NothingSelected)
{
    // [GIVEN] Three instruments, nothing selected
    addPart(u"A");
    addPart(u"B");
    addPart(u"C");
    createModel(); // [A:0] A B C

    // [THEN] Nothing can be moved
    EXPECT_FALSE(m_model->isMovingUpAvailable());
    EXPECT_FALSE(m_model->isMovingDownAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, IsMovingUpAndDownAvailable_Instruments)
{
    // [GIVEN] Three instruments
    addPart(u"A");
    addPart(u"B");
    addPart(u"C");
    createModel(); // [A:0] A B C

    std::shared_ptr<int> downChanges = countEmits(&LayoutPanelTreeModel::isMovingDownAvailableChanged);

    // [WHEN] A is selected
    m_model->selectRow(row(1));

    // [THEN] It can only move down, since the top markings row stays first
    EXPECT_FALSE(m_model->isMovingUpAvailable());
    EXPECT_TRUE(m_model->isMovingDownAvailable());

    // [WHEN] B is selected
    m_model->selectRow(row(2));

    // [THEN] It can move both ways
    EXPECT_TRUE(m_model->isMovingUpAvailable());
    EXPECT_TRUE(m_model->isMovingDownAvailable());

    // [WHEN] C is selected
    m_model->selectRow(row(3));

    // [THEN] It can only move up
    EXPECT_TRUE(m_model->isMovingUpAvailable());
    EXPECT_FALSE(m_model->isMovingDownAvailable());

    // [THEN] "Move down" changed only twice
    EXPECT_EQ(*downChanges, 2);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, IsMovingUpAndDownAvailable_TopMarkings)
{
    // [GIVEN] Two instruments
    addPart(u"A");
    addPart(u"B");
    createModel(); // [A:0] A B

    // [WHEN] The top markings row is selected
    m_model->selectRow(row(0));

    // [THEN] It can't move
    EXPECT_FALSE(m_model->isMovingUpAvailable());
    EXPECT_FALSE(m_model->isMovingDownAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, IsMovingUpAndDownAvailable_Staves)
{
    // [GIVEN] An instrument with three staves
    addPart(u"A", 3);
    createModel(); // [A:0] A

    // [WHEN] The first staff is selected
    m_model->selectRow(childRow(1, 0));

    // [THEN] It can only move down
    EXPECT_FALSE(m_model->isMovingUpAvailable());
    EXPECT_TRUE(m_model->isMovingDownAvailable());

    // [WHEN] The last staff is selected
    m_model->selectRow(childRow(1, 2));

    // [THEN] It can't move below the "add staff" row
    EXPECT_TRUE(m_model->isMovingUpAvailable());
    EXPECT_FALSE(m_model->isMovingDownAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, IsMovingUpAndDownAvailable_AdjacentInstruments)
{
    // [GIVEN] Three instruments
    addPart(u"A");
    addPart(u"B");
    addPart(u"C");
    createModel(); // [A:0] A B C

    // [WHEN] B and C are selected
    selectRows({ row(2), row(3) });

    // [THEN] They can only move up
    EXPECT_TRUE(m_model->isMovingUpAvailable());
    EXPECT_FALSE(m_model->isMovingDownAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, IsMovingUpAndDownAvailable_MarkingsAndInstrument)
{
    // [GIVEN] Three instruments, system markings above A and B
    addPart(u"A");
    const Part* b = addPart(u"B");
    addPart(u"C");
    addSystemObjects(b->staff(0));
    createModel(); // [A:0] A [B:0] B C

    // [WHEN] The markings row above B and B are selected
    selectRows({ row(2), row(3) });

    // [THEN] Rows of different kinds can't move together
    EXPECT_FALSE(m_model->isMovingUpAvailable());
    EXPECT_FALSE(m_model->isMovingDownAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, IsRemovingAvailable_RootRows)
{
    // [GIVEN] Two instruments, system markings above A and B
    addPart(u"A");
    const Part* b = addPart(u"B");
    addSystemObjects(b->staff(0));
    createModel(); // [A:0] A [B:0] B

    // [THEN] Nothing selected, nothing to remove
    EXPECT_FALSE(m_model->isRemovingAvailable());

    // [THEN] The top markings row can't be removed
    m_model->selectRow(row(0));
    EXPECT_FALSE(m_model->isRemovingAvailable());

    // [THEN] Other markings rows can
    m_model->selectRow(row(2));
    EXPECT_TRUE(m_model->isRemovingAvailable());

    // [THEN] All instruments can be removed together
    selectRows({ row(1), row(3) });
    EXPECT_TRUE(m_model->isRemovingAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, IsRemovingAvailable_Staves)
{
    // [GIVEN] A with two staves, and B with one
    addPart(u"A", 2);
    addPart(u"B");
    createModel(); // [A:0] A B

    // [THEN] One of A's staves can be removed
    m_model->selectRow(childRow(1, 0));
    EXPECT_TRUE(m_model->isRemovingAvailable());

    // [THEN] Not all of them
    selectRows({ childRow(1, 0), childRow(1, 1) });
    EXPECT_FALSE(m_model->isRemovingAvailable());

    // [THEN] Not B's only staff
    m_model->selectRow(childRow(2, 0));
    EXPECT_FALSE(m_model->isRemovingAvailable());

    // [THEN] Not the "add staff" row
    m_model->selectRow(childRow(1, 2));
    EXPECT_FALSE(m_model->isRemovingAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, IsRemovingAvailable_DifferentParents)
{
    // [GIVEN] A with two staves, and B
    addPart(u"A", 2);
    addPart(u"B");
    createModel(); // [A:0] A B

    // [WHEN] A staff of A and the instrument B are selected
    selectRows({ childRow(1, 0), row(2) });

    // [THEN] They can't be removed together
    EXPECT_FALSE(m_model->isRemovingAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, RemoveSelectedRows_WithoutParts)
{
    // [GIVEN] Two instruments, no part scores
    addPart(u"A");
    addPart(u"B");
    createModel(); // [A:0] A B

    std::shared_ptr<int> emptyChanges = countEmits(&LayoutPanelTreeModel::isEmptyChanged);

    // [THEN] B is removed without asking
    EXPECT_CALL(*m_interactive, warningSync(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*m_parts, removeParts(_)).Times(1);

    // [WHEN] B is removed
    m_model->selectRow(row(2));
    m_model->removeSelectedRows();

    // [THEN] Its row is gone
    EXPECT_EQ(rowNames(), "[A:0] A");
    EXPECT_EQ(*emptyChanges, 1);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, RemoveSelectedRows_WithParts_Confirmed)
{
    // [GIVEN] Two instruments and a part score
    addPart(u"A");
    addPart(u"B");
    addExcerpt();
    createModel(); // [A:0] A B

    ON_CALL(*m_interactive, buttonData(_)).WillByDefault([](IInteractive::Button button) {
        return IInteractive::ButtonData(button, "");
    });

    // [THEN] The user is asked, and says yes
    EXPECT_CALL(*m_interactive, warningSync(_, _, _, _, _, _))
    .WillOnce(Return(IInteractive::Result(static_cast<int>(IInteractive::Button::Yes))));
    EXPECT_CALL(*m_parts, removeParts(_)).Times(1);

    // [WHEN] B is removed
    m_model->selectRow(row(2));
    m_model->removeSelectedRows();

    // [THEN] Its row is gone
    EXPECT_EQ(rowNames(), "[A:0] A");
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, RemoveSelectedRows_WithParts_Declined)
{
    // [GIVEN] Two instruments and a part score
    addPart(u"A");
    addPart(u"B");
    addExcerpt();
    createModel(); // [A:0] A B

    ON_CALL(*m_interactive, buttonData(_)).WillByDefault([](IInteractive::Button button) {
        return IInteractive::ButtonData(button, "");
    });

    // [THEN] The user is asked, and says no
    EXPECT_CALL(*m_interactive, warningSync(_, _, _, _, _, _))
    .WillOnce(Return(IInteractive::Result(static_cast<int>(IInteractive::Button::No))));
    EXPECT_CALL(*m_parts, removeParts(_)).Times(0);

    // [WHEN] B is removed
    m_model->selectRow(row(2));
    m_model->removeSelectedRows();

    // [THEN] Its row stays
    EXPECT_EQ(rowNames(), "[A:0] A B");
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, RemoveSelectedRows_MarkingsWithParts)
{
    // [GIVEN] Two instruments, system markings above A and B, and a part score
    addPart(u"A");
    const Part* b = addPart(u"B");
    addSystemObjects(b->staff(0));
    addExcerpt();
    createModel(); // [A:0] A [B:0] B

    // [THEN] Markings are removed without asking
    EXPECT_CALL(*m_interactive, warningSync(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*m_parts, removeSystemObjects(_)).Times(1);

    // [WHEN] The markings row above B is removed
    m_model->selectRow(row(2));
    m_model->removeSelectedRows();

    // [THEN] Its row is gone
    EXPECT_EQ(rowNames(), "[A:0] A B");
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, RemoveSelectedRows_StaffWithParts)
{
    // [GIVEN] An instrument with two staves, and a part score
    addPart(u"A", 2);
    addExcerpt();
    createModel(); // [A:0] A

    // [THEN] The staff is removed without asking
    EXPECT_CALL(*m_interactive, warningSync(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*m_masterParts, removeStaves(_)).Times(1);

    // [WHEN] The first staff is removed
    m_model->selectRow(childRow(1, 0));
    m_model->removeSelectedRows();

    // [THEN] Its row is gone
    EXPECT_EQ(childTypes(1), (ItemTypes { ItemType::STAFF, ItemType::CONTROL_ADD_STAFF }));
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, RemoveSelectedRows_NotAvailable)
{
    // [GIVEN] Two instruments
    addPart(u"A");
    addPart(u"B");
    createModel(); // [A:0] A B

    // [THEN] Nothing is removed
    EXPECT_CALL(*m_parts, removeParts(_)).Times(0);
    EXPECT_CALL(*m_parts, removeSystemObjects(_)).Times(0);

    // [WHEN] The top markings row is removed
    m_model->selectRow(row(0));
    m_model->removeSelectedRows();

    // [THEN] All rows stay
    EXPECT_EQ(rowNames(), "[A:0] A B");
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, RemoveSelectedRows_ScoreRemovesMarkings)
{
    // [GIVEN] Three instruments, system markings above A and B
    addPart(u"A");
    const Part* b = addPart(u"B");
    addPart(u"C");
    addSystemObjects(b->staff(0));
    createModel(); // [A:0] A [B:0] B C

    std::shared_ptr<int> resets = countEmits(&QAbstractItemModel::modelReset);

    // [THEN] Removing C also removes B's markings from the score
    EXPECT_CALL(*m_parts, removeParts(_)).WillOnce(InvokeWithoutArgs([this, b]() {
        removeSystemObjects(b->staff(0));
        m_systemObjectStavesChanged.notify();
    }));

    // [WHEN] C is removed
    m_model->selectRow(row(4));
    m_model->removeSelectedRows();

    // [THEN] The markings row above B follows the score without reloading the panel
    EXPECT_EQ(rowNames(), "[A:0] A B");
    EXPECT_EQ(*resets, 0);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, MoveRows_InvalidArguments)
{
    // [GIVEN] Three instruments
    addPart(u"A");
    addPart(u"B");
    addPart(u"C");
    createModel(); // [A:0] A B C

    // [THEN] Nothing is sent to the score
    EXPECT_CALL(*m_parts, reorderParts(_, _)).Times(0);

    // [WHEN] Rows are moved with bad arguments
    // [THEN] Nothing moves
    EXPECT_FALSE(moveRootRows(-1, 1, 2));
    EXPECT_FALSE(moveRootRows(1, 0, 2));
    EXPECT_FALSE(moveRootRows(1, 1, -1));
    EXPECT_FALSE(moveRootRows(2, 1, 2));
    EXPECT_FALSE(moveRootRows(3, 1, 5));
    EXPECT_FALSE(moveRootRows(4, 1, 0));
    EXPECT_EQ(rowNames(), "[A:0] A B C");
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, MoveSelectedRowsDown)
{
    // [GIVEN] Three instruments
    addPart(u"A");
    addPart(u"B");
    addPart(u"C");
    createModel(); // [A:0] A B C

    // [THEN] The new order is sent to the score once
    EXPECT_CALL(*m_parts, reorderParts(_, _)).Times(1);

    // [WHEN] B is moved down
    m_model->selectRow(row(2));
    m_model->moveSelectedRowsDown();

    // [THEN] B is last, still selected, and can't move further down
    EXPECT_EQ(rowNames(), "[A:0] A C B");
    EXPECT_TRUE(item(3)->isSelected());
    EXPECT_FALSE(m_model->isMovingDownAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, MoveSelectedRowsUp)
{
    // [GIVEN] Three instruments
    addPart(u"A");
    addPart(u"B");
    addPart(u"C");
    createModel(); // [A:0] A B C

    // [THEN] Each move is sent to the score
    EXPECT_CALL(*m_parts, reorderParts(_, _)).Times(2);

    // [WHEN] C is moved up
    m_model->selectRow(row(3));
    m_model->moveSelectedRowsUp();

    // [THEN] C can still move up
    EXPECT_EQ(rowNames(), "[A:0] A C B");
    EXPECT_TRUE(m_model->isMovingUpAvailable());

    // [WHEN] C is moved up again
    m_model->moveSelectedRowsUp();

    // [THEN] C is right below the top markings row, and can't move further up
    EXPECT_EQ(rowNames(), "[A:0] C A B");
    EXPECT_FALSE(m_model->isMovingUpAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, MoveSelectedRowsUpAndDown_NotAvailable)
{
    // [GIVEN] Two instruments
    addPart(u"A");
    addPart(u"B");
    createModel(); // [A:0] A B

    // [THEN] Nothing is sent to the score
    EXPECT_CALL(*m_parts, reorderParts(_, _)).Times(0);

    // [WHEN] A is moved up and B is moved down
    m_model->selectRow(row(1));
    m_model->moveSelectedRowsUp();
    m_model->selectRow(row(2));
    m_model->moveSelectedRowsDown();

    // [THEN] Nothing moves
    EXPECT_EQ(rowNames(), "[A:0] A B");
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, MoveSelectedRowsDown_Staff)
{
    // [GIVEN] An instrument with two staves
    const Part* a = addPart(u"A", 2);
    createModel(); // [A:0] A

    // [THEN] Only the staff order is sent to the score
    EXPECT_CALL(*m_parts, reorderStaves(_)).Times(1);
    EXPECT_CALL(*m_parts, reorderParts(_, _)).Times(0);

    // [WHEN] The first staff is moved down
    m_model->selectRow(childRow(1, 0));
    m_model->moveSelectedRowsDown();

    // [THEN] It's the last staff, above the "add staff" row
    EXPECT_EQ(childItem(1, 1)->id(), a->staff(0)->id());
    EXPECT_FALSE(m_model->isMovingDownAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, EndActiveDrag_AppliesOrder)
{
    // [GIVEN] Three instruments
    addPart(u"A");
    addPart(u"B");
    addPart(u"C");
    createModel(); // [A:0] A B C

    // [THEN] Nothing is sent to the score while dragging
    EXPECT_CALL(*m_parts, reorderParts(_, _)).Times(0);

    // [WHEN] A is dragged down past B and C
    m_model->startActiveDrag();
    EXPECT_TRUE(moveRootRows(1, 1, 2));
    EXPECT_TRUE(moveRootRows(2, 1, 3));

    Mock::VerifyAndClearExpectations(m_parts.get());

    // [THEN] The order is sent once, on drop
    EXPECT_CALL(*m_parts, reorderParts(_, _)).Times(1);

    // [WHEN] A is dropped
    m_model->endActiveDrag();

    // [THEN] A is last
    EXPECT_EQ(rowNames(), "[A:0] B C A");
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, EndActiveDrag_WithoutMoves)
{
    // [GIVEN] Two instruments
    addPart(u"A");
    addPart(u"B");
    createModel(); // [A:0] A B

    // [THEN] Nothing is sent to the score
    EXPECT_CALL(*m_parts, reorderParts(_, _)).Times(0);

    // [WHEN] A drag ends without moving anything
    m_model->startActiveDrag();
    m_model->endActiveDrag();
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, MoveRows_ScoreSkipsMarkingsMove)
{
    // [GIVEN] Three instruments, system markings above each of them
    addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    addSystemObjects(b->staff(0));
    addSystemObjects(c->staff(0));
    createModel(); // [A:0] A [B:0] B [C:0] C

    // [THEN] The score keeps all markings where they are
    EXPECT_CALL(*m_parts, reorderParts(_, _)).Times(1);

    // [WHEN] The markings row above B is moved down past B, onto C's markings
    EXPECT_TRUE(moveRootRows(2, 1, 3));

    // [THEN] The markings rows are rebuilt from the score
    EXPECT_EQ(rowNames(), "[A:0] A [B:0] B [C:0] C");
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, MoveRows_ScoreAddsMarkings)
{
    // [GIVEN] Three instruments
    addPart(u"A");
    addPart(u"B");
    const Part* c = addPart(u"C");
    createModel(); // [A:0] A B C

    std::shared_ptr<int> resets = countEmits(&QAbstractItemModel::modelReset);

    // [THEN] While the order is applied, the score adds markings to C
    EXPECT_CALL(*m_parts, reorderParts(_, _)).WillOnce(InvokeWithoutArgs([this, c]() {
        addSystemObjects(c->staff(0));
        m_systemObjectStavesChanged.notify();
    }));

    // [WHEN] C is moved up past B
    EXPECT_TRUE(moveRootRows(3, 1, 2));

    // [THEN] A markings row is added above C without reloading the panel
    EXPECT_EQ(rowNames(), "[A:0] A [C:0] C B");
    EXPECT_EQ(*resets, 0);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, MoveRows_PartsChangeDuringMove)
{
    // [GIVEN] Three instruments
    addPart(u"A");
    addPart(u"B");
    addPart(u"C");
    createModel(); // [A:0] A B C

    std::shared_ptr<int> resets = countEmits(&QAbstractItemModel::modelReset);

    // [THEN] The score reports changed instruments while the order is applied
    EXPECT_CALL(*m_parts, reorderParts(_, _)).WillOnce(InvokeWithoutArgs([this]() {
        m_partsNotifier.changed();
    }));

    // [WHEN] C is moved up
    EXPECT_TRUE(moveRootRows(3, 1, 2));

    // [THEN] The panel keeps its rows
    EXPECT_EQ(rowNames(), "[A:0] A C B");
    EXPECT_EQ(*resets, 0);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, MoveRows_NotationChangeDuringMove)
{
    // [GIVEN] Three instruments
    addPart(u"A");
    addPart(u"B");
    addPart(u"C");
    createModel(); // [A:0] A B C

    std::shared_ptr<int> resets = countEmits(&QAbstractItemModel::modelReset);

    // [THEN] Another score becomes current while the order is applied, and the panel waits
    EXPECT_CALL(*m_parts, reorderParts(_, _)).WillOnce(InvokeWithoutArgs([this, resets]() {
        m_currentNotationChanged.notify();
        EXPECT_EQ(*resets, 0);
    }));

    // [WHEN] C is moved up
    EXPECT_TRUE(moveRootRows(3, 1, 2));

    // [THEN] The panel reloads once the move is done
    EXPECT_EQ(*resets, 1);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, SelectRow_MarksItemsSelected)
{
    // [GIVEN] Two instruments
    addPart(u"A");
    addPart(u"B");
    createModel(); // [A:0] A B

    // [WHEN] A is selected
    m_model->selectRow(row(1));

    // [THEN] A's row is selected
    EXPECT_TRUE(item(1)->isSelected());
    EXPECT_FALSE(item(2)->isSelected());

    // [WHEN] B is selected
    m_model->selectRow(row(2));

    // [THEN] Only B's row is selected
    EXPECT_FALSE(item(1)->isSelected());
    EXPECT_TRUE(item(2)->isSelected());

    // [WHEN] The selection is cleared
    m_model->clearSelection();

    // [THEN] Nothing is selected, and nothing can be moved or removed
    EXPECT_FALSE(item(2)->isSelected());
    EXPECT_FALSE(m_model->isMovingUpAvailable());
    EXPECT_FALSE(m_model->isRemovingAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, SelectRow_TopMarkingsNotMarkedSelected)
{
    // [GIVEN] An instrument
    addPart(u"A");
    createModel(); // [A:0] A

    // [WHEN] The top markings row is selected
    m_model->selectRow(row(0));

    // [THEN] Its row doesn't show as selected
    EXPECT_FALSE(item(0)->isSelected());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, SelectedItemsType)
{
    // [GIVEN] An instrument with two staves
    addPart(u"A", 2);
    createModel(); // [A:0] A

    std::shared_ptr<int> typeChanges = countEmits(&LayoutPanelTreeModel::selectedItemsTypeChanged);

    // [THEN] Nothing is selected
    EXPECT_EQ(m_model->selectedItemsType(), static_cast<int>(ItemType::UNDEFINED));

    // [WHEN] A is selected
    m_model->selectRow(row(1));
    EXPECT_EQ(m_model->selectedItemsType(), static_cast<int>(ItemType::PART));

    // [WHEN] The first staff, then the second staff is selected
    m_model->selectRow(childRow(1, 0));
    m_model->selectRow(childRow(1, 1));
    EXPECT_EQ(m_model->selectedItemsType(), static_cast<int>(ItemType::STAFF));

    // [WHEN] The selection is cleared
    m_model->clearSelection();
    EXPECT_EQ(m_model->selectedItemsType(), static_cast<int>(ItemType::UNDEFINED));

    // [THEN] The type changed three times
    EXPECT_EQ(*typeChanges, 3);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, UpdateSelectedRows_SelectsInstrumentRows)
{
    // [GIVEN] Three instruments
    addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    createModel(); // [A:0] A B C

    // [WHEN] Elements of B and C are selected in the score
    m_selectedElements = { addTextElement(b->staff(0)), addTextElement(c->staff(0)) };
    m_selectionChanged.notify();

    // [THEN] B's and C's rows are selected
    EXPECT_FALSE(item(1)->isSelected());
    EXPECT_TRUE(item(2)->isSelected());
    EXPECT_TRUE(item(3)->isSelected());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, UpdateSelectedRows_WithoutInstrument)
{
    // [GIVEN] Two instruments, A selected in the panel
    addPart(u"A");
    addPart(u"B");
    createModel(); // [A:0] A B
    m_model->selectRow(row(1));

    // [WHEN] An element without a staff is selected in the score
    m_selectedElements = { addTextElement(nullptr) };
    m_selectionChanged.notify();

    // [THEN] Nothing is selected in the panel
    EXPECT_FALSE(item(1)->isSelected());
    EXPECT_FALSE(item(2)->isSelected());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, SetLayoutPanelVisible_Selection)
{
    // [GIVEN] Two instruments, the panel is hidden
    addPart(u"A");
    const Part* b = addPart(u"B");
    createModel(); // [A:0] A B
    m_model->setLayoutPanelVisible(false);

    // [WHEN] An element of B is selected in the score
    m_selectedElements = { addTextElement(b->staff(0)) };
    m_selectionChanged.notify();

    // [THEN] The hidden panel doesn't follow
    EXPECT_FALSE(item(2)->isSelected());

    // [WHEN] The panel is shown
    m_model->setLayoutPanelVisible(true);

    // [THEN] B's row is selected
    EXPECT_TRUE(item(2)->isSelected());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, IsAddingSystemMarkingsAvailable)
{
    // [GIVEN] Three instruments, system markings only on the top staff
    addPart(u"A");
    const Part* b = addPart(u"B");
    addPart(u"C");
    createModel(); // [A:0] A B C

    // [THEN] Markings can be added
    EXPECT_TRUE(m_model->isAddingSystemMarkingsAvailable());

    // [WHEN] The score adds markings to B
    addSystemObjects(b->staff(0));
    m_systemObjectStavesChanged.notify(); // [A:0] A [B:0] B C

    // [THEN] Markings can still be added
    EXPECT_TRUE(m_model->isAddingSystemMarkingsAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, IsAddingSystemMarkingsAvailable_HalfOfRows)
{
    // [GIVEN] Two instruments, system markings above both
    addPart(u"A");
    const Part* b = addPart(u"B");
    addSystemObjects(b->staff(0));

    // [WHEN] The panel is loaded
    createModel(); // [A:0] A [B:0] B

    // [THEN] No more markings can be added
    EXPECT_FALSE(m_model->isAddingSystemMarkingsAvailable());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, AddSystemMarkings_SelectedInstrument)
{
    // [GIVEN] Three instruments
    addPart(u"A");
    addPart(u"B");
    const Part* c = addPart(u"C");
    createModel(); // [A:0] A B C

    // [THEN] Markings are added to C's top staff
    EXPECT_CALL(*m_masterParts, addSystemObjects(IDList { c->staff(0)->id() }));

    // [WHEN] C is selected and markings are added
    m_model->selectRow(row(3));
    m_model->addSystemMarkings();
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, AddSystemMarkings_FirstInstrumentWithoutMarkings)
{
    // [GIVEN] Three instruments, A selected, it has the top markings
    addPart(u"A");
    const Part* b = addPart(u"B");
    addPart(u"C");
    createModel(); // [A:0] A B C
    m_model->selectRow(row(1));

    // [THEN] Markings are added to B, the first instrument without them
    EXPECT_CALL(*m_masterParts, addSystemObjects(IDList { b->staff(0)->id() }));

    // [WHEN] Markings are added
    m_model->addSystemMarkings();
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, AddSystemMarkings_AllInstrumentsHaveMarkings)
{
    // [GIVEN] Two instruments, system markings above both
    addPart(u"A");
    const Part* b = addPart(u"B");
    addSystemObjects(b->staff(0));
    createModel(); // [A:0] A [B:0] B

    // [THEN] Nothing is added
    EXPECT_CALL(*m_masterParts, addSystemObjects(_)).Times(0);

    // [WHEN] Markings are added
    m_model->addSystemMarkings();
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, SetupPartsConnections_SystemObjectStavesChanged)
{
    // [GIVEN] Three instruments
    addPart(u"A");
    addPart(u"B");
    const Part* c = addPart(u"C");
    createModel(); // [A:0] A B C

    std::shared_ptr<int> resets = countEmits(&QAbstractItemModel::modelReset);

    // [WHEN] The score adds markings to C
    addSystemObjects(c->staff(0));
    m_systemObjectStavesChanged.notify();

    // [THEN] The panel reloads with a markings row above C
    EXPECT_EQ(rowNames(), "[A:0] A B [C:0] C");
    EXPECT_EQ(*resets, 1);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, OnScoreChanged_UpdatesSettings)
{
    // [GIVEN] An instrument
    addPart(u"A");
    createModel();

    std::shared_ptr<int> hideChanges = countEmits(&LayoutPanelTreeModel::isHideEmptyStavesEnabledChanged);

    // [WHEN] The score turns on stave sharing and hides empty staves
    m_score->style().set(Sid::enableStaveSharing, true);
    m_score->style().set(Sid::hideEmptyStaves, true);
    m_scoreChanges.send(ScoreChanges());

    // [THEN] The panel follows
    EXPECT_TRUE(m_model->isStaveSharingEnabled());
    EXPECT_TRUE(m_model->isHideEmptyStavesEnabled());
    EXPECT_EQ(*hideChanges, 1);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, SetupNotationConnections_TextEditingIgnored)
{
    // [GIVEN] An instrument
    addPart(u"A");
    createModel();

    // [WHEN] Empty staves are hidden while text is being edited
    m_score->style().set(Sid::hideEmptyStaves, true);

    ScoreChanges changes;
    changes.isTextEditing = true;
    m_scoreChanges.send(changes);

    // [THEN] The panel doesn't follow
    EXPECT_FALSE(m_model->isHideEmptyStavesEnabled());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, SetLayoutPanelVisible_ScoreChanges)
{
    // [GIVEN] An instrument, the panel is hidden
    addPart(u"A");
    createModel();
    m_model->setLayoutPanelVisible(false);

    // [WHEN] The score hides empty staves
    m_score->style().set(Sid::hideEmptyStaves, true);
    m_scoreChanges.send(ScoreChanges());

    // [THEN] The hidden panel doesn't follow
    EXPECT_FALSE(m_model->isHideEmptyStavesEnabled());

    // [WHEN] The panel is shown
    m_model->setLayoutPanelVisible(true);

    // [THEN] It catches up
    EXPECT_TRUE(m_model->isHideEmptyStavesEnabled());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, OnNotationChanged)
{
    // [GIVEN] An instrument
    addPart(u"A");
    createModel(); // [A:0] A

    std::shared_ptr<int> addingChanges = countEmits(&LayoutPanelTreeModel::isAddingAvailableChanged);

    // [WHEN] The score is closed
    ON_CALL(*m_session, currentNotation()).WillByDefault(Return(INotationPtr()));
    m_currentNotationChanged.notify();

    // [THEN] The panel is empty
    EXPECT_EQ(rowCount(), 0);
    EXPECT_TRUE(m_model->isEmpty());
    EXPECT_FALSE(m_model->isAddingAvailable());

    // [WHEN] The score is opened again
    ON_CALL(*m_session, currentNotation()).WillByDefault(Return(m_notation));
    m_currentNotationChanged.notify();

    // [THEN] The panel shows it
    EXPECT_EQ(rowNames(), "[A:0] A");
    EXPECT_TRUE(m_model->isAddingAvailable());
    EXPECT_EQ(*addingChanges, 2);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, SetupPartsConnections_PartListChanged)
{
    // [GIVEN] An instrument
    addPart(u"A");
    createModel(); // [A:0] A

    std::shared_ptr<int> resets = countEmits(&QAbstractItemModel::modelReset);

    // [WHEN] The score adds B
    addPart(u"B");
    m_partsNotifier.changed();

    // [THEN] The panel reloads with B
    EXPECT_EQ(rowNames(), "[A:0] A B");
    EXPECT_EQ(*resets, 1);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, SetupPartsConnections_PartChanged)
{
    // [GIVEN] An instrument
    const Part* a = addPart(u"A");
    createModel(); // [A:0] A

    std::shared_ptr<int> resets = countEmits(&QAbstractItemModel::modelReset);

    // [WHEN] The score renames it
    const_cast<Part*>(a)->setLongNameAll(u"Oboe");
    m_partsNotifier.itemChanged(a);

    // [THEN] Its row follows without reloading the panel
    EXPECT_EQ(rowNames(), "[Oboe:0] Oboe");
    EXPECT_EQ(*resets, 0);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, SetupStavesConnections_StaffAdded)
{
    // [GIVEN] An instrument with one staff
    const Part* a = addPart(u"A");
    createModel(); // [A:0] A

    std::shared_ptr<int> resets = countEmits(&QAbstractItemModel::modelReset);

    // [WHEN] The score adds a staff to it
    Staff* staff = Factory::createStaff(const_cast<Part*>(a));
    m_score->appendStaff(staff);
    m_staffNotifiers[a->id()].itemAdded(staff);

    // [THEN] A staff row is added above the "add staff" row, without reloading the panel
    EXPECT_EQ(childTypes(1), (ItemTypes { ItemType::STAFF, ItemType::STAFF, ItemType::CONTROL_ADD_STAFF }));
    EXPECT_EQ(childItem(1, 1)->id(), staff->id());
    EXPECT_EQ(*resets, 0);

    // [WHEN] The score reports the same staff again
    m_staffNotifiers[a->id()].itemAdded(staff);

    // [THEN] No row is added
    EXPECT_EQ(m_model->rowCount(row(1)), 3);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, SetupStavesConnections_StaffRemoved)
{
    // [GIVEN] An instrument with two staves
    const Part* a = addPart(u"A", 2);
    createModel(); // [A:0] A

    std::shared_ptr<int> resets = countEmits(&QAbstractItemModel::modelReset);

    // [WHEN] The score removes the second staff
    m_staffNotifiers[a->id()].itemRemoved(a->staff(1));

    // [THEN] Its row is gone, without reloading the panel
    EXPECT_EQ(childTypes(1), (ItemTypes { ItemType::STAFF, ItemType::CONTROL_ADD_STAFF }));
    EXPECT_EQ(childItem(1, 0)->id(), a->staff(0)->id());
    EXPECT_EQ(*resets, 0);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, SetupStavesConnections_StaffChanged)
{
    // [GIVEN] An instrument with a visible staff
    const Part* a = addPart(u"A");
    createModel(); // [A:0] A
    ASSERT_TRUE(childItem(1, 0)->isVisible());

    // [WHEN] The score hides the staff
    a->staff(0)->setVisible(false);
    m_staffNotifiers[a->id()].itemChanged(a->staff(0));

    // [THEN] Its row follows
    EXPECT_FALSE(childItem(1, 0)->isVisible());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, SetupExcerptNameConnection)
{
    // [GIVEN] A part score
    addPart(u"A");
    m_score->setExcerpt(&m_excerpt);
    createModel();

    std::shared_ptr<int> scoreChanges = countEmits(&LayoutPanelTreeModel::scoreChanged);

    // [WHEN] The part score is renamed
    m_excerpt.setName(u"Flute");

    // [THEN] The panel's score name is updated
    EXPECT_EQ(*scoreChanges, 1);

    m_score->setExcerpt(nullptr);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, SetupPartsConnections_SharedPartsChanged)
{
    // [GIVEN] An instrument, stave sharing off
    addPart(u"A");
    createModel();

    // [WHEN] The score turns on stave sharing
    m_score->style().set(Sid::enableStaveSharing, true);
    m_sharedPartsChanged.notify();

    // [THEN] The panel follows
    EXPECT_TRUE(m_model->isStaveSharingEnabled());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, ToggleHideEmptyStaves)
{
    // [GIVEN] An instrument, empty staves shown
    addPart(u"A");
    createModel();

    // [THEN] The style is changed in one undoable step
    {
        InSequence seq;
        EXPECT_CALL(*m_undoStack, prepareChanges(_));
        EXPECT_CALL(*m_style, setStyleValue(StyleId::hideEmptyStaves, _));
        EXPECT_CALL(*m_undoStack, commitChanges());
    }

    // [WHEN] Empty staves are hidden
    m_model->toggleHideEmptyStaves(true);

    // [THEN] The panel follows
    EXPECT_TRUE(m_model->isHideEmptyStavesEnabled());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, ToggleHideEmptyStaves_Unchanged)
{
    // [GIVEN] An instrument, empty staves shown
    addPart(u"A");
    createModel();

    // [THEN] Nothing changes
    EXPECT_CALL(*m_undoStack, prepareChanges(_)).Times(0);
    EXPECT_CALL(*m_style, setStyleValue(_, _)).Times(0);

    // [WHEN] Empty staves are shown
    m_model->toggleHideEmptyStaves(false);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, ToggleStaveSharing)
{
    // [GIVEN] An instrument, stave sharing off
    addPart(u"A");
    createModel();

    // [THEN] The score turns on stave sharing
    EXPECT_CALL(*m_parts, toggleStaveSharing(true)).WillOnce(InvokeWithoutArgs([this]() {
        m_score->style().set(Sid::enableStaveSharing, true);
    }));

    // [WHEN] Stave sharing is turned on
    m_model->toggleStaveSharing(true);

    // [THEN] The panel follows
    EXPECT_TRUE(m_model->isStaveSharingEnabled());
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, ChangeVisibilityOfSelectedRows)
{
    // [GIVEN] Three instruments, A and B selected
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    createModel(); // [A:0] A B C
    selectRows({ row(1), row(2) });

    std::shared_ptr<int> resets = countEmits(&QAbstractItemModel::modelReset);

    // [THEN] A and B are hidden, and the score reports changed instruments meanwhile
    EXPECT_CALL(*m_parts, setPartVisible(a->id(), false)).WillOnce(InvokeWithoutArgs([this]() {
        m_partsNotifier.changed();
    }));
    EXPECT_CALL(*m_parts, setPartVisible(b->id(), false));
    EXPECT_CALL(*m_parts, setPartVisible(c->id(), _)).Times(0);

    // [WHEN] The selected rows are hidden
    m_model->changeVisibilityOfSelectedRows(false);

    // [THEN] The panel keeps its rows
    EXPECT_FALSE(item(1)->isVisible());
    EXPECT_FALSE(item(2)->isVisible());
    EXPECT_TRUE(item(3)->isVisible());
    EXPECT_EQ(*resets, 0);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, ChangeEnabledOfSelectedRows)
{
    // [GIVEN] An instrument and a disabled shared-staves group, the group selected
    addPart(u"A");
    const Part* s = addSharedPart(u"S");
    createModel(); // [A:0] A S
    m_model->selectRow(row(2));

    std::shared_ptr<int> resets = countEmits(&QAbstractItemModel::modelReset);

    // [THEN] The group is enabled, and the score reports changed instruments meanwhile
    EXPECT_CALL(*m_parts, setSharedPartEnabled(s->id(), true)).WillOnce(InvokeWithoutArgs([this]() {
        m_partsNotifier.changed();
    }));

    // [WHEN] The selected rows are enabled
    m_model->changeEnabledOfSelectedRows(true);

    // [THEN] The panel keeps its rows
    EXPECT_TRUE(item(2)->isEnabled());
    EXPECT_EQ(*resets, 0);
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, AddInstruments)
{
    // [GIVEN] An instrument
    addPart(u"A");
    createModel();

    // [THEN] The instruments dialog is requested
    EXPECT_CALL(*m_dispatcher, dispatch(Field(&muse::rcommand::Request::command, INSTRUMENTS_SELECT_COMMAND), _))
    .WillOnce([](const muse::rcommand::Request& request, bool) {
        return async::make_promise<muse::rcommand::Response>([request](auto resolve) {
            return resolve(muse::rcommand::make_response(request, make_ok()));
        });
    });

    // [WHEN] Instruments are added
    m_model->addInstruments();
}

TEST_F(InstrumentsScene_LayoutPanelTreeModelTests, AddInstrumentsKeyboardShortcut)
{
    // [GIVEN] The instruments dialog has two shortcuts
    muse::shortcuts::Shortcut shortcut;
    shortcut.sequences = { "I", "Shift+I" };
    ON_CALL(*m_shortcuts, shortcut(INSTRUMENTS_SELECT_COMMAND)).WillByDefault(ReturnRef(shortcut));

    addPart(u"A");
    createModel();

    // [THEN] The panel shows the first one
    EXPECT_EQ(m_model->addInstrumentsKeyboardShortcut(), QString("I"));

    // [WHEN] The shortcuts are removed
    std::shared_ptr<int> shortcutChanges = countEmits(&LayoutPanelTreeModel::addInstrumentsKeyboardShortcutChanged);
    shortcut.sequences.clear();
    m_shortcutsChanged.notify();

    // [THEN] The panel shows none
    EXPECT_EQ(*shortcutChanges, 1);
    EXPECT_TRUE(m_model->addInstrumentsKeyboardShortcut().isEmpty());
}
