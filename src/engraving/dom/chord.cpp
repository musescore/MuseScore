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

#include "chord.h"

#include <cmath>
#include <vector>

#include "containers.h"

#include "accidental.h"
#include "arpeggio.h"
#include "articulation.h"
#include "beam.h"
#include "chordline.h"
#include "drumset.h"
#include "factory.h"
#include "guitarbend.h"
#include "hook.h"
#include "key.h"
#include "ledgerline.h"
#include "measure.h"
#include "mscore.h"
#include "navigate.h"
#include "note.h"
#include "notedot.h"
#include "noteevent.h"
#include "noteline.h"
#include "ornament.h"
#include "part.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "stafftype.h"
#include "stem.h"
#include "stemslash.h"
#include "stretchedbend.h"
#include "stringdata.h"
#include "system.h"
#include "tie.h"
#include "tremolosinglechord.h"
#include "tremolotwochord.h"
#include "trill.h"
#include "tuplet.h"
#include "undo.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#endif

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   upNote
//---------------------------------------------------------

Note* Chord::upNote() const
{
    assert(!m_notes.empty());

    Note* result = m_notes.back();
    const Staff* stf = staff();
    if (!stf) {
        return result;
    }

    const StaffType* st  = stf->staffTypeForElement(this);
    if (st->isDrumStaff()) {
        for (Note* n : m_notes) {
            if (n->line() < result->line()) {
                result = n;
            }
        }
    } else if (st->isTabStaff()) {
        int line = st->lines() - 1;          // start at bottom line
        int noteLine;
        // scan each note: if TAB strings are not in sequential order,
        // visual order of notes might not correspond to pitch order
        for (Note* n : m_notes) {
            noteLine = st->physStringToVisual(n->string());
            if (noteLine < line) {
                line   = noteLine;
                result = n;
            }
        }
    }
    return result;
}

//---------------------------------------------------------
//   downNote
//---------------------------------------------------------

Note* Chord::downNote() const
{
    assert(!m_notes.empty());

    Note* result = m_notes.front();
    const Staff* stf = staff();
    if (!stf) {
        return result;
    }

    const StaffType* st  = stf->staffTypeForElement(this);
    if (st->isDrumStaff()) {
        for (Note* n : m_notes) {
            if (n->line() > result->line()) {
                result = n;
            }
        }
    } else if (st->isTabStaff()) {
        int line        = 0;          // start at top line
        int noteLine;
        // scan each note: if TAB strings are not in sequential order,
        // visual order of notes might not correspond to pitch order
        for (Note* n : m_notes) {
            noteLine = st->physStringToVisual(n->string());
            if (noteLine > line) {
                line = noteLine;
                result = n;
            }
        }
    }
    return result;
}

//---------------------------------------------------------
//   upLine / downLine
//---------------------------------------------------------

int Chord::upLine() const
{
    return onTabStaff() ? upString() * 2 : upNote()->line();
}

int Chord::downLine() const
{
    return onTabStaff() ? downString() * 2 : downNote()->line();
}

//---------------------------------------------------------
//   upString / downString
//
//    return the topmost / bottommost string used by chord
//    Top and bottom refer to the DRAWN position, not the position in the instrument
//    (i.e., upside-down TAB are taken into account)
//
//    If no staff, always return 0
//    If staff is not a TAB, always returns TOP and BOTTOM staff lines
//---------------------------------------------------------

int Chord::upString() const
{
    // if no staff or staff not a TAB, return 0 (=topmost line)
    const Staff* st = staff();
    if (!st) {
        return 0;
    }

    const StaffType* tab = st->staffTypeForElement(this);
    if (!tab->isTabStaff()) {
        return 0;
    }

    int line = tab->lines() - 1;              // start at bottom line
    int noteLine;
    // scan each note: if TAB strings are not in sequential order,
    // visual order of notes might not correspond to pitch order
    size_t n = m_notes.size();
    for (size_t i = 0; i < n; ++i) {
        noteLine = tab->physStringToVisual(m_notes.at(i)->string());
        if (noteLine < line) {
            line = noteLine;
        }
    }
    return line;
}

int Chord::downString() const
{
    const Staff* st = staff();
    if (!st) {                                         // if no staff, return 0
        return 0;
    }

    const StaffType* tab = st->staffTypeForElement(this);
    if (!tab->isTabStaff()) {                // if staff not a TAB, return bottom line
        return st->lines(tick()) - 1;
    }

    int line = 0;           // start at top line
    int noteLine;
    size_t n = m_notes.size();
    for (size_t i = 0; i < n; ++i) {
        noteLine = tab->physStringToVisual(m_notes.at(i)->string());
        if (noteLine > line) {
            line = noteLine;
        }
    }
    return line;
}

//---------------------------------------------------------
//   noteDistances
//---------------------------------------------------------

std::vector<int> Chord::noteDistances() const
{
    const StaffType* staffType = this->staffType();
    assert(staffType);
    bool isTabStaff = staffType->isTabStaff();
    int staffMiddleLine = staffType->middleLine();

    std::vector<int> distances;
    for (Note* note : m_notes) {
        int noteLine = isTabStaff ? note->string() : note->line();
        distances.push_back(noteLine - staffMiddleLine);
    }
    return distances;
}

//---------------------------------------------------------
//   Chord
//---------------------------------------------------------

Chord::Chord(Segment* parent)
    : ChordRest(ElementType::CHORD, parent)
{
    m_stem             = 0;
    m_hook             = 0;
    m_stemDirection    = DirectionV::AUTO;
    m_arpeggio         = 0;
    m_spanArpeggio     = 0;
    m_endsNoteAnchoredLine    = false;
    m_noteType         = NoteType::NORMAL;
    m_stemSlash        = 0;
    m_noStem           = false;
    m_showStemSlash    = m_noteType == NoteType::ACCIACCATURA;
    m_playEventType    = PlayEventType::Auto;
    m_spaceLw          = 0.;
    m_spaceRw          = 0.;
    m_crossMeasure     = CrossMeasure::UNKNOWN;
    m_graceIndex       = 0;
    m_combineVoice     = AutoOnOff::AUTO;
}

