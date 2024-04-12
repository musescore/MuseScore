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

#include <set>
#include <map>

#include <gtest/gtest.h>

#include "engraving/compat/midi/pitchwheelrenderer.h"

using namespace mu;
using namespace mu::engraving;
static int DEFAULT_CHANNEL = 0;

class PitchWheelRender_Tests : public ::testing::Test
{
};

int pitch(const NPlayEvent& ev)
{
    return ev.dataA() + ev.dataB() * 128;
}

//---------------------------------------------------------
///   note
///   read/write test of note
//---------------------------------------------------------

TEST_F(PitchWheelRender_Tests, generateRanges)
{
    PitchWheelRenderer::PitchWheelFunction func1;
    PitchWheelRenderer::PitchWheelFunction func2;
    PitchWheelRenderer::PitchWheelFunction func3;
    PitchWheelRenderer::PitchWheelFunction func4;
    PitchWheelRenderer::PitchWheelFunction func5;
    func1.mStartTick = 30;
    func1.mEndTick = 60;
    func2.mStartTick = 50;
    func3.mEndTick = 60;
    func3.mStartTick = 40;
    func3.mEndTick = 80;
    func4.mStartTick = 100;
    func4.mEndTick = 200;
    func5.mStartTick = 10;
    func5.mEndTick = 50;
    using FuncList = std::list<PitchWheelRenderer::PitchWheelFunction>;
    std::map<int, int, std::greater<> > ranges;
    PitchWheelRenderer::generateRanges(FuncList { func1 }, ranges);
    EXPECT_EQ(ranges.at(30), 60);
    PitchWheelRenderer::generateRanges(FuncList { func2 }, ranges);
    EXPECT_EQ(ranges.at(30), 60);
    PitchWheelRenderer::generateRanges(FuncList { func3 }, ranges);
    EXPECT_EQ(ranges.at(30), 80);
    PitchWheelRenderer::generateRanges(FuncList { func4 }, ranges);
    EXPECT_EQ(ranges.at(30), 80);
    EXPECT_EQ(ranges.at(100), 200);
    PitchWheelRenderer::generateRanges(FuncList { func5 }, ranges);
    EXPECT_EQ(ranges.at(10), 80);
    EXPECT_EQ(ranges.find(30), ranges.end());
    EXPECT_EQ(ranges.at(100), 200);
}

TEST_F(PitchWheelRender_Tests, simpleLinear)
{
    PitchWheelSpecs wheelSpec;
    PitchWheelRenderer render(wheelSpec);

    PitchWheelRenderer::PitchWheelFunction func;
    func.mStartTick = 0;
    func.mEndTick = 100;
    //! y = ax + b
    int a = 1;
    int b = 0;

    auto linearFunc = [ startTick = func.mStartTick, a, b] (uint32_t tick) {
        float x = (float)(tick - startTick);
        float y = a * x + b;
        return (int)y;
    };
    func.func = linearFunc;
    render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);

    EventsHolder events = render.renderPitchWheel();

    EXPECT_EQ(events[DEFAULT_CHANNEL].begin()->second.channel(), 0);

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 10);

    EXPECT_EQ(pitch(events[DEFAULT_CHANNEL].find(10)->second), 8202);
    EXPECT_EQ(pitch(events[DEFAULT_CHANNEL].find(90)->second), 8282);
//    EXPECT_EQ(pitch(events.find(100)->second), 8192);
}

TEST_F(PitchWheelRender_Tests, twoReverseFunctions)
{
    PitchWheelSpecs wheelSpec;
    PitchWheelRenderer render(wheelSpec);

    PitchWheelRenderer::PitchWheelFunction func;
    func.mStartTick = 0;
    func.mEndTick = 100;
    int a = 1;
    int b = 0;
    auto linearFunc = [ startTick = func.mStartTick, a, b] (uint32_t tick) {
        float x = (float)(tick - startTick);
        float y = a * x + b;
        return (int)y;
    };
    func.func = linearFunc;
    render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);

    auto reverseLinearFunc = [ startTick = func.mStartTick, a, b] (uint32_t tick) {
        float x = (float)(tick - startTick);
        float y = -1 * a * x + b;
        return (int)y;
    };
    func.func = reverseLinearFunc;
    render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);

    EventsHolder events = render.renderPitchWheel();
    EXPECT_EQ(events.size(), 1);
}

