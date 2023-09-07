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
#include "engraving/dom/noteevent.h"

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


static int getEventsCount(EventsHolder& events)
{
    int eventsCount = 0;
    for (size_t i = 0; i < events.size(); ++i) {
        eventsCount += (int)events[i].size();
    }
    return eventsCount;
}

static void checkEventInterval(EventsHolder& events, int tickStart, int tickEnd, int pitch, int volume, bool slide = false,
                               int channel = DEFAULT_CHANNEL)
{
    auto it = events[channel].find(tickStart);
    EXPECT_TRUE(it != events[channel].end());
    if (it == events[channel].end()) {
        return;
    }

    EXPECT_EQ(it->second.pitch(), pitch);
    EXPECT_EQ(it->second.velo(), volume);
    EXPECT_EQ(it->second.channel(), channel);
    EXPECT_EQ(it->second.slide(), slide);

    events[channel].erase(it);

    it = events[channel].find(tickEnd);
    EXPECT_TRUE(it != events[channel].end());
    if (it == events[channel].end()) {
        return;
    }

    EXPECT_EQ(it->second.pitch(), pitch);
    EXPECT_EQ(it->second.velo(), NOTE_OFF_VOLUME);
    EXPECT_EQ(it->second.channel(), channel);
    EXPECT_EQ(it->second.slide(), slide);

    events[channel].erase(it);
}

static EventsHolder renderMidiEvents(const String& fileName, bool eachStringHasChannel = false, bool instrumentsHaveEffects = false)
{
    MasterScore* score = ScoreRW::readScore(MIDIRENDERER_TESTS_DIR + fileName);
    EXPECT_TRUE(score);

    MidiRenderer render(score);
    EventsHolder events;
    MidiRenderer::Context ctx;

    ctx.synthState = mu::engraving::SynthesizerState();
    ctx.metronome = false;
    ctx.eachStringHasChannel = eachStringHasChannel;
    ctx.instrumentsHaveEffects = instrumentsHaveEffects;
    score->renderMidi(events, ctx, true);

    return events;
}

static EventsHolder getNoteOnEvents(const EventsHolder& events)
{
    EventsHolder filteredEventMap;
    for (size_t i = 0; i < events.size(); ++i) {
        for (auto ev: events[i]) {
            if (ev.second.type() != EventType::ME_NOTEON) {
                continue;
            }
            filteredEventMap[i].insert({ev.first, ev.second});
        }
    }

    return filteredEventMap;
}

/*****************************************************************************

    ENABLED TESTS BELOW

*****************************************************************************/
TEST_F(MidiRenderer_Tests, mergePitchWheelEvents)
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

    EventsHolder pitchWheelEvents = render.renderPitchWheel();
    EventsHolder noteEvents;
    NPlayEvent note1_ON{ 144, 0, 59, 96 };
    NPlayEvent note1_OFF{ 144, 0, 59, 0 };
    NPlayEvent note2_ON{ 144, 0, 61, 96 };
    NPlayEvent note2_OFF{ 144, 0, 61, 0 };
    noteEvents[DEFAULT_CHANNEL].insert(std::make_pair(0, note1_ON));
    noteEvents[DEFAULT_CHANNEL].insert(std::make_pair(90, note1_OFF));
    noteEvents[DEFAULT_CHANNEL].insert(std::make_pair(200, note2_ON));
    noteEvents[DEFAULT_CHANNEL].insert(std::make_pair(300, note2_OFF));
    noteEvents.mergePitchWheelEvents(pitchWheelEvents);
    EXPECT_NE(noteEvents[DEFAULT_CHANNEL].find(199), noteEvents[DEFAULT_CHANNEL].end());
}

TEST_F(MidiRenderer_Tests, subscriptOperator)
{
    EventsHolder events;
    events[0];
    EXPECT_EQ(events.size(), 1);
    events[1];
    EXPECT_EQ(events.size(), 2);
    events[3];
    EXPECT_EQ(events.size(), 4);
    events[50];
    EXPECT_EQ(events.size(), 51);
    EventsHolder events2;
    events2[10];
    EXPECT_EQ(events2.size(), 11);
    events2[192];
    EXPECT_EQ(events2.size(), 193);
}

