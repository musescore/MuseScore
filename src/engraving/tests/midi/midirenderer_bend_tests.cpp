/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "utils/scorerw.h"
#include "engraving/compat/midi/compatmidirender.h"
#include "engraving/rw/mscloader.h"

using namespace mu;
using namespace mu::engraving;
class MidiRendererBend_Tests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_useRead302 = MScore::useRead302InTestMode;
        MScore::useRead302InTestMode = false;
    }

    void TearDown() override
    {
        MScore::useRead302InTestMode = m_useRead302;
    }

private:
    bool m_useRead302 = false;
};

static const String MIDIRENDERER_BEND_TESTS_DIR = u"midi/midirenderer_bend_data/";
static constexpr int DEFAULT_CHANNEL = 0;
static constexpr int NOTE_OFF_VOLUME = 0;

static void checkEventInterval(EventsHolder& events, int tickStart, int tickEnd, int pitch, int volume,
                               MidiInstrumentEffect effect = MidiInstrumentEffect::NONE,
                               int channel = DEFAULT_CHANNEL)
{
    auto it = events[channel].find(tickStart);
    EXPECT_NE(it, events[channel].end());
    if (it == events[channel].end()) {
        return;
    }

    EXPECT_EQ(it->second.pitch(), pitch);
    EXPECT_EQ(it->second.velo(), volume);
    EXPECT_EQ(it->second.channel(), channel);
    EXPECT_EQ(it->second.effect(), effect);

    events[channel].erase(it);

    it = events[channel].find(tickEnd);
    EXPECT_NE(it, events[channel].end());
    if (it == events[channel].end()) {
        return;
    }

    EXPECT_EQ(it->second.pitch(), pitch);
    EXPECT_EQ(it->second.velo(), NOTE_OFF_VOLUME);
    EXPECT_EQ(it->second.channel(), channel);
    EXPECT_EQ(it->second.effect(), effect);

    events[channel].erase(it);
}

static void checkPitchBend(EventsHolder& events, int tick, int pitchValue, MidiInstrumentEffect effect = MidiInstrumentEffect::NONE,
                           int channel = DEFAULT_CHANNEL)
{
    auto it = events[channel].find(tick);
    EXPECT_NE(it, events[channel].end());
    if (it == events[channel].end()) {
        return;
    }

    int dataA = pitchValue % 128;
    int dataB = pitchValue / 128;
    EXPECT_EQ(it->second.dataA(), dataA);
    EXPECT_EQ(it->second.dataB(), dataB);
    EXPECT_EQ(it->second.channel(), channel);
    EXPECT_EQ(it->second.effect(), effect);

    events[channel].erase(it);
}

static EventsHolder renderMidiEvents(const String& fileName, bool eachStringHasChannel = false, bool instrumentsHaveEffects = false)
{
    MasterScore* score = ScoreRW::readScore(MIDIRENDERER_BEND_TESTS_DIR + fileName);
    EXPECT_TRUE(score);

    EventsHolder events;
    CompatMidiRendererInternal::Context ctx;

    ctx.eachStringHasChannel = eachStringHasChannel;
    ctx.instrumentsHaveEffects = instrumentsHaveEffects;
    CompatMidiRender::renderScore(score, events, ctx, true);

    return events;
}

