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
        m_repository = std::make_shared<NiceMock<InstrumentsRepositoryMock> >();

        ON_CALL(*m_session, masterNotation()).WillByDefault(Return(m_masterNotation));
        ON_CALL(*m_session, currentNotation()).WillByDefault(Return(m_notation));

        ON_CALL(*m_masterNotation, notation()).WillByDefault(Return(m_notation));
        ON_CALL(*m_masterNotation, parts()).WillByDefault(Return(m_parts));
        ON_CALL(*m_notation, parts()).WillByDefault(Return(m_parts));

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

    //! The shared part comes before its instruments, as in the score
    const Part* addSharedPart(const std::vector<const Part*>& parts)
    {
        SharedPart* sharedPart = new SharedPart(m_score);
        m_score->appendPart(sharedPart);
        m_score->appendStaff(Factory::createStaff(sharedPart));

        for (const Part* part : parts) {
            sharedPart->addOriginPart(m_score->partById(part->id()));
        }

        return sharedPart;
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
    // [GIVEN] A, and B and C on a shared part S (which isn't listed)
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    const Part* s = addSharedPart({ b, c });

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
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    const Part* s = addSharedPart({ b, c });

    m_model->load();

    // [WHEN] C is moved above A
    m_model->selectRow(2);
    m_model->moveSelectionUp();
    m_model->moveSelectionUp();

    // [THEN] S is kept before C, and listed once
    EXPECT_EQ(keptPartIds(), std::vector<ID>({ s->id(), c->id(), a->id(), b->id() }));
}

TEST_F(InstrumentsScene_InstrumentsOnScoreListModelTests, Instruments_KeepsSharedPartWhileAnInstrumentIsLeft)
{
    // [GIVEN] A, and B and C on a shared part S
    const Part* a = addPart(u"A");
    const Part* b = addPart(u"B");
    const Part* c = addPart(u"C");
    const Part* s = addSharedPart({ b, c });

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
    m_model->moveSelectionDown();

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
    m_model->moveSelectionUp();

    // [THEN] The order becomes customized
    EXPECT_EQ(m_model->orders(), QStringList({ "Orchestral", "Orchestral (Customized)", "Choral", "Custom" }));
    EXPECT_EQ(m_model->currentOrderIndex(), 1);

    // [WHEN] It's moved back
    m_model->moveSelectionDown();

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