TEST_F(MidiRenderer_Tests, oneGuitarNote)
{
    constexpr int defVol = 96; // f

    EventsHolder events = renderMidiEvents(u"one_guitar_note.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 2);

    checkEventInterval(events, 0, 479, 59, defVol);
}

TEST_F(MidiRenderer_Tests, onePercussionNote)
{
    constexpr int defVol = 80; // mf

    EventsHolder events = renderMidiEvents(u"one_percussion_note.mscx");

    EXPECT_EQ(events.size(), 1);

    EXPECT_EQ(events[DEFAULT_CHANNEL].find(0)->second, noteEvent(41, defVol, DEFAULT_CHANNEL));
}

TEST_F(MidiRenderer_Tests, graceBeforeBeat)
{
    constexpr int defVol = 96; // f

    EventsHolder events = renderMidiEvents(u"grace_before_beat.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 8);

    checkEventInterval(events, 0, 239, 59, defVol);
    checkEventInterval(events, 240, 479, 55, defVol);
    checkEventInterval(events, 480, 959, 57, defVol);
}

TEST_F(MidiRenderer_Tests, graceOnBeat)
{
    constexpr int defVol = 96; // f

    EventsHolder events = renderMidiEvents(u"grace_on_beat.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 8);

    checkEventInterval(events, 0, 479, 59, defVol);
    checkEventInterval(events, 480, 719, 55, defVol);
    checkEventInterval(events, 720, 959, 57, defVol);
}

TEST_F(MidiRenderer_Tests, ghostNote)
{
    constexpr int defVol = 96; // f
    constexpr int ghostVol = defVol * NoteEvent::GHOST_VELOCITY_MULTIPLIER;

    EventsHolder events = renderMidiEvents(u"ghost_note.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 5);

    checkEventInterval(events, 0, 479, 59, defVol);
    checkEventInterval(events, 480, 959, 57, ghostVol);
}

TEST_F(MidiRenderer_Tests, simpleTremolo)
{
    constexpr int defVol = 96; // f

    EventsHolder events = renderMidiEvents(u"simple_tremolo.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 11);

    checkEventInterval(events, 0, 239, 59, defVol);
    checkEventInterval(events, 240, 479, 59, defVol);
    checkEventInterval(events, 480, 719, 59, defVol);
    checkEventInterval(events, 720, 959, 59, defVol);
}

TEST_F(MidiRenderer_Tests, legatoGlissando)
{
    constexpr int defVol = 96; // forte

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"simple_glissando_legato.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 10);

    checkEventInterval(events, 0, 642, 59, defVol);
    checkEventInterval(events, 643, 747, 58, defVol, true /* slide */);
    checkEventInterval(events, 748, 852, 57, defVol, true /* slide */);
    checkEventInterval(events, 854, 958, 56, defVol, true /* slide */);
    checkEventInterval(events, 960, 1439, 55, defVol, true /* slide */);
}

TEST_F(MidiRenderer_Tests, sameStringNoEffects)
{
    constexpr int defVol = 80; // mf

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"channels.mscx", false, false));

    checkEventInterval(events, 0, 959, 60, defVol, false, DEFAULT_CHANNEL);
    checkEventInterval(events, 480, 1439, 64, defVol, false, DEFAULT_CHANNEL);
    checkEventInterval(events, 960, 1919, 60, defVol, false, DEFAULT_CHANNEL);
}

TEST_F(MidiRenderer_Tests, sameStringWithEffects)
{
    constexpr int defVol = 80; // mf

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"channels.mscx", false, true));

    checkEventInterval(events, 0, 959, 60, defVol, false, DEFAULT_CHANNEL);
    checkEventInterval(events, 480, 1439, 64, defVol, false, DEFAULT_CHANNEL);
    checkEventInterval(events, 960, 1919, 60, defVol, false, DEFAULT_CHANNEL + 1);
}

TEST_F(MidiRenderer_Tests, diffStringNoEffects)
{
    constexpr int defVol = 80; // mf

    auto midievents = renderMidiEvents(u"channels.mscx", true, false);
    EventsHolder events = getNoteOnEvents(midievents);

    EXPECT_EQ(events[DEFAULT_CHANNEL].size() + events[DEFAULT_CHANNEL + 1].size(), 6);

    checkEventInterval(events, 0, 959, 60, defVol, false, DEFAULT_CHANNEL);
    checkEventInterval(events, 480, 1439, 64, defVol, false, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 960, 1919, 60, defVol, false, DEFAULT_CHANNEL);
}

TEST_F(MidiRenderer_Tests, diffStringWithEffects)
{
    constexpr int defVol = 80; // mf

    auto midiEvents = renderMidiEvents(u"channels.mscx", true, true);
    EventsHolder events = getNoteOnEvents(midiEvents);

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 2);
    int eventsCount =  getEventsCount(events);
    EXPECT_EQ(eventsCount, 6);

    checkEventInterval(events, 0, 959, 60, defVol, false, DEFAULT_CHANNEL);
    checkEventInterval(events, 480, 1439, 64, defVol, false, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 960, 1919, 60, defVol, false, DEFAULT_CHANNEL + 2);
}

