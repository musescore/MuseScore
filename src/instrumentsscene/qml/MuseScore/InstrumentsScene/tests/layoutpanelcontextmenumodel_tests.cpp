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

#include "async/notification.h"

#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/layoutpanelcontextmenumodel.h"

#include "notation/tests/mocks/instrumentsrepositorymock.h"
#include "notation/tests/mocks/masternotationmock.h"
#include "notation/tests/mocks/notationmock.h"
#include "notation/tests/mocks/notationpartsmock.h"
#include "project/tests/mocks/projectsessionmock.h"
#include "shortcuts_v2/tests/mocks/commandshortcutsregistermock.h"

using namespace ::testing;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;
using namespace muse::uicomponents;

namespace mu::instrumentsscene {
class InstrumentsScene_LayoutPanelContextMenuModelTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_session = std::make_shared<NiceMock<mu::project::ProjectSessionMock> >();
        m_masterNotation = std::make_shared<NiceMock<MasterNotationMock> >();
        m_notation = std::make_shared<NiceMock<NotationMock> >();
        m_partNotation = std::make_shared<NiceMock<NotationMock> >();
        m_masterParts = std::make_shared<NiceMock<NotationPartsMock> >();
        m_repository = std::make_shared<NiceMock<InstrumentsRepositoryMock> >();
        m_shortcuts = std::make_shared<NiceMock<muse::shortcuts::CommandShortcutsRegisterMock> >();

        ON_CALL(*m_session, masterNotation()).WillByDefault(Return(m_masterNotation));
        ON_CALL(*m_session, currentNotation()).WillByDefault(Return(m_notation));
        ON_CALL(*m_session, currentNotationChanged()).WillByDefault(Return(m_currentNotationChanged));

        ON_CALL(*m_masterNotation, notation()).WillByDefault(Return(m_notation));
        ON_CALL(*m_masterNotation, parts()).WillByDefault(Return(m_masterParts));

        ON_CALL(*m_masterParts, scoreOrder()).WillByDefault(ReturnPointee(&m_scoreOrder));
        ON_CALL(*m_masterParts, scoreOrderChanged()).WillByDefault(Return(m_scoreOrderChanged));

        ON_CALL(*m_repository, orders()).WillByDefault(ReturnRef(m_orders));
        ON_CALL(*m_shortcuts, shortcut(_)).WillByDefault(ReturnRef(m_noShortcut));

        m_orders = { order(u"orchestral", u"Orchestral"), order(u"choral", u"Choral") };
        m_scoreOrder = m_orders.front();

        m_model = new LayoutPanelContextMenuModel();
        m_model->projectSession.set(m_session);
        m_model->instrumentsRepository.set(m_repository);
        m_model->commandShortcutsRegister.set(m_shortcuts);
    }

    void TearDown() override
    {
        delete m_model;
    }

    static ScoreOrder order(const String& id, const String& name)
    {
        ScoreOrder result;
        result.id = id;
        result.name = TranslatableString::untranslatable(name);

        return result;
    }

    QStringList itemIds() const
    {
        const MenuItemList& items = m_model->items();

        QStringList ids;
        ids.reserve(items.size());

        for (const MenuItem* item : items) {
            ids << item->id();
        }

        return ids;
    }

    const MenuItem* orderingMenu() const
    {
        return m_model->items().front();
    }

    LayoutPanelContextMenuModel* m_model = nullptr;

    std::shared_ptr<mu::project::ProjectSessionMock> m_session;
    std::shared_ptr<MasterNotationMock> m_masterNotation;
    std::shared_ptr<NotationMock> m_notation;
    std::shared_ptr<NotationMock> m_partNotation;
    std::shared_ptr<NotationPartsMock> m_masterParts;

    //! The orders in the repository, and the main score's
    ScoreOrderList m_orders;
    ScoreOrder m_scoreOrder;

    async::Notification m_currentNotationChanged;
    async::Notification m_scoreOrderChanged;

private:
    std::shared_ptr<InstrumentsRepositoryMock> m_repository;
    std::shared_ptr<muse::shortcuts::CommandShortcutsRegisterMock> m_shortcuts;
    muse::shortcuts::Shortcut m_noShortcut;
};
}