Chord::Chord(const Chord& c, bool link)
    : ChordRest(c, link)
{
    if (link) {
        score()->undo(new Link(this, const_cast<Chord*>(&c)));
    }

    for (Note* onote : c.m_notes) {
        Note* nnote = Factory::copyNote(*onote, link);
        add(nnote);
    }
    for (Chord* gn : c.graceNotes()) {
        Chord* nc = new Chord(*gn, link);
        add(nc);
    }
    for (Articulation* a : c.m_articulations) {      // make deep copy
        Articulation* na = a->clone();
        if (link) {
            score()->undo(new Link(na, a));
        }
        na->setParent(this);
        na->setTrack(track());
        m_articulations.push_back(na);
    }
    m_stem          = 0;
    m_hook          = 0;
    m_endsNoteAnchoredLine = false;
    m_arpeggio      = 0;
    m_stemSlash     = 0;

    m_spanArpeggio   = c.m_spanArpeggio;
    m_graceIndex     = c.m_graceIndex;
    m_noStem         = c.m_noStem;
    m_showStemSlash  = c.m_showStemSlash;
    m_playEventType  = c.m_playEventType;
    m_stemDirection  = c.m_stemDirection;
    m_noteType       = c.m_noteType;
    m_crossMeasure   = CrossMeasure::UNKNOWN;
    m_combineVoice     = c.m_combineVoice;

    if (c.m_stem) {
        add(Factory::copyStem(*(c.m_stem)));
    }
    if (c.m_hook) {
        add(new Hook(*(c.m_hook)));
    }
    if (c.m_stemSlash) {
        add(Factory::copyStemSlash(*(c.m_stemSlash)));
    }
    if (c.m_arpeggio) {
        Arpeggio* a = new Arpeggio(*(c.m_arpeggio));
        add(a);
        if (link) {
            score()->undo(new Link(a, const_cast<Arpeggio*>(c.m_arpeggio)));
        }
    }

    if (c.m_tremoloSingleChord) {
        TremoloSingleChord* t = Factory::copyTremoloSingleChord(*(c.m_tremoloSingleChord));
        if (link) {
            score()->undo(new Link(t, const_cast<TremoloSingleChord*>(c.m_tremoloSingleChord)));
        }
        add(t);
    } else if (c.m_tremoloTwoChord) {
        if (c.m_tremoloTwoChord->chord1() == &c) {
            TremoloTwoChord* t = Factory::copyTremoloTwoChord(*(c.m_tremoloTwoChord));
            if (link) {
                score()->undo(new Link(t, const_cast<TremoloTwoChord*>(c.m_tremoloTwoChord)));
            }
            t->setChords(this, nullptr);
            add(t);
        }
    }

    for (EngravingItem* e : c.el()) {
        if (e->isChordLine()) {
            ChordLine* cl = toChordLine(e);
            ChordLine* ncl = Factory::copyChordLine(*cl);
            add(ncl);
            if (cl->note()) {
                ncl->setNote(findNote(cl->note()->pitch()));
            }
            if (link) {
                score()->undo(new Link(ncl, cl));
            }
        } else if (e->isStretchedBend()) {
            StretchedBend* sb = toStretchedBend(e);
            StretchedBend* nsb = Factory::copyStretchedBend(*sb);
            add(nsb);
            if (Note* originalNote = sb->note()) {
                for (Note* note : notes()) {
                    if (note->pitch() == originalNote->pitch() && note->string() == originalNote->string()) {
                        nsb->setNote(note);
                        note->setStretchedBend(nsb);
                        break;
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------
//   undoUnlink
//---------------------------------------------------------

void Chord::undoUnlink()
{
    ChordRest::undoUnlink();
    for (Note* n : m_notes) {
        n->undoUnlink();
    }
    for (Chord* gn : graceNotes()) {
        gn->undoUnlink();
    }
    for (Articulation* a : m_articulations) {
        a->undoUnlink();
    }
/*      if (_glissando)
            _glissando->undoUnlink(); */
    if (m_arpeggio) {
        m_arpeggio->undoUnlink();
    }

    if (tremoloSingleChord()) {
        tremoloSingleChord()->undoUnlink();
    }

    for (EngravingItem* e : el()) {
        if (e->type() == ElementType::CHORDLINE) {
            e->undoUnlink();
        }
    }
}

//---------------------------------------------------------
//   ~Chord
//---------------------------------------------------------

Chord::~Chord()
{
    muse::DeleteAll(m_articulations);

    if (tremoloTwoChord()) {
        if (tremoloTwoChord()->chord1() == this) {
            tremoloTwoChord()->setChord1(nullptr);
        } else if (tremoloTwoChord()->chord2() == this) {
            tremoloTwoChord()->setChord2(nullptr);
        }
    }

    delete m_arpeggio;
    delete m_stemSlash;
    delete m_stem;
    delete m_hook;
    muse::DeleteAll(m_ledgerLines);
    muse::DeleteAll(m_graceNotes);
    muse::DeleteAll(m_notes);
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
AccessibleItemPtr Chord::createAccessible()
{
    return std::make_shared<AccessibleItem>(this, AccessibleItem::Group);
}

#endif

//---------------------------------------------------------
//   noteHeadWidth
//---------------------------------------------------------

double Chord::noteHeadWidth() const
{
    return score()->noteHeadWidth() * mag();
}

//---------------------------------------------------------
//   rightEdge
//---------------------------------------------------------

double Chord::rightEdge() const
{
    double right = 0.0;
    for (Note* n : notes()) {
        right = std::max(right, x() + n->x() + n->bboxRightPos());
    }

    return right;
}

TremoloType Chord::tremoloType() const
{
    if (tremoloSingleChord()) {
        return tremoloSingleChord()->tremoloType();
    } else if (tremoloTwoChord()) {
        return tremoloTwoChord()->tremoloType();
    } else {
        return TremoloType::INVALID_TREMOLO;
    }
}

TremoloTwoChord* Chord::tremoloTwoChord() const
{
    return m_tremoloTwoChord;
}

TremoloSingleChord* Chord::tremoloSingleChord() const
{
    return m_tremoloSingleChord;
}

void Chord::setTremoloTwoChord(TremoloTwoChord* tr, bool applyLogic)
{
    if (m_tremoloTwoChord && tr && tr == m_tremoloTwoChord) {
        return;
    }

    if (m_tremoloTwoChord) {
        TDuration d;
        const Fraction f = ticks();
        if (f.numerator() > 0) {
            d = TDuration(f);
        } else {
            d = m_tremoloTwoChord->durationType();
            const int dots = d.dots();
            d = d.shift(1);
            d.setDots(dots);
        }

        setDurationType(d);
        Chord* other = m_tremoloTwoChord->chord1() == this ? m_tremoloTwoChord->chord2() : m_tremoloTwoChord->chord1();
        m_tremoloTwoChord = nullptr;
        if (other) {
            other->setTremoloTwoChord(nullptr);
        }
    }
    m_tremoloSingleChord = nullptr;

    if (tr && applyLogic) {
        TDuration d = tr->durationType();
        if (!d.isValid()) {
            d = durationType();
            const int dots = d.dots();
            d = d.shift(-1);
            d.setDots(dots);
            tr->setDurationType(d);
        }

        setDurationType(d);
        Chord* other = tr->chord1() == this ? tr->chord2() : tr->chord1();
        m_tremoloTwoChord = tr;
        if (other) {
            other->setTremoloTwoChord(tr);
        }
    }

    m_tremoloTwoChord = tr;
}

void Chord::setTremoloSingleChord(TremoloSingleChord* tr)
{
    if (m_tremoloSingleChord && tr && tr == m_tremoloSingleChord) {
        return;
    }

    if (m_tremoloTwoChord) {
        TDuration d;
        const Fraction f = ticks();
        if (f.numerator() > 0) {
            d = TDuration(f);
        } else {
            d = m_tremoloTwoChord->durationType();
            const int dots = d.dots();
            d = d.shift(1);
            d.setDots(dots);
        }

        setDurationType(d);
        Chord* other = m_tremoloTwoChord->chord1() == this ? m_tremoloTwoChord->chord2() : m_tremoloTwoChord->chord1();
        m_tremoloTwoChord = nullptr;
        if (other) {
            other->setTremoloTwoChord(nullptr);
        }
    }

    m_tremoloSingleChord = tr;
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Chord::add(EngravingItem* e)
{
    if (e->explicitParent() != this) {
        e->setParent(this);
    }
    e->setTrack(track());
    switch (e->type()) {
    case ElementType::NOTE:
    {
        Note* note = toNote(e);
        bool tab = staff() && staff()->isTabStaff(tick());
        bool found = false;

        for (unsigned idx = 0; idx < m_notes.size(); ++idx) {
            if (tab) {
                // _notes should be sorted by string
                if (note->string() > m_notes[idx]->string()) {
                    m_notes.insert(m_notes.begin() + idx, note);
                    found = true;
                    break;
                }
            } else {
                // _notes should be sorted by line position,
                // but it's often not yet possible since line is unknown
                // use pitch instead, and line as a second sort criteria.
                if (note->pitch() <= m_notes[idx]->pitch()) {
                    if (note->pitch() == m_notes[idx]->pitch() && note->line() >= m_notes[idx]->line()) {
                        m_notes.insert(m_notes.begin() + idx + 1, note);
                    } else {
                        m_notes.insert(m_notes.begin() + idx, note);
                    }
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            m_notes.push_back(note);
        }
        note->connectTiedNotes();
        if (voice() && measure() && note->visible()) {
            measure()->setHasVoices(staffIdx(), true);
        }
    }
        score()->setPlaylistDirty();
        break;
    case ElementType::ARPEGGIO:
        m_arpeggio = toArpeggio(e);
        break;
    case ElementType::TREMOLO_TWOCHORD:
        setTremoloTwoChord(item_cast<TremoloTwoChord*>(e));
        break;
    case ElementType::TREMOLO_SINGLECHORD:
        setTremoloSingleChord(item_cast<TremoloSingleChord*>(e));
        break;
    case ElementType::GLISSANDO:
        m_endsNoteAnchoredLine = true;
        break;
    case ElementType::STEM:
        assert(!m_stem);
        m_stem = toStem(e);
        break;
    case ElementType::HOOK:
        m_hook = toHook(e);
        break;
    case ElementType::STRETCHED_BEND:
    case ElementType::CHORDLINE:
    case ElementType::FRET_CIRCLE:
        addEl(e);
        break;
    case ElementType::STEM_SLASH:
        assert(!m_stemSlash);
        m_stemSlash = toStemSlash(e);
        break;
    case ElementType::CHORD:
    {
        Chord* gc = toChord(e);
        assert(gc->noteType() != NoteType::NORMAL);
        size_t idx = gc->graceIndex();
        gc->setFlag(ElementFlag::MOVABLE, true);
        m_graceNotes.insert(m_graceNotes.begin() + idx, gc);
    }
    break;
    case ElementType::LEDGER_LINE:
        ASSERT_X("Chord::add ledgerline");
        break;
    case ElementType::ARTICULATION:
    case ElementType::ORNAMENT:
    {
        Articulation* a = toArticulation(e);
        if (a->layoutCloseToNote()) {
            if (a->isStaccato()) {
                // staccato is always the first articulation
                m_articulations.insert(m_articulations.begin(), a);
            } else {
                auto i = m_articulations.begin();
                while (i != m_articulations.end() && (*i)->layoutCloseToNote()) {
                    i++;
                }
                m_articulations.insert(i, a);
            }
        } else {
            m_articulations.push_back(a);
        }
    }
    break;
    default:
        ChordRest::add(e);
        return;
    }

    e->added();
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Chord::remove(EngravingItem* e)
{
    if (!e) {
        return;
    }

    switch (e->type()) {
    case ElementType::NOTE:
    {
        Note* note = toNote(e);
        auto i = std::find(m_notes.begin(), m_notes.end(), note);
        if (i != m_notes.end()) {
            m_notes.erase(i);
            note->disconnectTiedNotes();
            for (Spanner* s : note->spannerBack()) {
                note->removeSpannerBack(s);
            }
            for (Spanner* s : note->spannerFor()) {
                note->removeSpannerFor(s);
            }
            if (StretchedBend* stretchedBend = note->stretchedBend()) {
                removeEl(stretchedBend);
            }
        } else {
            LOGD("Chord::remove() note %p not found!", e);
        }
        if (voice() && measure() && note->visible()) {
            measure()->checkMultiVoices(staffIdx());
        }
        score()->setPlaylistDirty();
    }
    break;

    case ElementType::ARPEGGIO:
        if (m_spanArpeggio == m_arpeggio) {
            m_spanArpeggio = nullptr;
        }
        m_arpeggio = nullptr;
        break;
    case ElementType::TREMOLO_TWOCHORD:
        setTremoloTwoChord(nullptr);
        break;
    case ElementType::TREMOLO_SINGLECHORD:
        setTremoloSingleChord(nullptr);
        break;
    case ElementType::GLISSANDO:
        m_endsNoteAnchoredLine = false;
        break;
    case ElementType::STEM:
        m_stem = 0;
        break;
    case ElementType::HOOK:
        m_hook = 0;
        break;
    case ElementType::STEM_SLASH:
        assert(m_stemSlash);
        if (m_stemSlash->selected() && score()) {
            score()->deselect(m_stemSlash);
        }
        m_stemSlash = 0;
        break;
    case ElementType::STRETCHED_BEND:
    {
        StretchedBend* stretchedBend = toStretchedBend(e);
        auto it = std::find_if(m_notes.begin(), m_notes.end(), [stretchedBend](Note* note) {
                return note->stretchedBend() == stretchedBend;
            });
        if (it != m_notes.end()) {
            (*it)->setStretchedBend(nullptr);
        }
    }
    // fallthrough
    case ElementType::CHORDLINE:
    case ElementType::FRET_CIRCLE:
        removeEl(e);
        break;
    case ElementType::CHORD:
    {
        auto i = std::find(m_graceNotes.begin(), m_graceNotes.end(), toChord(e));
        Chord* grace = *i;
        grace->setGraceIndex(i - m_graceNotes.begin());
        m_graceNotes.erase(i);
    }
    break;
    case ElementType::ARTICULATION:
    case ElementType::ORNAMENT:
    {
        Articulation* a = toArticulation(e);
        if (!muse::remove(m_articulations, a)) {
            LOGD("ChordRest::remove(): articulation not found");
        }
    }
    break;
    default:
        ChordRest::remove(e);
        return;
    }

    e->removed();
}

//---------------------------------------------------------
//   maxHeadWidth
//---------------------------------------------------------

double Chord::maxHeadWidth() const
{
    // determine max head width in chord
    double hw = 0;
    for (const Note* n : m_notes) {
        double t = n->headWidth();
        if (t > hw) {
            hw = t;
        }
    }
    return hw;
}

//---------------------------------------------------------
//   selectedNote
//---------------------------------------------------------

Note* Chord::selectedNote() const
{
    Note* note = 0;
    size_t n = m_notes.size();
    for (size_t i = 0; i < n; ++i) {
        Note* currentNote = m_notes.at(i);
        if (currentNote->selected()) {
            if (note) {
                return 0;
            }
            note = currentNote;
        }
    }
    return note;
}

//---------------------------------------------------------
//   upPos
//---------------------------------------------------------

double Chord::upPos() const
{
    return upNote()->pos().y();
}

//---------------------------------------------------------
//   downPos
//---------------------------------------------------------

double Chord::downPos() const
{
    return downNote()->pos().y();
}

bool Chord::allNotesTiedToNext() const
{
    Chord* tiedChord = nullptr;
    for (Note* note : m_notes) {
        if (!note->tieFor()) {
            return false;
        }

        Note* endNote = note->tieFor()->endNote();
        Chord* endChord = endNote ? endNote->chord() : nullptr;
        if (!endChord) {
            return false;
        }

        if (!tiedChord) {
            tiedChord = endChord;
            continue;
        }

        if (endChord != tiedChord) {
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------
//   processSiblings
//---------------------------------------------------------

void Chord::processSiblings(std::function<void(EngravingItem*)> func, bool includeTemporarySiblings) const
{
    if (m_hook) {
        func(m_hook);
    }
    if (m_stem) {
        func(m_stem);
    }
    if (m_stemSlash) {
        func(m_stemSlash);
    }
    if (m_arpeggio) {
        func(m_arpeggio);
    }
    if (m_tremoloTwoChord) {
        func(m_tremoloTwoChord);
    }
    if (m_tremoloSingleChord) {
        func(m_tremoloSingleChord);
    }
    if (includeTemporarySiblings) {
        for (LedgerLine* ll : m_ledgerLines) {
            func(ll);
        }
    }
    for (Articulation* a : m_articulations) {
        func(a);
    }
    for (Note* note : m_notes) {
        func(note);
    }
    for (Chord* chord : m_graceNotes) {    // process grace notes last, needed for correct shape calculation
        func(chord);
    }
}

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Chord::setTrack(track_idx_t val)
{
    ChordRest::setTrack(val);
    processSiblings([val](EngravingItem* e) { e->setTrack(val); }, true);
}

bool Chord::shouldCombineVoice() const
{
    return combineVoice() == AutoOnOff::ON || (combineVoice() == AutoOnOff::AUTO && style().styleB(Sid::combineVoice));
}

bool Chord::combineVoice(const Chord* chord1, const Chord* chord2)
{
    return chord1->shouldCombineVoice() && chord2->shouldCombineVoice();
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Chord::setScore(Score* s)
{
    ChordRest::setScore(s);
    processSiblings([s](EngravingItem* e) { e->setScore(s); }, true);
}

Fraction Chord::endTickIncludingTied() const
{
    const Chord* lastTied = this;
    while (lastTied) {
        const Chord* next = lastTied->nextTiedChord();
        if (next) {
            lastTied = next;
        } else {
            break;
        }
    }
    return lastTied->tick() + lastTied->actualTicks();
}

Chord* Chord::prev() const
{
    ChordRest* prev = prevChordRest(const_cast<Chord*>(this));
    if (prev && prev->isChord()) {
        return static_cast<Chord*>(prev);
    }
    return nullptr;
}

Chord* Chord::next() const
{
    ChordRest* next = nextChordRest(const_cast<Chord*>(this));
    if (next && next->isChord()) {
        return static_cast<Chord*>(next);
    }
    return nullptr;
}

void Chord::resizeLedgerLinesTo(size_t newSize)
{
    int ledgerLineCountDiff = static_cast<int>(newSize - m_ledgerLines.size());
    if (ledgerLineCountDiff > 0) {
        for (int i = 0; i < ledgerLineCountDiff; ++i) {
            m_ledgerLines.push_back(new LedgerLine(score()->dummy()));
        }
    } else {
        for (int i = 0; i < std::abs(ledgerLineCountDiff); ++i) {
            delete m_ledgerLines.back();
            m_ledgerLines.pop_back();
        }
    }

    assert(m_ledgerLines.size() == newSize);
}

void Chord::setBeamExtension(double extension)
{
    if (m_stem) {
        double baseLength = m_stem->absoluteFromSpatium(m_stem->baseLength());
        m_stem->setBaseLength(std::max(Spatium::fromMM(baseLength + extension, spatium()), Spatium(0.0)));
        m_defaultStemLength = std::max(m_defaultStemLength + extension, m_stem->absoluteFromSpatium(m_stem->baseLength()));
    }
}

bool Chord::shouldHaveStem() const
{
    const Staff* staff = this->staff();
    const StaffType* staffType = staff ? staff->staffTypeForElement(this) : nullptr;

    return !m_noStem
           && durationType().hasStem()
           && !(durationType().type() == DurationType::V_HALF && staffType && staffType->isTabStaff()
                && staffType->minimStyle() == TablatureMinimStyle::NONE)
           && !(measure() && measure()->stemless(staffIdx()))
           && !(staffType && staffType->isTabStaff() && staffType->stemless());
}

bool Chord::shouldHaveHook() const
{
    return shouldHaveStem()
           && durationType().hooks() > 0
           && !beam()
           && !tremoloTwoChord();
}

void Chord::createStem()
{
    Stem* stem = Factory::createStem(this);
    stem->setParent(this);
    stem->setGenerated(true);
    //! score()->undoAddElement calls add(), which assigns this created stem to _stem
    score()->doUndoAddElement(stem);
}

void Chord::removeStem()
{
    if (m_stem) {
        score()->undoRemoveElement(m_stem);
    }
    if (m_hook) {
        score()->undoRemoveElement(m_hook);
    }
    if (m_stemSlash) {
        score()->undoRemoveElement(m_stemSlash);
    }
}

void Chord::createHook()
{
    Hook* hook = new Hook(this);
    hook->setParent(this);
    hook->setGenerated(true);
    score()->doUndoAddElement(hook);
}

//---------------------------------------------------------
//    underBeam: true, if grace note is placed under a beam.
//---------------------------------------------------------

bool Chord::underBeam() const
{
    if (m_noteType == NoteType::NORMAL) {
        return false;
    }
    const Chord* cr = toChord(explicitParent());
    Beam* beam = cr->beam();
    if (!beam || !cr->beam()->up()) {
        return false;
    }
    size_t s = beam->elements().size();
    if (isGraceBefore()) {
        if (beam->elements()[0] != cr) {
            return true;
        }
    }
    if (isGraceAfter()) {
        if (beam->elements()[s - 1] != cr) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   updatePercussionNotes
//---------------------------------------------------------

static void updatePercussionNotes(Chord* c, const Drumset* drumset)
{
    TRACEFUNC;
    for (Chord* ch : c->graceNotes()) {
        updatePercussionNotes(ch, drumset);
    }
    std::vector<Note*> lnotes(c->notes());    // we need a copy!
    for (Note* note : lnotes) {
        if (!drumset) {
            note->setLine(0);
        } else {
            int pitch = note->pitch();
            if (!drumset->isValid(pitch)) {
                note->setLine(0);
                //! NOTE May be called too often
                //LOGW("unmapped drum note %d", pitch);
            } else if (!note->fixed()) {
                note->undoChangeProperty(Pid::HEAD_GROUP, int(drumset->noteHead(pitch)));
                note->setLine(drumset->line(pitch));
            }
        }
    }
}

//---------------------------------------------------------
//   cmdUpdateNotes
//---------------------------------------------------------

void Chord::cmdUpdateNotes(AccidentalState* as, staff_idx_t staffIdx)
{
    // TAB_STAFF is different, as each note has to be fretted
    // in the context of the all of the chords of the whole segment

    const Staff* st = staff();
    StaffGroup staffGroup = st->staffTypeForElement(this)->group();
    if (staffGroup == StaffGroup::TAB) {
        Fraction tick = this->tick();
        const StringData* stringData = part()->stringData(tick, st->idx());
        for (Chord* ch : graceNotes()) {
            stringData->fretChords(ch);
        }
        stringData->fretChords(this);
        return;
    } else {
        // if not tablature, use instrument->useDrumset to set staffGroup (to allow pitched to unpitched in same staff)
        staffGroup = st->part()->instrument(this->tick())->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;
    }

    // PITCHED_ and PERCUSSION_STAFF can go note by note

    if (staffGroup == StaffGroup::STANDARD) {
        const std::vector<Chord*> gnb(graceNotesBefore());
        for (Chord* ch : gnb) {
            if (ch->vStaffIdx() != staffIdx) {
                continue;
            }
            std::vector<Note*> notes(ch->notes());        // we need a copy!
            for (Note* note : notes) {
                note->updateAccidental(as);
            }
            ch->sortNotes();
        }
        if (vStaffIdx() == staffIdx) {
            std::vector<Note*> lnotes(notes());      // we need a copy!
            for (Note* note : lnotes) {
                if (note->tieBackNonPartial() && note->tpc() == note->tieBack()->startNote()->tpc()) {
                    // same pitch
                    if (note->accidental() && note->accidental()->role() == AccidentalRole::AUTO) {
                        // not courtesy
                        // TODO: remove accidental only if note is not
                        // on new system
                        score()->undoRemoveElement(note->accidental());
                    }
                }
                note->updateAccidental(as);
            }
            for (Articulation* art : m_articulations) {
                if (!art->isOrnament()) {
                    continue;
                }
                Ornament* ornament = toOrnament(art);
                ornament->computeNotesAboveAndBelow(as);
            }
            for (Spanner* spanner : startingSpanners()) {
                if (spanner->isTrill()) {
                    Ornament* ornament = toTrill(spanner)->ornament();
                    if (ornament) {
                        ornament->setParent(this);
                        ornament->computeNotesAboveAndBelow(as);
                    }
                }
            }
            sortNotes();
        }
        const std::vector<Chord*> gna(graceNotesAfter());
        for (Chord* ch : gna) {
            if (ch->vStaffIdx() != staffIdx) {
                continue;
            }
            std::vector<Note*> notes(ch->notes());        // we need a copy!
            for (Note* note : notes) {
                note->updateAccidental(as);
            }
            ch->sortNotes();
        }
    } else if (staffGroup == StaffGroup::PERCUSSION) {
        const Instrument* instrument = part()->instrument(this->tick());
        const Drumset* drumset = instrument->drumset();
        if (!drumset) {
            LOGW("no drumset");
        }
        updatePercussionNotes(this, drumset);
        sortNotes();
    }
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF Chord::pagePos() const
{
    if (isGrace()) {
        PointF p(pos());
        if (explicitParent() == 0) {
            return p;
        }
        p.rx() = pageX();

        const Chord* pc = static_cast<const Chord*>(explicitParent());
        System* system = pc->segment()->system();
        if (!system) {
            return p;
        }
        p.ry() += system->staffYpage(vStaffIdx()) + staffOffsetY();
        return p;
    }
    return EngravingItem::pagePos();
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Chord::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    for (Articulation* a : m_articulations) {
        a->scanElements(data, func, all);
    }
    if (m_hook) {
        func(data, m_hook);
    }
    if (m_stem) {
        func(data, m_stem);
    }
    if (m_stemSlash) {
        func(data, m_stemSlash);
    }
    if (m_arpeggio) {
        func(data, m_arpeggio);
    }
    if (m_tremoloTwoChord && (tremoloChordType() != TremoloChordType::TremoloSecondChord)) {
        func(data, m_tremoloTwoChord);
    }
    if (m_tremoloSingleChord) {
        func(data, m_tremoloSingleChord);
    }
    const Staff* st = staff();
    if ((st && st->showLedgerLines(tick())) || !st) {       // also for palette
        for (LedgerLine* ll : m_ledgerLines) {
            func(data, ll);
        }
    }
    for (Note* note : m_notes) {
        note->scanElements(data, func, all);
    }
    for (Chord* chord : m_graceNotes) {
        chord->scanElements(data, func, all);
    }
    for (EngravingItem* e : el()) {
        e->scanElements(data, func, all);
    }
    ChordRest::scanElements(data, func, all);
}

//---------------------------------------------------------
//   isChordPlayable
//   @note Now every related to chord element has it's own "PLAY" property,
//         However, there is no way to control these properties outside the scope of the chord since the new inspector.
//         So we'll use a chord as a proxy entity for "PLAY" property handling
//---------------------------------------------------------

bool Chord::isChordPlayable() const
{
    if (!m_notes.empty()) {
        if (m_notes.front()->isPreBendStart()) {
            return false;
        }

        return m_notes.front()->getProperty(Pid::PLAY).toBool();
    } else if (tremoloTwoChord()) {
        return tremoloTwoChord()->getProperty(Pid::PLAY).toBool();
    } else if (tremoloSingleChord()) {
        return tremoloSingleChord()->getProperty(Pid::PLAY).toBool();
    } else if (m_arpeggio) {
        return m_arpeggio->getProperty(Pid::PLAY).toBool();
    }

    return false;
}

//---------------------------------------------------------
//   setIsChordPlayable
//---------------------------------------------------------

void Chord::setIsChordPlayable(const bool isPlayable)
{
    for (Note* note : m_notes) {
        note->undoChangeProperty(Pid::PLAY, isPlayable);
    }

    if (m_arpeggio) {
        m_arpeggio->undoChangeProperty(Pid::PLAY, isPlayable);
    }

    if (tremoloTwoChord()) {
        tremoloTwoChord()->undoChangeProperty(Pid::PLAY, isPlayable);
    }

    if (tremoloSingleChord()) {
        tremoloSingleChord()->undoChangeProperty(Pid::PLAY, isPlayable);
    }

    triggerLayout();
}

//---------------------------------------------------------
//   findNote
//---------------------------------------------------------

Note* Chord::findNote(int pitch, int skip) const
{
    size_t ns = m_notes.size();
    for (size_t i = 0; i < ns; ++i) {
        Note* n = m_notes.at(i);
        if (n->pitch() == pitch) {
            if (skip == 0) {
                return n;
            } else {
                --skip;
            }
        }
    }
    return 0;
}

void Chord::undoChangeSpanArpeggio(Arpeggio* a)
{
    if (m_spanArpeggio == a) {
        return;
    }

    // TODO: change arpeggio for links
    score()->undo(new ChangeSpanArpeggio(this, a));
}

ChordLine* Chord::chordLine() const
{
    for (EngravingItem* item : el()) {
        if (item && item->isChordLine()) {
            return toChordLine(item);
        }
    }

    return nullptr;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* Chord::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    switch (e->type()) {
    case ElementType::ARTICULATION:
    case ElementType::ORNAMENT:
    {
        Articulation* atr = toArticulation(e);
        Articulation* oa = hasArticulation(atr);
        if (oa) {
            delete atr;
            atr = 0;
            // if attribute is already there, remove
            // score()->cmdRemove(oa); // unexpected behaviour?
            score()->select(oa, SelectType::SINGLE, 0);
        } else {
            // intuit anchor for this accidental based on other accidentals in the chord
            int aboveBelow = 0;
            bool mixed = false;
            for (Articulation* a : m_articulations) {
                PropertyFlags pf = a->propertyFlags(Pid::ARTICULATION_ANCHOR);
                if (pf == PropertyFlags::STYLED) {
                    continue;
                }
                if (a->anchor() == ArticulationAnchor::TOP) {
                    if (aboveBelow < 0) {
                        mixed = true;
                        break;
                    }
                    ++aboveBelow;
                } else if (a->anchor() == ArticulationAnchor::BOTTOM) {
                    if (aboveBelow > 0) {
                        mixed = true;
                        break;
                    }
                    --aboveBelow;
                }
            }
            if (!mixed && aboveBelow != 0) {
                if (aboveBelow > 0) {
                    atr->setAnchor(ArticulationAnchor::TOP);
                } else {
                    atr->setAnchor(ArticulationAnchor::BOTTOM);
                }
                atr->setPropertyFlags(Pid::ARTICULATION_ANCHOR, PropertyFlags::UNSTYLED);
            }
            atr->setParent(this);
            atr->setTrack(track());

            // Immediately add if the chord has no existing articulations, toggle otherwise...
            if (m_articulations.empty()) {
                score()->undoAddElement(atr);
            } else {
                score()->toggleArticulation(this, atr);
            }
        }
        return atr;
    }

    case ElementType::CHORDLINE:
        e->setParent(this);
        e->setTrack(track());
        score()->undoAddElement(e);
        break;

    case ElementType::TREMOLO_SINGLECHORD:
        if (tremoloSingleChord()) {
            bool sameType = (e->subtype() == tremoloSingleChord()->subtype());
            score()->undoRemoveElement(tremoloSingleChord());
            if (sameType) {
                delete e;
                return nullptr;
            }
        }
        e->setParent(this);
        e->setTrack(track());
        score()->undoAddElement(e);
        break;

    case ElementType::TREMOLO_TWOCHORD:
    {
        TremoloTwoChord* t = item_cast<TremoloTwoChord*>(e);
        Segment* s = segment()->next();
        while (s) {
            if (s->element(track()) && s->element(track())->isChord()) {
                break;
            }
            s = s->next();
        }
        if (s == 0) {
            LOGD("no segment for second note of tremolo found");
            delete e;
            return 0;
        }
        Chord* ch2 = toChord(s->element(track()));
        if (ch2->ticks() != ticks()) {
            LOGD("no matching chord for second note of tremolo found");
            delete e;
            return 0;
        }
        t->setChords(this, ch2);
    }
        if (tremoloTwoChord()) {
            bool sameType = (e->subtype() == tremoloTwoChord()->subtype());
            score()->undoRemoveElement(tremoloTwoChord());
            if (sameType) {
                delete e;
                return 0;
            }
        }
        e->setParent(this);
        e->setTrack(track());
        score()->undoAddElement(e);
        break;

    case ElementType::ARPEGGIO:
    {
        Arpeggio* a = toArpeggio(e);
        if (arpeggio()) {
            score()->undoRemoveElement(arpeggio());
        }
        a->setTrack(track());
        a->setParent(this);
        score()->undoAddElement(a);
    }
        return e;

    default:
        return ChordRest::drop(data);
    }
    return 0;
}

void Chord::setColor(const Color& color)
{
    ChordRest::setColor(color);

    for (Note* note : m_notes) {
        note->undoChangeProperty(Pid::COLOR, PropertyValue::fromValue(color));
    }
}

//---------------------------------------------------------
//   setStemDirection
//---------------------------------------------------------

void Chord::setStemDirection(DirectionV d)
{
    m_stemDirection = d;
}

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void Chord::localSpatiumChanged(double oldValue, double newValue)
{
    ChordRest::localSpatiumChanged(oldValue, newValue);
    for (EngravingItem* e : graceNotes()) {
        e->localSpatiumChanged(oldValue, newValue);
    }
    if (m_hook) {
        m_hook->localSpatiumChanged(oldValue, newValue);
    }
    if (m_stem) {
        m_stem->localSpatiumChanged(oldValue, newValue);
    }
    if (m_stemSlash) {
        m_stemSlash->localSpatiumChanged(oldValue, newValue);
    }
    if (m_arpeggio) {
        m_arpeggio->localSpatiumChanged(oldValue, newValue);
    }

    if (m_tremoloSingleChord) {
        m_tremoloSingleChord->localSpatiumChanged(oldValue, newValue);
    } else if (m_tremoloTwoChord && (tremoloChordType() != TremoloChordType::TremoloSecondChord)) {
        m_tremoloTwoChord->localSpatiumChanged(oldValue, newValue);
    }

    for (EngravingItem* e : articulations()) {
        e->localSpatiumChanged(oldValue, newValue);
    }
    for (Note* note : notes()) {
        note->localSpatiumChanged(oldValue, newValue);
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Chord::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::NO_STEM:         return noStem();
    case Pid::SHOW_STEM_SLASH: return showStemSlash();
    case Pid::SMALL:           return isSmall();
    case Pid::STEM_DIRECTION:  return PropertyValue::fromValue<DirectionV>(stemDirection());
    case Pid::PLAY: return isChordPlayable();
    case Pid::COMBINE_VOICE: return PropertyValue::fromValue<AutoOnOff>(combineVoice());
    default:
        return ChordRest::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Chord::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::NO_STEM:         return false;
    case Pid::SHOW_STEM_SLASH: return noteType() == NoteType::ACCIACCATURA;
    case Pid::SMALL:           return false;
    case Pid::STEM_DIRECTION:  return PropertyValue::fromValue<DirectionV>(DirectionV::AUTO);
    case Pid::PLAY: return true;
    case Pid::COMBINE_VOICE: return AutoOnOff::AUTO;
    default:
        return ChordRest::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Chord::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::NO_STEM:
        setNoStem(v.toBool());
        break;
    case Pid::SHOW_STEM_SLASH:
        requestShowStemSlash(v.toBool());
        break;
    case Pid::SMALL:
        setSmall(v.toBool());
        break;
    case Pid::STEM_DIRECTION:
        setStemDirection(v.value<DirectionV>());
        break;
    case Pid::PLAY:
        setIsChordPlayable(v.toBool());
        break;
    case Pid::COMBINE_VOICE:
        setCombineVoice(v.value<AutoOnOff>());
        break;
    default:
        return ChordRest::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   hasArticulation
//---------------------------------------------------------

Articulation* Chord::hasArticulation(const Articulation* aa)
{
    for (Articulation* a : m_articulations) {
        if (a->subtype() == aa->subtype()) {
            return a;
        }
    }
    return 0;
}

void Chord::updateArticulations(const std::set<SymId>& newArticulationIds, ArticulationsUpdateMode updateMode)
{
    Articulation* staccato = nullptr;
    Articulation* accent = nullptr;
    Articulation* marcato = nullptr;
    Articulation* tenuto = nullptr;
    ArticulationAnchor overallAnchor = ArticulationAnchor::AUTO;
    bool mixedDirections = false;
    if (!m_articulations.empty()) {
        std::vector<Articulation*> articsToRemove;
        // split all articulations
        for (Articulation* artic : m_articulations) {
            if (!artic->isDouble()) {
                continue;
            }
            auto splitSyms = splitArticulations({ artic->symId() });
            for (const SymId& id : splitSyms) {
                Articulation* newArticulation = Factory::createArticulation(score()->dummy()->chord());
                newArticulation->setSymId(id);
                newArticulation->setAnchor(artic->anchor());
                newArticulation->setPropertyFlags(Pid::ARTICULATION_ANCHOR, artic->propertyFlags(Pid::ARTICULATION_ANCHOR));
                if (!hasArticulation(newArticulation)) {
                    score()->toggleArticulation(this, newArticulation);
                } else {
                    delete newArticulation;
                }
            }
            articsToRemove.push_back(artic);
        }
        for (Articulation* artic : articsToRemove) {
            score()->undoRemoveElement(artic);
        }
        // now we have guaranteed no more combined artics
        // take an inventory of which articulations are already present
        for (Articulation* artic : m_articulations) {
            PropertyFlags pf = artic->propertyFlags(Pid::ARTICULATION_ANCHOR);
            if (!mixedDirections && pf == PropertyFlags::UNSTYLED && artic->anchor() != ArticulationAnchor::AUTO) {
                if (overallAnchor != ArticulationAnchor::AUTO && artic->anchor() != overallAnchor) {
                    mixedDirections = true;
                    overallAnchor = ArticulationAnchor::AUTO;
                } else {
                    overallAnchor = artic->anchor();
                }
            }
            if (artic->isStaccato()) {
                staccato = artic;
            } else if (artic->isAccent()) {
                accent = artic;
            } else if (artic->isMarcato()) {
                marcato = artic;
            } else if (artic->isTenuto()) {
                tenuto = artic;
            }
        }
    }
    std::set<SymId> newArtics;
    // get symbol id's for correct direction
    for (const SymId& id : newArticulationIds) {
        if (id == SymId::articAccentAbove || id == SymId::articAccentBelow) {
            if (marcato && updateMode == ArticulationsUpdateMode::Insert) {
                // adding an accent, which replaces marcato
                score()->undoRemoveElement(marcato);
            }
        } else if (id == SymId::articMarcatoAbove || id == SymId::articMarcatoBelow) {
            if (accent && updateMode == ArticulationsUpdateMode::Insert) {
                // adding a marcato, which replaces accent
                score()->undoRemoveElement(accent);
            }
        }
        newArtics.insert(id);
    }

    // newArtics now contains the articulations in the correct direction
    if (updateMode == ArticulationsUpdateMode::Remove) {
        // remove articulations from _articulations that are found in in newArtics
        for (const SymId& id : newArtics) {
            switch (id) {
            case SymId::articAccentAbove:
            case SymId::articAccentBelow:
                score()->undoRemoveElement(accent);
                accent = nullptr;
                break;
            case SymId::articMarcatoAbove:
            case SymId::articMarcatoBelow:
                score()->undoRemoveElement(marcato);
                marcato = nullptr;
                break;
            case SymId::articTenutoAbove:
            case SymId::articTenutoBelow:
                score()->undoRemoveElement(tenuto);
                tenuto = nullptr;
                break;
            case SymId::articStaccatoAbove:
            case SymId::articStaccatoBelow:
                score()->undoRemoveElement(staccato);
                staccato = nullptr;
                break;
            default:
                break;
            }
        }
    } else {
        // add articulations from newArtics that are not found in m_articulations
        for (const SymId& id : newArtics) {
            Articulation* newArticulation = Factory::createArticulation(score()->dummy()->chord());
            newArticulation->setSymId(id);
            if (overallAnchor != ArticulationAnchor::AUTO) {
                newArticulation->setAnchor(overallAnchor);
                newArticulation->setPropertyFlags(Pid::ARTICULATION_ANCHOR, PropertyFlags::UNSTYLED);
            }
            if (!hasArticulation(newArticulation)) {
                score()->toggleArticulation(this, newArticulation);
            } else {
                delete newArticulation;
            }
        }
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Chord::reset()
{
    undoChangeProperty(Pid::STEM_DIRECTION, DirectionV::AUTO);
    ChordRest::reset();
}

//---------------------------------------------------------
//   slash
//---------------------------------------------------------

bool Chord::slash()
{
    Note* n = upNote();
    return n->fixed();
}

//---------------------------------------------------------
//   setSlash
//---------------------------------------------------------

void Chord::setSlash(bool flag, bool stemless)
{
    int line = 0;
    NoteHeadGroup head = NoteHeadGroup::HEAD_SLASH;

    if (!flag) {
        // restore to normal
        undoChangeProperty(Pid::NO_STEM, false);
        undoChangeProperty(Pid::SMALL, false);
        undoChangeProperty(Pid::OFFSET, PointF());
        for (Note* n : m_notes) {
            n->undoChangeProperty(Pid::HEAD_GROUP, NoteHeadGroup::HEAD_NORMAL);
            n->undoChangeProperty(Pid::FIXED, false);
            n->undoChangeProperty(Pid::FIXED_LINE, 0);
            n->undoChangeProperty(Pid::PLAY, true);
            n->undoChangeProperty(Pid::VISIBLE, true);
            for (NoteDot* dot : n->dots()) {
                dot->undoChangeProperty(Pid::VISIBLE, true);
            }
            if (staff()->isDrumStaff(tick())) {
                const Drumset* ds = part()->instrument(tick())->drumset();
                int pitch = n->pitch();
                if (ds && ds->isValid(pitch)) {
                    undoChangeProperty(Pid::STEM_DIRECTION, ds->stemDirection(pitch));
                    n->undoChangeProperty(Pid::HEAD_GROUP, ds->noteHead(pitch));
                }
            }
        }
        return;
    }

    // make stemless if asked
    if (stemless) {
        undoChangeProperty(Pid::NO_STEM, true);
        undoChangeProperty(Pid::BEAM_MODE, BeamMode::NONE);
    }

    const StaffType* staffType = this->staffType();

    // voice-dependent attributes - line, size, offset, head
    if (track() % VOICES < 2) {
        // use middle line
        line = staffType->middleLine();
        undoChangeProperty(Pid::STEM_DIRECTION, PropertyValue::fromValue<DirectionV>(DirectionV::DOWN));
    } else {
        // set small
        undoChangeProperty(Pid::SMALL, true);
        // set outside the staff
        double y = 0.0;
        if (track() % 2) {
            line = staffType->bottomLine() + 1;
            y    = 0.5 * spatium();
            undoChangeProperty(Pid::STEM_DIRECTION, PropertyValue::fromValue<DirectionV>(DirectionV::DOWN));
        } else {
            line = -1;
            if (!staffType->isDrumStaff()) {
                y = -0.5 * spatium();
            }
            undoChangeProperty(Pid::STEM_DIRECTION, PropertyValue::fromValue<DirectionV>(DirectionV::UP));
        }
        // for non-drum staves, add an additional offset
        // for drum staves, no offset, but use normal head
        if (!staffType->isDrumStaff()) {
            // undoChangeProperty(Pid::OFFSET, PointF(0.0, y));
            mutldata()->moveY(y);
        } else {
            head = NoteHeadGroup::HEAD_NORMAL;
        }
    }

    size_t ns = m_notes.size();
    for (size_t i = 0; i < ns; ++i) {
        Note* n = m_notes[i];
        n->undoChangeProperty(Pid::HEAD_GROUP, static_cast<int>(head));
        n->undoChangeProperty(Pid::FIXED, true);
        n->undoChangeProperty(Pid::FIXED_LINE, line);
        n->undoChangeProperty(Pid::PLAY, false);
        // hide all but first notehead
        if (i) {
            n->undoChangeProperty(Pid::VISIBLE, false);
            for (NoteDot* dot : n->dots()) {
                dot->undoChangeProperty(Pid::VISIBLE, false);
            }
        }
    }
}

//---------------------------------------------------------
//  updateEndsNoteAnchoredLine
//    sets/resets the chord m_endsNoteAnchoredLine according any note anchored line
//    end into this chord or no.
//---------------------------------------------------------

void Chord::updateEndsNoteAnchoredLine()
{
    m_endsNoteAnchoredLine = false;         // assume no note anchored line ends here
    // scan all chord notes for note anchored lines ending on this chord
    for (Note* note : notes()) {
        for (Spanner* sp : note->spannerBack()) {
            bool isNoteAnchoredTextLine = sp->isNoteLine() && toNoteLine(sp)->enforceMinLength();
            if (sp->type() == ElementType::GLISSANDO || isNoteAnchoredTextLine) {
                m_endsNoteAnchoredLine = true;
                return;
            }
        }
    }
}

//---------------------------------------------------------
//   removeMarkings
//    - this is normally called after cloning a chord to tie a note over the barline
//    - there is no special undo handling; the assumption is that undo will simply remove the cloned chord
//    - two note tremolos are converted into simple notes
//    - single note tremolos are optionally retained
//---------------------------------------------------------

void Chord::removeMarkings(bool keepTremolo)
{
    if (m_tremoloSingleChord && !keepTremolo) {
        remove(m_tremoloSingleChord);
    }

    if (m_tremoloTwoChord && !keepTremolo) {
        remove(m_tremoloTwoChord);
    }

    if (arpeggio()) {
        remove(arpeggio());
    }

    if (m_spanArpeggio) {
        m_spanArpeggio = nullptr;
    }

    muse::DeleteAll(graceNotes());
    graceNotes().clear();
    muse::DeleteAll(articulations());
    articulations().clear();
    for (Note* n : notes()) {
        for (EngravingItem* e : n->el()) {
            n->remove(e);
        }
    }
    ChordRest::removeMarkings(keepTremolo);
}

//---------------------------------------------------------
//   intrinsicMag
//   returns the INTRINSIC mag of the chord (i.e. NOT scaled
//   by staff size)
//---------------------------------------------------------

double Chord::intrinsicMag() const
{
    double m = 1.0;
    if (isSmall()) {
        m *= style().styleD(Sid::smallNoteMag);
    }
    if (m_noteType != NoteType::NORMAL) {
        bool tabGraceSizeException = staffType()->isTabStaff() && isPreBendOrGraceBendStart()
                                     && !style().styleB(Sid::useCueSizeFretForGraceBends);
        if (!tabGraceSizeException) {
            m *= style().styleD(Sid::graceNoteMag);
        }
    }
    return m;
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double Chord::mag() const
{
    const Staff* st = staff();
    return (st ? st->staffMag(this) : 1.0) * intrinsicMag();
}

//---------------------------------------------------------
//   segment
//---------------------------------------------------------

Segment* Chord::segment() const
{
    EngravingItem* e = parentItem();
    for (; e && e->type() != ElementType::SEGMENT; e = e->parentItem()) {
    }
    return toSegment(e);
}

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure* Chord::measure() const
{
    EngravingItem* e = parentItem();
    for (; e && e->type() != ElementType::MEASURE; e = e->parentItem()) {
    }
    return toMeasure(e);
}

//---------------------------------------------------------
//   graceNotesBefore
//---------------------------------------------------------

GraceNotesGroup& Chord::graceNotesBefore(bool filterUnplayable) const
{
    m_graceNotesBefore.clear();
    for (Chord* c : m_graceNotes) {
        assert(c->noteType() != NoteType::NORMAL && c->noteType() != NoteType::INVALID);
        if (c->noteType() & (
                NoteType::ACCIACCATURA
                | NoteType::APPOGGIATURA
                | NoteType::GRACE4
                | NoteType::GRACE16
                | NoteType::GRACE32)) {
            if (filterUnplayable && !c->isChordPlayable()) {
                continue;
            }

            m_graceNotesBefore.push_back(c);
        }
    }
    return m_graceNotesBefore;
}

//---------------------------------------------------------
//   graceNotesAfter
//---------------------------------------------------------

GraceNotesGroup& Chord::graceNotesAfter(bool filterUnplayable) const
{
    m_graceNotesAfter.clear();
    for (int i = static_cast<int>(m_graceNotes.size()) - 1; i >= 0; i--) {
        Chord* c = m_graceNotes[i];
        assert(c->noteType() != NoteType::NORMAL && c->noteType() != NoteType::INVALID);
        if (c->noteType() & (NoteType::GRACE8_AFTER | NoteType::GRACE16_AFTER | NoteType::GRACE32_AFTER)) {
            if (filterUnplayable && !c->isChordPlayable()) {
                continue;
            }

            m_graceNotesAfter.push_back(c);
        }
    }
    return m_graceNotesAfter;
}

Chord* Chord::graceNoteAt(size_t idx) const
{
    if (idx > m_graceNotes.size()) {
        return nullptr;
    }

    return m_graceNotes.at(idx);
}

//---------------------------------------------------------
//   allGraceChordsOfMainChord
//   returns a list containing all grace notes (chords) attached to the main chord and the main chord itself, in order
//---------------------------------------------------------
std::vector<Chord*> Chord::allGraceChordsOfMainChord()
{
    Chord* mainChord = isGrace() ? toChord(explicitParent()) : this;
    std::vector<Chord*> chords = { mainChord };
    const GraceNotesGroup& gnBefore = mainChord->graceNotesBefore();
    const GraceNotesGroup& gnAfter = mainChord->graceNotesAfter();
    chords.insert(chords.begin(), gnBefore.begin(), gnBefore.end());
    chords.insert(chords.end(), gnAfter.begin(), gnAfter.end());
    return chords;
}

//---------------------------------------------------------
//   setShowStemSlashInAdvance
//---------------------------------------------------------

void Chord::setShowStemSlashInAdvance()
{
    if (m_noteType == NoteType::NORMAL) {
        return;
    }
    if (isGraceBefore()) {
        GraceNotesGroup& graceBefore = toChord(explicitParent())->graceNotesBefore();
        Chord* grace = graceBefore.empty() ? nullptr : graceBefore.front();
        if (grace && grace->beamMode() != BeamMode::NONE && grace->beamMode() != BeamMode::BEGIN) {
            grace->requestShowStemSlash(showStemSlash());
        }
    }
    if (isGraceAfter()) {
        GraceNotesGroup& graceAfter = toChord(explicitParent())->graceNotesAfter();
        Chord* grace = graceAfter.empty() ? nullptr : graceAfter.back();
        if (grace && grace->beamMode() != BeamMode::NONE) {
            grace->requestShowStemSlash(showStemSlash());
        }
    }
}

//---------------------------------------------------------
//   requestShowStemSlash
//---------------------------------------------------------

void Chord::requestShowStemSlash(bool show)
{
    if (m_noteType == NoteType::NORMAL) {
        return;
    }
    if (beam()) {
        for (ChordRest* chordRest : beam()->elements()) {
            if (chordRest->isChord()) {
                Chord* chord = toChord(chordRest);
                chord->setShowStemSlash(show);
            }
        }
    } else {
        setShowStemSlash(show);
    }
}

//---------------------------------------------------------
//   sortNotes
//---------------------------------------------------------

static bool noteIsBefore(const Note* n1, const Note* n2)
{
    const int l1 = n1->line();
    const int l2 = n2->line();
    if (l1 != l2) {
        return l1 > l2;
    }

    const int p1 = n1->pitch();
    const int p2 = n2->pitch();
    if (p1 != p2) {
        return p1 < p2;
    }

    if (n1->tieBack()) {
        if (n2->tieBack() && !n2->incomingPartialTie()) {
            const Note* sn1 = n1->tieBack()->startNote();
            const Note* sn2 = n2->tieBack()->startNote();
            if (sn1->chord() == sn2->chord()) {
                return sn1->unisonIndex() < sn2->unisonIndex();
            }
            return sn1->chord()->isBefore(sn2->chord());
        } else {
            return true;       // place tied notes before
        }
    }

    return false;
}

void Chord::sortNotes()
{
    std::sort(notes().begin(), notes().end(), noteIsBefore);
}

//---------------------------------------------------------
//   nextTiedChord
//    Return next chord if all notes in this chord are tied to it.
//    Set backwards=true to return the previous chord instead.
//
//    Note: the next chord might have extra notes that are not tied
//    back to this one. Set sameSize=true to return 0 in this case.
//---------------------------------------------------------

Chord* Chord::nextTiedChord(bool backwards, bool sameSize) const
{
    Segment* nextSeg = backwards ? segment()->prev1(SegmentType::ChordRest) : segment()->next1(SegmentType::ChordRest);
    if (!nextSeg) {
        return 0;
    }
    ChordRest* nextCR = nextSeg->nextChordRest(track(), backwards);
    if (!nextCR || !nextCR->isChord()) {
        return 0;
    }
    Chord* next = toChord(nextCR);
    if (sameSize && notes().size() != next->notes().size()) {
        return 0;     // sizes don't match so some notes can't be tied
    }
    if (tuplet() != next->tuplet()) {
        return 0;     // next chord belongs to a different tuplet
    }
    for (Note* n : m_notes) {
        Tie* tie = backwards ? n->tieBack() : n->tieFor();
        if (!tie) {
            return 0;       // not tied
        }
        Note* nn = backwards ? tie->startNote() : tie->endNote();
        if (!nn || nn->chord() != next) {
            return 0;       // tied to note in wrong voice, or tied over rest
        }
    }
    return next;   // all notes in this chord are tied to notes in next chord
}

//---------------------------------------------------------
//   setNoteType
//---------------------------------------------------------

void Chord::setNoteType(NoteType t)
{
    m_noteType = t;
    setProperty(Pid::SHOW_STEM_SLASH, propertyDefault(Pid::SHOW_STEM_SLASH));
}

//---------------------------------------------------------
//   toGraceAfter
//---------------------------------------------------------

void Chord::toGraceAfter()
{
    switch (noteType()) {
    case NoteType::APPOGGIATURA:  setNoteType(NoteType::GRACE8_AFTER);
        break;
    case NoteType::GRACE16:       setNoteType(NoteType::GRACE16_AFTER);
        break;
    case NoteType::GRACE32:       setNoteType(NoteType::GRACE32_AFTER);
        break;
    default: break;
    }
}

bool Chord::isPreBendOrGraceBendStart() const
{
    if (!isGrace()) {
        return false;
    }

    for (const Note* note : m_notes) {
        GuitarBend* gb = note->bendFor();
        if (gb && (gb->type() == GuitarBendType::PRE_BEND || gb->type() == GuitarBendType::GRACE_NOTE_BEND)) {
            return true;
        }
    }

    return false;
}

bool Chord::isGraceBendEnd() const
{
    if (isGrace() || m_graceNotes.empty()) {
        return false;
    }

    for (const Note* note : m_notes) {
        GuitarBend* bendBack = note->bendBack();
        if (bendBack && bendBack->type() == GuitarBendType::GRACE_NOTE_BEND) {
            return true;
        }
    }

    return false;
}

bool Chord::preOrGraceBendSpacingExceptionInTab() const
{
    if (!staffType()->isTabStaff() || !isGrace()) {
        return false;
    }

    std::vector<GuitarBend*> bends;
    for (Note* note : m_notes) {
        GuitarBend* bendFor = note->bendFor();
        if (bendFor) {
            GuitarBendType bendType = bendFor->type();
            if (bendType == GuitarBendType::PRE_BEND || bendType == GuitarBendType::GRACE_NOTE_BEND) {
                bends.push_back(bendFor);
                break;
            }
        }
    }

    if (bends.empty() || bends.size() < m_notes.size()) {
        return false;
    }

    Chord* endChord = bends.front()->endNote()->chord();
    if (!endChord) {
        return false;
    }

    GuitarBendType type = bends.front()->type();
    for (GuitarBend* gb : bends) {
        if (gb->type() != type || (gb->endNote() && gb->endNote()->chord() != endChord)) {
            return false;
        }
    }

    if (type == GuitarBendType::PRE_BEND) {
        return true;
    }

    for (Note* note : endChord->notes()) {
        GuitarBend* bendBack = note->bendBack();
        if (bendBack) {
            Note* startNote = bendBack->startNote();
            if (!startNote || startNote->chord() != this) {
                return false;
            }
        }
    }

    return bends.size() < endChord->notes().size();
}

void Chord::setIsTrillCueNote(bool v)
{
    m_isTrillCueNote = v;
}

//---------------------------------------------------------
//   tremoloChordType
//---------------------------------------------------------

TremoloChordType Chord::tremoloChordType() const
{
    if (m_tremoloTwoChord) {
        if (m_tremoloTwoChord->chord1() == this) {
            return TremoloChordType::TremoloFirstChord;
        } else if (m_tremoloTwoChord->chord2() == this) {
            return TremoloChordType::TremoloSecondChord;
        } else {
            ASSERT_X(String(u"Chord::tremoloChordType(): inconsistency"));
        }
    } else if (m_tremoloSingleChord) {
        return TremoloChordType::TremoloSingle;
    }

    return TremoloChordType::TremoloNone;
}

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

EngravingItem* Chord::nextElement()
{
    EngravingItem* e = score()->selection().element();
    if (!e && !score()->selection().elements().empty()) {
        e = score()->selection().elements().front();
    }

    switch (e->type()) {
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
    case ElementType::FINGERING:
    case ElementType::TEXT:
    case ElementType::BEND: {
        Note* n = toNote(e->explicitParent());
        if (n == m_notes.front()) {
            if (m_arpeggio) {
                return m_arpeggio;
            } else if (m_tremoloTwoChord) {
                return m_tremoloTwoChord;
            } else if (m_tremoloSingleChord) {
                return m_tremoloSingleChord;
            }
            break;
        }
        for (auto& i : m_notes) {
            if (i == n) {
                return *(&i - 1);
            }
        }
        break;
    }

    case ElementType::GUITAR_BEND_SEGMENT:
    case ElementType::GLISSANDO_SEGMENT:
    case ElementType::NOTELINE_SEGMENT:
    case ElementType::LAISSEZ_VIB_SEGMENT:
    case ElementType::PARTIAL_TIE_SEGMENT:
    case ElementType::TIE_SEGMENT: {
        SpannerSegment* s = toSpannerSegment(e);
        Spanner* sp = s->spanner();
        EngravingItem* elSt = sp->startElement();
        assert(elSt->type() == ElementType::NOTE);
        Note* n = toNote(elSt);
        assert(n != NULL);
        if (n == m_notes.front()) {
            if (m_arpeggio) {
                return m_arpeggio;
            } else if (m_tremoloTwoChord) {
                return m_tremoloTwoChord;
            } else if (m_tremoloSingleChord) {
                return m_tremoloSingleChord;
            }
            break;
        }
        for (auto& i : m_notes) {
            if (i == n) {
                return *(&i - 1);
            }
        }
        break;
    }
    case ElementType::ARPEGGIO:
        if (m_tremoloTwoChord) {
            return m_tremoloTwoChord;
        } else if (m_tremoloSingleChord) {
            return m_tremoloSingleChord;
        }
        break;

    case ElementType::ACCIDENTAL:
        e = e->parentItem();
    // fall through

    case ElementType::NOTE: {
        if (e == m_notes.front()) {
            if (m_arpeggio) {
                return m_arpeggio;
            } else if (m_tremoloTwoChord) {
                return m_tremoloTwoChord;
            } else if (m_tremoloSingleChord) {
                return m_tremoloSingleChord;
            }
            break;
        }
        for (auto& i : m_notes) {
            if (i == e) {
                return *(&i - 1);
            }
        }
    }
    break;

    case ElementType::CHORD:
        return m_notes.back();

    default:
        break;
    }

    return ChordRest::nextElement();
}

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

EngravingItem* Chord::prevElement()
{
    EngravingItem* e = score()->selection().element();
    if (!e && !score()->selection().elements().empty()) {
        e = score()->selection().elements().back();
    }
    switch (e->type()) {
    case ElementType::NOTE: {
        if (isGrace()) {
            ChordRest* prev = prevChordRest(this);
            if (prev) {
                if (prev->isChord()) {
                    return toChord(prev)->notes().back();
                }
                return prev;
            }
        }

        GraceNotesGroup& graceNotesBefore = this->graceNotesBefore();
        if (!graceNotesBefore.empty()) {
            if (Chord* graceNotesBeforeLastChord = graceNotesBefore.back()) {
                if (e == graceNotesBeforeLastChord->notes().back()) {
                    break;
                }

                Note* prevNote = graceNotesBeforeLastChord->notes().back();
                if (prevNote->isPreBendStart() || prevNote->isGraceBendStart()) {
                    return prevNote->bendFor()->frontSegment();
                }

                ChordRest* prev = prevChordRest(this);
                if (prev) {
                    if (prev->isChord()) {
                        return toChord(prev)->notes().back();
                    }
                    return prev;
                }
            }
        }

        if (e == m_notes.back()) {
            break;
        }
        Note* prevNote = nullptr;
        for (auto& i : m_notes) {
            if (i == e) {
                prevNote = *(&i + 1);
            }
        }
        EngravingItem* next = prevNote->lastElementBeforeSegment();
        return next;
    }

    case ElementType::CHORD:
        return m_notes.front();

    case ElementType::TREMOLO_TWOCHORD:
    case ElementType::TREMOLO_SINGLECHORD:
        if (m_arpeggio) {
            return m_arpeggio;
        }
    // fall through
    case ElementType::ARPEGGIO: {
        Note* n = m_notes.front();
        EngravingItem* elN = n->lastElementBeforeSegment();
        assert(elN != NULL);
        return elN;
    }

    default:
        break;
    }
    return ChordRest::prevElement();
}

//---------------------------------------------------------
//   lastElementBeforeSegment
//---------------------------------------------------------

EngravingItem* Chord::lastElementBeforeSegment()
{
    if (m_tremoloSingleChord) {
        return m_tremoloSingleChord;
    } else if (m_tremoloTwoChord) {
        return m_tremoloTwoChord;
    } else if (m_arpeggio) {
        return m_arpeggio;
    } else {
        Note* n = m_notes.front();
        EngravingItem* elN = n->lastElementBeforeSegment();
        assert(elN != NULL);
        return elN;
    }
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* Chord::nextSegmentElement()
{
    for (track_idx_t v = track() + 1; staffIdx() == v / VOICES; ++v) {
        EngravingItem* e = segment()->element(v);
        if (e) {
            if (e->type() == ElementType::CHORD) {
                return toChord(e)->notes().back();
            }

            return e;
        }
    }

    return ChordRest::nextSegmentElement();
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* Chord::prevSegmentElement()
{
    EngravingItem* el = score()->selection().element();
    if (!el && !score()->selection().elements().empty()) {
        el = score()->selection().elements().front();
    }
    EngravingItem* e = segment()->lastInPrevSegments(el->staffIdx());
    if (e) {
        if (e->isChord()) {
            return toChord(e)->notes().front();
        }
        return e;
    }

    return ChordRest::prevSegmentElement();
}

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

String Chord::accessibleExtraInfo() const
{
    String rez;

    for (const Chord* c : graceNotes()) {
        if (!score()->selectionFilter().canSelect(c)) {
            continue;
        }
        for (const Note* n : c->notes()) {
            rez = String(u"%1 %2").arg(rez, n->screenReaderInfo());
        }
    }

    for (Articulation* a : articulations()) {
        if (!score()->selectionFilter().canSelect(a)) {
            continue;
        }
        rez = String(u"%1 %2").arg(rez, a->screenReaderInfo());
    }

    if (arpeggio() && score()->selectionFilter().canSelect(arpeggio())) {
        rez = String(u"%1 %2").arg(rez, arpeggio()->screenReaderInfo());
    }

    if (tremoloTwoChord() && score()->selectionFilter().canSelect(tremoloTwoChord())) {
        rez = String(u"%1 %2").arg(rez, tremoloTwoChord()->screenReaderInfo());
    }

    if (tremoloSingleChord() && score()->selectionFilter().canSelect(tremoloSingleChord())) {
        rez = String(u"%1 %2").arg(rez, tremoloSingleChord()->screenReaderInfo());
    }

    for (EngravingItem* e : el()) {
        if (!score()->selectionFilter().canSelect(e)) {
            continue;
        }
        rez = String(u"%1 %2").arg(rez, e->screenReaderInfo());
    }

    return String(u"%1 %2").arg(rez, ChordRest::accessibleExtraInfo());
}

void Chord::undoChangeProperty(Pid id, const PropertyValue& newValue)
{
    undoChangeProperty(id, newValue, propertyFlags(id));
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void Chord::undoChangeProperty(Pid id, const PropertyValue& newValue, PropertyFlags ps)
{
    if (id == Pid::VISIBLE) {
        processSiblings([=](EngravingItem* element) {
            element->undoChangeProperty(id, newValue, ps);
        }, false);
    }

    ChordRest::undoChangeProperty(id, newValue, ps);
}

std::set<SymId> Chord::articulationSymbolIds() const
{
    std::set<SymId> result;
    for (const Articulation* artic: m_articulations) {
        result.insert(artic->symId());
    }

    return result;
}

//---------------------------------------------------------
//   getNoteEventLists
//    Get contents of all NoteEventLists for all notes in
//    the chord.
//---------------------------------------------------------

std::vector<NoteEventList> Chord::getNoteEventLists()
{
    std::vector<NoteEventList> ell;
    if (notes().empty()) {
        return ell;
    }
    for (size_t i = 0; i < notes().size(); ++i) {
        ell.push_back(NoteEventList(notes()[i]->playEvents()));
    }
    return ell;
}

//---------------------------------------------------------
//   setNoteEventLists
//    Set contents of all NoteEventLists for all notes in
//    the chord.
//---------------------------------------------------------

void Chord::setNoteEventLists(std::vector<NoteEventList>& ell)
{
    if (notes().empty()) {
        return;
    }
    IF_ASSERT_FAILED(ell.size() == notes().size()) {
        return;
    }
    for (size_t i = 0; i < ell.size(); i++) {
        notes()[i]->setPlayEvents(ell[i]);
    }
}

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------
void Chord::styleChanged()
{
    auto updateElementsStyle = [](void*, EngravingItem* e) {
        e->styleChanged();
    };
    scanElements(0, updateElementsStyle);
}

void Chord::computeKerningExceptions()
{
    m_allowKerningAbove = true;
    m_allowKerningBelow = true;
    for (Articulation* art : m_articulations) {
        if (art->up()) {
            m_allowKerningAbove = false;
        } else {
            m_allowKerningBelow = false;
        }
    }
    if ((m_startEndSlurs.startUp || m_startEndSlurs.endUp) && !ldata()->up) {
        m_allowKerningAbove = false;
    }
    if ((m_startEndSlurs.startDown || m_startEndSlurs.endDown) && ldata()->up) {
        m_allowKerningBelow = false;
    }
}

Ornament* Chord::findOrnament(bool forPlayback) const
{
    for (Articulation* art : m_articulations) {
        if (art->isOrnament()) {
            return toOrnament(art);
        }
    }
    for (Spanner* spanner : m_startingSpanners) {
        if (spanner->isTrill()) {
            return toTrill(spanner)->ornament();
        }
    }
    if (forPlayback) {
        for (Spanner* spanner : m_endingSpanners) {
            if (spanner->isTrill()) {
                return toTrill(spanner)->ornament();
            }
        }
    }
    return nullptr;
}

//---------------------------------
// firstGraceOrNote
//---------------------------------
Note* Chord::firstGraceOrNote()
{
    GraceNotesGroup& graceNotesBefore = this->graceNotesBefore();
    if (!graceNotesBefore.empty()) {
        if (Chord* graceNotesBeforeFirstChord = graceNotesBefore.front()) {
            return graceNotesBeforeFirstChord->notes().front();
        }
    }

    return this->notes().back();
}

//---------------------------------
// GRACE NOTES
//---------------------------------

GraceNotesGroup::GraceNotesGroup(Chord* c)
    : EngravingItem(ElementType::GRACE_NOTES_GROUP, c), _parent(c) {}

void GraceNotesGroup::setPos(double x, double y)
{
    EngravingItem::setPos(x, y);
    for (unsigned i = 0; i < this->size(); ++i) {
        Chord* chord = this->at(i);
        chord->mutldata()->move(PointF(x, y));
    }
}

void GraceNotesGroup::addToShape()
{
    for (Chord* grace : *this) {
        const PointF yOffset = grace->staffOffset();
        staff_idx_t staffIdx = grace->staffIdx();
        staff_idx_t vStaffIdx = grace->vStaffIdx();
        Shape& s = _appendedSegment->staffShape(staffIdx);
        s.add(grace->shape(LD_ACCESS::PASS).translate(grace->pos() + yOffset));
        if (vStaffIdx != staffIdx) {
            // Cross-staff grace notes add their shape to both the origin and the destination staff
            Shape& s2 = _appendedSegment->staffShape(vStaffIdx);
            s2.add(grace->shape().translate(grace->pos() + yOffset));
        }
    }
}
}