TEST_F(MidiRenderer_Tests, tremoloAndGlissando)
{
    constexpr int defVol = 96; // f

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"tremolo_and_glissando.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 14);

    checkEventInterval(events, 0, 239, 59, defVol);
    checkEventInterval(events, 240, 479, 59, defVol);
    checkEventInterval(events, 480, 642, 59, defVol);
    checkEventInterval(events, 643, 747, 58, defVol, true /* slide */);
    checkEventInterval(events, 748, 852, 57, defVol, true /* slide */);
    checkEventInterval(events, 854, 958, 56, defVol, true /* slide */);
    checkEventInterval(events, 960, 1439, 55, defVol, true /* slide */);
}

TEST_F(MidiRenderer_Tests, slideInFromBelow)
{
    constexpr int defVol = 80; // mf

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"slide_in_from_below.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 10);

    checkEventInterval(events, 0, 419, 60, defVol);
    checkEventInterval(events, 420, 438, 60, defVol, true /* slide */);
    checkEventInterval(events, 440, 458, 61, defVol, true /* slide */);
    checkEventInterval(events, 460, 478, 62, defVol, true /* slide */);
    checkEventInterval(events, 480, 959, 63, defVol);
}

TEST_F(MidiRenderer_Tests, slideInFromAbove)
{
    constexpr int defVol = 80; // mf

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"slide_in_from_above.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 10);

    checkEventInterval(events, 0, 419, 60, defVol);
    checkEventInterval(events, 420, 438, 66, defVol, true /* slide */);
    checkEventInterval(events, 440, 458, 65, defVol, true /* slide */);
    checkEventInterval(events, 460, 478, 64, defVol, true /* slide */);
    checkEventInterval(events, 480, 959, 63, defVol);
}

TEST_F(MidiRenderer_Tests, slideOutFromAbove)
{
    constexpr int defVol = 80; // mf

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"slide_out_from_above.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 10);

    checkEventInterval(events, 0, 419, 60, defVol);
    checkEventInterval(events, 420, 438, 59, defVol, true /* slide */);
    checkEventInterval(events, 439, 457, 58, defVol, true /* slide */);
    checkEventInterval(events, 459, 477, 57, defVol, true /* slide */);
    checkEventInterval(events, 480, 959, 60, defVol);
}

TEST_F(MidiRenderer_Tests, slideOutFromBelow)
{
    constexpr int defVol = 80; // mf

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"slide_out_from_below.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 10);

    checkEventInterval(events, 0, 419, 60, defVol);
    checkEventInterval(events, 420, 438, 61, defVol, true /* slide */);
    checkEventInterval(events, 439, 457, 62, defVol, true /* slide */);
    checkEventInterval(events, 459, 477, 63, defVol, true /* slide */);
    checkEventInterval(events, 480, 959, 60, defVol);
}

TEST_F(MidiRenderer_Tests, tremoloSlideIn)
{
    constexpr int defVol = 96; // f

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"tremolo_and_slide_in.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 16);

    checkEventInterval(events, 0, 223, 59, defVol);
    checkEventInterval(events, 224, 447, 59, defVol);
    checkEventInterval(events, 449, 672, 59, defVol);
    checkEventInterval(events, 673, 896, 59, defVol);
    checkEventInterval(events, 901, 919, 56, defVol, true /* slide */);
    checkEventInterval(events, 921, 939, 57, defVol, true /* slide */);
    checkEventInterval(events, 940, 958, 58, defVol, true /* slide */);
    checkEventInterval(events, 960, 1439, 59, defVol);
}

TEST_F(MidiRenderer_Tests, tremoloSlideOut)
{
    constexpr int defVol = 96; // f

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"tremolo_and_slide_out.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 20);

    checkEventInterval(events, 0, 263, 59, defVol);
    checkEventInterval(events, 264, 527, 59, defVol);
    checkEventInterval(events, 529, 792, 59, defVol);
    checkEventInterval(events, 794, 1057, 59, defVol);
    checkEventInterval(events, 1059, 1322, 59, defVol);
    checkEventInterval(events, 1324, 1587, 59, defVol);
    checkEventInterval(events, 1589, 1852, 59, defVol);
    checkEventInterval(events, 1860, 1878, 58, defVol, true /* slide */);
    checkEventInterval(events, 1879, 1897, 57, defVol, true /* slide */);
    checkEventInterval(events, 1898, 1916, 56, defVol, true /* slide */);
}

