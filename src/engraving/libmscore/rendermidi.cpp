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

/**
 \file
 render score into event list
*/

#include <set>
#include <cmath>
#include <tuple>

#include "compat/midi/event.h"
#include "compat/midi/midirender.h"
#include "style/style.h"
#include "types/constants.h"

#include "arpeggio.h"
#include "articulation.h"
#include "bend.h"
#include "changeMap.h"
#include "chord.h"
#include "durationtype.h"
#include "dynamic.h"
#include "easeInOut.h"
#include "glissando.h"
#include "hairpin.h"
#include "instrument.h"
#include "masterscore.h"
#include "measure.h"
#include "measurerepeat.h"
#include "note.h"
#include "noteevent.h"
#include "palmmute.h"
#include "part.h"
#include "repeatlist.h"
#include "score.h"
#include "segment.h"
#include "sig.h"
#include "slur.h"
#include "staff.h"
#include "stafftextbase.h"
#include "stretchedbend.h"
#include "swing.h"
#include "synthesizerstate.h"
#include "tempo.h"
#include "tie.h"
#include "tremolo.h"
#include "trill.h"
#include "undo.h"
#include "utils.h"
#include "volta.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   updateSwing
//---------------------------------------------------------

void Score::updateSwing()
{
    for (Staff* s : _staves) {
        s->clearSwingList();
    }
    Measure* fm = firstMeasure();
    if (!fm) {
        return;
    }
    for (Segment* s = fm->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        for (const EngravingItem* e : s->annotations()) {
            if (!e->isStaffTextBase()) {
                continue;
            }
            const StaffTextBase* st = toStaffTextBase(e);
            if (st->xmlText().isEmpty()) {
                continue;
            }
            Staff* staff = st->staff();
            if (!st->swing()) {
                continue;
            }
            SwingParameters sp;
            sp.swingRatio = st->swingParameters().swingRatio;
            sp.swingUnit = st->swingParameters().swingUnit;
            if (st->systemFlag()) {
                for (Staff* sta : _staves) {
                    sta->insertIntoSwingList(s->tick(), sp);
                }
            } else {
                staff->insertIntoSwingList(s->tick(), sp);
            }
        }
    }
}

//---------------------------------------------------------
//   updateCapo
//---------------------------------------------------------

void Score::updateCapo()
{
    for (Staff* s : _staves) {
        s->clearCapoList();
    }
    Measure* fm = firstMeasure();
    if (!fm) {
        return;
    }
    for (Segment* s = fm->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        for (EngravingItem* e : s->annotations()) {
            if (e->isHarmony()) {
                toHarmony(e)->realizedHarmony().setDirty(true);
            }
            if (!e->isStaffTextBase()) {
                continue;
            }
            const StaffTextBase* st = toStaffTextBase(e);
            if (st->xmlText().isEmpty()) {
                continue;
            }
            Staff* staff = st->staff();
            if (st->capo() == 0) {
                continue;
            }
            staff->insertIntoCapoList(s->tick(), st->capo());
        }
    }
}

//---------------------------------------------------------
//   updateChannel
//---------------------------------------------------------

void Score::updateChannel()
{
    for (Staff* s : staves()) {
        for (voice_idx_t i = 0; i < VOICES; ++i) {
            s->clearChannelList(i);
        }
    }
    Measure* fm = firstMeasure();
    if (!fm) {
        return;
    }
    for (Segment* s = fm->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        for (const EngravingItem* e : s->annotations()) {
            if (e->isInstrumentChange()) {
                for (Staff* staff : e->part()->staves()) {
                    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
                        staff->insertIntoChannelList(voice, s->tick(), 0);
                    }
                }
                continue;
            }
            if (!e->isStaffTextBase()) {
                continue;
            }
            const StaffTextBase* st = toStaffTextBase(e);
            for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
                String an(st->channelName(voice));
                if (an.isEmpty()) {
                    continue;
                }
                Staff* staff = Score::staff(st->staffIdx());
                int a = staff->part()->instrument(s->tick())->channelIdx(an);
                if (a != -1) {
                    staff->insertIntoChannelList(voice, s->tick(), a);
                }
            }
        }
    }

    for (auto it = spanner().cbegin(); it != spanner().cend(); ++it) {
        Spanner* spanner = (*it).second;
        if (spanner->isPalmMute()) {
            toPalmMute(spanner)->setChannel();
        }
        if (spanner->isVolta()) {
            toVolta(spanner)->setChannel();
        }
    }

    for (Segment* s = fm->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        for (Staff* st : staves()) {
            track_idx_t strack = st->idx() * VOICES;
            track_idx_t etrack = strack + VOICES;
            for (track_idx_t track = strack; track < etrack; ++track) {
                if (!s->element(track)) {
                    continue;
                }
                EngravingItem* e = s->element(track);
                if (e->type() != ElementType::CHORD) {
                    continue;
                }
                Chord* c = toChord(e);
                size_t channel = st->channel(c->tick(), c->voice());
                Instrument* instr = c->part()->instrument(c->tick());
                if (channel >= instr->channel().size()) {
                    LOGD() << "Channel " << channel << " too high. Max " << instr->channel().size();
                    channel = 0;
                }
                for (Note* note : c->notes()) {
                    if (note->hidden()) {
                        continue;
                    }
                    note->setSubchannel(static_cast<int>(channel));
                }
            }
        }
    }
}

//---------------------------------------------------------
//   updateHairpin
//---------------------------------------------------------

void Score::updateHairpin(Hairpin* h)
{
    Staff* st = h->staff();
    Fraction tick  = h->tick();
    Fraction tick2 = h->tick2();
    int veloChange  = h->veloChange();
    ChangeMethod method = h->veloChangeMethod();

    // Make the change negative when the hairpin is a diminuendo
    HairpinType htype = h->hairpinType();
    ChangeDirection direction = ChangeDirection::INCREASING;
    if (htype == HairpinType::DECRESC_HAIRPIN || htype == HairpinType::DECRESC_LINE) {
        veloChange *= -1;
        direction = ChangeDirection::DECREASING;
    }

    switch (h->dynRange()) {
    case DynamicRange::STAFF:
        st->velocities().addRamp(tick, tick2, veloChange, method, direction);
        break;
    case DynamicRange::PART:
        for (Staff* s : st->part()->staves()) {
            s->velocities().addRamp(tick, tick2, veloChange, method, direction);
        }
        break;
    case DynamicRange::SYSTEM:
        for (Staff* s : _staves) {
            s->velocities().addRamp(tick, tick2, veloChange, method, direction);
        }
        break;
    }
}

//---------------------------------------------------------
//   updateVelo
//    calculate velocity for all notes
//---------------------------------------------------------

