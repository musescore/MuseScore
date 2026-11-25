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
#include <deque>

#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/stafftype.h"

#include "instrumentsscene/qml/MuseScore/InstrumentsScene/internal/instrumentlistmodel.h"

#include "notation/tests/mocks/instrumentsrepositorymock.h"

using namespace ::testing;
using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace mu::engraving;
using namespace muse;

namespace mu::instrumentsscene {
class InstrumentsScene_InstrumentListModelTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_repository = std::make_shared<NiceMock<InstrumentsRepositoryMock> >();

        ON_CALL(*m_repository, genres()).WillByDefault(ReturnRef(m_genres));
        ON_CALL(*m_repository, groups()).WillByDefault(ReturnRef(m_groups));
        ON_CALL(*m_repository, instrumentTemplates()).WillByDefault(ReturnRef(m_templates));
        ON_CALL(*m_repository, instrumentTemplate(_)).WillByDefault(Invoke([this](const String& id) -> const InstrumentTemplate& {
            for (const InstrumentTemplate* templ : m_templates) {
                if (templ->id == id) {
                    return *templ;
                }
            }

            return m_noTemplate;
        }));

        setUpCatalog();

        m_model = new InstrumentListModel();
        m_model->repository.set(m_repository);
    }

    void TearDown() override
    {
        delete m_model;

        instrumentGroups.erase(std::remove_if(instrumentGroups.begin(), instrumentGroups.end(), [this](const InstrumentGroup* group) {
            return std::find(m_groups.cbegin(), m_groups.cend(), group) != m_groups.cend();
        }), instrumentGroups.end());
    }

    void setUpCatalog()
    {
        // Common: a flute (woodwinds), a violin (strings), and a C and a B♭ trumpet (brass); orchestral only: a viola (strings)
        m_common.id = COMMON_GENRE_ID;
        m_common.name = u"Common";
        m_orchestral.id = u"orchestral";
        m_orchestral.name = u"Orchestral";
        m_genres = { &m_orchestral, &m_common };

        InstrumentGroup& woodwinds = addGroup(u"woodwinds", u"Woodwinds");
        InstrumentGroup& strings = addGroup(u"strings", u"Strings");
        InstrumentGroup& brass = addGroup(u"brass", u"Brass");

        addTemplate(u"flute", u"Flute", woodwinds, m_common).description = u"A flute";
        addTemplate(u"violin", u"Violin", strings, m_common);
        addTemplate(u"viola", u"Viola", strings, m_orchestral);

        InstrumentTemplate& trumpetC = addTemplate(u"trumpet-c", u"Trumpet", brass, m_common);
        trumpetC.trait.name = u"C";
        trumpetC.trait.type = TraitType::Transposition;
        trumpetC.description = u"A C trumpet";

        InstrumentTemplate& trumpetBb = addTemplate(u"trumpet-bb", u"Trumpet", brass, m_common);
        trumpetBb.trait.name = u"B♭";
        trumpetBb.trait.type = TraitType::Transposition;
        trumpetBb.trait.isDefault = true;
        trumpetBb.description = u"A B♭ trumpet";
    }

    void addGuitars()
    {
        // A guitar; its tablature version; a guitar based on it without tablature; and a second tablature version
        InstrumentGroup& plucked = addGroup(u"plucked", u"Plucked strings");

        addTemplate(u"guitar", u"Guitar", plucked, m_common);

        InstrumentTemplate& tablature = addTemplate(u"guitar-tablature", u"Guitar (tablature)", plucked, m_common);
        tablature.initId = u"guitar";
        tablature.staffTypePreset = StaffType::preset(StaffTypes::TAB_6COMMON);

        InstrumentTemplate& steel = addTemplate(u"guitar-steel", u"Steel guitar", plucked, m_common);
        steel.initId = u"guitar";

        InstrumentTemplate& simpleTablature = addTemplate(u"guitar-tablature-simple", u"Guitar (simple tablature)", plucked, m_common);
        simpleTablature.initId = u"guitar";
        simpleTablature.staffTypePreset = StaffType::preset(StaffTypes::TAB_6SIMPLE);
    }

    InstrumentGroup& addGroup(const String& id, const String& name)
    {
        InstrumentGroup& group = m_groupStore.emplace_back();
        group.id = id;
        group.name = name;

        m_groups.push_back(&group);
        instrumentGroups.push_back(&group);

        return group;
    }

    InstrumentTemplate& addTemplate(const String& id, const String& trackName, InstrumentGroup& group, const InstrumentGenre& genre)
    {
        InstrumentTemplate& templ = m_templateStore.emplace_back();
        templ.id = id;
        templ.trackName = trackName;
        templ.groupId = group.id;
        templ.genres = { &genre };
        templ.sequenceOrder = static_cast<int>(m_templates.size());

        group.instrumentTemplates.push_back(&templ);
        m_templates.push_back(&templ);

        return templ;
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

    InstrumentListModel* m_model = nullptr;

private:
    std::shared_ptr<InstrumentsRepositoryMock> m_repository;

    InstrumentGenre m_common;
    InstrumentGenre m_orchestral;
    InstrumentGenreList m_genres;

    std::deque<InstrumentGroup> m_groupStore;
    InstrumentGroupList m_groups;

    std::deque<InstrumentTemplate> m_templateStore;
    InstrumentTemplateList m_templates;
    InstrumentTemplate m_noTemplate;
};
}