TEST_F(MidiRendererBend_Tests, simple)
{
    constexpr int defVol = 80; // mf
    constexpr int pwReset = 8192;

    EventsHolder events = renderMidiEvents(u"simple.mscx", true, true);

    EXPECT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].size(), 45);

    /// bend with startFactor 0, endFactor 1
    checkEventInterval(events, 0, 239, 52, defVol);
    checkPitchBend(events, 0, 8192);
    checkPitchBend(events, 10, 8201);
    checkPitchBend(events, 20, 8229);
    checkPitchBend(events, 30, 8277);
    checkPitchBend(events, 40, 8343);
    checkPitchBend(events, 50, 8429);
    checkPitchBend(events, 60, 8533);
    checkPitchBend(events, 70, 8656);
    checkPitchBend(events, 80, 8798);
    checkPitchBend(events, 90, 8960);
    checkPitchBend(events, 100, 9140);
    checkPitchBend(events, 110, 9339);
    checkPitchBend(events, 120, 9557);

    /// bend with startFactor 0.25, endFactor 0.75
    checkPitchBend(events, 470, pwReset);
    checkEventInterval(events, 480, 719, 52, defVol);
    checkPitchBend(events, 480, 8192);
    checkPitchBend(events, 510, 8192);
    checkPitchBend(events, 520, 8229);
    checkPitchBend(events, 530, 8343);
    checkPitchBend(events, 540, 8533);
    checkPitchBend(events, 550, 8798);
    checkPitchBend(events, 560, 9140);
    checkPitchBend(events, 570, 9557);
    checkPitchBend(events, 600, 9557);

    /// bend with 2+1/4 tones
    checkPitchBend(events, 950, pwReset);
    checkEventInterval(events, 960, 1199, 52, defVol);
    checkPitchBend(events, 960, 8192);
    checkPitchBend(events, 970, 8213);
    checkPitchBend(events, 980, 8277);
    checkPitchBend(events, 990, 8384);
    checkPitchBend(events, 1000, 8533);
    checkPitchBend(events, 1010, 8725);
    checkPitchBend(events, 1020, 8960);
    checkPitchBend(events, 1030, 9237);
    checkPitchBend(events, 1040, 9557);
    checkPitchBend(events, 1050, 9920);
    checkPitchBend(events, 1060, 10325);
    checkPitchBend(events, 1070, 10773);
    checkPitchBend(events, 1080, 11264);

    EXPECT_EQ(events[0].size(), 2);
}

TEST_F(MidiRendererBend_Tests, combined)
{
    constexpr int defVol = 80; // mf
    constexpr int pwReset = 8192;

    EventsHolder events = renderMidiEvents(u"combined.mscx", true, true);

    EXPECT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].size(), 73);

    /// bend+bend
    checkEventInterval(events, 0, 479, 52, defVol);
    checkPitchBend(events, 0, 8192);
    checkPitchBend(events, 10, 8201);
    checkPitchBend(events, 20, 8229);
    checkPitchBend(events, 30, 8277);
    checkPitchBend(events, 40, 8343);
    checkPitchBend(events, 50, 8429);
    checkPitchBend(events, 60, 8533);
    checkPitchBend(events, 70, 8656);
    checkPitchBend(events, 80, 8798);
    checkPitchBend(events, 90, 8960);
    checkPitchBend(events, 100, 9140);
    checkPitchBend(events, 110, 9339);
    checkPitchBend(events, 120, 9557); /// 1 tone up
    checkPitchBend(events, 130, 9566);
    checkPitchBend(events, 140, 9595);
    checkPitchBend(events, 150, 9642);
    checkPitchBend(events, 160, 9709);
    checkPitchBend(events, 170, 9794);
    checkPitchBend(events, 180, 9898);
    checkPitchBend(events, 190, 10021);
    checkPitchBend(events, 200, 10164);
    checkPitchBend(events, 210, 10325);
    checkPitchBend(events, 220, 10505);
    checkPitchBend(events, 230, 10704);
    checkPitchBend(events, 240, 10922); /// 2 tones up

    /// bend+release
    checkPitchBend(events, 710, pwReset);
    checkEventInterval(events, 720, 1199, 52, defVol);
    checkPitchBend(events, 720, 8192);
    checkPitchBend(events, 750, 8192);
    checkPitchBend(events, 760, 8258);
    checkPitchBend(events, 770, 8457);
    checkPitchBend(events, 780, 8789);
    checkPitchBend(events, 790, 9253);
    checkPitchBend(events, 800, 9851);
    checkPitchBend(events, 810, 10581);
    checkPitchBend(events, 840, 10581);
    checkPitchBend(events, 850, 10576);
    checkPitchBend(events, 860, 10562);
    checkPitchBend(events, 870, 10538);
    checkPitchBend(events, 880, 10505);
    checkPitchBend(events, 890, 10462);
    checkPitchBend(events, 900, 10410);
    checkPitchBend(events, 910, 10349);
    checkPitchBend(events, 920, 10277);
    checkPitchBend(events, 930, 10197);
    checkPitchBend(events, 940, 10107);
    checkPitchBend(events, 950, 10007);
    checkPitchBend(events, 960, 9898);

    /// bend+note
    checkPitchBend(events, 1430, pwReset);
    checkEventInterval(events, 1440, 1679, 52, defVol);
    checkPitchBend(events, 1440, 8192);
    checkPitchBend(events, 1450, 8201);
    checkPitchBend(events, 1460, 8229);
    checkPitchBend(events, 1470, 8277);
    checkPitchBend(events, 1480, 8343);
    checkPitchBend(events, 1490, 8429);
    checkPitchBend(events, 1500, 8533);
    checkPitchBend(events, 1510, 8656);
    checkPitchBend(events, 1520, 8798);
    checkPitchBend(events, 1530, 8960);
    checkPitchBend(events, 1540, 9140);
    checkPitchBend(events, 1550, 9339);
    checkPitchBend(events, 1560, 9557);

    checkPitchBend(events, 1670, pwReset);
    checkEventInterval(events, 1680, 1919, 52, defVol);

    EXPECT_EQ(events[0].size(), 3);
}

