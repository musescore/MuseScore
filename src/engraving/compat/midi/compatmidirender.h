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

#ifndef MU_ENGRAVING_COMPATMIDIRENDER_H
#define MU_ENGRAVING_COMPATMIDIRENDER_H

#include <set>
#include <cmath>
#include <tuple>

#include "compat/midi/event.h"
#include "style/style.h"
#include "types/constants.h"

#include "compatmidirenderinternal.h"

#include "dom/arpeggio.h"
#include "dom/articulation.h"
#include "dom/chord.h"
#include "dom/durationtype.h"
#include "dom/dynamic.h"
#include "dom/easeInOut.h"
#include "dom/glissando.h"
#include "dom/hairpin.h"
#include "dom/instrument.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/measurerepeat.h"
#include "dom/note.h"
#include "dom/noteevent.h"
#include "dom/palmmute.h"
#include "dom/part.h"
#include "dom/repeatlist.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/sig.h"
#include "dom/slur.h"
#include "dom/staff.h"
#include "dom/stafftextbase.h"
#include "dom/swing.h"
#include "dom/synthesizerstate.h"
#include "dom/tempo.h"
#include "dom/tie.h"
#include "dom/trill.h"
#include "dom/undo.h"
#include "dom/utils.h"
#include "dom/volta.h"
#include "dom/capo.h"

#include "log.h"

namespace mu::engraving {
class CompatMidiRender
{
public:
    static void renderScore(Score* score, EventsHolder& events, const CompatMidiRendererInternal::Context& ctx, bool expandRepeats);
    static int getControllerForSnd(Score* score, int globalSndController);
    static void createPlayEvents(const Score* score, Measure const* start = nullptr, Measure const* end = nullptr,
                                 const CompatMidiRendererInternal::Context& context = CompatMidiRendererInternal::Context());

    static int tick(const CompatMidiRendererInternal::Context& ctx, int tick);
private:
    static void createPlayEvents(const Score* score, const CompatMidiRendererInternal::Context& context, Chord* chord,
                                 Chord* prevChord = nullptr, Chord* nextChord = nullptr);
    static void createGraceNotesPlayEvents(const Score* score, const Fraction& tick, Chord* chord, int& ontime, int& trailtime);
    static std::vector<NoteEventList> renderChord(const CompatMidiRendererInternal::Context& context, Chord* chord, Chord* prevChord,
                                                  int gateTime, int ontime, int trailtime);
    static void renderArpeggio(Chord* chord, std::vector<NoteEventList>& ell, int ontime);
    static void renderTremolo(Chord* chord, std::vector<NoteEventList>& ell, int& ontime, double tremoloPartOfChord = 1.0);
    static void renderChordArticulation(const CompatMidiRendererInternal::Context& context, Chord* chord, std::vector<NoteEventList>& ell,
                                        int& gateTime, double graceOnBeatProportion, bool tremoloBefore = false);
    static void updateGateTime(const Instrument* instr, int& gateTime, const String& articulationName,
                               const CompatMidiRendererInternal::Context& context);
    static void renderGlissando(NoteEventList* events, Note* notestart, double graceOnBeatProportion, bool tremoloBefore = false);
    static bool renderNoteArticulation(NoteEventList* events, Note* note, bool chromatic, int requestedTicksPerNote,
                                       const std::vector<int>& prefix, const std::vector<int>& body, bool repeatp, bool sustainp,
                                       const std::vector<int>& suffix, int fastestFreq = 64, int slowestFreq = 8, // 64 Hz and 8 Hz
                                       double graceOnBeatProportion = 0, bool tremoloBefore = false);
    static bool renderNoteArticulation(NoteEventList* events, Note* note, bool chromatic, SymId articulationType,
                                       OrnamentStyle ornamentStyle);
    static bool renderNoteArticulation(NoteEventList* events, Note* note, bool chromatic, TrillType trillType, OrnamentStyle ornamentStyle);
    static void createSlideInNotePlayEvents(Note* note, Chord* prevChord, NoteEventList* el);
    static void createSlideOutNotePlayEvents(Note* note, NoteEventList* el, int onTime, bool hasTremolo);
    // Helpers
    static Chord* getChordFromSegment(Segment* segment, track_idx_t track);
    static Trill* findFirstTrill(Chord* chord);
    static int adjustTrailtime(int trailtime, Chord* currentChord, Chord* nextChord);
    static bool noteIsGlissandoStart(Note* note);
    static int slideLengthChordDependent(Chord* chord);
    static std::set<size_t> getNotesIndexesToRender(Chord* chord);
    static Glissando* backGlissando(Note* note);
    static bool isGlissandoValid(Glissando* glissando);
    static const Drumset* getDrumset(const Chord* chord);
    static int totalTiedNoteTicks(Note* note);
};
}

#endif //MU_ENGRAVING_COMPATMIDIRENDER_H
