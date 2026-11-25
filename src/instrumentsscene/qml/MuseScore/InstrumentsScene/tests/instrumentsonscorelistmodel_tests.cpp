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

#include <algorithm>

#include "async/changednotify.h"
#include "async/notifylist.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/scoreorder.h"
#include "engraving/dom/sharedpart.h"

#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/instrumentsonscorelistmodel.h"

#include "notation/tests/mocks/instrumentsrepositorymock.h"
#include "notation/tests/mocks/masternotationmock.h"
#include "notation/tests/mocks/notationmock.h"
#include "notation/tests/mocks/notationpartsmock.h"
#include "notation/tests/mocks/notationstylemock.h"
#include "project/tests/mocks/projectsessionmock.h"

using namespace ::testing;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;

namespace mu::instrumentsscene {
class InstrumentsScene_InstrumentsOnScoreListModelTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_score = compat::ScoreAccess::createMasterScore(nullptr);

        m_session = std::make_shared<NiceMock<mu::project::ProjectSessionMock> >();
        m_masterNotation = std::make_shared<NiceMock<MasterNotationMock> >();
        m_notation = std::make_shared<NiceMock<NotationMock> >();
        m_parts = std::make_shared<NiceMock<NotationPartsMock> >();
        m_style = std::make_shared<NiceMock<NotationStyleMock> >();
        m_repository = std::make_shared<NiceMock<InstrumentsRepositoryMock> >();

        ON_CALL(*m_session, masterNotation()).WillByDefault(Return(m_masterNotation));
        ON_CALL(*m_session, currentNotation()).WillByDefault(Return(m_notation));

        ON_CALL(*m_masterNotation, notation()).WillByDefault(Return(m_notation));
        ON_CALL(*m_masterNotation, parts()).WillByDefault(Return(m_parts));
        ON_CALL(*m_notation, parts()).WillByDefault(Return(m_parts));
        ON_CALL(*m_notation, style()).WillByDefault(Return(m_style));

        ON_CALL(*m_style, styleValue(_)).WillByDefault(Invoke([this](const StyleId& styleId) -> const PropertyValue& {
            return m_score->style().styleV(styleId);
        }));

        ON_CALL(*m_parts, partList()).WillByDefault(Invoke([this]() {
            const std::vector<const Part*> list(m_score->parts().begin(), m_score->parts().end());
            return async::NotifyList<const Part*>(list, m_partsNotifier.notify());
        }));
        ON_CALL(*m_parts, scoreOrder()).WillByDefault(ReturnPointee(&m_scoreOrder));

        setUpInstrumentTemplates();

        ON_CALL(*m_repository, instrumentTemplate(_)).WillByDefault(Invoke([this](const String& id) -> const InstrumentTemplate& {
            if (id == m_flute.id) {
                return m_flute;
            }
            if (id == m_violin.id) {
                return m_violin;
            }
            return m_noTemplate;
        }));
        ON_CALL(*m_repository, orders()).WillByDefault(ReturnRef(m_orders));

        //! Orchestral: soloists, then woodwinds, then strings. Choral: strings before woodwinds
        m_orders = {
            order(u"orchestral", u"Orchestral", { u"<soloists>", m_woodwinds.id, m_strings.id }),
            order(u"choral", u"Choral", { m_strings.id, m_woodwinds.id }),
        };
        m_scoreOrder = customOrder();