TEST_F(MidiRendererBend_Tests, tied)
{
    constexpr int defVol = 80; // mf
    EventsHolder events = renderMidiEvents(u"tied.mscx", true, true);

    EXPECT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].size(), 52);

    checkEventInterval(events, 0, 1919, 52, defVol);
    checkPitchBend(events, 0, 8192);
    checkPitchBend(events, 10, 8196);
    checkPitchBend(events, 20, 8210);
    checkPitchBend(events, 30, 8234);
    checkPitchBend(events, 40, 8267);
    checkPitchBend(events, 50, 8310);
    checkPitchBend(events, 60, 8362);
    checkPitchBend(events, 70, 8424);
    checkPitchBend(events, 80, 8495);
    checkPitchBend(events, 90, 8576);

    checkPitchBend(events, 100, 8666);
    checkPitchBend(events, 110, 8765);
    checkPitchBend(events, 120, 8874);
    checkPitchBend(events, 130, 8993);
    checkPitchBend(events, 140, 9121);
    checkPitchBend(events, 150, 9258);
    checkPitchBend(events, 160, 9405);
    checkPitchBend(events, 170, 9562);
    checkPitchBend(events, 180, 9728);
    checkPitchBend(events, 190, 9903);

    checkPitchBend(events, 200, 10088);
    checkPitchBend(events, 210, 10282);
    checkPitchBend(events, 220, 10486);
    checkPitchBend(events, 230, 10699);
    checkPitchBend(events, 240, 10922);

    checkPitchBend(events, 480, 10922);
    checkPitchBend(events, 490, 10917);
    checkPitchBend(events, 500, 10903);
    checkPitchBend(events, 510, 10880);
    checkPitchBend(events, 520, 10846);
    checkPitchBend(events, 530, 10804);
    checkPitchBend(events, 540, 10752);
    checkPitchBend(events, 550, 10690);
    checkPitchBend(events, 560, 10619);
    checkPitchBend(events, 570, 10538);
    checkPitchBend(events, 580, 10448);
    checkPitchBend(events, 590, 10349);

    checkPitchBend(events, 600, 10240);
    checkPitchBend(events, 610, 10121);
    checkPitchBend(events, 620, 9993);
    checkPitchBend(events, 630, 9856);
    checkPitchBend(events, 640, 9709);
    checkPitchBend(events, 650, 9552);
    checkPitchBend(events, 660, 9386);
    checkPitchBend(events, 670, 9211);
    checkPitchBend(events, 680, 9026);
    checkPitchBend(events, 690, 8832);

    checkPitchBend(events, 700, 8628);
    checkPitchBend(events, 710, 8414);
    checkPitchBend(events, 720, 8192);

    EXPECT_EQ(events[0].size(), 0);
}