TEST_F(MidiRenderer_Tests, slideInAndOut)
{
    constexpr int defVol = 80; // mf

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"slide_in_and_out.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 14);

    checkEventInterval(events, 420, 438, 64, defVol, true /* slide */);
    checkEventInterval(events, 440, 458, 65, defVol, true /* slide */);
    checkEventInterval(events, 460, 478, 66, defVol, true /* slide */);
    checkEventInterval(events, 480, 899, 67, defVol);
    checkEventInterval(events, 900, 918, 68, defVol, true /* slide */);
    checkEventInterval(events, 919, 937, 69, defVol, true /* slide */);
    checkEventInterval(events, 939, 957, 70, defVol, true /* slide */);
}

TEST_F(MidiRenderer_Tests, sameStringDifferentStaves)
{
    constexpr int defVol = 80; // mf

    auto midiEvents = renderMidiEvents(u"same_string_diff_staves.mscx", true);
    EventsHolder events = getNoteOnEvents(midiEvents);

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 4);

    checkEventInterval(events, 0, 239, 62, defVol, false, DEFAULT_CHANNEL);
    checkEventInterval(events, 240, 479, 62, defVol, false, DEFAULT_CHANNEL);
    checkEventInterval(events, 0, 1919, 35, defVol, false, DEFAULT_CHANNEL + 1);
}

TEST_F(MidiRenderer_Tests, trillOnHiddenStaff)
{
    constexpr int mfVol = 80;
    constexpr int fVol = 96;

    auto midiEvents = renderMidiEvents(u"trill_on_hidden_staff.mscx");
    EventsHolder events = getNoteOnEvents(midiEvents);

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 2);
    int eventsCount = getEventsCount(events);
    EXPECT_EQ(eventsCount, 18);

    checkEventInterval(events, 0, 1919, 60, mfVol, false, DEFAULT_CHANNEL);
    checkEventInterval(events, 1920, 1979, 79, fVol, false, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 1980, 2039, 81, fVol, false, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 2040, 2099, 79, fVol, false, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 2100, 2159, 81, fVol, false, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 2160, 2219, 79, fVol, false, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 2220, 2279, 81, fVol, false, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 2280, 2339, 79, fVol, false, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 2340, 2399, 81, fVol, false, DEFAULT_CHANNEL + 1);
}

TEST_F(MidiRenderer_Tests, letRingRepeat)
{
    constexpr int defVol = 80; // mf

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"let_ring_repeat.mscx", true, false));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 4);

    checkEventInterval(events, 1920, 3840, 60, defVol, false, DEFAULT_CHANNEL);
    checkEventInterval(events, 2880, 3840, 64, defVol, false, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 5760, 7680, 60, defVol, false, DEFAULT_CHANNEL);
    checkEventInterval(events, 6720, 7680, 64, defVol, false, DEFAULT_CHANNEL + 1);
}

TEST_F(MidiRenderer_Tests, slideToTiedNote)
{
    constexpr int defVol = 96; // f

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"slide_to_tied_note.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 8);

    checkEventInterval(events, 420, 438, 66, defVol, true /* slide */);
    checkEventInterval(events, 440, 458, 67, defVol, true /* slide */);
    checkEventInterval(events, 460, 478, 68, defVol, true /* slide */);
    checkEventInterval(events, 480, 1919, 69, defVol);
}

TEST_F(MidiRenderer_Tests, twoSlides)
{
    constexpr int defVol = 80; // mf

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"two_slides.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 16);

    checkEventInterval(events, 1860, 1878, 56, defVol, true /* slide */);
    checkEventInterval(events, 1880, 1898, 57, defVol, true /* slide */);
    checkEventInterval(events, 1900, 1918, 58, defVol, true /* slide */);
    checkEventInterval(events, 1920, 2339, 59, defVol);
    checkEventInterval(events, 2340, 2358, 60, defVol, true /* slide */);
    checkEventInterval(events, 2360, 2378, 61, defVol, true /* slide */);
    checkEventInterval(events, 2380, 2398, 62, defVol, true /* slide */);
    checkEventInterval(events, 2400, 2879, 63, defVol);
}