TEST_F(InstrumentsScene_LayoutPanelContextMenuModelTests, Load_MainScore)
{
    // [WHEN] The menu is loaded for the main score
    m_model->load();

    // [THEN] It has the ordering menu, a separator and the collapse/expand items
    const MenuItemList& items = m_model->items();
    ASSERT_EQ(items.size(), 4);

    EXPECT_EQ(items.at(0)->id(), QString("ordering-menu"));
    EXPECT_EQ(items.at(0)->translatedTitle(), QString("Instrument ordering"));

    EXPECT_TRUE(items.at(1)->id().isEmpty());

    EXPECT_EQ(items.at(2)->id(), QString("collapse-all-instruments"));
    EXPECT_EQ(items.at(2)->translatedTitle(), QString("Collapse all instruments"));

    EXPECT_EQ(items.at(3)->id(), QString("expand-all-instruments"));
    EXPECT_EQ(items.at(3)->translatedTitle(), QString("Expand all instruments"));

    // [THEN] The orders are listed with "Custom" last, and the score's order is checked
    const MenuItemList orders = items.at(0)->subitems();
    ASSERT_EQ(orders.size(), 3);

    EXPECT_EQ(orders.at(0)->translatedTitle(), QString("Orchestral"));
    EXPECT_TRUE(orders.at(0)->checked());

    EXPECT_EQ(orders.at(1)->translatedTitle(), QString("Choral"));
    EXPECT_FALSE(orders.at(1)->checked());

    EXPECT_EQ(orders.at(2)->translatedTitle(), QString("Custom"));
    EXPECT_FALSE(orders.at(2)->checked());
}

TEST_F(InstrumentsScene_LayoutPanelContextMenuModelTests, Load_PartScore)
{
    // [GIVEN] A part score is open
    ON_CALL(*m_session, currentNotation()).WillByDefault(Return(m_partNotation));

    // [WHEN] The menu is loaded
    m_model->load();

    // [THEN] Only the collapse/expand items are shown
    EXPECT_EQ(itemIds(), (QStringList { "collapse-all-instruments", "expand-all-instruments" }));
}

TEST_F(InstrumentsScene_LayoutPanelContextMenuModelTests, Load_NoScore)
{
    // [GIVEN] No score is open
    ON_CALL(*m_session, masterNotation()).WillByDefault(Return(nullptr));
    ON_CALL(*m_session, currentNotation()).WillByDefault(Return(nullptr));

    // [WHEN] The menu is loaded
    m_model->load();

    // [THEN] The menu is empty
    EXPECT_TRUE(m_model->items().empty());
}

TEST_F(InstrumentsScene_LayoutPanelContextMenuModelTests, Load_CustomOrderInRepository)
{
    // [GIVEN] The repository already has the custom order
    m_orders.push_back(customOrder());

    // [WHEN] The menu is loaded
    m_model->load();

    // [THEN] "Custom" is listed once, last
    const MenuItemList orders = orderingMenu()->subitems();
    ASSERT_EQ(orders.size(), 3);

    EXPECT_EQ(orders.at(2)->translatedTitle(), QString("Custom"));
    EXPECT_FALSE(orders.at(2)->checked());
}

TEST_F(InstrumentsScene_LayoutPanelContextMenuModelTests, Load_ScoreOrderNotInRepository)
{
    // [GIVEN] The score's order isn't in the repository
    m_scoreOrder = order(u"band", u"Band");

    // [WHEN] The menu is loaded
    m_model->load();

    // [THEN] The score's order is listed after "Custom", and is the only one checked
    const MenuItemList orders = orderingMenu()->subitems();
    ASSERT_EQ(orders.size(), 4);

    EXPECT_EQ(orders.at(0)->translatedTitle(), QString("Orchestral"));
    EXPECT_FALSE(orders.at(0)->checked());

    EXPECT_EQ(orders.at(1)->translatedTitle(), QString("Choral"));
    EXPECT_FALSE(orders.at(1)->checked());

    EXPECT_EQ(orders.at(2)->translatedTitle(), QString("Custom"));
    EXPECT_FALSE(orders.at(2)->checked());

    EXPECT_EQ(orders.at(3)->translatedTitle(), QString("Band"));
    EXPECT_TRUE(orders.at(3)->checked());
}