TEST_F(MidiRendererBend_Tests, prebend)
{
    constexpr int defVol = 80; // mf
    constexpr int pwReset = 8192;

    EventsHolder events = renderMidiEvents(u"prebend.mscx", true, true);

    EXPECT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].size(), 35);

    /// prebend only
    checkEventInterval(events, 0, 239, 52, defVol);

    /// prebend+bend
    checkEventInterval(events, 480, 719, 52, defVol);
    checkPitchBend(events, 480, 8192);
    checkPitchBend(events, 490, 8201);
    checkPitchBend(events, 500, 8229);
    checkPitchBend(events, 510, 8277);
    checkPitchBend(events, 520, 8343);
    checkPitchBend(events, 530, 8429);
    checkPitchBend(events, 540, 8533);
    checkPitchBend(events, 550, 8656);
    checkPitchBend(events, 560, 8798);
    checkPitchBend(events, 570, 8960);
    checkPitchBend(events, 580, 9140);
    checkPitchBend(events, 590, 9339);
    checkPitchBend(events, 600, 9557);

    /// prebend+tie+bend
    checkPitchBend(events, 950, pwReset);
    checkEventInterval(events, 960, 1319, 52, defVol);
    checkPitchBend(events, 1080, 8192);
    checkPitchBend(events, 1090, 8201);
    checkPitchBend(events, 1100, 8229);
    checkPitchBend(events, 1110, 8277);
    checkPitchBend(events, 1120, 8343);
    checkPitchBend(events, 1130, 8429);
    checkPitchBend(events, 1140, 8533);
    checkPitchBend(events, 1150, 8656);
    checkPitchBend(events, 1160, 8798);
    checkPitchBend(events, 1170, 8960);
    checkPitchBend(events, 1180, 9140);
    checkPitchBend(events, 1190, 9339);
    checkPitchBend(events, 1200, 9557);

    EXPECT_EQ(events[0].size(), 2);
}

TEST_F(MidiRendererBend_Tests, slightbend)
{
    constexpr int defVol = 80; // mf
    constexpr int pwReset = 8192;

    EventsHolder events = renderMidiEvents(u"slightbend.mscx", true, true);

    EXPECT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].size(), 46);

    /// slight bend 0 -> 1
    checkEventInterval(events, 0, 119, 52, defVol);
    checkPitchBend(events, 0, 8192);
    checkPitchBend(events, 10, 8194);
    checkPitchBend(events, 20, 8201);
    checkPitchBend(events, 30, 8213);
    checkPitchBend(events, 40, 8229);
    checkPitchBend(events, 50, 8251);
    checkPitchBend(events, 60, 8277);
    checkPitchBend(events, 70, 8308);
    checkPitchBend(events, 80, 8343);
    checkPitchBend(events, 90, 8384);
    checkPitchBend(events, 100, 8429);
    checkPitchBend(events, 110, 8478);

    /// slight bend 0.25 -> 0.75
    checkPitchBend(events, 230, pwReset);
    checkEventInterval(events, 240, 359, 52, defVol);
    checkPitchBend(events, 240, 8192);
    checkPitchBend(events, 270, 8192);
    checkPitchBend(events, 280, 8201);
    checkPitchBend(events, 290, 8229);
    checkPitchBend(events, 300, 8277);
    checkPitchBend(events, 310, 8343);
    checkPitchBend(events, 320, 8429);
    checkPitchBend(events, 330, 8533);

    /// slight bend + note
    checkPitchBend(events, 470, pwReset);
    checkEventInterval(events, 480, 599, 52, defVol);
    checkPitchBend(events, 480, 8192);
    checkPitchBend(events, 490, 8194);
    checkPitchBend(events, 500, 8201);
    checkPitchBend(events, 510, 8213);
    checkPitchBend(events, 520, 8229);
    checkPitchBend(events, 530, 8251);
    checkPitchBend(events, 540, 8277);
    checkPitchBend(events, 550, 8308);
    checkPitchBend(events, 560, 8343);
    checkPitchBend(events, 570, 8384);
    checkPitchBend(events, 580, 8429);
    checkPitchBend(events, 590, 8478);
    checkPitchBend(events, 595, pwReset);
    checkEventInterval(events, 600, 719, 52, defVol);
    EXPECT_EQ(events[0].size(), 3);
}

