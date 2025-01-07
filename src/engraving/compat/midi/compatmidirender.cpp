#include "compatmidirender.h"

#include "global/realfn.h"

#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"
#include "dom/navigate.h"

namespace mu::engraving {
static int slideTicks(const Chord* chord);
static int graceBendTicks(const Chord* chord);

void CompatMidiRender::renderScore(Score* score, EventsHolder& events, const CompatMidiRendererInternal::Context& ctx, bool expandRepeats)
{
    score->masterScore()->setExpandRepeats(expandRepeats);
    CompatMidiRendererInternal internal{ score };
    internal.renderScore(events, ctx, expandRepeats);
}

int CompatMidiRender::getControllerForSnd(Score* score, int globalSndController)
{
    const int DEFAULT_CC = CTRL_BREATH;

    SynthesizerState s = score->synthesizerState();
    int method = s.method();
    int controller = s.ccToUse();

    if (method == -1) {
        controller = (globalSndController == -1) ? DEFAULT_CC : globalSndController;
    }

    return controller;
}

void CompatMidiRender::createPlayEvents(const Score* score, Measure const* start, Measure const* const end,
                                        const CompatMidiRendererInternal::Context& context)
{
    if (!start) {
        start = score->firstMeasure();
    }

    track_idx_t etrack = score->nstaves() * VOICES;
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
                    for (const mu::engraving::Note* n : toChord(e)->notes()) {
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
                EngravingItem* item = seg->element(track);
                if (!item) {
                    continue;
                }

                if (!item->isChord()) {
                    prevChord = nullptr;
                    continue;
                }

                Chord* chord = toChord(item);
                Chord* nextChord = nullptr;
                if (ChordRest* chr = nextChordRest(chord, true); chr && chr->isChord()) {
                    nextChord = static_cast<Chord*>(chr);
                }

                if (!nextChord) {
                    // check if next chord is in next measure
                    Measure* nextMeasure = m->nextMeasure();
                    if (nextMeasure) {
                        nextChord = CompatMidiRender::getChordFromSegment(nextMeasure->first(st), track);
                    }
                }

                CompatMidiRender::createPlayEvents(score, context, chord, prevChord, nextChord);
                prevChord = chord;
            }
        }
    }
}

void CompatMidiRender::createPlayEvents(const Score* score, const CompatMidiRendererInternal::Context& context, Chord* chord,
                                        Chord* prevChord, Chord* nextChord)
{
    int gateTime = 100;

    Fraction tick = chord->tick();
    Slur* slur = 0;
    for (auto sp : score->spannerMap().map()) {
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
        instr->updateGateTime(&gateTime, String());
    }

    int ontime    = 0;
    int trailtime = 0;

    CompatMidiRender::createGraceNotesPlayEvents(score, tick, chord, ontime, trailtime);       // ontime and trailtime are modified by this call depending on grace notes before and after
    trailtime = CompatMidiRender::adjustTrailtime(trailtime, chord, nextChord);

    SwingParameters st = chord->staff()->swing(tick);
    // Check if swing needs to be applied
    if (st.swingUnit && !chord->tuplet()) {
        Swing::swingAdjustParams(chord, st, ontime, gateTime);
    }
    //
    //    render normal (and articulated) chords
    //
    std::vector<NoteEventList> el = CompatMidiRender::renderChord(context, chord, prevChord, gateTime, ontime, trailtime);
    if (chord->playEventType() == PlayEventType::Auto) {
        chord->setNoteEventLists(el);
    }
    // don't change event list if type is PlayEventType::User
}

//---------------------------------------------------------
//   renderChord
//    ontime and trailtime in 1/1000 of duration
//    ontime signifies how much gap to leave, i.e., how late the note should start because of graceNotesBefore which have already been rendered
//    trailtime signifies how much gap to leave after the note to allow for graceNotesAfter to be rendered
//---------------------------------------------------------