void Score::updateVelo()
{
    //
    //    collect Dynamics
    //
    if (!firstMeasure()) {
        return;
    }

    for (Staff* st : _staves) {
        st->velocities().clear();
        st->velocityMultiplications().clear();
    }
    for (size_t staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
        Staff* st      = staff(staffIdx);
        ChangeMap& velo = st->velocities();
        ChangeMap& mult = st->velocityMultiplications();
        Part* prt      = st->part();
        size_t partStaves = prt->nstaves();
        staff_idx_t partStaff  = Score::staffIdx(prt);

        for (Segment* s = firstMeasure()->first(); s; s = s->next1()) {
            Fraction tick = s->tick();
            for (const EngravingItem* e : s->annotations()) {
                if (e->staffIdx() != staffIdx) {
                    continue;
                }
                if (e->type() != ElementType::DYNAMIC) {
                    continue;
                }
                const Dynamic* d = toDynamic(e);
                int v            = d->velocity();

                // treat an invalid dynamic as no change, i.e. a dynamic set to 0
                if (v < 1) {
                    continue;
                }

                v = std::clamp(v, 1, 127);             //  illegal values

                // If a dynamic has 'velocity change' update its ending
                int change = d->changeInVelocity();
                ChangeDirection direction = ChangeDirection::INCREASING;
                if (change < 0) {
                    direction = ChangeDirection::DECREASING;
                }

                staff_idx_t dStaffIdx = d->staffIdx();
                switch (d->dynRange()) {
                case DynamicRange::STAFF:
                    if (dStaffIdx == staffIdx) {
                        velo.addFixed(tick, v);
                        if (change != 0) {
                            Fraction etick = tick + d->velocityChangeLength();
                            ChangeMethod method = ChangeMethod::NORMAL;
                            velo.addRamp(tick, etick, change, method, direction);
                        }
                    }
                    break;
                case DynamicRange::PART:
                    if (dStaffIdx >= partStaff && dStaffIdx < partStaff + partStaves) {
                        for (staff_idx_t i = partStaff; i < partStaff + partStaves; ++i) {
                            ChangeMap& stVelo = staff(i)->velocities();
                            stVelo.addFixed(tick, v);
                            if (change != 0) {
                                Fraction etick = tick + d->velocityChangeLength();
                                ChangeMethod method = ChangeMethod::NORMAL;
                                stVelo.addRamp(tick, etick, change, method, direction);
                            }
                        }
                    }
                    break;
                case DynamicRange::SYSTEM:
                    for (size_t i = 0; i < nstaves(); ++i) {
                        ChangeMap& stVelo = staff(i)->velocities();
                        stVelo.addFixed(tick, v);
                        if (change != 0) {
                            Fraction etick = tick + d->velocityChangeLength();
                            ChangeMethod method = ChangeMethod::NORMAL;
                            stVelo.addRamp(tick, etick, change, method, direction);
                        }
                    }
                    break;
                }
            }

            if (s->isChordRestType()) {
                for (size_t i = staffIdx * VOICES; i < (staffIdx + 1) * VOICES; ++i) {
                    EngravingItem* el = s->element(i);
                    if (!el || !el->isChord()) {
                        continue;
                    }

                    Chord* chord = toChord(el);
                    Instrument* instr = chord->part()->instrument();

                    double veloMultiplier = 1;
                    for (Articulation* a : chord->articulations()) {
                        if (a->playArticulation()) {
                            veloMultiplier *= instr->getVelocityMultiplier(a->articulationName());
                        }
                    }

                    if (veloMultiplier == 1.0) {
                        continue;
                    }

                    // TODO this should be a (configurable?) constant somewhere
                    static Fraction ARTICULATION_CHANGE_TIME_MAX = Fraction(1, 16);
                    Fraction ARTICULATION_CHANGE_TIME = std::min(s->ticks(), ARTICULATION_CHANGE_TIME_MAX);
                    int start = veloMultiplier * MidiRenderer::ARTICULATION_CONV_FACTOR;
                    int change = (veloMultiplier - 1) * MidiRenderer::ARTICULATION_CONV_FACTOR;
                    mult.addFixed(chord->tick(), start);
                    mult.addRamp(chord->tick(),
                                 chord->tick() + ARTICULATION_CHANGE_TIME, change, ChangeMethod::NORMAL, ChangeDirection::DECREASING);
                }
            }
        }

        for (const auto& sp : _spanner.map()) {
            Spanner* s = sp.second;
            if (s->type() != ElementType::HAIRPIN || sp.second->staffIdx() != staffIdx) {
                continue;
            }
            Hairpin* h = toHairpin(s);
            updateHairpin(h);
        }
    }

    for (Staff* st : _staves) {
        st->velocities().cleanup();
        st->velocityMultiplications().cleanup();
    }

    for (auto it = spanner().cbegin(); it != spanner().cend(); ++it) {
        Spanner* spanner = (*it).second;
        if (!spanner->isVolta()) {
            continue;
        }
        Volta* volta = toVolta(spanner);
        volta->setVelocity();
    }
}

const Drumset* getDrumset(const Chord* chord)
{
    if (chord->staff() && chord->staff()->isDrumStaff(chord->tick())) {
        const Drumset* ds = chord->staff()->part()->instrument(chord->tick())->drumset();
        return ds;
    }
    return nullptr;
}

//---------------------------------------------------------
//   renderTremolo
//---------------------------------------------------------

