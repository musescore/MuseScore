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
#include "engraving/dom/mscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/sharedpart.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/text.h"

#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/roottreeitem.h"
#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/sharedparttreeitem.h"

#include "notation/tests/mocks/masternotationmock.h"
#include "notation/tests/mocks/notationmock.h"
#include "notation/tests/mocks/notationpartsmock.h"

using namespace ::testing;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;

namespace mu::instrumentsscene {
class InstrumentsScene_SharedPartTreeItemTests : public ::testing::Test
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

    EngravingItem* addElement(const Staff* staff)
    {
        Text* text = Factory::createText(m_score->dummy(), TextStyleType::DEFAULT, false);
        text->setTrack(staff->idx() * VOICES);
        m_elements.push_back(text);

        return text;
    }

    Part* addPart(const String& name)
    {
        Part* part = new Part(m_score);
        part->setLongNameAll(name);
        m_score->appendPart(part);
        m_score->appendStaff(Factory::createStaff(part));

        return part;
    }

    //! A shared-staves group right before its instruments, as EditStaveSharing creates it
    SharedPart* addSharedPart(const std::vector<String>& memberNames, const String& name = u"")
    {
        SharedPart* sharedPart = new SharedPart(m_score);
        sharedPart->setLongNameAll(name);
        m_score->appendPart(sharedPart);
        m_score->appendStaff(Factory::createStaff(sharedPart));

        for (const String& memberName : memberNames) {
            sharedPart->addOriginPart(addPart(memberName));
        }

        return sharedPart;
    }

    void setStaveSharingEnabled(bool enabled)
    {
        m_score->style().set(Sid::enableStaveSharing, enabled);
    }

    SharedPartTreeItem* buildSharedPartTreeItem(const SharedPart* sharedPart)
    {
        SharedPartTreeItem* item = new SharedPartTreeItem(m_masterNotation, m_notation, m_root);
        item->init(sharedPart);

        return item;
    }

    PartTreeItem* buildPartTreeItem(const Part* part)
    {
        PartTreeItem* item = new PartTreeItem(m_masterNotation, m_notation, m_root);
        item->init(part);

        return item;
    }

    static void notifyScoreChanged(AbstractLayoutPanelTreeItem* item, const ScoreChanges& changes = ScoreChanges())
    {
        item->onScoreChanged(changes);
    }

    MasterScore* m_score = nullptr;
    RootTreeItem* m_root = nullptr;
    std::shared_ptr<NotationPartsMock> m_parts;

private:
    std::vector<EngravingItem*> m_elements;

    std::shared_ptr<NotationMock> m_notation;
    std::shared_ptr<MasterNotationMock> m_masterNotation;
};
}

TEST_F(InstrumentsScene_SharedPartTreeItemTests, Init_Combined)
{
    // [GIVEN] Stave sharing is on, and three trumpets are combined
    setStaveSharingEnabled(true);
    const SharedPart* sharedPart = addSharedPart({ u"Trumpet 1", u"Trumpet 2", u"Trumpet 3" });

    // [THEN] Building the row doesn't change the score
    EXPECT_CALL(*m_parts, setSharedPartEnabled(_, _)).Times(0);

    // [WHEN] The group's row is built
    SharedPartTreeItem* item = buildSharedPartTreeItem(sharedPart);

    // [THEN] The row is named after the trumpets' number range
    EXPECT_EQ(item->id(), sharedPart->id());
    EXPECT_EQ(item->title(), QString("Trumpet 1–3"));

    // [THEN] It's enabled and expandable, with settings
    EXPECT_TRUE(item->isEnabled());
    EXPECT_TRUE(item->isExpandable());
    EXPECT_TRUE(item->settingsAvailable());
    EXPECT_TRUE(item->settingsEnabled());
}

TEST_F(InstrumentsScene_SharedPartTreeItemTests, Init_NotCombined)
{
    // [GIVEN] Stave sharing is on, but the group isn't combined
    setStaveSharingEnabled(true);
    SharedPart* sharedPart = addSharedPart({ u"Trumpet 1", u"Trumpet 2" });
    sharedPart->setProperty(Pid::SHARED_PART_ENABLED, false);

    // [THEN] Building the row doesn't change the score
    EXPECT_CALL(*m_parts, setSharedPartEnabled(_, _)).Times(0);

    // [WHEN] The group's row is built
    SharedPartTreeItem* item = buildSharedPartTreeItem(sharedPart);

    // [THEN] It's disabled and can't be expanded
    EXPECT_FALSE(item->isEnabled());
    EXPECT_FALSE(item->isExpandable());
}

TEST_F(InstrumentsScene_SharedPartTreeItemTests, Init_StaveSharingOff)
{
    // [GIVEN] A combined group, but stave sharing is off
    setStaveSharingEnabled(false);
    const SharedPart* sharedPart = addSharedPart({ u"Trumpet 1", u"Trumpet 2" });

    // [WHEN] The group's row is built
    SharedPartTreeItem* item = buildSharedPartTreeItem(sharedPart);

    // [THEN] It's disabled and can't be expanded
    EXPECT_FALSE(item->isEnabled());
    EXPECT_FALSE(item->isExpandable());
}