std::vector<NoteEventList> CompatMidiRender::renderChord(const CompatMidiRendererInternal::Context& context, Chord* chord, Chord* prevChord,
                                                         int gateTime, int ontime, int trailtime)
{
    const std::vector<mu::engraving::Note*>& notes = chord->notes();
    if (notes.empty()) {
        return std::vector<NoteEventList>();
    }

    std::vector<NoteEventList> ell(notes.size(), NoteEventList());

    bool arpeggio = false;

    /// when note has glissando or slide from note, tremolo should be shortened
    int baseLength = NoteEvent::NOTE_LENGTH - ontime - trailtime;
    double tremoloLength = (double)baseLength / NoteEvent::NOTE_LENGTH;

    bool glissandoStart = std::any_of(notes.begin(), notes.end(), [] (mu::engraving::Note* note) {
        return CompatMidiRender::noteIsGlissandoStart(note);
    });

    if (glissandoStart) {
        tremoloLength = 0.5 * baseLength / NoteEvent::NOTE_LENGTH;
    } else {
        bool hasSlideOut = std::any_of(notes.begin(), notes.end(), [] (mu::engraving::Note* note) {
            return note->hasSlideFromNote();
        });

        if (hasSlideOut) {
            tremoloLength = (baseLength - CompatMidiRender::slideLengthChordDependent(chord)) / (double)NoteEvent::NOTE_LENGTH;
        }
    }

    bool tremolo = false;

    if (chord->arpeggio() && chord->arpeggio()->playArpeggio()) {
        CompatMidiRender::renderArpeggio(chord, ell, ontime);
        arpeggio = true;
    } else {
        if (chord->tremoloType() != TremoloType::INVALID_TREMOLO) {
            CompatMidiRender::renderTremolo(chord, ell, ontime, tremoloLength);
            tremolo = true;
        }

        CompatMidiRender::renderChordArticulation(context, chord, ell, gateTime, (double)ontime / NoteEvent::NOTE_LENGTH, tremolo);
    }

    // Check each note and apply gateTime
    for (size_t i : getNotesIndexesToRender(chord)) {
        mu::engraving::Note* note = chord->notes()[i];
        NoteEventList* el = &ell[i];

        CompatMidiRender::createSlideOutNotePlayEvents(note, el, ontime, tremolo);
        if (arpeggio) {
            continue;           // don't add extra events and apply gateTime to arpeggio
        }
        // If we are here then we still need to render the note.
        // Render its body if necessary and apply gateTime.
        if (el->empty() && chord->tremoloChordType() != TremoloChordType::TremoloSecondChord) {
            el->push_back(NoteEvent(0, ontime, 1000 - trailtime,
                                    !note->ghost() ? NoteEvent::DEFAULT_VELOCITY_MULTIPLIER : NoteEvent::GHOST_VELOCITY_MULTIPLIER));

            Glissando* gl = CompatMidiRender::backGlissando(note);
            if (gl && CompatMidiRender::isGlissandoValid(gl) && !gl->glissandoShift()) {
                el->back().setSlide(true);
            }
        }

        CompatMidiRender::createSlideInNotePlayEvents(note, prevChord, el);

        if (note->isHammerOn() && el->size() == 1) {
            el->front().setHammerPull(true);
        }

        for (NoteEvent& e : *el) {
            e.setLen(e.len() * gateTime / 100);
        }

        std::sort(el->begin(), el->end(), [](const NoteEvent& left, const NoteEvent& right) {
            int l1 = left.ontime();
            int l2 = -left.offset();
            int r1 = right.ontime();
            int r2 = -right.offset();

            return std::tie(l1, l2) < std::tie(r1, r2);
        });
    }

    return ell;
}

//---------------------------------------------------------
//   renderArpeggio
//---------------------------------------------------------

void CompatMidiRender::renderArpeggio(Chord* chord, std::vector<NoteEventList>& ell, int ontime)
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

        auto tempoRatio = chord->score()->tempomap()->tempo(chord->tick().ticks()).val / Constants::DEFAULT_TEMPO.val;
        int ot = (l * j * 1000) / chord->upNote()->playTicks()
                 * tempoRatio * chord->arpeggio()->Stretch();
        ot = std::clamp(ot + ontime, ot, 1000);

        events->push_back(NoteEvent(0, ot, 1000 - ot));
        j++;
    }
}

