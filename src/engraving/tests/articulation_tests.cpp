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

#include <memory>

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/rw/rwregister.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

TEST(Engraving_ArticulationTests, ChannelSurvivesSaveReload)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(u"articulation_data/channel.mscx"));
    ASSERT_TRUE(score);
    Chord* chord = toChord(score->firstMeasure()->firstChordRest(0));
    ASSERT_EQ(chord->articulations().size(), 3);
    for (const Articulation* articulation : chord->articulations()) {
        EXPECT_EQ(articulation->channelName(), u"pizzicato");
        EXPECT_FALSE(articulation->playArticulation());

        XmlReader xml(ScoreRW::writeElement(articulation));
        ASSERT_TRUE(xml.readNextStartElement());
        bool hasChannel = false;
        while (xml.readNextStartElement()) {
            hasChannel |= xml.name() == "channel";
            EXPECT_FALSE(xml.name() == "channe");
            xml.skipCurrentElement();
        }
        EXPECT_TRUE(hasChannel);
    }

    const String output(u"articulation-channel-test.mscx");
    ASSERT_TRUE(ScoreRW::saveScore(score.get(), output));
    score.reset();
    score.reset(ScoreRW::readScore(output, true));
    ASSERT_TRUE(score);
    chord = toChord(score->firstMeasure()->firstChordRest(0));
    ASSERT_EQ(chord->articulations().size(), 3);
    for (const Articulation* articulation : chord->articulations()) {
        EXPECT_EQ(articulation->channelName(), u"pizzicato");
        EXPECT_FALSE(articulation->playArticulation());
    }
}

TEST(Engraving_ArticulationTests, ReadHistoricalChannelSpelling)
{
    std::unique_ptr<MasterScore> score(compat::ScoreAccess::createMasterScore(nullptr));
    for (int version : { 400, 410, 460, 500 }) {
        SCOPED_TRACE(version);
        for (const char* data : {
            "<Articulation><channe name=\"pizzicato\"/><play>0</play></Articulation>",
            "<Ornament><channe name=\"pizzicato\"/><play>0</play></Ornament>"
        }) {
            SCOPED_TRACE(data);
            XmlReader xml { muse::ByteArray(data) };
            ASSERT_TRUE(xml.readNextStartElement());
            std::unique_ptr<EngravingItem> item(Factory::createItemByName(xml.name(), score->dummy()));
            ASSERT_TRUE(item);
            rw::RWRegister::reader(version)->readItem(item.get(), xml);
            const Articulation* articulation = static_cast<const Articulation*>(item.get());
            EXPECT_EQ(articulation->channelName(), u"pizzicato");
            EXPECT_FALSE(articulation->playArticulation());
        }
    }
}