TEST_F(PitchWheelRender_Tests, channelTest)
{
    PitchWheelSpecs wheelSpec;
    PitchWheelRenderer render(wheelSpec);

    PitchWheelRenderer::PitchWheelFunction func;
    func.mStartTick = 0;
    func.mEndTick = 30;
    //! y = ax + b
    int a = 1;
    int b = 0;

    auto linearFunc = [ startTick = func.mStartTick, a, b] (uint32_t tick) {
        float x = (float)(tick - startTick);
        float y = a * x + b;
        return (int)y;
    };
    func.func = linearFunc;
    render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);
    render.addPitchWheelFunction(func, 1, 0, MidiInstrumentEffect::NONE);

    EventsHolder events = render.renderPitchWheel();

    EXPECT_EQ(events[0].size(), 3);
    EXPECT_EQ(events[0].count(0), 1);
    EXPECT_EQ(events[1].count(0), 1);
}

TEST_F(PitchWheelRender_Tests, twoConnectedFunctions)
{
    PitchWheelSpecs wheelSpec;
    PitchWheelRenderer render(wheelSpec);

    {
        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = 0;
        func.mEndTick = 30;
        //! y = ax + b
        int a = 1;
        int b = 0;

        auto firstFunc = [ startTick = func.mStartTick, a, b] (uint32_t tick) {
            float x = (float)(tick - startTick);
            float y = a * x + b;
            return (int)y;
        };
        func.func = firstFunc;
        render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);
    }

    {
        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = 30;
        func.mEndTick = 60;
        //! y = ax + b
        int a = 1;
        int b = 30;

        auto secondFunc = [ startTick = func.mStartTick, a, b] (uint32_t tick) {
            float x = (float)(tick - startTick);
            float y = a * x + b;
            return (int)y;
        };
        func.func = secondFunc;
        render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);
    }

    EventsHolder events = render.renderPitchWheel();

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 6);
    std::set<int> expectedValues = { 8192, 8202, 8212, 8222, 8232, 8242 };
    for (const auto& ev : events[DEFAULT_CHANNEL]) {
        EXPECT_TRUE(expectedValues.count(pitch(ev.second)) > 0);
    }
}

TEST_F(PitchWheelRender_Tests, twoDevidedFunctions)
{
    PitchWheelSpecs wheelSpec;
    PitchWheelRenderer render(wheelSpec);

    {
        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = 0;
        func.mEndTick = 30;
        //! y = ax + b
        int a = 1;
        int b = 0;

        auto firstFunc = [ startTick = func.mStartTick, a, b] (uint32_t tick) {
            float x = (float)(tick - startTick);
            float y = a * x + b;
            return (int)y;
        };
        func.func = firstFunc;
        render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);
    }

    {
        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = 40;
        func.mEndTick = 60;
        //! y = ax + b
        int a = 1;
        int b = 30;

        auto secondFunc = [ startTick = func.mStartTick, a, b] (uint32_t tick) {
            float x = (float)(tick - startTick);
            float y = a * x + b;
            return (int)y;
        };
        func.func = secondFunc;
        render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);
    }

    EventsHolder events = render.renderPitchWheel();

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 5);
    std::multimap<int, int> expectedValues = { { 0, 8192 }, { 10, 8202 }, { 20, 8212 }, { 40, 8222 }, { 50, 8232 } };
    for (const auto& ev : events[DEFAULT_CHANNEL]) {
        auto it = expectedValues.find(ev.first);
        EXPECT_TRUE(it != expectedValues.end());
        EXPECT_EQ(it->second, pitch(ev.second));
    }
}

