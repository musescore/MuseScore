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

#include <gtest/gtest.h>

#include <map>
#include <memory>

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/rw/xmlreader.h"
#include "engraving/types/typesconv.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

//---------------------------------------------------------
// A styled property the user has overridden (UNSTYLED) must survive a
// write/read round trip: the value is restored, it is still UNSTYLED
// (so the next save keeps it too), and it is written exactly once.
//---------------------------------------------------------

namespace {
// A valid value different from `current`, used to override a styled property
PropertyValue overrideValue(Pid pid, const PropertyValue& current)
{
    switch (pid) {
    // font styles are stored as INT
    case Pid::FONT_STYLE:
    case Pid::BEGIN_FONT_STYLE:
    case Pid::CONTINUE_FONT_STYLE:
    case Pid::END_FONT_STYLE:
        return current.toInt() ^ int(FontStyle::Bold);
    default:
        break;
    }

    switch (current.type()) {
    case P_TYPE::BOOL:        return !current.toBool();
    case P_TYPE::REAL:        return current.toReal() + 0.5;
    case P_TYPE::SPATIUM:     return Spatium(current.value<Spatium>().val() + 0.5);
    case P_TYPE::STRING:      return current.value<String>() + u"x";
    case P_TYPE::POINT:       return current.value<PointF>() + PointF(1.0, 1.0);
    case P_TYPE::COLOR:       return current.value<Color>() == Color::RED ? Color::BLUE : Color::RED;
    case P_TYPE::PLACEMENT_V: return current.value<PlacementV>() == PlacementV::ABOVE ? PlacementV::BELOW : PlacementV::ABOVE;
    case P_TYPE::ALIGN_H:     return current.value<AlignH>() == AlignH::LEFT ? AlignH::RIGHT : AlignH::LEFT;
    case P_TYPE::ALIGN: {
        Align align = current.value<Align>();
        align.horizontal = align.horizontal == AlignH::LEFT ? AlignH::RIGHT : AlignH::LEFT;
        return align;
    }
    case P_TYPE::LINE_TYPE:   return current.value<LineType>() == LineType::SOLID ? LineType::DASHED : LineType::SOLID;
    case P_TYPE::HOOK_TYPE:   return current.value<HookType>() == HookType::HOOK_90 ? HookType::HOOK_45 : HookType::HOOK_90;
    case P_TYPE::GLISS_TYPE:  return current.value<GlissandoType>() == GlissandoType::WAVY ? GlissandoType::STRAIGHT : GlissandoType::WAVY;
    case P_TYPE::GLISS_STYLE:
        return current.value<GlissandoStyle>() == GlissandoStyle::PORTAMENTO ? GlissandoStyle::CHROMATIC : GlissandoStyle::PORTAMENTO;
    default:
        return PropertyValue();
    }
}

void expectSameValue(const PropertyValue& actual, const PropertyValue& expected)
{
    if (expected.type() == P_TYPE::POINT) {
        // offsets are written in spatium units, so allow for rounding
        EXPECT_NEAR(actual.value<PointF>().x(), expected.value<PointF>().x(), 0.001);
        EXPECT_NEAR(actual.value<PointF>().y(), expected.value<PointF>().y(), 0.001);
        return;
    }
    EXPECT_TRUE(actual == expected) << "value not restored";
}

// How many times each tag appears directly under the root element
std::map<std::string, int> childTagCounts(const muse::ByteArray& xmlData)
{
    std::map<std::string, int> counts;
    XmlReader xml(xmlData);
    xml.readNextStartElement();
    while (xml.readNextStartElement()) {
        ++counts[xml.name().ascii()];
        xml.skipCurrentElement();
    }
    return counts;
}
}

class Engraving_StyledPropertiesRWTests : public ::testing::TestWithParam<ElementType>
{
protected:
    void SetUp() override
    {
        m_score = compat::ScoreAccess::createMasterScore(nullptr);
    }

    void TearDown() override
    {
        delete m_score;
    }

    std::unique_ptr<EngravingItem> createItem() const
    {
        return std::unique_ptr<EngravingItem>(Factory::createItem(GetParam(), m_score->dummy()));
    }

    static void overrideProperty(EngravingItem* item, Pid pid)
    {
        const PropertyValue current = item->getProperty(pid);
        ASSERT_TRUE(current.isValid()) << "in the style table, but getProperty() doesn't support it";
        const PropertyValue value = overrideValue(pid, current);
        ASSERT_TRUE(value.isValid()) << "overrideValue() has no case for this property type";
        item->setProperty(pid, value);
        item->setPropertyFlags(pid, PropertyFlags::UNSTYLED);
    }

    MasterScore* m_score = nullptr;
};

TEST_P(Engraving_StyledPropertiesRWTests, overriddenPropertySurvivesRoundTrip)
{
    for (const StyledProperty& sp : *createItem()->styledProperties()) {
        SCOPED_TRACE(propertyName(sp.pid));

        std::unique_ptr<EngravingItem> item = createItem();
        overrideProperty(item.get(), sp.pid);
        const PropertyValue value = item->getProperty(sp.pid);

        std::unique_ptr<EngravingItem> readItem(ScoreRW::writeReadElement(item.get()));

        EXPECT_EQ(readItem->propertyFlags(sp.pid), PropertyFlags::UNSTYLED);
        expectSameValue(readItem->getProperty(sp.pid), value);
    }
}

TEST_P(Engraving_StyledPropertiesRWTests, eachPropertyWrittenOnce)
{
    std::unique_ptr<EngravingItem> item = createItem();
    for (const StyledProperty& sp : *item->styledProperties()) {
        SCOPED_TRACE(propertyName(sp.pid));
        overrideProperty(item.get(), sp.pid);
    }

    for (const auto& [tag, count] : childTagCounts(ScoreRW::writeElement(item.get()))) {
        if (tag != "Segment") {
            EXPECT_EQ(count, 1) << "<" << tag << "> written " << count << " times";
        }
    }
}

// All SLine subclasses, except those that can't be written and read back on their own
// with ScoreRW::writeReadElement():
// - Glissando, GuitarBend, NoteLine: note-anchored, so they can't be written without start/end notes
// - Volta: measure-anchored, its start/end measure references can't be resolved outside a score
// - PickScrape: no reader handles it (TRead::readItem has no case for it)
// - GuitarBendHold: never written, it is regenerated during layout
// Spanner segments aren't standalone elements either: they're only written inside their spanner.
INSTANTIATE_TEST_SUITE_P(StyledPropertiesRW, Engraving_StyledPropertiesRWTests,
                         ::testing::Values(ElementType::GRADUAL_TEMPO_CHANGE,
                                           ElementType::HAIRPIN,
                                           ElementType::HARMONIC_MARK,
                                           ElementType::LET_RING,
                                           ElementType::LYRICSLINE,
                                           ElementType::OTTAVA,
                                           ElementType::PALM_MUTE,
                                           ElementType::PARTIAL_LYRICSLINE,
                                           ElementType::PEDAL,
                                           ElementType::RASGUEADO,
                                           ElementType::TEXTLINE,
                                           ElementType::TRILL,
                                           ElementType::VIBRATO,
                                           ElementType::WHAMMY_BAR),
                         [](const ::testing::TestParamInfo<ElementType>& info) {
    return std::string(TConv::toXml(info.param).ascii());
});