static void renderTremolo(Chord* chord, std::vector<NoteEventList>& ell, double tremoloPartOfChord = 1.0)
{
    Segment* seg = chord->segment();
    Tremolo* tremolo = chord->tremolo();
    int notes = int(chord->notes().size());

    // check if tremolo was rendered before for drum staff
    const Drumset* ds = getDrumset(chord);
    if (ds) {
        for (Note* n : chord->notes()) {
            DrumInstrumentVariant div = ds->findVariant(n->pitch(), chord->articulations(), chord->tremolo());
            if (div.pitch != INVALID_PITCH && div.tremolo == tremolo->tremoloType()) {
                return;         // already rendered
            }
        }
    }

    // we cannot render buzz roll with MIDI events only
    if (tremolo->tremoloType() == TremoloType::BUZZ_ROLL) {
        return;
    }

    // render tremolo with multiple events
    if (chord->tremoloChordType() == TremoloChordType::TremoloFirstNote) {
        int t = Constants::division / (1 << (tremolo->lines() + chord->durationType().hooks()));
        if (t == 0) {   // avoid crash on very short tremolo
            t = 1;
        }
        SegmentType st = SegmentType::ChordRest;
        Segment* seg2 = seg->next(st);
        track_idx_t track = chord->track();
        while (seg2 && !seg2->element(track)) {
            seg2 = seg2->next(st);
        }

        if (!seg2) {
            return;
        }

        EngravingItem* s2El = seg2->element(track);
        if (s2El) {
            if (!s2El->isChord()) {
                return;
            }
        } else {
            return;
        }

        Chord* c2 = toChord(s2El);
        if (c2->type() == ElementType::CHORD) {
            int notes2 = int(c2->notes().size());
            int tnotes = std::max(notes, notes2);
            int tticks = chord->ticks().ticks() * 2;       // use twice the size
            int n = tticks / t;
            n /= 2;
            int l = 2000 * t / tticks;
            for (int k = 0; k < tnotes; ++k) {
                NoteEventList* events;
                if (k < notes) {
                    // first chord has note
                    events = &ell[k];
                    events->clear();
                } else {
                    // otherwise reuse note 0
                    events = &ell[0];
                }
                if (k < notes && k < notes2) {
                    // both chords have note
                    int p1 = chord->notes()[k]->pitch();
                    int p2 = c2->notes()[k]->pitch();
                    int dpitch = p2 - p1;
                    for (int i = 0; i < n; ++i) {
                        events->push_back(NoteEvent(0, l * i * 2, l));
                        events->push_back(NoteEvent(dpitch, l * i * 2 + l, l));
                    }
                } else if (k < notes) {
                    // only first chord has note
                    for (int i = 0; i < n; ++i) {
                        events->push_back(NoteEvent(0, l * i * 2, l));
                    }
                } else {
                    // only second chord has note
                    // reuse note 0 of first chord
                    int p1 = chord->notes()[0]->pitch();
                    int p2 = c2->notes()[k]->pitch();
                    int dpitch = p2 - p1;
                    for (int i = 0; i < n; ++i) {
                        events->push_back(NoteEvent(dpitch, l * i * 2 + l, l));
                    }
                }
            }
        } else {
            LOGD("Chord::renderTremolo: cannot find 2. chord");
        }
    } else if (chord->tremoloChordType() == TremoloChordType::TremoloSecondNote) {
        for (int k = 0; k < notes; ++k) {
            NoteEventList* events = &(ell)[k];
            events->clear();
        }
    } else if (chord->tremoloChordType() == TremoloChordType::TremoloSingle) {
        int t = Constants::division / (1 << (tremolo->lines() + chord->durationType().hooks()));
        if (t == 0) {   // avoid crash on very short tremolo
            t = 1;
        }
        int n = chord->ticks().ticks() / t * tremoloPartOfChord;
        int l = 1000 / n * tremoloPartOfChord;
        for (int k = 0; k < notes; ++k) {
            NoteEventList* events = &(ell)[k];
            events->clear();
            for (int i = 0; i < n; ++i) {
                events->push_back(NoteEvent(0, l * i, l));
            }
        }
    }
}

//---------------------------------------------------------
//   renderArpeggio
//---------------------------------------------------------

void renderArpeggio(Chord* chord, std::vector<NoteEventList>& ell)
{
    int notes = int(chord->notes().size());
    int l = 64;
    while (l && (l * notes > chord->upNote()->playTicks())) {
        l = 2 * l / 3;
    }
    int start, end, step;
    bool up = chord->arpeggio()->arpeggioType() != ArpeggioType::DOWN && chord->arpeggio()->arpeggioType() != ArpeggioType::DOWN_STRAIGHT;
    if (up) {
        start = 0;
        end   = notes;
        step  = 1;
    } else {
        start = notes - 1;
        end   = -1;
        step  = -1;
    }
    int j = 0;
    for (int i = start; i != end; i += step) {
        NoteEventList* events = &(ell)[i];
        events->clear();

        auto tempoRatio = chord->score()->tempomap()->tempo(chord->tick().ticks()).val / Constants::defaultTempo.val;
        int ot = (l * j * 1000) / chord->upNote()->playTicks()
                 * tempoRatio * chord->arpeggio()->Stretch();

        events->push_back(NoteEvent(0, ot, 1000 - ot));
        j++;
    }
}

//---------------------------------------------------------
// totalTiedNoteTicks
//      return the total of the actualTicks of the given note plus
//      the chain of zero or more notes tied to it to the right.
//---------------------------------------------------------

int totalTiedNoteTicks(Note* note)
{
    Fraction total = note->chord()->actualTicks();
    while (note->tieFor() && note->tieFor()->endNote() && (note->chord()->tick() < note->tieFor()->endNote()->chord()->tick())) {
        note = note->tieFor()->endNote();
        total += note->chord()->actualTicks();
    }
    return total.ticks();
}

//---------------------------------------------------------
//   noteHasGlissando
// true if note is the end of a glissando
//---------------------------------------------------------