TEST_F(MidiRendererBend_Tests, slides)
{
    constexpr int defVol = 80; // mf
    constexpr int pwReset = 8192;

    EventsHolder events = renderMidiEvents(u"slides.mscx", true, true);

    EXPECT_EQ(events.size(), 4);
    EXPECT_EQ(events[0].size(), 28);
    EXPECT_EQ(events[1].size(), 18);
    EXPECT_EQ(events[2].size(), 3);
    EXPECT_EQ(events[3].size(), 9);

    checkEventInterval(events, 0, 179, 52, defVol);
    checkPitchBend(events, 0, 8192);
    checkPitchBend(events, 10, 8201);
    checkPitchBend(events, 20, 8229);
    checkPitchBend(events, 30, 8277);
    checkPitchBend(events, 40, 8343);
    checkPitchBend(events, 50, 8429);
    checkPitchBend(events, 60, 8533);
    checkPitchBend(events, 70, 8656);
    checkPitchBend(events, 80, 8798);
    checkPitchBend(events, 90, 8960);
    checkPitchBend(events, 100, 9140);
    checkPitchBend(events, 110, 9339);
    checkPitchBend(events, 120, 9557);
    checkEventInterval(events, 180, 198, 53, defVol, MidiInstrumentEffect::SLIDE, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 199, 217, 52, defVol, MidiInstrumentEffect::SLIDE, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 219, 237, 51, defVol, MidiInstrumentEffect::SLIDE, DEFAULT_CHANNEL + 1);

    checkEventInterval(events, 420, 438, 55, defVol, MidiInstrumentEffect::SLIDE, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 440, 458, 54, defVol, MidiInstrumentEffect::SLIDE, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 460, 478, 53, defVol, MidiInstrumentEffect::SLIDE, DEFAULT_CHANNEL + 1);
    checkPitchBend(events, 470, pwReset);
    checkEventInterval(events, 480, 719, 52, defVol);
    checkPitchBend(events, 480, 8192);
    checkPitchBend(events, 510, 8192);
    checkPitchBend(events, 520, 8229);
    checkPitchBend(events, 530, 8343);
    checkPitchBend(events, 540, 8533);
    checkPitchBend(events, 550, 8798);
    checkPitchBend(events, 560, 9140);
    checkPitchBend(events, 570, 9557);
    checkPitchBend(events, 600, 9557);

    checkEventInterval(events, 960, 1019, 52, defVol, MidiInstrumentEffect::NONE, DEFAULT_CHANNEL + 2);
    checkEventInterval(events, 1020, 1038, 53, defVol, MidiInstrumentEffect::SLIDE, DEFAULT_CHANNEL + 3);
    checkEventInterval(events, 1039, 1057, 54, defVol, MidiInstrumentEffect::SLIDE, DEFAULT_CHANNEL + 3);
    checkEventInterval(events, 1059, 1077, 55, defVol, MidiInstrumentEffect::SLIDE, DEFAULT_CHANNEL + 3);

    EXPECT_EQ(events[0].size(), 1);
    EXPECT_EQ(events[1].size(), 6);
    EXPECT_EQ(events[2].size(), 1);
    EXPECT_EQ(events[3].size(), 3);
}