TEST_F(InstrumentsScene_SharedPartTreeItemTests, SetEnabled_CombinesOnScore)
{
    // [GIVEN] A group that isn't combined
    setStaveSharingEnabled(true);
    SharedPart* sharedPart = addSharedPart({ u"Trumpet 1", u"Trumpet 2" });
    sharedPart->setProperty(Pid::SHARED_PART_ENABLED, false);
    SharedPartTreeItem* item = buildSharedPartTreeItem(sharedPart);

    // [THEN] The group is combined on the score
    EXPECT_CALL(*m_parts, setSharedPartEnabled(sharedPart->id(), true));

    // [WHEN] Its switch is turned on
    item->setIsEnabled(true);
}

TEST_F(InstrumentsScene_SharedPartTreeItemTests, OnScoreChanged_FollowsCombinedState)
{
    // [GIVEN] A combined group
    setStaveSharingEnabled(true);
    SharedPart* sharedPart = addSharedPart({ u"Trumpet 1", u"Trumpet 2" });
    SharedPartTreeItem* item = buildSharedPartTreeItem(sharedPart);

    // [THEN] Following the score doesn't change it back
    EXPECT_CALL(*m_parts, setSharedPartEnabled(_, _)).Times(0);

    // [WHEN] The group is no longer combined on the score
    sharedPart->setProperty(Pid::SHARED_PART_ENABLED, false);
    notifyScoreChanged(item);

    // [THEN] The row follows
    EXPECT_FALSE(item->isEnabled());
    EXPECT_FALSE(item->isExpandable());
}

TEST_F(InstrumentsScene_SharedPartTreeItemTests, OnScoreChanged_UpdatesTitleWhenInstrumentChanged)
{
    // [GIVEN] A group of two trumpets
    SharedPart* sharedPart = addSharedPart({ u"Trumpet 1", u"Trumpet 2" });
    SharedPartTreeItem* item = buildSharedPartTreeItem(sharedPart);

    // [WHEN] Both trumpets are renamed on the score
    sharedPart->originParts().at(0)->setLongNameAll(u"Cornet 1");
    sharedPart->originParts().at(1)->setLongNameAll(u"Cornet 2");

    ScoreChanges changes;
    changes.changedObjects[sharedPart->originParts().at(0)] = {};
    notifyScoreChanged(item, changes);

    // [THEN] The title follows
    EXPECT_EQ(item->title(), QString("Cornet 1–2"));
}

TEST_F(InstrumentsScene_SharedPartTreeItemTests, OnScoreChanged_KeepsTitleOnOtherChanges)
{
    // [GIVEN] A group of two trumpets, and another instrument
    SharedPart* sharedPart = addSharedPart({ u"Trumpet 1", u"Trumpet 2" });
    Part* other = addPart(u"Flute");
    SharedPartTreeItem* item = buildSharedPartTreeItem(sharedPart);

    // [WHEN] A trumpet is renamed, but only the other instrument is reported as changed
    sharedPart->originParts().at(0)->setLongNameAll(u"Cornet 1");

    ScoreChanges changes;
    changes.changedObjects[other] = {};
    notifyScoreChanged(item, changes);

    // [THEN] The title isn't recomputed
    EXPECT_EQ(item->title(), QString("Trumpet 1–2"));
}

TEST_F(InstrumentsScene_SharedPartTreeItemTests, ApplyDrop_JoinsGroup)
{
    // [GIVEN] A group of two trumpets, and a third trumpet
    SharedPart* sharedPart = addSharedPart({ u"Trumpet 1", u"Trumpet 2" });
    const Part* part = addPart(u"Trumpet 3");
    SharedPartTreeItem* item = buildSharedPartTreeItem(sharedPart);
    const PartTreeItem* partItem = buildPartTreeItem(part);

    // [THEN] The third trumpet joins the group on the score
    EXPECT_CALL(*m_parts, movePartsToSharedPart(IDList { part->id() }, sharedPart->id()));

    // [WHEN] It's dropped on the group
    item->applyDrop(partItem);
}

TEST_F(InstrumentsScene_SharedPartTreeItemTests, IsSelectedInScore_NotSelected)
{
    // [GIVEN] A group of two trumpets
    SharedPart* sharedPart = addSharedPart({ u"Trumpet 1", u"Trumpet 2" });
    SharedPartTreeItem* item = buildSharedPartTreeItem(sharedPart);

    // [THEN] The header doesn't follow the selection of the combined staff, nor of its trumpets
    EXPECT_FALSE(item->isSelectedInScore({ addElement(sharedPart->staff(0)) }));
    EXPECT_FALSE(item->isSelectedInScore({ addElement(sharedPart->originParts().at(0)->staff(0)) }));
}