        m_model = new InstrumentsOnScoreListModel();
        m_model->context.set(m_session);
        m_model->repository.set(m_repository);
    }

    void TearDown() override
    {
        delete m_model;
        delete m_score;

        instrumentGroups.erase(std::remove(instrumentGroups.begin(), instrumentGroups.end(), &m_group), instrumentGroups.end());
    }

    //! Flute (woodwinds) and violin (strings), known to the score orders by their families
    void setUpInstrumentTemplates()
    {
        m_woodwinds.id = u"woodwinds";
        m_strings.id = u"strings";

        m_flute.id = u"flute";
        m_flute.trackName = u"Flute";
        m_flute.description = u"A flute";
        m_flute.family = &m_woodwinds;
        m_flute.sequenceOrder = 0;

        m_violin.id = u"violin";
        m_violin.trackName = u"Violin";
        m_violin.description = u"A violin";
        m_violin.family = &m_strings;
        m_violin.sequenceOrder = 1;

        m_group.instrumentTemplates = { &m_flute, &m_violin };
        instrumentGroups.push_back(&m_group);
    }

    static ScoreOrder order(const String& id, const String& name, const std::vector<String>& families = {})
    {
        ScoreOrder result;
        result.id = id;
        result.name = TranslatableString::untranslatable(name);

        for (const String& family : families) {
            ScoreGroup group;
            group.family = family;
            result.groups.push_back(group);
        }

        return result;
    }

    const Part* addPart(const String& name, const String& instrumentId = String(), bool soloist = false)
    {
        Part* part = new Part(m_score);
        part->setLongNameAll(name);
        part->instrument()->setId(instrumentId);
        part->setSoloist(soloist);
        m_score->appendPart(part);
        m_score->appendStaff(Factory::createStaff(part));

        return part;
    }

    //! Its instruments are added after it (addToSharedPart()), as in the score
    SharedPart* addSharedPart(const String& name, const String& instrumentId = String())
    {
        SharedPart* sharedPart = new SharedPart(m_score);
        sharedPart->setLongNameAll(name);
        sharedPart->instrument()->setId(instrumentId);
        m_score->appendPart(sharedPart);
        m_score->appendStaff(Factory::createStaff(sharedPart));

        return sharedPart;
    }

    const Part* addToSharedPart(SharedPart* sharedPart, const String& name, const String& instrumentId = String())
    {
        const Part* part = addPart(name, instrumentId);
        sharedPart->addOriginPart(m_score->partById(part->id()));

        return part;
    }

    //! "Combine instruments onto shared staves"
    void setCombineOnSharedStaves(bool combine)
    {
        m_score->style().set(Sid::enableStaveSharing, combine);
    }

    int role(const QByteArray& name) const
    {
        return m_model->roleNames().key(name);
    }

    QVariant rowData(int row, const QByteArray& roleName) const
    {
        return m_model->data(m_model->index(row), role(roleName));
    }

    QStringList rowNames() const
    {
        QStringList names;
        for (int row = 0; row < m_model->rowCount(); ++row) {
            names << rowData(row, "name").toString();
        }

        return names;
    }

    //! The parts that are kept on accepting, in order
    std::vector<ID> keptPartIds() const
    {
        std::vector<ID> ids;
        for (const QVariant& instrument : m_model->instruments()) {
            ids.push_back(ID(instrument.toMap().value("partId").toString()));
        }

        return ids;
    }

    QVariantMap keptInstrument(int index) const
    {
        return m_model->instruments().at(index).toMap();
    }

    InstrumentsOnScoreListModel* m_model = nullptr;
    MasterScore* m_score = nullptr;

    std::shared_ptr<mu::project::ProjectSessionMock> m_session;

    //! The orders in the repository, and the score's
    ScoreOrderList m_orders;
    ScoreOrder m_scoreOrder;

private:
    std::shared_ptr<MasterNotationMock> m_masterNotation;
    std::shared_ptr<NotationMock> m_notation;
    std::shared_ptr<NotationPartsMock> m_parts;
    std::shared_ptr<NotationStyleMock> m_style;
    std::shared_ptr<InstrumentsRepositoryMock> m_repository;

    async::ChangedNotifier<const Part*> m_partsNotifier;

    InstrumentFamily m_woodwinds;
    InstrumentFamily m_strings;
    InstrumentTemplate m_flute;
    InstrumentTemplate m_violin;
    InstrumentTemplate m_noTemplate;
    InstrumentGroup m_group;
};
}

