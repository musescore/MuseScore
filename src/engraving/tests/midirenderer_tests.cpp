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

#include <gtest/gtest.h>

#include "utils/scorerw.h"
#include "engraving/compat/midi/midirender.h"
#include "engraving/infrastructure/localfileinfoprovider.h"
#include "engraving/rw/mscloader.h"
#include "engraving/libmscore/noteevent.h"

using namespace mu;
using namespace mu::engraving;

class MidiRenderer_Tests : public ::testing::Test
{
};

static const String MIDIRENDERER_TESTS_DIR = u"midirenderer_data/";
static constexpr int DEFAULT_CHANNEL = 0;
static constexpr int NOTE_OFF_VOLUME = 0;

static NPlayEvent noteEvent(int pitch, int volume, int channel)
{
    return NPlayEvent(EventType::ME_NOTEON, channel, pitch, volume);
}

static void checkEventInterval(EventMap& events, int tickStart, int tickEnd, int pitch, int volume, int channel = DEFAULT_CHANNEL)
{
    auto it1 = events.find(tickStart);
    auto it2 = events.find(tickEnd);

    EXPECT_TRUE(it1 != events.end());
    EXPECT_TRUE(it2 != events.end());

    EXPECT_EQ(it1->second, noteEvent(pitch, volume, channel));
    EXPECT_EQ(it2->second, noteEvent(pitch, NOTE_OFF_VOLUME, channel));

    events.erase(it1);
    events.erase(it2);
}

static EventMap renderMidiEvents(const String& fileName, bool eachStringHasChannel = false, bool instrumentsHaveEffects = false)
{
    MasterScore* score = ScoreRW::readScore(MIDIRENDERER_TESTS_DIR + fileName);
    EXPECT_TRUE(score);

    MidiRenderer render(score);
    EventMap events;
    MidiRenderer::Context ctx;

    ctx.synthState = mu::engraving::SynthesizerState();
    ctx.metronome = false;
    ctx.eachStringHasChannel = eachStringHasChannel;
    ctx.instrumentsHaveEffects = instrumentsHaveEffects;
    score->renderMidi(&events, ctx, true);

    return events;
}

static EventMap getNoteOnEvents(const EventMap& events)
{
    EventMap filteredEventMap;
    for (const auto& ev : events) {
        if (ev.second.type() != EventType::ME_NOTEON) {
            continue;
        }

        filteredEventMap.insert({ ev.first, ev.second });
    }

    return filteredEventMap;
}

/*****************************************************************************

    ENABLED TESTS BELOW

*****************************************************************************/

TEST_F(MidiRenderer_Tests, oneGuitarNote)
{
    constexpr int defVol = 96; // f

    EventMap events = renderMidiEvents(u"one_guitar_note.mscx");

    EXPECT_EQ(events.size(), 2);

    checkEventInterval(events, 0, 479, 59, defVol);
}

TEST_F(MidiRenderer_Tests, onePercussionNote)
{
    constexpr int defVol = 80; // mf

    EventMap events = renderMidiEvents(u"one_percussion_note.mscx");

    EXPECT_EQ(events.size(), 1);

    EXPECT_EQ(events.find(0)->second, noteEvent(41, defVol, DEFAULT_CHANNEL));
}

TEST_F(MidiRenderer_Tests, graceBeforeBeat)
{
    constexpr int defVol = 96; // f

    EventMap events = renderMidiEvents(u"grace_before_beat.mscx");

    EXPECT_EQ(events.size(), 6);

    checkEventInterval(events, 0, 239, 59, defVol);
    checkEventInterval(events, 240, 479, 55, defVol);
    checkEventInterval(events, 480, 959, 57, defVol);
}

TEST_F(MidiRenderer_Tests, graceOnBeat)
{
    constexpr int defVol = 96; // f

    EventMap events = renderMidiEvents(u"grace_on_beat.mscx");

    EXPECT_EQ(events.size(), 6);

    checkEventInterval(events, 0, 479, 59, defVol);
    checkEventInterval(events, 480, 719, 55, defVol);
    checkEventInterval(events, 720, 959, 57, defVol);
}