TEST_F(MidiRenderer_Tests, reducedSlides)
{
    constexpr int defVol = 80; // mf

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"reduced_slides.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 42);

    /// fret 0: no slides
    checkEventInterval(events, 0, 959, 60, defVol);
    checkEventInterval(events, 960, 1919, 64, defVol);

    /// fret 1: no slides
    checkEventInterval(events, 1920, 2879, 60, defVol);
    checkEventInterval(events, 2880, 3839, 65, defVol);

    /// fret 2: 1 slide
    checkEventInterval(events, 3840, 4738, 60, defVol);
    checkEventInterval(events, 4741, 4799, 65, defVol, true /* slide */);
    checkEventInterval(events, 4800, 5759, 66, defVol);

    /// fret 3: 2 slides
    checkEventInterval(events, 5760, 6658, 60, defVol);
    checkEventInterval(events, 6661, 6689, 65, defVol, true /* slide */);
    checkEventInterval(events, 6691, 6719, 66, defVol, true /* slide */);
    checkEventInterval(events, 6720, 7679, 67, defVol);

    /// fret 4 (or more): 3 slides
    checkEventInterval(events, 7680, 8578, 60, defVol);
    checkEventInterval(events, 8581, 8599, 65, defVol, true /* slide */);
    checkEventInterval(events, 8600, 8618, 66, defVol, true /* slide */);
    checkEventInterval(events, 8619, 8637, 67, defVol, true /* slide */);
    checkEventInterval(events, 8640, 9599, 68, defVol);

    /// fret 0 (but slide in from above) : 3 slides
    checkEventInterval(events, 9600, 10498, 60, defVol);
    checkEventInterval(events, 10501, 10519, 67, defVol, true /* slide */);
    checkEventInterval(events, 10520, 10538, 66, defVol, true /* slide */);
    checkEventInterval(events, 10539, 10557, 65, defVol, true /* slide */);
    checkEventInterval(events, 10560, 11519, 64, defVol);
}

TEST_F(MidiRenderer_Tests, shortNoteBeforeSlide)
{
    constexpr int defVol = 80; // mf

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"short_note_before_slide.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 20);

    /// 32th note before slide
    checkEventInterval(events, 0, 29, 62, defVol);
    checkEventInterval(events, 31, 39, 72, defVol, true /* slide */);
    checkEventInterval(events, 40, 48, 71, defVol, true /* slide */);
    checkEventInterval(events, 50, 58, 70, defVol, true /* slide */);
    checkEventInterval(events, 60, 539, 69, defVol);

    /// 64th note before slide
    checkEventInterval(events, 1920, 1934, 62, defVol);
    checkEventInterval(events, 1936, 1939, 72, defVol, true /* slide */);
    checkEventInterval(events, 1940, 1943, 71, defVol, true /* slide */);
    checkEventInterval(events, 1945, 1948, 70, defVol, true /* slide */);
    checkEventInterval(events, 1950, 2429, 69, defVol);
}

TEST_F(MidiRenderer_Tests, shortNoteWithSlide)
{
    constexpr int defVol = 80; // mf

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"short_note_with_slide.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 16);

    /// 32th note with slide
    checkEventInterval(events, 0, 29, 69, defVol);
    checkEventInterval(events, 30, 38, 68, defVol, true /* slide */);
    checkEventInterval(events, 39, 47, 67, defVol, true /* slide */);
    checkEventInterval(events, 49, 57, 66, defVol, true /* slide */);

    /// 64th note with slide
    checkEventInterval(events, 1920, 1934, 69, defVol);
    checkEventInterval(events, 1935, 1938, 68, defVol, true /* slide */);
    checkEventInterval(events, 1939, 1942, 67, defVol, true /* slide */);
    checkEventInterval(events, 1944, 1947, 66, defVol, true /* slide */);
}

TEST_F(MidiRenderer_Tests, slideInAfterRest)
{
    constexpr int defVol = 80; // mf

    EventsHolder events = getNoteOnEvents(renderMidiEvents(u"slide_in_after_rest.mscx"));

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 8);

    checkEventInterval(events, 420, 438, 60, defVol, true /* slide */);
    checkEventInterval(events, 440, 458, 61, defVol, true /* slide */);
    checkEventInterval(events, 460, 478, 62, defVol, true /* slide */);
    checkEventInterval(events, 480, 959, 63, defVol);
}

/*****************************************************************************

    DISABLED TESTS BELOW

*****************************************************************************/