TEST_F(PitchWheelRender_Tests, twoOverlappedFunctions)
{
    PitchWheelSpecs wheelSpec;
    PitchWheelRenderer render(wheelSpec);

    {
        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = 0;
        func.mEndTick = 30;
        //! y = ax + b
        int a = 1;
        int b = 0;

        auto firstFunc = [ startTick = func.mStartTick, a, b] (uint32_t tick) {
            float x = (float)(tick - startTick);
            float y = a * x + b;
            return (int)y;
        };
        func.func = firstFunc;
        render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);
    }

    {
        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = 20;
        func.mEndTick = 40;
        //! y = ax + b
        int a = 1;
        int b = 10;
        auto secondFunc = [ startTick = func.mStartTick, a, b] (uint32_t tick) {
            float x = (float)(tick - startTick);
            float y = a * x + b;
            return (int)y;
        };
        func.func = secondFunc;
        render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);
    }

    EventsHolder events = render.renderPitchWheel();

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 4);
    std::multimap<int, int> pitches;
    std::multimap<int, int> expectedValues = { { 0, 8192 }, { 10, 8202 }, { 20, 8222 }, { 30, 8212 } };
    for (const auto& ev : events[DEFAULT_CHANNEL]) {
        auto it = expectedValues.find(ev.first);
        EXPECT_TRUE(it != expectedValues.end());
        EXPECT_EQ(it->second, pitch(ev.second));
        pitches.insert({ ev.first, pitch(ev.second) });
    }
}

TEST_F(PitchWheelRender_Tests, threeDevidedFunctions)
{
    PitchWheelSpecs wheelSpec;
    PitchWheelRenderer render(wheelSpec);

    {
        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = 10;
        func.mEndTick = 20;

        auto firstFunc = [](uint32_t) {
            return 10;
        };
        func.func = firstFunc;
        render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);
    }

    {
        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = 30;
        func.mEndTick = 40;

        auto firstFunc = [](uint32_t) {
            return 20;
        };
        func.func = firstFunc;
        render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);
    }

    {
        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = 60;
        func.mEndTick = 70;

        auto firstFunc = [](uint32_t) {
            return 30;
        };
        func.func = firstFunc;
        render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);
    }

    EventsHolder events = render.renderPitchWheel();

    std::multimap<int, int> pitches;
    std::multimap<int, int> expectedValues = { { 10, 8202 }, { 20, 8192 }, { 30, 8212 }, { 40, 8192 }, { 60, 8222 } };
    for (const auto& ev : events[DEFAULT_CHANNEL]) {
        auto it = expectedValues.find(ev.first);
        EXPECT_TRUE(it != expectedValues.end());
        EXPECT_EQ(it->second, pitch(ev.second));
        pitches.insert({ ev.first, pitch(ev.second) });
    }
}

TEST_F(PitchWheelRender_Tests, threeOverLappedFunctions)
{
    PitchWheelSpecs wheelSpec;
    PitchWheelRenderer render(wheelSpec);

    {
        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = 0;
        func.mEndTick = 60;

        auto firstFunc = [](uint32_t) {
            return 10;
        };
        func.func = firstFunc;
        render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);
    }

    {
        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = 20;
        func.mEndTick = 50;

        auto firstFunc = [](uint32_t) {
            return 10;
        };
        func.func = firstFunc;
        render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);
    }

    {
        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = 40;
        func.mEndTick = 70;

        auto firstFunc = [](uint32_t) {
            return 10;
        };
        func.func = firstFunc;
        render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);
    }

    EventsHolder events = render.renderPitchWheel();

    std::multimap<int, int> pitches;
    std::multimap<int, int> expectedValues = { { 0, 8202 }, { 20, 8212 }, { 40, 8222 }, { 50, 8212 }, { 60, 8202 }, { 70, 8192 } };
    for (const auto& ev : events[DEFAULT_CHANNEL]) {
        auto it = expectedValues.find(ev.first);
        EXPECT_TRUE(it != expectedValues.end());
        EXPECT_EQ(it->second, pitch(ev.second));
        pitches.insert({ ev.first, pitch(ev.second) });
    }
}