static bool noteHasGlissando(Note* note)
{
    for (Spanner* spanner : note->spannerFor()) {
        if ((spanner->type() == ElementType::GLISSANDO)
            && spanner->endElement()
            && (ElementType::NOTE == spanner->endElement()->type())) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   renderNoteArticulation
// prefix, vector of int, normally something like {0,-1,0,1} modeling the prefix of tremblement relative to the base note
// body, vector of int, normally something like {0,-1,0,1} modeling the possibly repeated tremblement relative to the base note
// tickspernote, number of ticks, either _16h or _32nd, i.e., Constants::division/4 or Constants::division/8
// repeatp, true means repeat the body as many times as possible to fill the time slice.
// sustainp, true means the last note of the body is sustained to fill remaining time slice
//---------------------------------------------------------

static bool renderNoteArticulation(NoteEventList* events, Note* note, bool chromatic, int requestedTicksPerNote,
                                   const std::vector<int>& prefix, const std::vector<int>& body,
                                   bool repeatp, bool sustainp, const std::vector<int>& suffix,
                                   int fastestFreq = 64, int slowestFreq = 8, // 64 Hz and 8 Hz
                                   double graceOnBeatProportion = 0, bool clearPrevEvents = true)
{
    if (clearPrevEvents) {
        events->clear();
    }

    Chord* chord = note->chord();
    int maxticks = totalTiedNoteTicks(note);
    int space = 1000 * maxticks;
    int numrepeat = 1;
    int sustain   = 0;
    int ontime    = 0;

    ///@NOTE grace note + glissando combination is treated separetely, in this method grace notes for glissando are not added
    /// graceOnBeatProportion is used for calculating grace note "on beat" offset
    int gnb = noteHasGlissando(note) ? 0 : int(note->chord()->graceNotesBefore().size());
    int p = int(prefix.size());
    int b = int(body.size());
    int s = int(suffix.size());
    int gna = int(note->chord()->graceNotesAfter().size());

    int ticksPerNote = 0;

    if (gnb + p + b + s + gna <= 0) {
        return false;
    }

    Fraction tick = chord->tick();
    BeatsPerSecond tempo = chord->score()->tempo(tick);
    int ticksPerSecond = tempo.val * Constants::division;

    int minTicksPerNote = int(ticksPerSecond / fastestFreq);
    int maxTicksPerNote = (0 == slowestFreq) ? 0 : int(ticksPerSecond / slowestFreq);

    // for fast tempos, we have to slow down the tremblement frequency, i.e., increase the ticks per note
    if (requestedTicksPerNote >= minTicksPerNote) {
    } else { // try to divide the requested frequency by a power of 2 if possible, if not, use the maximum frequency, ie., minTicksPerNote
        ticksPerNote = requestedTicksPerNote;
        while (ticksPerNote < minTicksPerNote) {
            ticksPerNote *= 2;       // decrease the tremblement frequency
        }
        if (ticksPerNote > maxTicksPerNote) {
            ticksPerNote = minTicksPerNote;
        }
    }

    ticksPerNote = std::max(requestedTicksPerNote, minTicksPerNote);

    if (slowestFreq <= 0) { // no slowest freq given such as something silly like glissando with 4 notes over 8 counts.
    } else if (ticksPerNote <= maxTicksPerNote) { // in a good range, so we don't need to adjust ticksPerNote
    } else {
        // for slow tempos, such as adagio, we may need to speed up the tremblement frequency, i.e., decrease the ticks per note, to make it sound reasonable.
        ticksPerNote = requestedTicksPerNote;
        while (ticksPerNote > maxTicksPerNote) {
            ticksPerNote /= 2;
        }
        if (ticksPerNote < minTicksPerNote) {
            ticksPerNote = minTicksPerNote;
        }
    }
    // calculate whether to shorten the duration value.
    if (ticksPerNote * (gnb + p + b + s + gna) <= maxticks) {
        // plenty of space to play the notes without changing the requested trill note duration
    } else if (ticksPerNote == minTicksPerNote) {
        return false;     // the ornament is impossible to implement respecting the minimum duration and all the notes it contains
    } else {
        ticksPerNote = maxticks / (gnb + p + b + s + gna);      // integer division ignoring remainder
        if (slowestFreq <= 0) {
        } else if (ticksPerNote < minTicksPerNote) {
            return false;
        }
    }

    int millespernote = space * ticksPerNote / maxticks;     // rescale duration into per mille

    // local function:
    // look ahead in the given vector to see if the current note is the same pitch as the next note or next several notes.
    // If so, increment the duration by the appropriate note duration, and increment the index, j, to the next note index
    // of a different pitch.
    // The total duration of the tied note is returned, and the index is modified.
    auto tieForward = [millespernote](int& j, const std::vector<int>& vec) {
        int size = int(vec.size());
        int duration = millespernote;
        while (j < size - 1 && vec[j] == vec[j + 1]) {
            duration += millespernote;
            j++;
        }
        return duration;
    };

    // local function:
    //   append a NoteEvent either by calculating an articulationExcursion or by
    //   the given chromatic relative pitch.
    //   RETURNS the new ontime value.  The caller is expected to assign this value.
    auto makeEvent
        = [note, chord, chromatic, events](int pitch, int ontime, int duration,
                                           double velocityMultiplier = NoteEvent::DEFAULT_VELOCITY_MULTIPLIER, bool play = true) {
        if (note->ghost()) {
            velocityMultiplier *= NoteEvent::GHOST_VELOCITY_MULTIPLIER;
        }
        events->push_back(NoteEvent(chromatic ? pitch : chromaticPitchSteps(note, note, pitch),
                                    ontime / chord->actualTicks().ticks(),
                                    duration / chord->actualTicks().ticks(), velocityMultiplier, play));

        return ontime + duration;
    };

    // local function:
    //    Given a chord from a grace note, (normally the chord contains a single note) and create
    //    a NoteEvent as if the grace note were part of the articulation (such as trill).  This
    //    local function works for the graceNotesBefore() and also graceNotesAfter().
    //    If the grace note has play=false, then it will sound as a rest, but the other grace
    //    notes will still play.  This means graceExtend simply omits the call to append( NoteEvent(...))
    //    but still updates ontime +=millespernote.
    //    RETURNS the new value of ontime, so caller must make an assignment to the return value.
    auto graceExtend = [millespernote, chord, events](int notePitch, std::vector<Chord*> graceNotes, int ontime) {
        for (Chord* c : graceNotes) {
            for (Note* n : c->notes()) {
                // NoteEvent takes relative pitch as first argument.
                // The pitch is relative to the pitch of the note, the event is rendering
                if (n->play()) {
                    events->push_back(NoteEvent(n->pitch() - notePitch,
                                                ontime / chord->actualTicks().ticks(),
                                                millespernote / chord->actualTicks().ticks()));
                }
            }
            ontime += millespernote;
        }
        return ontime;
    };

    // calculate the number of times to repeat the body, and sustain the last note of the body
    // 1000 = P + numrepeat*B+sustain + S
    if (repeatp) {
        numrepeat = (space - millespernote * (gnb + p + s + gna)) / (millespernote * b);
    }
    if (sustainp) {
        sustain   = space - millespernote * (gnb + p + numrepeat * b + s + gna);
    }
    // render the graceNotesBefore
    ///@NOTE grace note + glissando combination is treated separetely
    if (!noteHasGlissando(note)) {
        ontime = graceExtend(note->pitch(), note->chord()->graceNotesBefore(), ontime);
    }

    // render the prefix
    for (int j = 0; j < p; j++) {
        ontime = makeEvent(prefix[j], ontime, tieForward(j, prefix));
    }

    if (b > 0) {
        // Check that we are doing a glissando
        bool isGlissando = false;
        std::vector<int> onTimes;
        for (Spanner* spanner : note->spannerFor()) {
            if (spanner->type() == ElementType::GLISSANDO) {
                Glissando* glissando = toGlissando(spanner);
                EaseInOut easeInOut(static_cast<double>(glissando->easeIn()) / 100.0,
                                    static_cast<double>(glissando->easeOut()) / 100.0);

                // shifting glissando sounds to the second half of glissando duration
                int totalDuration = millespernote * b;
                int glissandoDuration = totalDuration * (1 - graceOnBeatProportion) / 2;
                easeInOut.timeList(b, glissandoDuration, &onTimes);
                if (!onTimes.empty()) {
                    onTimes[0] += graceOnBeatProportion * totalDuration;
                }

                for (size_t i = 1; i < onTimes.size(); i++) {
                    onTimes[i] += totalDuration - glissandoDuration;
                }

                isGlissando = true;
                break;
            }
        }
        if (isGlissando) {
            const double defaultVelocityMultiplier = 1.0;
            const double glissandoVelocityMultiplier = NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

            // render the body, i.e. the glissando
            if (onTimes.size() > 1) {
                makeEvent(body[0], onTimes[0], onTimes[1] - onTimes[0],
                          defaultVelocityMultiplier, !note->tieBack());
            }

            for (int j = 1; j < b - 1; j++) {
                makeEvent(body[j], onTimes[j], onTimes[j + 1] - onTimes[j],
                          glissandoVelocityMultiplier);
            }

            if (b > 1) {
                makeEvent(body[b - 1], onTimes[b - 1], (millespernote * b - onTimes[b - 1]) + sustain, glissandoVelocityMultiplier);
            }
        } else {
            // render the body, but not the final repetition
            for (int r = 0; r < numrepeat - 1; r++) {
                for (int j = 0; j < b; j++) {
                    ontime = makeEvent(body[j], ontime, millespernote);
                }
            }
            // render the final repetition of body, but not the final note of the repetition
            for (int j = 0; j < b - 1; j++) {
                ontime = makeEvent(body[j], ontime, millespernote);
            }
            // render the final note of the final repeat of body
            ontime = makeEvent(body[b - 1], ontime, millespernote + sustain);
        }
    }
    // render the suffix
    for (int j = 0; j < s; j++) {
        ontime = makeEvent(suffix[j], ontime, tieForward(j, suffix));
    }
    // render graceNotesAfter
    graceExtend(note->pitch(), note->chord()->graceNotesAfter(), ontime);
    return true;
}

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

namespace {
std::set<OrnamentStyle> baroque  = { OrnamentStyle::BAROQUE };
std::set<OrnamentStyle> defstyle = { OrnamentStyle::DEFAULT };
std::set<OrnamentStyle> any; // empty set has the special meaning of any-style, rather than no-styles.
int _16th = Constants::division / 4;
int _32nd = _16th / 2;

std::vector<OrnamentExcursion> excursions = {
    //  articulation type            set of  duration       body         repeatp      suffix
    //                               styles          prefix                    sustainp
    { SymId::ornamentTurn,                any, _32nd, {},    { 1, 0, -1, 0 },   false, true, {} },
    { SymId::ornamentTurnInverted,        any, _32nd, {},    { -1, 0, 1, 0 },   false, true, {} },
    { SymId::ornamentTurnSlash,           any, _32nd, {},    { -1, 0, 1, 0 },   false, true, {} },
    { SymId::ornamentTrill,           baroque, _32nd, { 1, 0 }, { 1, 0 },        true,  true, {} },
    { SymId::ornamentTrill,          defstyle, _32nd, { 0, 1 }, { 0, 1 },        true,  true, {} },
    { SymId::brassMuteClosed,         baroque, _32nd, { 0, -1 }, { 0, -1 },      true,  true, {} },
    { SymId::ornamentMordent,             any, _32nd, {},    { 0, -1, 0 },     false, true, {} },
    { SymId::ornamentShortTrill,     defstyle, _32nd, {},    { 0, 1, 0 },      false, true, {} },// inverted mordent
    { SymId::ornamentShortTrill,      baroque, _32nd, { 1, 0, 1 }, { 0 },         false, true, {} },// short trill
    { SymId::ornamentTremblement,         any, _32nd, { 1, 0 }, { 1, 0 },        false, true, {} },
    { SymId::brassMuteClosed,        defstyle, _32nd, {},    { 0 },             false, true, {} },// regular hand-stopped brass
    { SymId::ornamentPrallMordent,        any, _32nd, {},    { 1, 0, -1, 0 },   false, true, {} },
    { SymId::ornamentLinePrall,           any, _32nd, { 2, 2, 2 }, { 1, 0 },       true,  true, {} },
    { SymId::ornamentUpPrall,             any, _16th, { -1, 0 }, { 1, 0 },        true,  true, { 1, 0 } },// p 144 Ex 152 [1]
    { SymId::ornamentUpMordent,           any, _16th, { -1, 0 }, { 1, 0 },        true,  true, { -1, 0 } },// p 144 Ex 152 [1]
    { SymId::ornamentPrecompMordentUpperPrefix, any, _16th, { 1, 1, 1, 0 }, { 1, 0 },    true,  true, {} },// p136 Cadence Appuyee [1] [2]
    { SymId::ornamentDownMordent,         any, _16th, { 1, 1, 1, 0 }, { 1, 0 },    true,  true, { -1, 0 } },// p136 Cadence Appuyee + mordent [1] [2]
    { SymId::ornamentPrallUp,             any, _16th, { 1, 0 }, { 1, 0 },        true,  true, { -1, 0 } },// p136 Double Cadence [1]
    { SymId::ornamentPrallDown,           any, _16th, { 1, 0 }, { 1, 0 },        true,  true, { -1, 0, 0, 0 } },// p144 ex 153 [1]
    { SymId::ornamentPrecompSlide,        any, _32nd, {},    { 0 },          false, true, {} },
    { SymId::ornamentShake3,              any, _32nd, { 1, 0 }, { 1, 0 },    true,  true, {} },
    { SymId::ornamentShakeMuffat1,        any, _32nd, { 1, 0 }, { 1, 0 },    true,  true, {} },
    { SymId::ornamentTremblementCouperin, any, _32nd, { 1, 1 }, { 0, 1 },        true, true, { 0, 0 } },
    { SymId::ornamentPinceCouperin,       any, _32nd, { 0 },    { 0, -1 },       true, true, { 0, 0 } }

    // [1] Some of the articulations/ornaments in the excursions table above come from
    // Baroque Music, Style and Performance A Handbook, by Robert Donington,(c) 1982
    // ISBN 0-393-30052-8, W. W. Norton & Company, Inc.

    // [2] In some cases, the example from [1] does not preserve the timing.
    // For example, illustrates 2+1/4 counts per half note.
};
}
//---------------------------------------------------------
//   renderNoteArticulation
//---------------------------------------------------------

bool renderNoteArticulation(NoteEventList* events, Note* note, bool chromatic, SymId articulationType, OrnamentStyle ornamentStyle)
{
    if (!note->staff()->isPitchedStaff(note->tick())) { // not enough info in tab staff
        return false;
    }

    std::vector<int> emptypattern = {};
    for (auto& oe : excursions) {
        if (oe.atype == articulationType && (0 == oe.ostyles.size()
                                             || oe.ostyles.end() != oe.ostyles.find(ornamentStyle))) {
            return renderNoteArticulation(events, note, chromatic, oe.duration,
                                          oe.prefix, oe.body, oe.repeatp, oe.sustainp, oe.suffix);
        }
    }
    return false;
}

//---------------------------------------------------------
//   renderNoteArticulation
//---------------------------------------------------------

static bool renderNoteArticulation(NoteEventList* events, Note* note, bool chromatic, TrillType trillType, OrnamentStyle ornamentStyle)
{
    std::map<TrillType, SymId> articulationMap = {
        { TrillType::TRILL_LINE,      SymId::ornamentTrill },
        { TrillType::UPPRALL_LINE,    SymId::ornamentUpPrall },
        { TrillType::DOWNPRALL_LINE,  SymId::ornamentPrecompMordentUpperPrefix },
        { TrillType::PRALLPRALL_LINE, SymId::ornamentTrill }
    };
    auto it = articulationMap.find(trillType);
    if (it == articulationMap.cend()) {
        return false;
    } else {
        return renderNoteArticulation(events, note, chromatic, it->second, ornamentStyle);
    }
}

//---------------------------------------------------------
//   renderGlissando
//---------------------------------------------------------

static void renderGlissando(NoteEventList* events, Note* notestart, double graceOnBeatProportion)
{
    std::vector<int> empty = {};
    std::vector<int> body;
    for (Spanner* spanner : notestart->spannerFor()) {
        if (spanner->type() == ElementType::GLISSANDO
            && toGlissando(spanner)->playGlissando()
            && Glissando::pitchSteps(spanner, body)) {
            renderNoteArticulation(events, notestart, true, Constants::division, empty, body, false, true, empty, 16, 0,
                                   graceOnBeatProportion, false);
        }
    }
}

//---------------------------------------------------------
// findFirstTrill
//  search the spanners in the score, finding the first one
//  which overlaps this chord and is of type ElementType::TRILL
//---------------------------------------------------------

static Trill* findFirstTrill(Chord* chord)
{
    auto spanners = chord->score()->spannerMap().findOverlapping(1 + chord->tick().ticks(),
                                                                 chord->tick().ticks() + chord->actualTicks().ticks() - 1);
    for (auto i : spanners) {
        if (i.value->type() != ElementType::TRILL) {
            continue;
        }
        if (i.value->track() != chord->track()) {
            continue;
        }
        Trill* trill = toTrill(i.value);
        if (trill->playArticulation() == false) {
            continue;
        }
        return trill;
    }
    return nullptr;
}

// In the case that graceNotesBefore or graceNotesAfter are attached to a note
// with an articulation such as a trill, then the grace notes are/will-be/have-been
// already merged into the articulation.
// So this predicate, graceNotesMerged, checks for this condition to avoid calling
// functions which would re-emit the grace notes by a different algorithm.

static bool graceNotesMerged(Chord* chord)
{
    if (findFirstTrill(chord)) {
        return true;
    }
    for (Articulation* a : chord->articulations()) {
        for (auto& oe : excursions) {
            if (oe.atype == a->symId()) {
                return true;
            }
        }
    }
    return false;
}

//---------------------------------------------------------
//   renderChordArticulation
//---------------------------------------------------------

static void renderChordArticulation(Chord* chord, std::vector<NoteEventList>& ell, int& gateTime, double graceOnBeatProportion)
{
    Segment* seg = chord->segment();
    Instrument* instr = chord->part()->instrument(seg->tick());
    int channel  = 0;    // note->subchannel();

    for (unsigned k = 0; k < chord->notes().size(); ++k) {
        NoteEventList* events = &ell[k];
        Note* note = chord->notes()[k];
        Trill* trill;

        if (noteHasGlissando(note)) {
            renderGlissando(events, note, graceOnBeatProportion);
        } else if (chord->staff()->isPitchedStaff(chord->tick()) && (trill = findFirstTrill(chord)) != nullptr) {
            renderNoteArticulation(events, note, false, trill->trillType(), trill->ornamentStyle());
        } else {
            for (Articulation* a : chord->articulations()) {
                if (!a->playArticulation()) {
                    continue;
                }
                if (!renderNoteArticulation(events, note, false, a->symId(), a->ornamentStyle())) {
                    instr->updateGateTime(&gateTime, channel, a->articulationName());
                }
            }
        }
    }
}

//---------------------------------------------------------
//   shouldRenderNote
//---------------------------------------------------------

static std::set<size_t> getNotesIndexesToRender(Chord* chord)
{
    std::set<size_t> notesIndexesToRender;

    auto& notes = chord->notes();
    /// not adding sounds for the same pitches in chord (for example, on different strings)
    std::map<int, int> longestPlayTicksForPitch;

    for (Note* note : notes) {
        int pitch = note->pitch();
        auto& pitchLength = longestPlayTicksForPitch[pitch];
        pitchLength = std::max(pitchLength, note->playTicks());
    }

    auto noteShouldBeRendered = [](Note* n) {
        while (n->tieBack() && n != n->tieBack()->startNote()) {
            n = n->tieBack()->startNote();
            if (findFirstTrill(n->chord())) {
                // The previous tied note probably has events for this note too.
                // That is, we don't need to render this note separately.
                return false;
            }

            for (Articulation* a : n->chord()->articulations()) {
                if (a->isOrnament()) {
                    return false;
                }
            }
        }

        return true;
    };

    for (size_t i = 0; i < notes.size(); i++) {
        Note* n = notes[i];
        if (noteShouldBeRendered(n) && longestPlayTicksForPitch[n->pitch()] == n->playTicks()) {
            notesIndexesToRender.insert(i);
        }
    }

    return notesIndexesToRender;
}

static void createSlideInNotePlayEvents(Note* note, int prevChordTicks, NoteEventList* el)
{
    if (!note->isSlideToNote()) {
        return;
    }

    const int slideNotes = NoteEvent::SLIDE_AMOUNT;
    const int currentTicks = note->chord()->ticks().ticks();
    // if previous chord exists, slide takes its half length, otherwise - current chord's half length
    const int slideTicks = (prevChordTicks == 0 ? currentTicks : prevChordTicks) / 2;
    const int totalSlideDuration = NoteEvent::NOTE_LENGTH * slideTicks / currentTicks;
    const int slideDuration = totalSlideDuration / slideNotes;

    int slideOn = 0;
    int pitchOffset = (note->slide().is(Note::SlideType::Plop) ? 1 : -1);
    int pitch = pitchOffset * slideNotes;
    for (int i = 0; i < slideNotes; ++i) {
        el->push_back(NoteEvent(pitch, 0, slideDuration, NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER, true, totalSlideDuration - slideOn));
        slideOn += slideDuration;
        pitch -= pitchOffset;
    }
}

static void createSlideOutNotePlayEvents(Note* note, NoteEventList* el, int onTime)
{
    if (!note->isSlideOutNote()) {
        return;
    }

    const int slideNotes = NoteEvent::SLIDE_AMOUNT;
    const int allSlidesDuration = NoteEvent::NOTE_LENGTH / 2;
    const int slideDuration = allSlidesDuration / slideNotes;
    int slideOn = NoteEvent::NOTE_LENGTH - allSlidesDuration;
    double velocity = !note->ghost() ? NoteEvent::DEFAULT_VELOCITY_MULTIPLIER : NoteEvent::GHOST_VELOCITY_MULTIPLIER;
    el->push_back(NoteEvent(0, onTime, slideOn, velocity, !note->tieBack()));

    int pitch = 0;
    int pitchOffset = note->slide().is(Note::SlideType::Doit) ? 1 : -1;
    for (int i = 0; i < slideNotes; ++i) {
        pitch += pitchOffset;
        el->push_back(NoteEvent(pitch, slideOn, slideDuration, velocity * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER));
        slideOn += slideDuration;
    }
}

//---------------------------------------------------------
//   renderChord
//    ontime and trailtime in 1/1000 of duration
//    ontime signifies how much gap to leave, i.e., how late the note should start because of graceNotesBefore which have already been rendered
//    trailtime signifies how much gap to leave after the note to allow for graceNotesAfter to be rendered
//---------------------------------------------------------

static std::vector<NoteEventList> renderChord(Chord* chord, Chord* prevChord, int gateTime, int ontime, int trailtime)
{
    const std::vector<Note*>& notes = chord->notes();
    if (notes.empty()) {
        return std::vector<NoteEventList>();
    }

    std::vector<NoteEventList> ell(notes.size(), NoteEventList());

    bool arpeggio = false;
    bool glissandoExists = std::any_of(notes.begin(), notes.end(), [] (Note* note) {
        return noteHasGlissando(note) || note->slide().is(Note::SlideType::Doit) || note->slide().is(Note::SlideType::Fall);
    });

    if (chord->arpeggio() && chord->arpeggio()->playArpeggio()) {
        renderArpeggio(chord, ell);
        arpeggio = true;
    } else {
        if (chord->tremolo()) {
            /// when note has glissando, half of chord is played tremolo, second half - glissando
            renderTremolo(chord, ell, glissandoExists ? 0.5 : 1);
        }

        renderChordArticulation(chord, ell, gateTime, (double)ontime / 1000);
    }

    // Check each note and apply gateTime
    for (size_t i : getNotesIndexesToRender(chord)) {
        Note* note = chord->notes()[i];
        NoteEventList* el = &ell[i];

        createSlideOutNotePlayEvents(note, el, ontime);
        if (arpeggio) {
            continue;       // don't add extra events and apply gateTime to arpeggio
        }
        // If we are here then we still need to render the note.
        // Render its body if necessary and apply gateTime.
        if (el->empty() && chord->tremoloChordType() != TremoloChordType::TremoloSecondNote) {
            el->push_back(NoteEvent(0, ontime, 1000 - trailtime,
                                    !note->ghost() ? NoteEvent::DEFAULT_VELOCITY_MULTIPLIER : NoteEvent::GHOST_VELOCITY_MULTIPLIER));
        }

        createSlideInNotePlayEvents(note, prevChord ? prevChord->ticks().ticks() : 0, el);

        for (NoteEvent& e : *el) {
            e.setLen(e.len() * gateTime / 100);
        }
    }
    return ell;
}

//---------------------------------------------------------
//   createGraceNotesPlayEvent
// as a side effect of createGraceNotesPlayEvents, ontime and trailtime (passed by ref)
// are modified.  ontime reflects the time needed to play the grace-notes-before, and
// trailtime reflects the time for the grace-notes-after.  These are used by the caller
// to effect the on/off time of the main note
//---------------------------------------------------------

void Score::createGraceNotesPlayEvents(const Fraction& tick, Chord* chord, int& ontime, int& trailtime)
{
    std::vector<Chord*> gnb = chord->graceNotesBefore();
    std::vector<Chord*> gna = chord->graceNotesAfter();
    int nb = int(gnb.size());
    int na = int(gna.size());
    if (0 == nb + na) {
        return;     // return immediately if no grace notes to deal with
    }
    // return immediately if the chord has a trill or articulation which effectively plays the graces notes.
    if (graceNotesMerged(chord)) {
        return;
    }
    // if there are graceNotesBefore and also graceNotesAfter, and the before grace notes are
    // not ACCIACCATURA, then the total time of all of them will be 50% of the time of the main note.
    // if the before grace notes are ACCIACCATURA then the grace notes after (if there are any).
    // get 50% of the time of the main note.
    // this is achieved by the two floating point weights: weighta and weightb whose total is 1.0
    // assuring that all the grace notes get the same duration, and their total is 50%.
    // exception is if the note is dotted or double-dotted; see below.
    float weighta = float(na) / (nb + na);

    int graceDuration = 0;
    bool drumset = (getDrumset(chord) != nullptr);
    const double ticksPerSecond = tempo(tick).val * Constants::division;
    const double chordTimeMS = (chord->actualTicks().ticks() / ticksPerSecond) * 1000;
    if (drumset) {
        int flamDuration = 15;     //ms
        graceDuration = flamDuration / chordTimeMS * 1000;     //ratio 1/1000 from the main note length
        ontime = graceDuration * nb;
    } else if (nb) {
        //
        //  render grace notes:
        //  simplified implementation:
        //  - grace notes start on the beat of the main note
        //  - duration: appoggiatura: 0.5  * duration of main note (2/3 for dotted notes, 4/7 for double-dotted)
        //              acciacatura: min of 0.5 * duration or 65ms fixed (independent of duration or tempo)
        //  - for appoggiaturas, the duration is divided by the number of grace notes
        //  - the grace note duration as notated does not matter
        //
        Chord* graceChord = gnb[0];

        if (graceChord->noteType() == NoteType::ACCIACCATURA) {
            ontime = 0;
            graceDuration = 0;
            weighta = 1.0;
        } else {
            const double graceTimeMS = (graceChord->actualTicks().ticks() / ticksPerSecond) * 1000;

            // 1000 occurs below as a unit for ontime
            ontime = std::min(500, static_cast<int>((graceTimeMS / chordTimeMS) * 1000));
            graceDuration = ontime / nb;
            weighta = 1.0;
            trailtime += ontime;
        }
    }

    for (int i = 0, on = 0; i < nb; ++i) {
        std::vector<NoteEventList> el;
        Chord* gc = gnb.at(i);
        size_t nn = gc->notes().size();
        for (unsigned ii = 0; ii < nn; ++ii) {
            NoteEventList nel;
            nel.push_back(NoteEvent(0, on, graceDuration));
            el.push_back(nel);
        }

        if (gc->playEventType() == PlayEventType::Auto) {
            gc->setNoteEventLists(el);
        }

        on += graceDuration;
    }
    if (na) {
        if (chord->dots() == 1) {
            trailtime = floor(667 * weighta);
        } else if (chord->dots() == 2) {
            trailtime = floor(571 * weighta);
        } else {
            trailtime = floor(500 * weighta);
        }
        int graceDuration1 = trailtime / na;
        int on = 1000 - trailtime;
        for (int i = 0; i < na; ++i) {
            std::vector<NoteEventList> el;
            Chord* gc = gna.at(i);
            size_t nn = gc->notes().size();
            for (size_t ii = 0; ii < nn; ++ii) {
                NoteEventList nel;
                nel.push_back(NoteEvent(0, on, graceDuration1));         // NoteEvent(pitch,ontime,len)
                el.push_back(nel);
            }

            if (gc->playEventType() == PlayEventType::Auto) {
                gc->setNoteEventLists(el);
            }
            on += graceDuration1;
        }
    }
}

//---------------------------------------------------------
//   createPlayEvents
//    create default play events
//---------------------------------------------------------

void Score::createPlayEvents(Chord* chord, Chord* prevChord)
{
    int gateTime = 100;

    Fraction tick = chord->tick();
    Slur* slur = 0;
    for (auto sp : _spanner.map()) {
        if (!sp.second->isSlur() || sp.second->staffIdx() != chord->staffIdx()) {
            continue;
        }
        Slur* s = toSlur(sp.second);
        if (tick >= s->tick() && tick < s->tick2()) {
            slur = s;
            break;
        }
    }
    // gateTime is 100% for slurred notes
    if (!slur) {
        Instrument* instr = chord->part()->instrument(tick);
        instr->updateGateTime(&gateTime, 0, u"");
    }

    int ontime    = 0;
    int trailtime = 0;
    createGraceNotesPlayEvents(tick, chord, ontime, trailtime);   // ontime and trailtime are modified by this call depending on grace notes before and after

    SwingParameters st = chord->staff()->swing(tick);
    // Check if swing needs to be applied
    if (st.swingUnit && !chord->tuplet()) {
        Swing::swingAdjustParams(chord, st, ontime, gateTime);
    }
    //
    //    render normal (and articulated) chords
    //
    std::vector<NoteEventList> el = renderChord(chord, prevChord, gateTime, ontime, trailtime);
    if (chord->playEventType() == PlayEventType::Auto) {
        chord->setNoteEventLists(el);
    }
    // don't change event list if type is PlayEventType::User
}

static void adjustPreviousChordLength(Chord* currentChord, Chord* prevChord)
{
    if (!prevChord) {
        return;
    }

    const std::vector<Chord*>& graceBeforeChords = currentChord->graceNotesBefore();
    std::vector<Chord*> graceNotesBeforeBar;
    std::copy_if(graceBeforeChords.begin(), graceBeforeChords.end(), std::back_inserter(graceNotesBeforeBar), [](Chord* ch) {
        return ch->noteType() == NoteType::ACCIACCATURA;
    });

    int prevTicks = prevChord->ticks().ticks();

    int reducedTicks = 0;

    const auto& notes = currentChord->notes();
    bool anySlidesIn = std::any_of(notes.begin(), notes.end(), [](const Note* note) { return note->isSlideToNote(); });

    if (!graceNotesBeforeBar.empty()) {
        reducedTicks = graceNotesBeforeBar[0]->ticks().ticks();
        if (reducedTicks >= prevTicks) {
            return;
        }
    } else if (anySlidesIn) {
        reducedTicks = prevTicks / 2;
    } else {
        return;
    }

    double lengthMultiply = std::min(1.0, (double)(prevTicks - reducedTicks) / prevTicks);

    for (size_t i = 0; i < prevChord->notes().size(); i++) {
        Note* prevChordNote = prevChord->notes()[i];
        NoteEventList evList;
        NoteEventList prevEvents = prevChordNote->playEvents();
        std::sort(prevEvents.begin(), prevEvents.end(), [](const NoteEvent& left, const NoteEvent& right) {
            int l1 = left.ontime();
            int l2 = -left.offset();
            int r1 = right.ontime();
            int r2 = -right.offset();

            return std::tie(l1, l2) < std::tie(r1, r2);
        });

        int curPos = prevEvents.front().ontime();

        for (size_t i = 0; i < prevEvents.size(); i++) {
            NoteEvent event;
            const NoteEvent& prevEvent = prevEvents[i];
            event.setOntime(curPos);
            event.setPitch(prevEvent.pitch());
            event.setOffset(prevEvent.offset());
            int prevLen = prevEvent.len();

            // not shortening events before beat
            if (prevEvent.offset() == 0) {
                int newEventLength = prevLen * lengthMultiply;
                event.setLen(newEventLength);
                curPos += newEventLength;
            } else {
                event.setLen(prevLen);
            }

            evList.push_back(event);
        }

        prevChordNote->setPlayEvents(evList);
    }
}

void Score::createPlayEvents(Measure const* start, Measure const* const end)
{
    if (!start) {
        start = firstMeasure();
    }

    track_idx_t etrack = nstaves() * VOICES;
    for (track_idx_t track = 0; track < etrack; ++track) {
        bool rangeEnded = false;
        Chord* prevChord = nullptr;
        for (Measure const* m = start; m; m = m->nextMeasure()) {
            constexpr SegmentType st = SegmentType::ChordRest;

            if (m == end) {
                rangeEnded = true;
            }
            if (rangeEnded) {
                // The range has ended, but we should collect events
                // for tied notes. So we'll check if this is the case.
                const Segment* seg = m->first(st);
                const EngravingItem* e = seg->element(track);
                bool tie = false;
                if (e && e->isChord()) {
                    for (const Note* n : toChord(e)->notes()) {
                        if (n->tieBack()) {
                            tie = true;
                            break;
                        }
                    }
                }
                if (!tie) {
                    break;
                }
            }

            // skip linked staves, except primary
            if (!m->score()->staff(track / VOICES)->isPrimaryStaff()) {
                continue;
            }
            for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
                EngravingItem* e = seg->element(track);
                if (e == 0) {
                    continue;
                }

                if (!e->isChord()) {
                    prevChord = nullptr;
                    continue;
                }

                Chord* chord = toChord(e);
                createPlayEvents(chord, prevChord);
                adjustPreviousChordLength(chord, prevChord);
                prevChord = chord;
            }
        }
    }
}

void Score::renderMidi(EventMap* events, const MidiRenderer::Context& ctx, bool expandRepeats)
{
    masterScore()->setExpandRepeats(expandRepeats);
    MidiRenderer(this).renderScore(events, ctx);
}
}