TEST_F(MidiRendererBend_Tests, gracebend)
{
    constexpr int defVol = 80; // mf
    constexpr int pwReset = 8192;

    EventsHolder events = renderMidiEvents(u"gracebend.mscx", true, true);

    EXPECT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].size(), 51);

    /// time factor: 0->1
    checkEventInterval(events, 360, 959, 52, defVol);
    checkPitchBend(events, 360, 8192);
    checkPitchBend(events, 370, 8201);
    checkPitchBend(events, 380, 8229);
    checkPitchBend(events, 390, 8277);
    checkPitchBend(events, 400, 8343);
    checkPitchBend(events, 410, 8429);
    checkPitchBend(events, 420, 8533);
    checkPitchBend(events, 430, 8656);
    checkPitchBend(events, 440, 8798);
    checkPitchBend(events, 450, 8960);
    checkPitchBend(events, 460, 9140);
    checkPitchBend(events, 470, 9339);
    checkPitchBend(events, 480, 9557);

    /// time factor: 0.25->0.75 (ignoring factor)
    checkPitchBend(events, 1310, pwReset);
    checkEventInterval(events, 1320, 1919, 52, defVol);
    checkPitchBend(events, 1320, 8192);
    checkPitchBend(events, 1330, 8201);
    checkPitchBend(events, 1340, 8229);
    checkPitchBend(events, 1350, 8277);
    checkPitchBend(events, 1360, 8343);
    checkPitchBend(events, 1370, 8429);
    checkPitchBend(events, 1380, 8533);
    checkPitchBend(events, 1390, 8656);
    checkPitchBend(events, 1400, 8798);
    checkPitchBend(events, 1410, 8960);
    checkPitchBend(events, 1420, 9140);
    checkPitchBend(events, 1430, 9339);
    checkPitchBend(events, 1440, 9557);

    /// note + gracebend
    checkPitchBend(events, 1910, pwReset);
    checkEventInterval(events, 1920, 2279, 54, defVol);
    checkPitchBend(events, 2270, pwReset);
    checkEventInterval(events, 2280, 2879, 52, defVol);
    checkPitchBend(events, 2280, 8192);
    checkPitchBend(events, 2290, 8201);
    checkPitchBend(events, 2300, 8229);
    checkPitchBend(events, 2310, 8277);
    checkPitchBend(events, 2320, 8343);
    checkPitchBend(events, 2330, 8429);
    checkPitchBend(events, 2340, 8533);
    checkPitchBend(events, 2350, 8656);
    checkPitchBend(events, 2360, 8798);
    checkPitchBend(events, 2370, 8960);
    checkPitchBend(events, 2380, 9140);
    checkPitchBend(events, 2390, 9339);
    checkPitchBend(events, 2400, 9557);

    EXPECT_EQ(events[0].size(), 1);
}

TEST_F(MidiRendererBend_Tests, bendGraceAfter)
{
    constexpr int defVol = 80; // mf
    constexpr int pwReset = 8192;

    EventsHolder events = renderMidiEvents(u"bend-grace-after.mscx");

    EXPECT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].size(), 54);

    checkEventInterval(events, 0, 479, 60, defVol);
    checkPitchBend(events, 0, pwReset);
    checkPitchBend(events, 111, 8192);
    checkPitchBend(events, 121, 8202);
    checkPitchBend(events, 131, 8235);
    checkPitchBend(events, 141, 8289);
    checkPitchBend(events, 151, 8366);
    checkPitchBend(events, 161, 8464);
    checkPitchBend(events, 171, 8583);
    checkPitchBend(events, 181, 8725);
    checkPitchBend(events, 191, 8888);
    checkPitchBend(events, 201, 9073);
    checkPitchBend(events, 211, 9280);
    checkPitchBend(events, 221, 9508);
    checkPitchBend(events, 231, 9557);

    checkPitchBend(events, 1910, pwReset);
    checkEventInterval(events, 1920, 2399, 60, defVol);
    checkPitchBend(events, 1920, 8192);
    checkPitchBend(events, 1988, 8192);
    checkPitchBend(events, 1998, 8229);
    checkPitchBend(events, 2008, 8343);
    checkPitchBend(events, 2018, 8533);
    checkPitchBend(events, 2028, 8798);
    checkPitchBend(events, 2038, 9140);
    checkPitchBend(events, 2048, 9557);
    checkPitchBend(events, 2160, 9557);

    checkPitchBend(events, 2170, 9546);
    checkPitchBend(events, 2180, 9514);
    checkPitchBend(events, 2190, 9460);
    checkPitchBend(events, 2200, 9384);
    checkPitchBend(events, 2210, 9287);
    checkPitchBend(events, 2220, 9168);
    checkPitchBend(events, 2230, 9028);
    checkPitchBend(events, 2240, 8866);
    checkPitchBend(events, 2250, 8683);
    checkPitchBend(events, 2260, 8478);
    checkPitchBend(events, 2270, 8252);
    checkPitchBend(events, 2280, 8192);

    checkPitchBend(events, 3830, pwReset);
    checkEventInterval(events, 3840, 4319, 64, defVol);
    checkPitchBend(events, 3840, 8192);
    checkPitchBend(events, 3850, 8199);
    checkPitchBend(events, 3860, 8221);
    checkPitchBend(events, 3870, 8258);
    checkPitchBend(events, 3880, 8310);
    checkPitchBend(events, 3890, 8377);
    checkPitchBend(events, 3900, 8458);
    checkPitchBend(events, 3910, 8554);
    checkPitchBend(events, 3920, 8666);
    checkPitchBend(events, 3930, 8792);
    checkPitchBend(events, 3940, 8874);

    EXPECT_TRUE(events[0].empty());
}