TEST_F(MidiRenderer_Tests, ghostNote)
{
    constexpr int defVol = 96; // f
    constexpr int ghostVol = defVol * NoteEvent::GHOST_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"ghost_note.mscx");

    EXPECT_EQ(events.size(), 4);

    checkEventInterval(events, 0, 479, 59, defVol);
    checkEventInterval(events, 480, 959, 57, ghostVol);
}

TEST_F(MidiRenderer_Tests, simpleTremolo)
{
    constexpr int defVol = 96; // f

    EventMap events = renderMidiEvents(u"simple_tremolo.mscx");

    EXPECT_EQ(events.size(), 8);

    checkEventInterval(events, 0, 239, 59, defVol);
    checkEventInterval(events, 240, 479, 59, defVol);
    checkEventInterval(events, 480, 719, 59, defVol);
    checkEventInterval(events, 720, 959, 59, defVol);
}

TEST_F(MidiRenderer_Tests, simpleGlissando)
{
    constexpr int defVol = 96; // forte
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"simple_glissando.mscx");

    EXPECT_EQ(events.size(), 10);

    checkEventInterval(events, 0, 599, 59, defVol);
    checkEventInterval(events, 600, 719, 58, glissVol);
    checkEventInterval(events, 720, 839, 57, glissVol);
    checkEventInterval(events, 840, 959, 56, glissVol);
    checkEventInterval(events, 960, 1439, 55, defVol);
}

TEST_F(MidiRenderer_Tests, sameStringNoEffects)
{
    constexpr int defVol = 80; // mf

    EventMap events = getNoteOnEvents(renderMidiEvents(u"channels.mscx", false, false));

    checkEventInterval(events, 0, 959, 60, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 480, 1439, 64, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 960, 1919, 60, defVol, DEFAULT_CHANNEL);
}

TEST_F(MidiRenderer_Tests, sameStringWithEffects)
{
    constexpr int defVol = 80; // mf

    EventMap events = getNoteOnEvents(renderMidiEvents(u"channels.mscx", false, true));

    checkEventInterval(events, 0, 959, 60, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 480, 1439, 64, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 960, 1919, 60, defVol, DEFAULT_CHANNEL + 1);
}

TEST_F(MidiRenderer_Tests, diffStringNoEffects)
{
    constexpr int defVol = 80; // mf

    EventMap events = getNoteOnEvents(renderMidiEvents(u"channels.mscx", true, false));

    EXPECT_EQ(events.size(), 6);

    checkEventInterval(events, 0, 959, 60, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 480, 1439, 64, defVol, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 960, 1919, 60, defVol, DEFAULT_CHANNEL);
}

TEST_F(MidiRenderer_Tests, diffStringWithEffects)
{
    constexpr int defVol = 80; // mf

    EventMap events = getNoteOnEvents(renderMidiEvents(u"channels.mscx", true, true));

    EXPECT_EQ(events.size(), 6);

    checkEventInterval(events, 0, 959, 60, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 480, 1439, 64, defVol, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 960, 1919, 60, defVol, DEFAULT_CHANNEL + 2);
}

TEST_F(MidiRenderer_Tests, tremoloAndGlissando)
{
    constexpr int defVol = 96; // f
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"tremolo_and_glissando.mscx");

    EXPECT_EQ(events.size(), 14);

    checkEventInterval(events, 0, 239, 59, defVol);
    checkEventInterval(events, 240, 479, 59, defVol);
    checkEventInterval(events, 480, 599, 59, defVol);
    checkEventInterval(events, 600, 719, 58, glissVol);
    checkEventInterval(events, 720, 839, 57, glissVol);
    checkEventInterval(events, 840, 959, 56, glissVol);
    checkEventInterval(events, 960, 1439, 55, defVol);
}

TEST_F(MidiRenderer_Tests, slideInFromBelow)
{
    constexpr int defVol = 80; // mf
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"slide_in_from_below.mscx");

    EXPECT_EQ(events.size(), 10);

    checkEventInterval(events, 0, 239, 60, defVol);
    checkEventInterval(events, 240, 318, 57, glissVol);
    checkEventInterval(events, 320, 398, 58, glissVol);
    checkEventInterval(events, 400, 478, 59, glissVol);
    checkEventInterval(events, 480, 959, 60, defVol);
}