TEST_F(InstrumentsScene_LayoutPanelContextMenuModelTests, Load_CustomizedScoreOrder)
{
    // [GIVEN] The score uses a customized orchestral order
    m_scoreOrder.customized = true;

    // [WHEN] The menu is loaded
    m_model->load();

    // [THEN] The customized order follows the orchestral order, and is checked instead of it
    const MenuItemList orders = orderingMenu()->subitems();
    ASSERT_EQ(orders.size(), 4);

    EXPECT_EQ(orders.at(0)->translatedTitle(), QString("Orchestral"));
    EXPECT_FALSE(orders.at(0)->checked());

    EXPECT_EQ(orders.at(1)->translatedTitle(), QString("Orchestral (Customized)"));
    EXPECT_TRUE(orders.at(1)->checked());

    EXPECT_EQ(orders.at(2)->translatedTitle(), QString("Choral"));
    EXPECT_FALSE(orders.at(2)->checked());

    EXPECT_EQ(orders.at(3)->translatedTitle(), QString("Custom"));
    EXPECT_FALSE(orders.at(3)->checked());
}

TEST_F(InstrumentsScene_LayoutPanelContextMenuModelTests, CurrentNotationChanged)
{
    // [GIVEN] The menu of the main score
    m_model->load();

    // [WHEN] A part score is opened
    ON_CALL(*m_session, currentNotation()).WillByDefault(Return(m_partNotation));
    m_currentNotationChanged.notify();

    // [THEN] The ordering menu is gone
    EXPECT_EQ(itemIds(), (QStringList { "collapse-all-instruments", "expand-all-instruments" }));
}

TEST_F(InstrumentsScene_LayoutPanelContextMenuModelTests, ScoreOrderChanged)
{
    // [GIVEN] The menu of a score with the orchestral order
    m_model->load();

    // [WHEN] The score's order changes to choral
    m_scoreOrder = m_orders.at(1);
    m_scoreOrderChanged.notify();

    // [THEN] Only the choral order is checked
    const MenuItemList orders = orderingMenu()->subitems();
    ASSERT_EQ(orders.size(), 3);

    EXPECT_EQ(orders.at(0)->translatedTitle(), QString("Orchestral"));
    EXPECT_FALSE(orders.at(0)->checked());

    EXPECT_EQ(orders.at(1)->translatedTitle(), QString("Choral"));
    EXPECT_TRUE(orders.at(1)->checked());

    EXPECT_EQ(orders.at(2)->translatedTitle(), QString("Custom"));
    EXPECT_FALSE(orders.at(2)->checked());
}

TEST_F(InstrumentsScene_LayoutPanelContextMenuModelTests, HandleMenuItem_Order)
{
    // [GIVEN] The menu of a score with the orchestral order
    m_model->load();

    // [THEN] The score gets the choral order
    EXPECT_CALL(*m_masterParts, setScoreOrder(Field(&ScoreOrder::id, String(u"choral"))));

    // [WHEN] The choral order is clicked
    m_model->handleMenuItem("choral");

    // [THEN] Only the choral order is checked
    const MenuItemList orders = orderingMenu()->subitems();
    ASSERT_EQ(orders.size(), 3);

    EXPECT_EQ(orders.at(0)->translatedTitle(), QString("Orchestral"));
    EXPECT_FALSE(orders.at(0)->checked());

    EXPECT_EQ(orders.at(1)->translatedTitle(), QString("Choral"));
    EXPECT_TRUE(orders.at(1)->checked());

    EXPECT_EQ(orders.at(2)->translatedTitle(), QString("Custom"));
    EXPECT_FALSE(orders.at(2)->checked());
}

TEST_F(InstrumentsScene_LayoutPanelContextMenuModelTests, HandleMenuItem_ExpandCollapseAll)
{
    // [GIVEN] The menu of the main score
    m_model->load();

    std::vector<bool> requests;
    QObject::connect(m_model, &LayoutPanelContextMenuModel::expandCollapseAllRequested, m_model, [&requests](bool expand) {
        requests.push_back(expand);
    });

    // [THEN] The score isn't changed
    EXPECT_CALL(*m_masterParts, setScoreOrder(_)).Times(0);

    // [WHEN] "Collapse all" and then "Expand all" are clicked
    m_model->handleMenuItem("collapse-all-instruments");
    m_model->handleMenuItem("expand-all-instruments");

    // [THEN] The panel is asked to collapse, then expand all instruments
    EXPECT_EQ(requests, (std::vector<bool> { false, true }));
}
