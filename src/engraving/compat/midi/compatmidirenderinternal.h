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

#ifndef __RENDERMIDI_H__
#define __RENDERMIDI_H__

#include <memory>

#include "../../dom/measure.h"
#include "../../dom/mscore.h"
#include "../../types/types.h"

#include "pitchwheelrenderer.h"
#include "pausemap.h"
#include "velocitymap.h"

namespace mu::engraving {
class EventsHolder;
class MasterScore;
class Staff;
class Instrument;
class Chord;
// This struct specifies how to render an articulation.
//   atype - the articulation type to implement, such as SymId::ornamentTurn
//   ostyles - the actual ornament has a property called ornamentStyle whose value is
//             a value of type OrnamentStyle.  This ostyles field indicates the
//             the set of ornamentStyles which apply to this rendition.
//   duration - the default duration for each note in the rendition, the final duration
//            rendered might be less than this if an articulation is attached to a note of
//            short duration.
//   prefix - vector of integers. indicating which notes to play at the beginning of rendering the
//            articulation.  0 represents the principle note, 1==> the note diatonically 1 above
//            -1 ==> the note diatonically 1 below.  E.g., in the key of G, if a turn articulation
//            occurs above the note F#, then 0==>F#, 1==>G, -1==>E.
//            These integers indicate which notes actual notes to play when rendering the ornamented
//            note.   However, if the same integer appears several times adjacently such as {0,0,0,1}
//            That means play the notes tied.  e.g., F# followed by G, but the duration of F# is 3x the
//            duration of the G.
//    body   - notes to play comprising the body of the rendered ornament.
//            The body differs from the prefix and suffix in several ways.
//            * body does not support tied notes: {0,0,0,1} means play 4 distinct notes (not tied).
//            * if there is sufficient duration in the principle note, AND repeatp is true, then body
//               will be rendered multiple times, as the duration allows.
//            * to avoid a time gap (or rest) in rendering the articulation, if sustainp is true,
//               then the final note of the body will be sustained to fill the left-over time.
//    suffix - similar to prefix but played once at the end of the rendered ornament.
//    repeatp  - whether the body is repeatable in its entirety.
//    sustainp - whether the final note of the body should be sustained to fill the remaining duration.
struct OrnamentExcursion {
    SymId atype;
    std::set<OrnamentStyle> ostyles;
    int duration;
    std::vector<int> prefix;
    std::vector<int> body;
    bool repeatp;
    bool sustainp;
    std::vector<int> suffix;
};
static const std::set<OrnamentStyle> baroque  = { OrnamentStyle::BAROQUE };
static const std::set<OrnamentStyle> defstyle = { OrnamentStyle::DEFAULT };
static const std::set<OrnamentStyle> any;             // empty set has the special meaning of any-style, rather than no-styles.
static constexpr int _16th = Constants::DIVISION / 4;
static constexpr int _32nd = _16th / 2;
constexpr int SLIDE_DURATION = _32nd;
constexpr int GRACE_BEND_DURATION = _16th;
static const std::vector<OrnamentExcursion> excursions = {
    //  articulation type            set of  duration       body         repeatp      suffix
    //                               styles          prefix                    sustainp
    { SymId::ornamentTurn,                any, _32nd, {},    { 1, 0, -1, 0 },   false, true, {} },
    { SymId::ornamentTurnInverted,        any, _32nd, {},    { -1, 0, 1, 0 },   false, true, {} },
    { SymId::ornamentTurnSlash,           any, _32nd, {},    { -1, 0, 1, 0 },   false, true, {} },
    { SymId::ornamentTrill,           baroque, _32nd, { 1, 0 }, { 1, 0 },        true,  true, {} },
    { SymId::ornamentTrill,          defstyle, _32nd, { 0, 1 }, { 0, 1 },        true,  true, {} },
    { SymId::brassMuteClosed,         baroque, _32nd, { 0, -1 }, { 0, -1 },      true,  true, {} },
    { SymId::ornamentMordent,             any, _32nd, {},    { 0, -1, 0 },     false, true, {} },
    { SymId::ornamentShortTrill,     defstyle, _32nd, {},    { 0, 1, 0 },      false, true, {} },                        // inverted mordent
    { SymId::ornamentShortTrill,      baroque, _32nd, { 1, 0, 1 }, { 0 },         false, true, {} },                        // short trill
    { SymId::ornamentTremblement,         any, _32nd, { 1, 0 }, { 1, 0 },        false, true, {} },
    { SymId::brassMuteClosed,        defstyle, _32nd, {},    { 0 },             false, true, {} },                        // regular hand-stopped brass
    { SymId::ornamentPrallMordent,        any, _32nd, {},    { 1, 0, -1, 0 },   false, true, {} },
    { SymId::ornamentLinePrall,           any, _32nd, { 2, 2, 2 }, { 1, 0 },       true,  true, {} },
    { SymId::ornamentUpPrall,             any, _16th, { -1, 0 }, { 1, 0 },        true,  true, { 1, 0 } },                        // p 144 Ex 152 [1]
    { SymId::ornamentUpMordent,           any, _16th, { -1, 0 }, { 1, 0 },        true,  true, { -1, 0 } },                        // p 144 Ex 152 [1]
    { SymId::ornamentPrecompMordentUpperPrefix, any, _16th, { 1, 1, 1, 0 }, { 1, 0 },    true,  true, {} },                        // p136 Cadence Appuyee [1] [2]
    { SymId::ornamentDownMordent,         any, _16th, { 1, 1, 1, 0 }, { 1, 0 },    true,  true, { -1, 0 } },                        // p136 Cadence Appuyee + mordent [1] [2]
    { SymId::ornamentPrallUp,             any, _16th, { 1, 0 }, { 1, 0 },        true,  true, { -1, 0 } },                        // p136 Double Cadence [1]
    { SymId::ornamentPrallDown,           any, _16th, { 1, 0 }, { 1, 0 },        true,  true, { -1, 0, 0, 0 } },                        // p144 ex 153 [1]
    { SymId::ornamentPrecompSlide,        any, _32nd, {},    { 0 },          false, true, {} }

    // [1] Some of the articulations/ornaments in the excursions table above come from
    // Baroque Music, Style and Performance A Handbook, by Robert Donington,(c) 1982
    // ISBN 0-393-30052-8, W. W. Norton & Company, Inc.

    // [2] In some cases, the example from [1] does not preserve the timing.
    // For example, illustrates 2+1/4 counts per half note.
};

//---------------------------------------------------------
//   CompatMidiRendererInternal
///   MIDI renderer for a score
//---------------------------------------------------------

class CompatMidiRendererInternal
{
public:

    //! @brief helper structure to find channel
    struct ChannelLookup {
        struct LookupData {
            constexpr static int INVALID_STRING = -1;

            int32_t string = INVALID_STRING;
            staff_idx_t staffIdx = 0;
            MidiInstrumentEffect effect = MidiInstrumentEffect::NONE;
            bool harmony = false;

            bool operator<(const LookupData& other) const;
        };

        std::map<int, std::map<LookupData, int> > channelsMap;
        uint32_t maxChannel = 0;

        uint32_t getChannel(uint32_t instrumentChannel, const LookupData& lookupData);
    };

    enum HarmonyChannelSetting {
        DISABLED = -1, // harmony chords should not be played
        DEFAULT = 0,   // harmony chords' channels are setup in default way
        LOOKUP         // harmony chords' channels are setup in specific way
    };

    struct Context
    {
        struct BuiltInArticulation {
            double velocityMultiplier = 1.0;
            int gateTime = 100;
        };

        static std::unordered_map<String, BuiltInArticulation> s_builtInArticulationsValues;

        bool eachStringHasChannel = false; //!to better display the guitar instrument, each string has its own channel
        bool instrumentsHaveEffects = false; //!when effect is applied, new channel should be used
        bool useDefaultArticulations = false; //!using default articulations means ignoring the ones stored for each instrument
        bool applyCaesuras = false; //! to add pauses (caesura) between midi events
        HarmonyChannelSetting harmonyChannelSetting = HarmonyChannelSetting::DEFAULT;
        std::unordered_set<std::string> partsWithMutedHarmony;

        int sndController = CTRL_BREATH;

        std::unordered_map<staff_idx_t, VelocityMap> velocitiesByStaff;
        std::unordered_map<staff_idx_t, VelocityMap> velocityMultiplicationsByStaff;
        std::unordered_map<String, std::unordered_set<String> > articulationsWithoutValuesByInstrument;

        std::shared_ptr<ChannelLookup> channels = std::make_shared<ChannelLookup>();
        std::shared_ptr<PauseMap> pauseMap = std::make_shared<PauseMap>();
    };

    explicit CompatMidiRendererInternal(Score* s);

    void renderScore(EventsHolder& events, const Context& ctx, bool expandRepeats);

    static const int ARTICULATION_CONV_FACTOR { 100000 };
    static bool graceNotesMerged(Chord* chord);

private:

    void renderStaff(EventsHolder& events, const Staff* sctx, PitchWheelRenderer& pitchWheelRenderer);

    void renderSpanners(EventsHolder& events, PitchWheelRenderer& pitchWheelRenderer);
    void doRenderSpanners(EventsHolder& events, Spanner* s, uint32_t channel, PitchWheelRenderer& pitchWheelRenderer,
                          MidiInstrumentEffect effect);

    void collectMeasureEvents(EventsHolder& events, Measure const* m, const Staff* sctx, int tickOffset,
                              PitchWheelRenderer& pitchWheelRenderer, std::array<Chord*, VOICES>& prevChords);
    void doCollectMeasureEvents(EventsHolder& events, Measure const* m, const Staff* sctx, int tickOffset,
                                PitchWheelRenderer& pitchWheelRenderer, std::array<Chord*, VOICES>& prevChords);

    struct ChordParams {
        bool letRing = false;
        bool palmMute = false;
        int endLetRingTick = 0;
    };

    ChordParams collectChordParams(const Chord* chord, int tickOffset) const;
    void collectGraceBeforeChordEvents(Chord* chord, Chord* prevChord, EventsHolder& events, double veloMultiplier, Staff* st,
                                       int tickOffset, PitchWheelRenderer& pitchWheelRenderer, MidiInstrumentEffect effect);
    void fillArticulationsInfo();

    Score* score = nullptr;
    Context m_context;
};
} // namespace mu::engraving

#endif