// ---- load()

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Load_ListsInstruments)
{
    // [GIVEN] A flute A, and a violin B as a soloist
    const Part* a = addPart(u"A", u"flute");
    const Part* b = addPart(u"B", u"violin", /*soloist*/ true);

    // [WHEN] The model is loaded
    m_model->load();

    // [THEN] Both are listed, in score order, with their soloist state
    EXPECT_EQ(rowNames(), QStringList({ "A", "B" }));
    EXPECT_FALSE(rowData(0, "isSoloist").toBool());
    EXPECT_TRUE(rowData(1, "isSoloist").toBool());

    // [THEN] They're kept as they are on accepting
    const QVariantMap keptA = keptInstrument(0);
    EXPECT_EQ(keptA.value("partId").toString(), a->id().toQString());
    EXPECT_EQ(keptA.value("instrumentId").toString(), "flute");
    EXPECT_TRUE(keptA.value("isExistingPart").toBool());

    const QVariantMap keptB = keptInstrument(1);
    EXPECT_EQ(keptB.value("partId").toString(), b->id().toQString());
    EXPECT_EQ(keptB.value("instrumentId").toString(), "violin");
    EXPECT_TRUE(keptB.value("isExistingPart").toBool());
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Load_WithoutScore_ListsNothing)
{
    // [GIVEN] The model was loaded for A, in choral order, and then the score was closed
    addPart(u"A");
    m_scoreOrder = m_orders.at(1);
    m_model->load();

    ON_CALL(*m_session, masterNotation()).WillByDefault(Return(nullptr));
    ON_CALL(*m_session, currentNotation()).WillByDefault(Return(nullptr));

    // [WHEN] The model is loaded again
    m_model->load();

    // [THEN] Nothing is listed, and the first order is chosen
    EXPECT_EQ(m_model->rowCount(), 0);
    EXPECT_EQ(m_model->currentOrderIndex(), 0);
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Load_Again_ReplacesRows)
{
    // [GIVEN] A is listed, and a flute was added to the model
    addPart(u"A");
    m_model->load();
    m_model->addInstruments({ "flute" });

    // [WHEN] The model is loaded again
    m_model->load();

    // [THEN] Only the score's instrument is listed
    EXPECT_EQ(rowNames(), QStringList({ "A" }));
}