// ---- load()

TEST_F(InstrumentsScene_InstrumentListModelTests, Load_ListsCommonGenreFirst)
{
    // [WHEN] The model is loaded
    m_model->load(false, QString());

    // [THEN] The common genre comes first, then all instruments, then the repository's other genres
    EXPECT_EQ(m_model->genres(), QStringList({ "Common", "All instruments", "Orchestral" }));
    EXPECT_EQ(m_model->currentGenreIndex(), 0);
}

TEST_F(InstrumentsScene_InstrumentListModelTests, Load_SelectsFirstGroup)
{
    // [WHEN] The model is loaded
    m_model->load(false, QString());

    // [THEN] The groups with common instruments are listed, and the first one is shown
    EXPECT_EQ(m_model->groups(), QStringList({ "Woodwinds", "Strings", "Brass" }));
    EXPECT_EQ(m_model->currentGroupIndex(), 0);
    EXPECT_EQ(rowNames(), QStringList({ "Flute" }));
}

TEST_F(InstrumentsScene_InstrumentListModelTests, Load_CurrentInstrument_SelectsItsGroup)
{
    // [WHEN] The model is loaded for replacing a violin
    m_model->load(false, "violin");

    // [THEN] The violin's group is shown, with its common instruments only
    EXPECT_EQ(m_model->currentGroupIndex(), 1);
    EXPECT_EQ(rowNames(), QStringList({ "Violin" }));
}

// ---- Genres and groups

TEST_F(InstrumentsScene_InstrumentListModelTests, SetCurrentGenreIndex_FiltersGroups)
{
    // [GIVEN] The woodwinds are shown
    m_model->load(false, QString());

    // [WHEN] The orchestral genre is chosen
    m_model->setCurrentGenreIndex(2);

    // [THEN] Only its group is listed, and shown instead of the woodwinds
    EXPECT_EQ(m_model->groups(), QStringList({ "Strings" }));
    EXPECT_EQ(m_model->currentGroupIndex(), 0);
    EXPECT_EQ(rowNames(), QStringList({ "Viola" }));
}

TEST_F(InstrumentsScene_InstrumentListModelTests, SetCurrentGroupIndex_ListsInSequenceOrder)
{
    // [GIVEN] All instruments are shown
    m_model->load(false, QString());
    m_model->setCurrentGenreIndex(1);

    // [WHEN] The strings are chosen
    m_model->setCurrentGroupIndex(1);

    // [THEN] All of them are listed, in the templates' order
    EXPECT_EQ(rowNames(), QStringList({ "Violin", "Viola" }));
}

// ---- Traits

TEST_F(InstrumentsScene_InstrumentListModelTests, Load_SameName_CombinesTraits)
{
    // [WHEN] The brass is shown
    m_model->load(false, QString());
    m_model->setCurrentGroupIndex(2);

    // [THEN] The trumpets are one row, with the default trait first and chosen
    EXPECT_EQ(rowNames(), QStringList({ "Trumpet" }));
    EXPECT_EQ(rowData(0, "traits").toStringList(), QStringList({ "B♭", "C" }));
    EXPECT_EQ(rowData(0, "currentTraitIndex").toInt(), 0);
    EXPECT_EQ(rowData(0, "description").toString(), "A B♭ trumpet");
}

