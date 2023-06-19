/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

    EventMap events = render.renderPitchWheel();

    EXPECT_EQ(events.begin()->second.channel(), 0);

    EXPECT_EQ(events.size(), 10);

    EXPECT_EQ(pitch(events.find(10)->second), 8202);
    EXPECT_EQ(pitch(events.find(90)->second), 8282);
    EXPECT_EQ(pitch(events.find(100)->second), 8192);
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

    EventMap events = render.renderPitchWheel();
    EXPECT_EQ(events.size(), 0);
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

    EventMap events = render.renderPitchWheel();
    std::set<int> channels;
    for (const auto& ev : events) {
        channels.insert(ev.second.channel());
    }

    EXPECT_EQ(channels.size(), 2);
    EXPECT_EQ(channels.count(0), 1);
    EXPECT_EQ(channels.count(1), 1);
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

    EventMap events = render.renderPitchWheel();

    EXPECT_EQ(events.size(), 6);
    std::set<int> expectedValues = { 8192, 8202, 8212, 8222, 8232, 8242 };
    for (const auto& ev : events) {
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

    EventMap events = render.renderPitchWheel();

    EXPECT_EQ(events.size(), 6);
    std::multimap<int, int> expectedValues = { { 10, 8202 }, { 20, 8212 }, { 30, 8192 }, { 40, 8222 }, { 50, 8232 }, { 60, 8192 } };
    for (const auto& ev : events) {
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

    EventMap events = render.renderPitchWheel();

    EXPECT_EQ(events.size(), 4);
    std::multimap<int, int> pitches;
    std::multimap<int, int> expectedValues = { { 10, 8202 }, { 20, 8222 }, { 30, 8212 }, { 40, 8192 } };
    for (const auto& ev : events) {
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

    EventMap events = render.renderPitchWheel();

    std::multimap<int, int> pitches;
    std::multimap<int, int> expectedValues = { { 10, 8202 }, { 20, 8192 }, { 30, 8212 }, { 40, 8192 }, { 60, 8222 }, { 70, 8192 } };
    for (const auto& ev : events) {
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

    EventMap events = render.renderPitchWheel();

    std::multimap<int, int> pitches;
    std::multimap<int, int> expectedValues = { { 0, 8202 }, { 20, 8212 }, { 40, 8222 }, { 50, 8212 }, { 60, 8202 }, { 70, 8192 } };
    for (const auto& ev : events) {
        auto it = expectedValues.find(ev.first);
        EXPECT_TRUE(it != expectedValues.end());
        EXPECT_EQ(it->second, pitch(ev.second));
        pitches.insert({ ev.first, pitch(ev.second) });
    }
}
