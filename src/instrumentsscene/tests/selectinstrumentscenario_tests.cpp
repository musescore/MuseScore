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

#include "async/asyncable.h"
#include "async/processevents.h"
#include "modularity/ioc.h"

#include "instrumentsscene/internal/selectinstrumentscenario.h"

#include "interactive/tests/mocks/interactivemock.h"
#include "notation/tests/mocks/instrumentsrepositorymock.h"

using namespace ::testing;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;

namespace {
const Uri SELECT_INSTRUMENT_URI("musescore://instruments/select");

//! The number of groups only tells orders with the same id apart
ScoreOrder makeOrder(const String& id, size_t groupCount = 0)
{
    ScoreOrder order;
    order.id = id;
    order.groups.resize(groupCount);

    return order;
}

InstrumentTemplate makeTemplate(const String& id)
{
    InstrumentTemplate templ;
    templ.id = id;

    return templ;
}

Val instrumentVal(const std::string& partId, const std::string& instrumentId, bool isExistingPart, bool isSoloist)
{
    return Val(ValMap {
            { "partId", Val(partId) },
            { "instrumentId", Val(instrumentId) },
            { "isExistingPart", Val(isExistingPart) },
            { "isSoloist", Val(isSoloist) },
        });
}

Val dialogResult(const ValList& instruments, const std::string& orderId, bool customized = false)
{
    return Val(ValMap {
            { "instruments", Val(instruments) },
            { "scoreOrder", Val(ValMap { { "id", Val(orderId) }, { "customized", Val(customized) } }) },
        });
}

UriQuery selectInstrumentsQuery()
{
    UriQuery query(SELECT_INSTRUMENT_URI);
    query.addParam("canSelectMultipleInstruments", Val(true));

    return query;
}

UriQuery selectInstrumentQuery(const std::string& currentInstrumentId)
{
    UriQuery query(SELECT_INSTRUMENT_URI);
    query.addParam("canSelectMultipleInstruments", Val(false));
    query.addParam("currentInstrumentId", Val(currentInstrumentId));

    return query;
}
}

namespace mu::instrumentsscene {
class InstrumentsScene_SelectInstrumentsScenarioTests : public ::testing::Test, public async::Asyncable
{
protected:
    void SetUp() override
    {
        m_scenario = std::make_shared<SelectInstrumentsScenario>(modularity::globalCtx());

        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        m_repository = std::make_shared<NiceMock<InstrumentsRepositoryMock> >();

        m_scenario->interactive.set(m_interactive);
        m_scenario->instrumentsRepository.set(m_repository);

        m_orchestralOrder = makeOrder(u"orchestral", 2);
    }

    //! The dialog isn't open yet, so it gets opened and closes with "result"
    void expectDialogOpened(const UriQuery& query, const Val& result)
    {
        ON_CALL(*m_interactive, isOpened(An<const Uri&>())).WillByDefault(Return(RetVal<bool>::make_ok(false)));

        EXPECT_CALL(*m_interactive, open(query))
        .WillOnce(Invoke([result](auto&&...) {
            return async::make_promise<Val>([result](auto resolve, auto) {
                return resolve(result);
            });
        }));
    }

    PartInstrumentListScoreOrder selectInstruments(const ScoreOrder& currentOrder)
    {
        PartInstrumentListScoreOrder result;
        bool resolved = false;

        m_scenario->selectInstruments(currentOrder).onResolve(this, [&](const PartInstrumentListScoreOrder& val) {
            result = val;
            resolved = true;
        });
        async::processMessages(); // selectInstruments() promise
        async::processMessages(); // dialog

        EXPECT_TRUE(resolved);
        return result;
    }

    std::shared_ptr<SelectInstrumentsScenario> m_scenario;
    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<InstrumentsRepositoryMock> m_repository;

    ScoreOrder m_orchestralOrder;
    ScoreOrder m_emptyOrder; // what the repository returns for ids it doesn't know
};
}

// ==================================================
// selectInstruments: instruments
// ==================================================