// ---- Score orders

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Load_ListsOrdersWithCustomLast)
{
    // [WHEN] The model is loaded
    m_model->load();

    // [THEN] The repository's orders are listed, then "Custom"
    EXPECT_EQ(m_model->orders(), QStringList({ "Orchestral", "Choral", "Custom" }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Load_SelectsScoresOrder)
{
    // [GIVEN] The score is in choral order
    m_scoreOrder = m_orders.at(1);

    // [WHEN] The model is loaded
    m_model->load();

    // [THEN] Choral is the current order
    EXPECT_EQ(m_model->currentOrderIndex(), 1);
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Load_AddsScoresOwnOrder)
{
    // [GIVEN] The score's order isn't in the repository (e.g. it came with a template)
    m_scoreOrder = order(u"template", u"From template");

    // [WHEN] The model is loaded
    m_model->load();

    // [THEN] It's listed last, and is the current order
    EXPECT_EQ(m_model->orders(), QStringList({ "Orchestral", "Choral", "Custom", "From template" }));
    EXPECT_EQ(m_model->currentOrderIndex(), 3);
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Load_CustomizedScoreOrder)
{
    // [GIVEN] The score's orchestral order is customized
    m_scoreOrder = m_orders.at(0);
    m_scoreOrder.customized = true;

    // [WHEN] The model is loaded
    m_model->load();

    // [THEN] The customized order is listed after orchestral, and is the current order
    EXPECT_EQ(m_model->orders(), QStringList({ "Orchestral", "Orchestral (Customized)", "Choral", "Custom" }));
    EXPECT_EQ(m_model->currentOrderIndex(), 1);
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, CurrentOrder_ReturnsIdAndCustomized)
{
    // [GIVEN] The current order is the customized orchestral one
    m_scoreOrder = m_orders.at(0);
    m_scoreOrder.customized = true;
    m_model->load();

    // [WHEN] The current order is asked for (on accepting)
    const QVariantMap currentOrder = m_model->currentOrder().toMap();

    // [THEN] It's orchestral, customized
    EXPECT_EQ(currentOrder.value("id").toString(), "orchestral");
    EXPECT_TRUE(currentOrder.value("customized").toBool());
}

// ---- instruments()

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Instruments_KeepsSharedPartBeforeItsInstruments)
{
    // [GIVEN] A, and B and C on a shared part S (which isn't listed while Combine is off)
    const Part* a = addPart(u"A");
    SharedPart* s = addSharedPart(u"S");
    const Part* b = addToSharedPart(s, u"B");
    const Part* c = addToSharedPart(s, u"C");

    // [WHEN] The model is loaded
    m_model->load();

    // [THEN] Only the instruments are listed
    EXPECT_EQ(m_model->rowCount(), 3);

    // [THEN] S is kept on accepting, before its first instrument
    EXPECT_EQ(keptPartIds(), std::vector<ID>({ a->id(), s->id(), b->id(), c->id() }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Instruments_SharedPartFollowsItsFirstInstrument)
{
    // [GIVEN] A, and B and C on a shared part S
    const Part* a = addPart(u"A");
    SharedPart* s = addSharedPart(u"S");
    const Part* b = addToSharedPart(s, u"B");
    const Part* c = addToSharedPart(s, u"C");

    m_model->load();

    // [WHEN] C is moved above A
    m_model->selectRow(2);
    m_model->moveSelectedBlocksUp();
    m_model->moveSelectedBlocksUp();

    // [THEN] S is kept before C, and listed once
    EXPECT_EQ(keptPartIds(), std::vector<ID>({ s->id(), c->id(), a->id(), b->id() }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Instruments_KeepsSharedPartWhileAnInstrumentIsLeft)
{
    // [GIVEN] A, and B and C on a shared part S
    const Part* a = addPart(u"A");
    SharedPart* s = addSharedPart(u"S");
    addToSharedPart(s, u"B");
    const Part* c = addToSharedPart(s, u"C");

    m_model->load();

    // [WHEN] B is removed
    m_model->selectRow(1);
    m_model->removeSelection();

    // [THEN] S is kept with C
    EXPECT_EQ(keptPartIds(), std::vector<ID>({ a->id(), s->id(), c->id() }));

    // [WHEN] C is removed too
    m_model->selectRow(1);
    m_model->removeSelection();

    // [THEN] S is removed with its last instrument
    EXPECT_EQ(keptPartIds(), std::vector<ID>({ a->id() }));
}

// ---- addInstruments()

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, AddInstruments_AddsNewInstrument)
{
    // [GIVEN] An empty score
    m_model->load();

    // [WHEN] A violin is added
    m_model->addInstruments({ "violin" });

    // [THEN] It's listed with its template's name and description
    EXPECT_EQ(rowNames(), QStringList({ "Violin" }));
    EXPECT_EQ(rowData(0, "description").toString(), "A violin");

    // [THEN] It's added to the score on accepting
    const QVariantMap kept = keptInstrument(0);
    EXPECT_FALSE(kept.value("isExistingPart").toBool());
    EXPECT_EQ(kept.value("instrumentId").toString(), "violin");
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, AddInstruments_SkipsUnknownTemplates)
{
    // [GIVEN] An empty score
    m_model->load();

    // [WHEN] An instrument without a template is added
    m_model->addInstruments({ "unknown" });

    // [THEN] Nothing is listed
    EXPECT_EQ(m_model->rowCount(), 0);
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, AddInstruments_InsertsInScoreOrder)
{
    // [GIVEN] A flute and a violin, in orchestral order
    m_scoreOrder = m_orders.at(0);
    addPart(u"A", u"flute");
    addPart(u"B", u"violin");
    m_model->load();

    // [WHEN] Another flute is added
    m_model->addInstruments({ "flute" });

    // [THEN] It goes after the flute, before the violin
    EXPECT_EQ(rowNames(), QStringList({ "A", "Flute", "B" }));
}

// ---- setData()

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, SetData_Soloist_MovesToSoloists)
{
    // [GIVEN] A flute and a violin, in orchestral order
    m_scoreOrder = m_orders.at(0);
    addPart(u"A", u"flute");
    const Part* b = addPart(u"B", u"violin");
    m_model->load();

    // [WHEN] The violin is made a soloist
    EXPECT_TRUE(m_model->setData(m_model->index(1), true, role("isSoloist")));

    // [THEN] It moves to the soloists, at the top
    EXPECT_EQ(rowNames(), QStringList({ "B", "A" }));
    EXPECT_TRUE(rowData(0, "isSoloist").toBool());

    // [THEN] It's made a soloist on accepting
    const QVariantMap kept = keptInstrument(0);
    EXPECT_EQ(kept.value("partId").toString(), b->id().toQString());
    EXPECT_TRUE(kept.value("isSoloist").toBool());
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, SetData_OtherRole_ReturnsFalse)
{
    // [GIVEN] A
    addPart(u"A");
    m_model->load();

    // [WHEN] Its name is set
    // [THEN] Nothing changes
    EXPECT_FALSE(m_model->setData(m_model->index(0), QString("X"), role("name")));
    EXPECT_EQ(rowNames(), QStringList({ "A" }));
}

// ---- Removing and moving

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, RemoveSelection_RemovesInstrument)
{
    // [GIVEN] A and B
    addPart(u"A");
    const Part* b = addPart(u"B");
    m_model->load();

    // [WHEN] A is removed
    m_model->selectRow(0);
    m_model->removeSelection();

    // [THEN] Only B is listed, and kept on accepting
    EXPECT_EQ(rowNames(), QStringList({ "B" }));
    EXPECT_EQ(keptPartIds(), std::vector<ID>({ b->id() }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, MoveSelectionDown_MovesRowDown)
{
    // [GIVEN] A, B and C
    addPart(u"A");
    addPart(u"B");
    addPart(u"C");
    m_model->load();

    // [WHEN] A is moved down
    m_model->selectRow(0);
    m_model->moveSelectedBlocksDown();

    // [THEN] It's below B
    EXPECT_EQ(rowNames(), QStringList({ "B", "A", "C" }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, OperationsAvailability_FollowsSelection)
{
    // [GIVEN] A, B and C, none selected
    addPart(u"A");
    addPart(u"B");
    addPart(u"C");
    m_model->load();

    // [THEN] Nothing can be moved or removed
    EXPECT_FALSE(m_model->isMovingUpAvailable());
    EXPECT_FALSE(m_model->isMovingDownAvailable());
    EXPECT_FALSE(m_model->isRemovingAvailable());

    // [WHEN] The first row is selected
    m_model->selectRow(0);

    // [THEN] It can be moved down and removed, not moved up
    EXPECT_FALSE(m_model->isMovingUpAvailable());
    EXPECT_TRUE(m_model->isMovingDownAvailable());
    EXPECT_TRUE(m_model->isRemovingAvailable());

    // [WHEN] The last row is selected
    m_model->selectRow(2);

    // [THEN] It can be moved up, not down
    EXPECT_TRUE(m_model->isMovingUpAvailable());
    EXPECT_FALSE(m_model->isMovingDownAvailable());
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, MoveOutOfOrder_CustomizesOrderAndBack)
{
    // [GIVEN] A flute and a violin, in orchestral order
    m_scoreOrder = m_orders.at(0);
    addPart(u"A", u"flute");
    addPart(u"B", u"violin");
    m_model->load();

    // [WHEN] The violin is moved above the flute
    m_model->selectRow(1);
    m_model->moveSelectedBlocksUp();

    // [THEN] The order becomes customized
    EXPECT_EQ(m_model->orders(), QStringList({ "Orchestral", "Orchestral (Customized)", "Choral", "Custom" }));
    EXPECT_EQ(m_model->currentOrderIndex(), 1);

    // [WHEN] It's moved back
    m_model->moveSelectedBlocksDown();

    // [THEN] The order is orchestral again
    EXPECT_EQ(m_model->orders(), QStringList({ "Orchestral", "Choral", "Custom" }));
    EXPECT_EQ(m_model->currentOrderIndex(), 0);
}

// ---- setCurrentOrderIndex()

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, SetCurrentOrderIndex_SortsInstruments)
{
    // [GIVEN] A flute and a violin, in orchestral order
    m_scoreOrder = m_orders.at(0);
    addPart(u"A", u"flute");
    addPart(u"B", u"violin");
    m_model->load();

    // [WHEN] Choral order is chosen
    m_model->setCurrentOrderIndex(1);

    // [THEN] The violin goes first
    EXPECT_EQ(rowNames(), QStringList({ "B", "A" }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, SetCurrentOrderIndex_KeepsOrderOfEqualInstruments)
{
    // [GIVEN] 20 instruments the score orders don't know (so they all sort the same)
    QStringList names;
    for (int i = 0; i < 20; ++i) {
        names << QString::number(i);
        addPart(String::fromQString(names.back()));
    }
    m_model->load();

    // [WHEN] Another order is chosen
    m_model->setCurrentOrderIndex(0);

    // [THEN] They keep their order
    EXPECT_EQ(rowNames(), names);
}

// ---- Shared-staves groups

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Load_CombineOn_ListsGroupBeforeItsInstruments)
{
    // [GIVEN] Combine is on; A, and B and C on a shared part S
    setCombineOnSharedStaves(true);
    addPart(u"A");
    SharedPart* s = addSharedPart(u"S");
    addToSharedPart(s, u"B");
    addToSharedPart(s, u"C");

    // [WHEN] The model is loaded
    m_model->load();

    // [THEN] S is listed as a group header, followed by its instruments
    EXPECT_EQ(rowNames(), QStringList({ "A", "S", "B", "C" }));

    EXPECT_EQ(rowData(0, "sharedGroupRole").toInt(), InstrumentsOnScoreListModel::NotInGroup);
    EXPECT_EQ(rowData(1, "sharedGroupRole").toInt(), InstrumentsOnScoreListModel::GroupHeader);
    EXPECT_EQ(rowData(2, "sharedGroupRole").toInt(), InstrumentsOnScoreListModel::GroupMember);
    EXPECT_EQ(rowData(3, "sharedGroupRole").toInt(), InstrumentsOnScoreListModel::GroupLastMember);

    // [THEN] The header and its instruments have the group's id
    const QString groupId = s->id().toQString();
    EXPECT_TRUE(rowData(0, "sharedGroupId").toString().isEmpty());
    EXPECT_EQ(rowData(1, "sharedGroupId").toString(), groupId);
    EXPECT_EQ(rowData(2, "sharedGroupId").toString(), groupId);
    EXPECT_EQ(rowData(3, "sharedGroupId").toString(), groupId);
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, MoveSelectionUp_GroupHeader_MovesWholeGroup)
{
    // [GIVEN] Combine is on; A, and B and C on a shared part S
    setCombineOnSharedStaves(true);
    addPart(u"A");
    SharedPart* s = addSharedPart(u"S");
    addToSharedPart(s, u"B");
    addToSharedPart(s, u"C");
    m_model->load();

    // [WHEN] S's header is moved up
    m_model->selectRow(1);
    m_model->moveSelectedBlocksUp();

    // [THEN] The whole group moves above A
    EXPECT_EQ(rowNames(), QStringList({ "S", "B", "C", "A" }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, MoveSelectionDown_SkipsGroup)
{
    // [GIVEN] Combine is on; A, and B and C on a shared part S
    setCombineOnSharedStaves(true);
    addPart(u"A");
    SharedPart* s = addSharedPart(u"S");
    addToSharedPart(s, u"B");
    addToSharedPart(s, u"C");
    m_model->load();

    // [WHEN] A is moved down
    m_model->selectRow(0);
    m_model->moveSelectedBlocksDown();

    // [THEN] It goes below the whole group, not in among its instruments
    EXPECT_EQ(rowNames(), QStringList({ "S", "B", "C", "A" }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, MoveAvailability_GroupInstrumentCantMove)
{
    // [GIVEN] Combine is on; A, B and C on a shared part S, and D
    setCombineOnSharedStaves(true);
    addPart(u"A");
    SharedPart* s = addSharedPart(u"S");
    addToSharedPart(s, u"B");
    addToSharedPart(s, u"C");
    addPart(u"D");
    m_model->load();

    // [WHEN] B is selected
    m_model->selectRow(2);

    // [THEN] It can't be moved, but can be removed
    EXPECT_FALSE(m_model->isMovingUpAvailable());
    EXPECT_FALSE(m_model->isMovingDownAvailable());
    EXPECT_TRUE(m_model->isRemovingAvailable());

    // [WHEN] Moving it is asked for anyway
    m_model->moveSelectedBlocksUp();

    // [THEN] Nothing moves
    EXPECT_EQ(rowNames(), QStringList({ "A", "S", "B", "C", "D" }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, MoveAvailability_SelectionWithGap_CantMove)
{
    // [GIVEN] A, B, C, D and E
    addPart(u"A");
    addPart(u"B");
    addPart(u"C");
    addPart(u"D");
    addPart(u"E");
    m_model->load();

    // [WHEN] B and D are selected
    selectRows({ 1, 3 });

    // [THEN] They can't be moved, but can be removed
    EXPECT_FALSE(m_model->isMovingUpAvailable());
    EXPECT_FALSE(m_model->isMovingDownAvailable());
    EXPECT_TRUE(m_model->isRemovingAvailable());

    // [WHEN] Moving them is asked for anyway
    m_model->moveSelectedBlocksUp();

    // [THEN] Nothing moves
    EXPECT_EQ(rowNames(), QStringList({ "A", "B", "C", "D", "E" }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, MoveSelectionUp_InstrumentAndGroupHeader_MovesBoth)
{
    // [GIVEN] Combine is on; A, B, and C and D on a shared part S
    setCombineOnSharedStaves(true);
    addPart(u"A");
    addPart(u"B");
    SharedPart* s = addSharedPart(u"S");
    addToSharedPart(s, u"C");
    addToSharedPart(s, u"D");
    m_model->load();

    // [WHEN] B and S's header are selected and moved up
    selectRows({ 1, 2 });
    m_model->moveSelectedBlocksUp();

    // [THEN] B and the whole group move above A
    EXPECT_EQ(rowNames(), QStringList({ "B", "S", "C", "D", "A" }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, SetCurrentOrderIndex_SortsGroupsAsAWhole)
{
    // [GIVEN] Combine is on; a violin and a flute on a shared part S, then another flute
    setCombineOnSharedStaves(true);
    SharedPart* s = addSharedPart(u"S");
    addToSharedPart(s, u"B", u"violin");
    addToSharedPart(s, u"C", u"flute");
    addPart(u"A", u"flute");
    m_model->load();

    // [WHEN] Orchestral order is chosen (woodwinds before strings)
    m_model->setCurrentOrderIndex(0);

    // [THEN] The group goes by its first instrument, the violin: after the flute, and still together
    EXPECT_EQ(rowNames(), QStringList({ "A", "S", "B", "C" }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, AddInstruments_GroupSortedByFirstInstrument_NotCustomized)
{
    // [GIVEN] Combine is on, in orchestral order; a flute, then a violin and a flute on a shared part S
    // (the group goes by its violin, so it's after the flute)
    m_scoreOrder = m_orders.at(0);
    setCombineOnSharedStaves(true);
    addPart(u"A", u"flute");
    SharedPart* s = addSharedPart(u"S");
    addToSharedPart(s, u"B", u"violin");
    addToSharedPart(s, u"C", u"flute");
    m_model->load();

    // [WHEN] A violin is added
    m_model->addInstruments({ "violin" });

    // [THEN] The order isn't customized
    EXPECT_EQ(m_model->orders(), QStringList({ "Orchestral", "Choral", "Custom" }));
    EXPECT_EQ(m_model->currentOrderIndex(), 0);
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, AddInstruments_NotInsideGroup)
{
    // [GIVEN] Combine is on, in orchestral order; a flute and a violin on a shared part S
    m_scoreOrder = m_orders.at(0);
    setCombineOnSharedStaves(true);
    SharedPart* s = addSharedPart(u"S");
    addToSharedPart(s, u"B", u"flute");
    addToSharedPart(s, u"C", u"violin");
    m_model->load();

    // [WHEN] A flute is added (which sorts between the group's flute and violin)
    m_model->addInstruments({ "flute" });

    // [THEN] It goes after the whole group
    EXPECT_EQ(rowNames(), QStringList({ "S", "B", "C", "Flute" }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, SetData_Soloist_GroupInstrumentStaysInGroup)
{
    // [GIVEN] Combine is on, in orchestral order; a flute and a violin on a shared part S
    m_scoreOrder = m_orders.at(0);
    setCombineOnSharedStaves(true);
    SharedPart* s = addSharedPart(u"S");
    addToSharedPart(s, u"B", u"flute");
    addToSharedPart(s, u"C", u"violin");
    m_model->load();

    // [WHEN] The violin is made a soloist
    EXPECT_TRUE(m_model->setData(m_model->index(2), true, role("isSoloist")));

    // [THEN] It stays in its group
    EXPECT_EQ(rowNames(), QStringList({ "S", "B", "C" }));
    EXPECT_TRUE(rowData(2, "isSoloist").toBool());
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, IsSharedGroupSelected_FollowsHeaderSelection)
{
    // [GIVEN] Combine is on; B and C on a shared part S, and D and E on a shared part T
    setCombineOnSharedStaves(true);
    SharedPart* s = addSharedPart(u"S");
    addToSharedPart(s, u"B");
    addToSharedPart(s, u"C");
    SharedPart* t = addSharedPart(u"T");
    addToSharedPart(t, u"D");
    addToSharedPart(t, u"E");
    m_model->load();

    const QString sId = s->id().toQString();
    const QString tId = t->id().toQString();

    // [THEN] No group is selected
    EXPECT_FALSE(m_model->isSharedGroupSelected(sId));

    // [WHEN] S's header is selected
    m_model->selectRow(0);

    // [THEN] S is selected, T isn't
    EXPECT_TRUE(m_model->isSharedGroupSelected(sId));
    EXPECT_FALSE(m_model->isSharedGroupSelected(tId));

    // [WHEN] One of S's instruments is selected instead
    m_model->selectRow(1);

    // [THEN] S itself isn't
    EXPECT_FALSE(m_model->isSharedGroupSelected(sId));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Load_CombineOff_InstrumentsAreNotInGroups)
{
    // [GIVEN] Combine is off; B and C on a shared part S
    setCombineOnSharedStaves(false);
    SharedPart* s = addSharedPart(u"S");
    addToSharedPart(s, u"B");
    addToSharedPart(s, u"C");

    // [WHEN] The model is loaded
    m_model->load();

    // [THEN] B and C aren't shown as a group
    EXPECT_EQ(rowData(0, "sharedGroupRole").toInt(), InstrumentsOnScoreListModel::NotInGroup);
    EXPECT_EQ(rowData(1, "sharedGroupRole").toInt(), InstrumentsOnScoreListModel::NotInGroup);
    EXPECT_TRUE(rowData(0, "sharedGroupId").toString().isEmpty());
    EXPECT_TRUE(rowData(1, "sharedGroupId").toString().isEmpty());
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Instruments_CombineOn_KeepsGroupAsListed)
{
    // [GIVEN] Combine is on; B and C on a shared part S, then A
    setCombineOnSharedStaves(true);
    SharedPart* s = addSharedPart(u"S");
    const Part* b = addToSharedPart(s, u"B");
    const Part* c = addToSharedPart(s, u"C");
    const Part* a = addPart(u"A");
    m_model->load();

    // [WHEN] A is moved above the group's header
    m_model->selectRow(3);
    m_model->moveSelectedBlocksUp();
    m_model->moveSelectedBlocksUp();
    m_model->moveSelectedBlocksUp();

    // [THEN] S is kept where its header is listed, as an existing part
    EXPECT_EQ(keptPartIds(), std::vector<ID>({ a->id(), s->id(), b->id(), c->id() }));
    EXPECT_TRUE(keptInstrument(1).value("isExistingPart").toBool());
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Instruments_CombineOn_RemovesGroupWithoutInstruments)
{
    // [GIVEN] Combine is on; A, and B and C on a shared part S
    setCombineOnSharedStaves(true);
    const Part* a = addPart(u"A");
    SharedPart* s = addSharedPart(u"S");
    addToSharedPart(s, u"B");
    addToSharedPart(s, u"C");
    m_model->load();

    // [WHEN] B and C are removed
    m_model->selectRow(2);
    m_model->removeSelection();
    m_model->selectRow(2);
    m_model->removeSelection();

    // [THEN] S is removed with them, though its header is still listed
    EXPECT_EQ(rowNames(), QStringList({ "A", "S" }));
    EXPECT_EQ(keptPartIds(), std::vector<ID>({ a->id() }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Instruments_CombineOn_RemovingGroupHeaderUngroups)
{
    // [GIVEN] Combine is on; B and C on a shared part S
    setCombineOnSharedStaves(true);
    SharedPart* s = addSharedPart(u"S");
    const Part* b = addToSharedPart(s, u"B");
    const Part* c = addToSharedPart(s, u"C");
    m_model->load();

    // [WHEN] The group's header is removed
    m_model->selectRow(0);
    m_model->removeSelection();

    // [THEN] B and C are kept, without S
    EXPECT_EQ(keptPartIds(), std::vector<ID>({ b->id(), c->id() }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, SetData_Soloist_GroupHeader_ReturnsFalse)
{
    // [GIVEN] Combine is on; B and C on a shared part S
    setCombineOnSharedStaves(true);
    SharedPart* s = addSharedPart(u"S");
    addToSharedPart(s, u"B");
    addToSharedPart(s, u"C");
    m_model->load();

    // [WHEN] The group's header is made a soloist
    // [THEN] Nothing changes
    EXPECT_FALSE(m_model->setData(m_model->index(0), true, role("isSoloist")));
    EXPECT_FALSE(rowData(0, "isSoloist").toBool());
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, AddInstruments_GroupHeaderDoesNotAffectScoreOrder)
{
    // [GIVEN] Combine is on, in orchestral order; two flutes on a shared part whose own instrument is a violin
    // (which would sort after the flutes)
    m_scoreOrder = m_orders.at(0);
    setCombineOnSharedStaves(true);
    SharedPart* s = addSharedPart(u"S", u"violin");
    addToSharedPart(s, u"B", u"flute");
    addToSharedPart(s, u"C", u"flute");
    m_model->load();

    // [WHEN] A violin is added
    m_model->addInstruments({ "violin" });

    // [THEN] The instruments are in orchestral order: it isn't customized
    EXPECT_EQ(m_model->orders(), QStringList({ "Orchestral", "Choral", "Custom" }));
    EXPECT_EQ(m_model->currentOrderIndex(), 0);
}