TEST_F(MidiRendererBend_Tests, bendAfterHammer)
{
    constexpr int defVol = 80; // mf
    constexpr int pwReset = 8192;

    EventsHolder events = renderMidiEvents(u"bend_after_hammer.mscx", true, true);

    EXPECT_EQ(events.size(), 2);
    EXPECT_EQ(events[0].size(), 2);
    EXPECT_EQ(events[1].size(), 16);

    checkEventInterval(events, 0, 479, 62, defVol);
    checkEventInterval(events, 480, 1679, 66, defVol, MidiInstrumentEffect::HAMMER_PULL, DEFAULT_CHANNEL + 1);
    checkPitchBend(events, 960, pwReset, MidiInstrumentEffect::HAMMER_PULL, DEFAULT_CHANNEL + 1);
    checkPitchBend(events, 970, 8201, MidiInstrumentEffect::HAMMER_PULL, DEFAULT_CHANNEL + 1);
    checkPitchBend(events, 980, 8229, MidiInstrumentEffect::HAMMER_PULL, DEFAULT_CHANNEL + 1);
    checkPitchBend(events, 990, 8277, MidiInstrumentEffect::HAMMER_PULL, DEFAULT_CHANNEL + 1);
    checkPitchBend(events, 1000, 8343, MidiInstrumentEffect::HAMMER_PULL, DEFAULT_CHANNEL + 1);
    checkPitchBend(events, 1010, 8429, MidiInstrumentEffect::HAMMER_PULL, DEFAULT_CHANNEL + 1);
    checkPitchBend(events, 1020, 8533, MidiInstrumentEffect::HAMMER_PULL, DEFAULT_CHANNEL + 1);
    checkPitchBend(events, 1030, 8656, MidiInstrumentEffect::HAMMER_PULL, DEFAULT_CHANNEL + 1);
    checkPitchBend(events, 1040, 8798, MidiInstrumentEffect::HAMMER_PULL, DEFAULT_CHANNEL + 1);
    checkPitchBend(events, 1050, 8960, MidiInstrumentEffect::HAMMER_PULL, DEFAULT_CHANNEL + 1);
    checkPitchBend(events, 1060, 9140, MidiInstrumentEffect::HAMMER_PULL, DEFAULT_CHANNEL + 1);
    checkPitchBend(events, 1070, 9339, MidiInstrumentEffect::HAMMER_PULL, DEFAULT_CHANNEL + 1);
    checkPitchBend(events, 1080, 9557, MidiInstrumentEffect::HAMMER_PULL, DEFAULT_CHANNEL + 1);

    EXPECT_EQ(events[0].size(), 0);
    EXPECT_EQ(events[1].size(), 1);
}