TEST_F(InstrumentsScene_SelectInstrumentsScenarioTests, SelectInstruments_ReturnsChosenInstruments)
{
    //! [GIVEN] The user keeps an existing soloist flute and adds a violin
    expectDialogOpened(selectInstrumentsQuery(), dialogResult({
        instrumentVal("1", "flute", true /*isExistingPart*/, true /*isSoloist*/),
        instrumentVal("", "violin", false /*isExistingPart*/, false /*isSoloist*/),
    }, "orchestral"));

    const InstrumentTemplate flute = makeTemplate(u"flute");
    const InstrumentTemplate violin = makeTemplate(u"violin");
    ON_CALL(*m_repository, instrumentTemplate(String(u"flute"))).WillByDefault(ReturnRef(flute));
    ON_CALL(*m_repository, instrumentTemplate(String(u"violin"))).WillByDefault(ReturnRef(violin));
    ON_CALL(*m_repository, order(_)).WillByDefault(ReturnRef(m_emptyOrder));

    //! [WHEN] Selecting instruments
    PartInstrumentListScoreOrder result = selectInstruments(ScoreOrder());

    //! [THEN] The instruments come back in order, with their templates from the repository
    ASSERT_EQ(result.instruments.size(), 2);

    EXPECT_EQ(result.instruments[0].partId, ID(1));
    EXPECT_TRUE(result.instruments[0].isExistingPart);
    EXPECT_TRUE(result.instruments[0].isSoloist);
    EXPECT_EQ(result.instruments[0].instrumentTemplate.id, u"flute");

    EXPECT_FALSE(result.instruments[1].isExistingPart);
    EXPECT_FALSE(result.instruments[1].isSoloist);
    EXPECT_EQ(result.instruments[1].instrumentTemplate.id, u"violin");
}

TEST_F(InstrumentsScene_SelectInstrumentsScenarioTests, SelectInstruments_DialogAlreadyOpen_Rejects)
{
    //! [GIVEN] The dialog is already open
    ON_CALL(*m_interactive, isOpened(An<const Uri&>())).WillByDefault(Return(RetVal<bool>::make_ok(true)));

    //! [THEN] No second dialog is opened
    EXPECT_CALL(*m_interactive, open(An<const UriQuery&>())).Times(0);

    //! [WHEN] Selecting instruments
    bool resolved = false;
    int rejectCode = 0;
    async::Promise<PartInstrumentListScoreOrder> promise = m_scenario->selectInstruments(ScoreOrder());
    promise.onResolve(this, [&resolved](const PartInstrumentListScoreOrder&) { resolved = true; });
    promise.onReject(this, [&rejectCode](int code, const std::string&) { rejectCode = code; });
    async::processMessages();

    //! [THEN] The request is cancelled
    EXPECT_FALSE(resolved);
    EXPECT_EQ(rejectCode, int(Ret::Code::Cancel));
}

// ==================================================
// selectInstruments: score order
// ==================================================

TEST_F(InstrumentsScene_SelectInstrumentsScenarioTests, SelectInstruments_BuiltInOrder_ComesFromRepository)
{
    //! [GIVEN] The user picks a built-in order, and the score has its own copy of it
    expectDialogOpened(selectInstrumentsQuery(), dialogResult({
        instrumentVal("", "flute", false /*isExistingPart*/, false /*isSoloist*/),
    }, "orchestral"));

    const InstrumentTemplate flute = makeTemplate(u"flute");
    ON_CALL(*m_repository, instrumentTemplate(_)).WillByDefault(ReturnRef(flute));
    ON_CALL(*m_repository, order(String(u"orchestral"))).WillByDefault(ReturnRef(m_orchestralOrder));

    //! [WHEN] Selecting instruments
    PartInstrumentListScoreOrder result = selectInstruments(makeOrder(u"orchestral", 1));

    //! [THEN] The built-in order is returned, not the score's copy
    EXPECT_EQ(result.scoreOrder.id, u"orchestral");
    EXPECT_EQ(result.scoreOrder.groups.size(), 2u);
}