TEST_F(MidiRenderer_Tests, slideInFromAbove)
{
    constexpr int defVol = 80; // mf
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"slide_in_from_above.mscx");

    EXPECT_EQ(events.size(), 10);

    checkEventInterval(events, 0, 239, 60, defVol);
    checkEventInterval(events, 240, 318, 63, glissVol);
    checkEventInterval(events, 320, 398, 62, glissVol);
    checkEventInterval(events, 400, 478, 61, glissVol);
    checkEventInterval(events, 480, 959, 60, defVol);
}

TEST_F(MidiRenderer_Tests, slideOutFromAbove)
{
    constexpr int defVol = 80; // mf
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"slide_out_from_above.mscx");

    EXPECT_EQ(events.size(), 10);

    checkEventInterval(events, 0, 239, 60, defVol);
    checkEventInterval(events, 240, 318, 59, glissVol);
    checkEventInterval(events, 319, 397, 58, glissVol);
    checkEventInterval(events, 399, 477, 57, glissVol);
    checkEventInterval(events, 480, 959, 60, defVol);
}

TEST_F(MidiRenderer_Tests, slideOutFromBelow)
{
    constexpr int defVol = 80; // mf
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"slide_out_from_below.mscx");

    EXPECT_EQ(events.size(), 10);

    checkEventInterval(events, 0, 239, 60, defVol);
    checkEventInterval(events, 240, 318, 61, glissVol);
    checkEventInterval(events, 319, 397, 62, glissVol);
    checkEventInterval(events, 399, 477, 63, glissVol);
    checkEventInterval(events, 480, 959, 60, defVol);
}

TEST_F(MidiRenderer_Tests, tremoloSlideIn)
{
    constexpr int defVol = 96; // f
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"tremolo_and_slide_in.mscx");

    EXPECT_EQ(events.size(), 16);

    checkEventInterval(events, 0, 119, 59, defVol);
    checkEventInterval(events, 120, 239, 59, defVol);
    checkEventInterval(events, 240, 359, 59, defVol);
    checkEventInterval(events, 360, 479, 59, defVol);
    checkEventInterval(events, 480, 638, 56, glissVol);
    checkEventInterval(events, 640, 798, 57, glissVol);
    checkEventInterval(events, 800, 958, 58, glissVol);
    checkEventInterval(events, 960, 1439, 59, defVol);
}

TEST_F(MidiRenderer_Tests, tremoloSlideOut)
{
    constexpr int defVol = 96; // f
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"tremolo_and_slide_out.mscx");

    EXPECT_EQ(events.size(), 14);

    checkEventInterval(events, 0, 239, 59, defVol);
    checkEventInterval(events, 240, 479, 59, defVol);
    checkEventInterval(events, 480, 719, 59, defVol);
    checkEventInterval(events, 720, 959, 59, defVol);
    checkEventInterval(events, 960, 1277, 58, glissVol);
    checkEventInterval(events, 1278, 1595, 57, glissVol);
    checkEventInterval(events, 1597, 1914, 56, glissVol);
}

TEST_F(MidiRenderer_Tests, slideInAndOut)
{
    constexpr int defVol = 80; // mf
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"slide_in_and_out.mscx");

    EXPECT_EQ(events.size(), 14);

    checkEventInterval(events, 240, 318, 57, glissVol);
    checkEventInterval(events, 320, 398, 58, glissVol);
    checkEventInterval(events, 400, 478, 59, glissVol);
    checkEventInterval(events, 480, 719, 60, defVol);
    checkEventInterval(events, 720, 798, 61, glissVol);
    checkEventInterval(events, 799, 877, 62, glissVol);
    checkEventInterval(events, 879, 957, 63, glissVol);
}

TEST_F(MidiRenderer_Tests, sameStringDifferentStaves)
{
    constexpr int defVol = 80; // mf

    EventMap events = getNoteOnEvents(renderMidiEvents(u"same_string_diff_staves.mscx", true));

    EXPECT_EQ(events.size(), 6);

    checkEventInterval(events, 0, 239, 62, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 240, 479, 62, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 0, 1919, 35, defVol, DEFAULT_CHANNEL + 1);
}

/*****************************************************************************

    DISABLED TESTS BELOW

*****************************************************************************/