TEST_F(InstrumentsScene_InstrumentListModelTests, SetData_CurrentTraitIndex_SelectsTemplate)
{
    // [GIVEN] The brass is shown
    m_model->load(false, QString());
    m_model->setCurrentGroupIndex(2);

    // [WHEN] The C trait is chosen
    EXPECT_TRUE(m_model->setData(m_model->index(0), 1, role("currentTraitIndex")));

    // [THEN] The C trumpet is shown and selected
    EXPECT_EQ(rowData(0, "currentTraitIndex").toInt(), 1);
    EXPECT_EQ(rowData(0, "description").toString(), "A C trumpet");
    EXPECT_EQ(m_model->selectedInstrumentIdList(), QStringList({ "trumpet-c" }));
}

// ---- Selection

TEST_F(InstrumentsScene_InstrumentListModelTests, SelectInstrument_SelectsRow)
{
    // [GIVEN] The woodwinds are shown
    m_model->load(false, QString());

    // [WHEN] The flute is selected
    m_model->selectInstrument(0);

    // [THEN] It's selected, and described
    EXPECT_TRUE(m_model->hasSelection());
    EXPECT_TRUE(rowData(0, "isSelected").toBool());
    EXPECT_EQ(m_model->selectedInstrumentIdList(), QStringList({ "flute" }));

    const QVariantMap selected = m_model->selectedInstrument().toMap();
    EXPECT_EQ(selected.value("instrumentId").toString(), "flute");
    EXPECT_EQ(selected.value("description").toString(), "A flute");
}

// ---- Search

TEST_F(InstrumentsScene_InstrumentListModelTests, SetSearchText_SortsByMatchPosition)
{
    // [GIVEN] The woodwinds are shown
    m_model->load(false, QString());

    // [WHEN] "T" is searched for
    m_model->setSearchText("T");

    // [THEN] Instruments of all groups that contain it, in any case, are listed, earliest match first
    EXPECT_EQ(rowNames(), QStringList({ "Trumpet", "Flute" }));
}

TEST_F(InstrumentsScene_InstrumentListModelTests, SetSearchText_Cleared_RestoresGenreAndGroup)
{
    // [GIVEN] The common strings are shown
    m_model->load(false, QString());
    m_model->setCurrentGroupIndex(1);

    // [WHEN] "vi" is searched for
    m_model->setSearchText("vi");

    // [THEN] All instruments are searched, in no group
    EXPECT_EQ(m_model->currentGenreIndex(), 1);
    EXPECT_EQ(m_model->currentGroupIndex(), -1);
    EXPECT_EQ(rowNames(), QStringList({ "Violin", "Viola" }));

    // [WHEN] The search is cleared
    m_model->setSearchText(QString());

    // [THEN] The common strings are shown again
    EXPECT_EQ(m_model->currentGenreIndex(), 0);
    EXPECT_EQ(m_model->currentGroupIndex(), 1);
    EXPECT_EQ(rowNames(), QStringList({ "Violin" }));
}

// ---- Tablature versions

TEST_F(InstrumentsScene_InstrumentListModelTests, Load_Adding_HidesTablatureVersion)
{
    // [GIVEN] A guitar with a tablature version, another tablature version and a guitar based on it
    addGuitars();

    // [WHEN] The model is loaded for adding instruments, and the guitars are shown
    m_model->load(true, QString());
    m_model->setCurrentGroupIndex(3);

    // [THEN] Only the tablature version that can be chosen on the guitar itself is hidden
    EXPECT_EQ(rowNames(), QStringList({ "Guitar", "Steel guitar", "Guitar (simple tablature)" }));
}

TEST_F(InstrumentsScene_InstrumentListModelTests, Load_Replacing_ListsTablatureVersion)
{
    // [GIVEN] A guitar with a tablature version, another tablature version and a guitar based on it
    addGuitars();

    // [WHEN] The model is loaded for replacing an instrument, and the guitars are shown
    m_model->load(false, QString());
    m_model->setCurrentGroupIndex(3);

    // [THEN] All of them are listed
    EXPECT_EQ(rowNames(), QStringList({ "Guitar", "Guitar (tablature)", "Steel guitar", "Guitar (simple tablature)" }));
}

TEST_F(InstrumentsScene_InstrumentListModelTests, SetSearchText_Adding_HidesTablatureVersion)
{
    // [GIVEN] A guitar with a tablature version, another tablature version and a guitar based on it; adding instruments
    addGuitars();
    m_model->load(true, QString());

    // [WHEN] "guitar" is searched for
    m_model->setSearchText("guitar");

    // [THEN] The tablature version is hidden from the results too
    EXPECT_EQ(rowNames(), QStringList({ "Guitar", "Guitar (simple tablature)", "Steel guitar" }));
}