TEST_F(InstrumentsScene_SelectInstrumentsScenarioTests, SelectInstruments_ScoreOwnOrder_IsKept)
{
    //! [GIVEN] The user keeps the score's own order (e.g. one that came with a template)
    expectDialogOpened(selectInstrumentsQuery(), dialogResult({
        instrumentVal("", "flute", false /*isExistingPart*/, false /*isSoloist*/),
    }, "template-order"));

    const InstrumentTemplate flute = makeTemplate(u"flute");
    ON_CALL(*m_repository, instrumentTemplate(_)).WillByDefault(ReturnRef(flute));
    ON_CALL(*m_repository, order(_)).WillByDefault(ReturnRef(m_emptyOrder));

    //! [WHEN] Selecting instruments
    PartInstrumentListScoreOrder result = selectInstruments(makeOrder(u"template-order", 1));

    //! [THEN] The score's order is returned, not the repository's empty one
    EXPECT_EQ(result.scoreOrder.id, u"template-order");
    EXPECT_EQ(result.scoreOrder.groups.size(), 1u);
}

TEST_F(InstrumentsScene_SelectInstrumentsScenarioTests, SelectInstruments_UnknownOrder_ReturnsEmptyOrder)
{
    //! [GIVEN] The dialog returns an order that's neither built in nor the score's
    expectDialogOpened(selectInstrumentsQuery(), dialogResult({
        instrumentVal("", "flute", false /*isExistingPart*/, false /*isSoloist*/),
    }, "unknown"));

    const InstrumentTemplate flute = makeTemplate(u"flute");
    ON_CALL(*m_repository, instrumentTemplate(_)).WillByDefault(ReturnRef(flute));
    ON_CALL(*m_repository, order(_)).WillByDefault(ReturnRef(m_emptyOrder));

    //! [WHEN] Selecting instruments
    PartInstrumentListScoreOrder result = selectInstruments(makeOrder(u"template-order", 1));

    //! [THEN] The repository's empty order is returned
    EXPECT_TRUE(result.scoreOrder.id.isEmpty());
}

TEST_F(InstrumentsScene_SelectInstrumentsScenarioTests, SelectInstruments_CustomizedFlag_ComesFromDialog)
{
    //! [GIVEN] The user customized the order
    expectDialogOpened(selectInstrumentsQuery(), dialogResult({
        instrumentVal("", "flute", false /*isExistingPart*/, false /*isSoloist*/),
    }, "orchestral", true /*customized*/));

    const InstrumentTemplate flute = makeTemplate(u"flute");
    ON_CALL(*m_repository, instrumentTemplate(_)).WillByDefault(ReturnRef(flute));
    ON_CALL(*m_repository, order(_)).WillByDefault(ReturnRef(m_orchestralOrder));

    //! [WHEN] Selecting instruments
    PartInstrumentListScoreOrder result = selectInstruments(ScoreOrder());

    //! [THEN] The order is marked as customized
    EXPECT_TRUE(result.scoreOrder.customized);
}

// ==================================================
// selectInstrument
// ==================================================

TEST_F(InstrumentsScene_SelectInstrumentsScenarioTests, SelectInstrument_ReturnsChosenTemplate)
{
    //! [GIVEN] The user picks a violin
    expectDialogOpened(selectInstrumentQuery("" /*currentInstrumentId*/), dialogResult({
        instrumentVal("", "violin", false /*isExistingPart*/, false /*isSoloist*/),
    }, "orchestral"));

    const InstrumentTemplate violin = makeTemplate(u"violin");
    ON_CALL(*m_repository, instrumentTemplate(String(u"violin"))).WillByDefault(ReturnRef(violin));
    ON_CALL(*m_repository, order(_)).WillByDefault(ReturnRef(m_emptyOrder));

    //! [WHEN] Selecting a single instrument
    InstrumentTemplate result;
    bool resolved = false;
    m_scenario->selectInstrument().onResolve(this, [&](const InstrumentTemplate& val) {
        result = val;
        resolved = true;
    });
    async::processMessages(); // selectInstrument() promise
    async::processMessages(); // selectInstruments() promise
    async::processMessages(); // dialog

    //! [THEN] Its template is returned
    EXPECT_TRUE(resolved);
    EXPECT_EQ(result.id, u"violin");
}