//---------------------------------------------------------
//   renderTremolo
//---------------------------------------------------------

void CompatMidiRender::renderTremolo(Chord* chord, std::vector<NoteEventList>& ell, int& ontime, double tremoloPartOfChord /* = 1.0 */)
{
    if (muse::RealIsNull(tremoloPartOfChord)) {
        return;
    }

    struct TremoloAdapter {
        TremoloSingleChord* singleChord = nullptr;
        TremoloTwoChord* twoChord = nullptr;
        TremoloAdapter(TremoloSingleChord* s, TremoloTwoChord* t)
            : singleChord(s), twoChord(t) {}

        TremoloType tremoloType() const
        {
            return singleChord ? singleChord->tremoloType()
                   : (twoChord ? twoChord->tremoloType() : TremoloType::INVALID_TREMOLO);
        }

        int lines() const { return singleChord ? singleChord->lines() : (twoChord ? twoChord->lines() : 0); }
    };

    TremoloAdapter tremolo = TremoloAdapter(chord->tremoloSingleChord(), chord->tremoloTwoChord());

    Segment* seg = chord->segment();
    //TremoloDispatcher* tremolo = chord->tremoloDispatcher();
    int notes = int(chord->notes().size());

    // check if tremolo was rendered before for drum staff
    const Drumset* ds = CompatMidiRender::getDrumset(chord);
    if (ds) {
        for (mu::engraving::Note* n : chord->notes()) {
            DrumInstrumentVariant div = ds->findVariant(n->pitch(), chord->articulations(), tremolo.tremoloType());
            if (div.pitch != INVALID_PITCH && div.tremolo == tremolo.tremoloType()) {
                return;             // already rendered
            }
        }
    }

    // we cannot render buzz roll with MIDI events only
    if (tremolo.tremoloType() == TremoloType::BUZZ_ROLL) {
        return;
    }

    // render tremolo with multiple events
    if (chord->tremoloChordType() == TremoloChordType::TremoloFirstChord) {
        int t = Constants::DIVISION / (1 << (tremolo.lines() + chord->durationType().hooks()));
        if (t == 0) {       // avoid crash on very short tremolo
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
            int tticks = chord->ticks().ticks() * 2;           // use twice the size
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
    } else if (chord->tremoloChordType() == TremoloChordType::TremoloSecondChord) {
        for (int k = 0; k < notes; ++k) {
            NoteEventList* events = &(ell)[k];
            events->clear();
        }
    } else if (chord->tremoloChordType() == TremoloChordType::TremoloSingle) {
        int t = Constants::DIVISION / (1 << (tremolo.lines() + chord->durationType().hooks()));
        if (t == 0) {       // avoid crash on very short tremolo
            t = 1;
        }

        int tremoloEventsSize = chord->ticks().ticks() / t * tremoloPartOfChord;

        constexpr int fullEventTime = 1000;
        int tremoloTime = fullEventTime * tremoloPartOfChord;
        int tremoloEventStep = tremoloTime / tremoloEventsSize;
        ontime += tremoloTime;

        for (int k = 0; k < notes; ++k) {
            NoteEventList* events = &(ell)[k];
            events->clear();
            for (int i = 0; i < tremoloEventsSize; i++) {
                events->push_back(NoteEvent(0, tremoloEventStep * i, tremoloEventStep));
            }
        }
    }
}

//---------------------------------------------------------
//   renderChordArticulation
//---------------------------------------------------------

void CompatMidiRender::renderChordArticulation(const CompatMidiRendererInternal::Context& context, Chord* chord,
                                               std::vector<NoteEventList>& ell, int& gateTime, double graceOnBeatProportion,
                                               bool tremoloBefore /* = false */)
{
    Segment* seg = chord->segment();
    Instrument* instr = chord->part()->instrument(seg->tick());

    for (unsigned k = 0; k < chord->notes().size(); ++k) {
        NoteEventList* events = &ell[k];
        Note* note = chord->notes()[k];
        Trill* trill;

        if (noteIsGlissandoStart(note)) {
            CompatMidiRender::renderGlissando(events, note, graceOnBeatProportion, tremoloBefore);
        } else if (chord->staff()->isPitchedStaff(chord->tick()) && (trill = findFirstTrill(chord)) != nullptr) {
            CompatMidiRender::renderNoteArticulation(events, note, false, trill->trillType(), trill->ornamentStyle());
        } else {
            for (Articulation* a : chord->articulations()) {
                if (!a->playArticulation()) {
                    continue;
                }
                if (!CompatMidiRender::renderNoteArticulation(events, note, false, a->symId(), a->ornamentStyle())) {
                    CompatMidiRender::updateGateTime(instr, gateTime, a->articulationName(), context);
                }
            }
        }
    }
}

void CompatMidiRender::updateGateTime(const Instrument* instrument, int& gateTime, const String& articulationName,
                                      const CompatMidiRendererInternal::Context& context)
{
    using Ctx = CompatMidiRendererInternal::Context;
    if (context.useDefaultArticulations) {
        auto it = Ctx::s_builtInArticulationsValues.find(articulationName);
        if (it != Ctx::s_builtInArticulationsValues.end()) {
            gateTime = std::min(gateTime, it->second.gateTime);
        } else {
            instrument->updateGateTime(&gateTime, articulationName);
        }

        return;
    }

    auto articulationsForInstrumentIt = context.articulationsWithoutValuesByInstrument.find(instrument->id());
    if (articulationsForInstrumentIt != context.articulationsWithoutValuesByInstrument.end()) {
        const auto& articulationsForInstrument = articulationsForInstrumentIt->second;
        if (articulationsForInstrument.find(articulationName) != articulationsForInstrument.end()) {
            gateTime = std::min(gateTime, Ctx::s_builtInArticulationsValues[articulationName].gateTime);
        } else {
            instrument->updateGateTime(&gateTime, articulationName);
        }
    }
}

//---------------------------------------------------------
//   createGraceNotesPlayEvent
// as a side effect of createGraceNotesPlayEvents, ontime and trailtime (passed by ref)
// are modified.  ontime reflects the time needed to play the grace-notes-before, and
// trailtime reflects the time for the grace-notes-after.  These are used by the caller
// to effect the on/off time of the main note
//---------------------------------------------------------

void CompatMidiRender::createGraceNotesPlayEvents(const Score* score, const Fraction& tick, Chord* chord, int& ontime, int& trailtime)
{
    std::vector<Chord*> gnb = chord->graceNotesBefore(true);
    std::vector<Chord*> gna = chord->graceNotesAfter(true);
    int nb = int(gnb.size());
    int na = int(gna.size());
    if (0 == nb + na) {
        return;         // return immediately if no grace notes to deal with
    }
    // return immediately if the chord has a trill or articulation which effectively plays the graces notes.
    if (CompatMidiRendererInternal::graceNotesMerged(chord)) {
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
    bool drumset = (CompatMidiRender::getDrumset(chord) != nullptr);
    const double ticksPerSecond = score->tempo(tick).val * Constants::DIVISION;
    const double chordTimeMS = (chord->actualTicks().ticks() / ticksPerSecond) * 1000;
    if (drumset) {
        int flamDuration = 15;         //ms
        graceDuration = flamDuration / chordTimeMS * 1000;         //ratio 1/1000 from the main note length
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
        const auto& graceNotes = graceChord->notes();
        bool graceBend = std::any_of(graceNotes.begin(), graceNotes.end(), [](Note* note) {
            return note->isGraceBendStart();
        });

        if (graceChord->noteType() == NoteType::ACCIACCATURA || graceBend) {
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
                nel.push_back(NoteEvent(0, on, graceDuration1));             // NoteEvent(pitch,ontime,len)
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
//   renderGlissando
//---------------------------------------------------------

void CompatMidiRender::renderGlissando(NoteEventList* events, Note* notestart, double graceOnBeatProportion,
                                       bool tremoloBefore /* = false */)
{
    std::vector<int> empty = {};
    std::vector<int> body;
    for (Spanner* s : notestart->spannerFor()) {
        if (s->isGlissando() && s->playSpanner() && Glissando::pitchSteps(s, body)) {
            CompatMidiRender::renderNoteArticulation(events, notestart, true, Constants::DIVISION, empty, body, false, true, empty, 16, 0,
                                                     graceOnBeatProportion, tremoloBefore);
        }
    }
}

//---------------------------------------------------------
//   renderNoteArticulation
// prefix, vector of int, normally something like {0,-1,0,1} modeling the prefix of tremblement relative to the base note
// body, vector of int, normally something like {0,-1,0,1} modeling the possibly repeated tremblement relative to the base note
// tickspernote, number of ticks, either _16h or _32nd, i.e., Constants::division/4 or Constants::division/8
// repeatp, true means repeat the body as many times as possible to fill the time slice.
// sustainp, true means the last note of the body is sustained to fill remaining time slice
//---------------------------------------------------------

bool CompatMidiRender::renderNoteArticulation(NoteEventList* events, Note* note, bool chromatic, int requestedTicksPerNote,
                                              const std::vector<int>& prefix, const std::vector<int>& body,
                                              bool repeatp, bool sustainp, const std::vector<int>& suffix,
                                              int fastestFreq /* = 64 */, int slowestFreq /* = 8 */, // 64 Hz and 8 Hz
                                              double graceOnBeatProportion /* = 0 */, bool tremoloBefore /* = false */)
{
    if (!tremoloBefore) {
        events->clear();
    }

    Chord* chord = note->chord();
    int maxticks = CompatMidiRender::totalTiedNoteTicks(note);
    int space = 1000 * maxticks;
    int numrepeat = 1;
    int sustain   = 0;
    int ontime    = 0;

    ///@NOTE grace note + glissando combination is treated separetely, in this method grace notes for glissando are not added
    /// graceOnBeatProportion is used for calculating grace note "on beat" offset
    int gnb = noteIsGlissandoStart(note) ? 0 : int(note->chord()->graceNotesBefore().size());
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
    int ticksPerSecond = tempo.val * Constants::DIVISION;

    int minTicksPerNote = int(ticksPerSecond / fastestFreq);
    int maxTicksPerNote = (0 == slowestFreq) ? 0 : int(ticksPerSecond / slowestFreq);

    // for fast tempos, we have to slow down the tremblement frequency, i.e., increase the ticks per note
    if (requestedTicksPerNote >= minTicksPerNote) {
    } else {     // try to divide the requested frequency by a power of 2 if possible, if not, use the maximum frequency, ie., minTicksPerNote
        ticksPerNote = requestedTicksPerNote;
        while (ticksPerNote < minTicksPerNote) {
            ticksPerNote *= 2;           // decrease the tremblement frequency
        }
        if (ticksPerNote > maxTicksPerNote) {
            ticksPerNote = minTicksPerNote;
        }
    }

    ticksPerNote = std::max(requestedTicksPerNote, minTicksPerNote);

    if (slowestFreq <= 0) {     // no slowest freq given such as something silly like glissando with 4 notes over 8 counts.
    } else if (ticksPerNote <= maxTicksPerNote) {     // in a good range, so we don't need to adjust ticksPerNote
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
        return false;         // the ornament is impossible to implement respecting the minimum duration and all the notes it contains
    } else {
        ticksPerNote = maxticks / (gnb + p + b + s + gna);          // integer division ignoring remainder
        if (slowestFreq <= 0) {
        } else if (ticksPerNote < minTicksPerNote) {
            return false;
        }
    }

    int millespernote = space * ticksPerNote / maxticks;         // rescale duration into per mille

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

    // calculate the number of times to repeat the body, and sustain the last note of the body
    // 1000 = P + numrepeat*B+sustain + S
    if (repeatp) {
        numrepeat = (space - millespernote * (gnb + p + s + gna)) / (millespernote * b);
    }
    if (sustainp) {
        sustain   = space - millespernote * (gnb + p + numrepeat * b + s + gna);
    }

    // render the prefix
    for (int j = 0; j < p; j++) {
        ontime = makeEvent(prefix[j], ontime, tieForward(j, prefix));
    }

    if (b > 0) {
        // Check that we are doing a glissando
        enum GlissandoState {
            NOT_FOUND,
            VALID,
            INVALID
        } glissandoState = GlissandoState::NOT_FOUND;

        std::vector<int> onTimes;
        for (Spanner* spanner : note->spannerFor()) {
            if (spanner->type() == ElementType::GLISSANDO) {
                Glissando* glissando = toGlissando(spanner);
                if (!isGlissandoValid(glissando)) {
                    glissandoState = GlissandoState::INVALID;
                    break;
                }

                EaseInOut easeInOut(static_cast<double>(glissando->easeIn()) / 100.0,
                                    static_cast<double>(glissando->easeOut()) / 100.0);

                // shifting glissando sounds to the second half of glissando duration
                int totalDuration = millespernote * b;
                constexpr double glissandoPart = 0.33;

                int glissandoDuration = tremoloBefore ? totalDuration * glissandoPart : totalDuration
                                        * (1 - graceOnBeatProportion) * glissandoPart;
                easeInOut.timeList(b - 1, glissandoDuration, &onTimes);

                if (!onTimes.empty()) {
                    onTimes[0] += graceOnBeatProportion * totalDuration;
                }

                if (onTimes.size() > 1) {
                    int offset = onTimes[1];
                    for (size_t i = 1; i < onTimes.size(); i++) {
                        onTimes[i] += totalDuration - glissandoDuration - offset;
                    }
                }

                glissandoState = GlissandoState::VALID;
                break;
            }
        }
        if (glissandoState == GlissandoState::VALID) {
            const double defaultVelocityMultiplier = 1.0;

            // render the body, i.e. the glissando
            if (onTimes.size() > 1) {
                makeEvent(body[0], onTimes[0], onTimes[1] - onTimes[0],
                          defaultVelocityMultiplier, !note->tieBack());

                Glissando* gl = backGlissando(note);
                if (gl && isGlissandoValid(gl) && !gl->glissandoShift()) {
                    events->back().setSlide(true);
                }
            }

            for (int j = 1; j < b - 1; j++) {
                makeEvent(body[j], onTimes[j], onTimes[j + 1] - onTimes[j],
                          defaultVelocityMultiplier);
                events->back().setSlide(true);
            }

            if (b > 1) {
                makeEvent(body[b - 1], onTimes[b - 1], (millespernote * b - onTimes[b - 1]) + sustain, defaultVelocityMultiplier);
                events->back().setSlide(true);
            }
        } else if (glissandoState != GlissandoState::INVALID) {
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

    return true;
}

//---------------------------------------------------------
//   renderNoteArticulation
//---------------------------------------------------------

bool CompatMidiRender::renderNoteArticulation(NoteEventList* events, Note* note, bool chromatic, SymId articulationType,
                                              OrnamentStyle ornamentStyle)
{
    if (!note->staff()->isPitchedStaff(note->tick())) {     // not enough info in tab staff
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

bool CompatMidiRender::renderNoteArticulation(NoteEventList* events, Note* note, bool chromatic, TrillType trillType,
                                              OrnamentStyle ornamentStyle)
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
        return CompatMidiRender::renderNoteArticulation(events, note, chromatic, it->second, ornamentStyle);
    }
}

void CompatMidiRender::createSlideInNotePlayEvents(Note* note, Chord* prevChord, NoteEventList* el)
{
    if (!note->hasSlideToNote()) {
        return;
    }

    int slideNotes = 0;
    bool upSlide = note->slideToType() == Note::SlideType::UpToNote;
    if (upSlide) {
        slideNotes = std::clamp(note->fret() - 1, 0, NoteEvent::SLIDE_AMOUNT);
    } else {
        slideNotes = NoteEvent::SLIDE_AMOUNT;
    }

    if (slideNotes == 0) {
        return;
    }

    int totalSlideDuration = 0;
    double currentTicks = note->chord()->ticks().ticks();
    if (prevChord) {
        /// event length is about current chord's events, but we adjust previous
        totalSlideDuration = (double)slideLengthChordDependent(prevChord) / currentTicks * prevChord->ticks().ticks();
    } else {
        totalSlideDuration = (double)NoteEvent::NOTE_LENGTH * SLIDE_DURATION / currentTicks;
    }

    const int slideDuration = totalSlideDuration / slideNotes;

    int slideOn = 0;
    int pitchOffset = upSlide ? -1 : 1;
    int pitch = pitchOffset * slideNotes;
    for (int i = 0; i < slideNotes; ++i) {
        NoteEvent event { pitch, 0, slideDuration, 1.0, true, totalSlideDuration - slideOn };
        event.setSlide(true);
        el->push_back(std::move(event));
        slideOn += slideDuration;
        pitch -= pitchOffset;
    }
}

void CompatMidiRender::createSlideOutNotePlayEvents(Note* note, NoteEventList* el, int onTime, bool hasTremolo)
{
    if (!note->hasSlideFromNote()) {
        return;
    }

    const int slideNotes = NoteEvent::SLIDE_AMOUNT;
    const int totalSlideDuration = slideLengthChordDependent(note->chord());
    const int slideDuration = totalSlideDuration / slideNotes;

    int slideOn = NoteEvent::NOTE_LENGTH - totalSlideDuration;
    double velocity = !note->ghost() ? NoteEvent::DEFAULT_VELOCITY_MULTIPLIER : NoteEvent::GHOST_VELOCITY_MULTIPLIER;
    if (!hasTremolo) {
        el->push_back(NoteEvent(0, onTime, slideOn - onTime, velocity, !note->tieBack()));
    }

    int pitch = 0;
    int pitchOffset = note->slideFromType() == Note::SlideType::UpFromNote ? 1 : -1;
    for (int i = 0; i < slideNotes; ++i) {
        pitch += pitchOffset;
        NoteEvent event { pitch, slideOn, slideDuration, velocity };
        event.setSlide(true);
        el->push_back(std::move(event));
        slideOn += slideDuration;
    }
}

Chord* CompatMidiRender::getChordFromSegment(Segment* segment, track_idx_t track)
{
    if (segment) {
        EngravingItem* item = segment->element(track);
        if (item && item->isChord()) {
            return toChord(item);
        }
    }

    return nullptr;
}

//---------------------------------------------------------
// findFirstTrill
//  search the spanners in the score, finding the first one
//  which overlaps this chord and is of type ElementType::TRILL
//---------------------------------------------------------

Trill* CompatMidiRender::findFirstTrill(Chord* chord)
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
        if (!trill->playSpanner()) {
            continue;
        }
        return trill;
    }
    return nullptr;
}

int CompatMidiRender::adjustTrailtime(int trailtime, Chord* currentChord, Chord* nextChord)
{
    if (!nextChord) {
        return trailtime;
    }

    const std::vector<Chord*>& graceBeforeChords = nextChord->graceNotesBefore();
    std::vector<Chord*> graceNotesBeforeBar;

    bool hasGraceBend = std::any_of(graceBeforeChords.begin(), graceBeforeChords.end(), [](Chord* ch) {
        return std::any_of(ch->notes().begin(), ch->notes().end(), [](Note* n) {
            return n->isGraceBendStart();
        });
    });

    if (!hasGraceBend) {
        std::copy_if(graceBeforeChords.begin(), graceBeforeChords.end(), std::back_inserter(graceNotesBeforeBar), [](Chord* ch) {
            return ch->noteType() == NoteType::ACCIACCATURA;
        });
    }

    const int currentTicks = currentChord->ticks().ticks();
    IF_ASSERT_FAILED(currentTicks > 0) {
        return trailtime;
    }

    int reducedTicks = 0;

    const auto& notes = nextChord->notes();
    if (hasGraceBend) {
        reducedTicks = graceBendTicks(currentChord);
    } else if (!graceNotesBeforeBar.empty()) {
        reducedTicks = std::min(graceNotesBeforeBar[0]->ticks().ticks(), currentTicks / 2);
    } else {
        bool anySlidesIn = std::any_of(notes.begin(), notes.end(), [](const Note* note) {
            return note->slideToType() == Note::SlideType::DownToNote
                   || (note->slideToType() == Note::SlideType::UpToNote && note->fret() > 1);
        });

        if (anySlidesIn) {
            reducedTicks = slideTicks(currentChord);
        }
    }

    trailtime += reducedTicks / (double)currentTicks * NoteEvent::NOTE_LENGTH;
    return trailtime;
}

//---------------------------------------------------------
//   noteIsGlissandoStart
//---------------------------------------------------------

bool CompatMidiRender::noteIsGlissandoStart(mu::engraving::Note* note)
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

int CompatMidiRender::slideLengthChordDependent(Chord* chord)
{
    const int currentTicks = chord->ticks().ticks();
    if (currentTicks <= SLIDE_DURATION) {
        return NoteEvent::NOTE_LENGTH / 2;
    }

    return (double)NoteEvent::NOTE_LENGTH * SLIDE_DURATION / currentTicks;
}

std::set<size_t> CompatMidiRender::getNotesIndexesToRender(Chord* chord)
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
        while (n->tieBackNonPartial() && n != n->tieBack()->startNote()) {
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

//---------------------------------------------------------
//   backGlissando
//---------------------------------------------------------

Glissando* CompatMidiRender::backGlissando(Note* note)
{
    for (Spanner* spanner : note->spannerBack()) {
        if ((spanner->type() == ElementType::GLISSANDO)
            && spanner->startElement()
            && (ElementType::NOTE == spanner->startElement()->type())) {
            return toGlissando(spanner);
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   isGlissandoValid
//
//   checking if glissando can be played
//---------------------------------------------------------

bool CompatMidiRender::isGlissandoValid(Glissando* glissando)
{
    EngravingItem* startItem = glissando->startElement();
    EngravingItem* endItem = glissando->endElement();
    if (!startItem || !endItem || !startItem->isNote() || !endItem->isNote()) {
        return false;
    }

    return toNote(startItem)->string() == toNote(endItem)->string();
}

const Drumset* CompatMidiRender::getDrumset(const Chord* chord)
{
    if (chord->staff() && chord->staff()->isDrumStaff(chord->tick())) {
        const Drumset* ds = chord->staff()->part()->instrument(chord->tick())->drumset();
        return ds;
    }
    return nullptr;
}

//---------------------------------------------------------
// totalTiedNoteTicks
//      return the total of the actualTicks of the given note plus
//      the chain of zero or more notes tied to it to the right.
//---------------------------------------------------------

int CompatMidiRender::totalTiedNoteTicks(Note* note)
{
    Fraction total = note->chord()->actualTicks();
    while (note->tieFor() && note->tieFor()->endNote() && (note->chord()->tick() < note->tieFor()->endNote()->chord()->tick())) {
        note = note->tieFor()->endNote();
        total += note->chord()->actualTicks();
    }
    return total.ticks();
}

static int slideTicks(const Chord* chord)
{
    const int currentTicks = chord->ticks().ticks();
    if (currentTicks <= SLIDE_DURATION) {
        return currentTicks / 2;
    }

    return SLIDE_DURATION;
}

static int graceBendTicks(const Chord* chord)
{
    const int currentTicks = chord->ticks().ticks();
    if (currentTicks / 2 <= GRACE_BEND_DURATION) {
        return currentTicks / 2;
    }

    return GRACE_BEND_DURATION;
}

int CompatMidiRender::tick(const CompatMidiRendererInternal::Context& ctx, int tick)
{
    return ctx.applyCaesuras ? ctx.pauseMap->tickWithPauses(tick) : tick;
}
}
