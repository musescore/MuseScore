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

#include "chord.h"

#include <cmath>
#include <vector>
#include <array>

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
#include "noteevent.h"
#include "ornament.h"
#include "part.h"
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
#include "tremolo.h"
#include "trill.h"
#include "tuplet.h"
#include "undo.h"
#include "compat/midi/compatmidirender.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#endif

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

namespace mu::engraving {
//---------------------------------------------------------
//   LedgerLineData
//---------------------------------------------------------

struct LedgerLineData {
    int line;
    double minX, maxX;
    bool visible;
    bool accidental;
};

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
    m_ledgerLines      = 0;
    m_stem             = 0;
    m_hook             = 0;
    m_stemDirection    = DirectionV::AUTO;
    m_arpeggio         = 0;
    m_spanArpeggio     = 0;
    m_tremolo          = 0;
    m_endsGlissando    = false;
    m_noteType         = NoteType::NORMAL;
    m_stemSlash        = 0;
    m_noStem           = false;
    m_playEventType    = PlayEventType::Auto;
    m_spaceLw          = 0.;
    m_spaceRw          = 0.;
    m_crossMeasure    = CrossMeasure::UNKNOWN;
    m_graceIndex   = 0;
}

Chord::Chord(const Chord& c, bool link)
    : ChordRest(c, link)
{
    if (link) {
        score()->undo(new Link(this, const_cast<Chord*>(&c)));
    }
    m_ledgerLines = 0;

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
    m_endsGlissando = false;
    m_arpeggio      = 0;
    m_stemSlash     = 0;
    m_tremolo       = 0;

    m_spanArpeggio   = c.m_spanArpeggio;
    m_graceIndex     = c.m_graceIndex;
    m_noStem         = c.m_noStem;
    m_playEventType  = c.m_playEventType;
    m_stemDirection  = c.m_stemDirection;
    m_noteType       = c.m_noteType;
    m_crossMeasure  = CrossMeasure::UNKNOWN;

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
    if (c.m_tremolo) {
        Tremolo* t = Factory::copyTremolo(*(c.m_tremolo));
        if (link) {
            score()->undo(new Link(t, const_cast<Tremolo*>(c.m_tremolo)));
        }
        if (c.m_tremolo->twoNotes()) {
            if (c.m_tremolo->chord1() == &c) {
                t->setChords(this, nullptr);
            } else {
                t->setChords(nullptr, this);
            }
        }
        add(t);
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
    if (m_tremolo && !m_tremolo->twoNotes()) {
        m_tremolo->undoUnlink();
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
    DeleteAll(m_articulations);

    if (m_tremolo) {
        if (m_tremolo->chord1() == this) {
            m_tremolo->setChord1(nullptr);
        } else if (m_tremolo->chord2() == this) {
            m_tremolo->setChord2(nullptr);
        }

        m_tremolo = nullptr;
    }

    delete m_arpeggio;
    delete m_stemSlash;
    delete m_stem;
    delete m_hook;
    for (LedgerLine* ll = m_ledgerLines; ll;) {
        LedgerLine* llNext = ll->next();
        delete ll;
        ll = llNext;
    }
    DeleteAll(m_graceNotes);
    DeleteAll(m_notes);
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
AccessibleItemPtr Chord::createAccessible()
{
    return std::make_shared<AccessibleItem>(this, AccessibleItem::Group);
}

#endif

bool Chord::containsEqualArticulations(const Chord* other) const
{
    if (!other) {
        return false;
    }

    if (m_articulations.size() != other->m_articulations.size()) {
        return false;
    }

    for (size_t i = 0; i < m_articulations.size(); ++i) {
        const Articulation* first = m_articulations.at(i);
        const Articulation* second = other->m_articulations.at(i);

        if (!first || !second) {
            return false;
        }

        if (first->symId() != second->symId()) {
            return false;
        }
    }

    return true;
}

bool Chord::containsEqualArpeggio(const Chord* other) const
{
    if (m_arpeggio && other->m_arpeggio) {
        if (m_arpeggio->arpeggioType() != other->m_arpeggio->arpeggioType()) {
            return false;
        }
    }

    return !m_arpeggio && !other->m_arpeggio;
}

bool Chord::containsEqualTremolo(const Chord* other) const
{
    if (m_tremolo && other->m_tremolo) {
        if (m_tremolo->tremoloType() != other->m_tremolo->tremoloType()) {
            return false;
        }
    }

    return !m_tremolo && !other->m_tremolo;
}

//---------------------------------------------------------
//   noteHeadWidth
//---------------------------------------------------------

double Chord::noteHeadWidth() const
{
    return score()->noteHeadWidth() * mag();
}

//! Returns Chord coordinates
double Chord::stemPosX() const
{
    const StaffType* staffType = this->staffType();
    if (staffType && staffType->isTabStaff()) {
        return staffType->chordStemPosX(this) * spatium();
    }
    return ldata()->up ? noteHeadWidth() : 0.0;
}

//! Returns page coordinates
PointF Chord::stemPos() const
{
    const Staff* staff = this->staff();
    const StaffType* staffType = staff ? staff->staffTypeForElement(this) : nullptr;
    if (staffType && staffType->isTabStaff()) {
        return pagePos() + staffType->chordStemPos(this) * spatium();
    }

    if (ldata()->up) {
        const Note* downNote = this->downNote();
        double nhw = m_notes.size() == 1 ? downNote->bboxRightPos() : noteHeadWidth();
        return pagePos() + PointF(nhw, downNote->pos().y());
    }

    return pagePos() + PointF(0.0, upNote()->pos().y());
}

//! Returns stem position of note on beam side
//! Returns page coordinates
PointF Chord::stemPosBeam() const
{
    const Staff* stf = this->staff();
    const StaffType* st = stf ? stf->staffTypeForElement(this) : nullptr;

    if (st && st->isTabStaff()) {
        return pagePos() + st->chordStemPosBeam(this) * spatium();
    }

    if (ldata()->up) {
        double nhw = noteHeadWidth();
        return pagePos() + PointF(nhw, upNote()->pos().y());
    }

    return pagePos() + PointF(0, downNote()->pos().y());
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

//---------------------------------------------------------
//   setTremolo
//---------------------------------------------------------

void Chord::setTremolo(Tremolo* tr, bool applyLogic)
{
    if (m_tremolo && tr && tr == m_tremolo) {
        return;
    }

    if (m_tremolo) {
        if (m_tremolo->twoNotes()) {
            TDuration d;
            const Fraction f = ticks();
            if (f.numerator() > 0) {
                d = TDuration(f);
            } else {
                d = m_tremolo->durationType();
                const int dots = d.dots();
                d = d.shift(1);
                d.setDots(dots);
            }

            setDurationType(d);
            Chord* other = m_tremolo->chord1() == this ? m_tremolo->chord2() : m_tremolo->chord1();
            m_tremolo = nullptr;
            if (other) {
                other->setTremolo(nullptr);
            }
        } else {
            m_tremolo = nullptr;
        }
    }

    if (tr) {
        if (applyLogic && tr->twoNotes()) {
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
            m_tremolo = tr;
            if (other) {
                other->setTremolo(tr);
            }
        } else {
            m_tremolo = tr;
        }
    } else {
        m_tremolo = nullptr;
    }
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
    case ElementType::TREMOLO:
        setTremolo(toTremolo(e));
        break;
    case ElementType::GLISSANDO:
        m_endsGlissando = true;
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
    case ElementType::TREMOLO:
        setTremolo(nullptr);
        break;
    case ElementType::GLISSANDO:
        m_endsGlissando = false;
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
        if (!mu::remove(m_articulations, a)) {
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
//   addLedgerLines
//---------------------------------------------------------

void Chord::addLedgerLines()
{
    // initialize for palette
    track_idx_t track = 0;                     // the track lines belong to
    // the line pos corresponding to the bottom line of the staff
    int lineBelow      = 8;                     // assuming 5-lined "staff"
    double lineDistance = 1;
    bool staffVisible  = true;
    int stepOffset = 0;                         // for staff type changes with a step offset

    if (segment()) {   //not palette
        Fraction tick = segment()->tick();
        staff_idx_t idx = staffIdx() + staffMove();
        track         = staff2track(idx);
        Staff* st     = score()->staff(idx);
        lineBelow     = (st->lines(tick) - 1) * 2;
        lineDistance  = st->lineDistance(tick);
        staffVisible  = !staff()->isLinesInvisible(tick);
        stepOffset = st->staffType(tick)->stepOffset();
    }

    // need ledger lines?
    if (downLine() + stepOffset <= lineBelow + 1 && upLine() + stepOffset >= -1) {
        return;
    }

    // the extra length of a ledger line to be added on each side of the notehead
    double extraLen = style().styleMM(Sid::ledgerLineLength);
    double hw;
    double minX, maxX;                           // note extrema in raster units
    int minLine, maxLine;
    bool visible = false;
    double x;

    // scan chord notes, collecting visibility and x and y extrema
    // NOTE: notes are sorted from bottom to top (line no. decreasing)
    // notes are scanned twice from outside (bottom or top) toward the staff
    // each pass stops at the first note without ledger lines
    size_t n = m_notes.size();
    for (size_t j = 0; j < 2; j++) {               // notes are scanned twice...
        int from, delta;
        std::vector<LedgerLineData> vecLines;
        hw = 0.0;
        minX = std::numeric_limits<double>::max();
        maxX = std::numeric_limits<double>::min();
        minLine = 0;
        maxLine = lineBelow;
        if (j == 0) {                           // ...once from lowest up...
            from  = 0;
            delta = +1;
        } else {
            from = int(n) - 1;                       // ...once from highest down
            delta = -1;
        }
        for (int i = from; i < int(n) && i >= 0; i += delta) {
            Note* note = m_notes.at(i);
            int l = note->line() + stepOffset;

            // if 1st pass and note not below staff or 2nd pass and note not above staff
            if ((!j && l <= lineBelow + 1) || (j && l >= -1)) {
                break;                          // stop this pass
            }
            // round line number to even number toward 0
            if (l < 0) {
                l = (l + 1) & ~1;
            } else {
                l = l & ~1;
            }

            if (note->visible()) {              // if one note is visible,
                visible = true;                 // all lines between it and the staff are visible
            }
            hw = std::max(hw, note->headWidth());

            //
            // Experimental:
            //  shorten ledger line to avoid collisions with accidentals
            //
            // bool accid = (note->accidental() && note->line() >= (l-1) && note->line() <= (l+1) );
            //
            // TODO : do something with this accid flag in the following code!
            //

            // check if note horiz. pos. is outside current range
            // if more length on the right, increase range
//                  note->layout();

            //ledger lines need the leftmost point of the notehead with a respect of bbox
            x = note->pos().x() + note->bboxXShift();
            if (x - extraLen * note->mag() < minX) {
                minX  = x - extraLen * note->mag();
                // increase width of all lines between this one and the staff
                for (auto& d : vecLines) {
                    if (!d.accidental && ((l < 0 && d.line >= l) || (l > 0 && d.line <= l))) {
                        d.minX = minX;
                    }
                }
            }
            // same for left side
            if (x + hw + extraLen * note->mag() > maxX) {
                maxX = x + hw + extraLen * note->mag();
                for (auto& d : vecLines) {
                    if ((l < 0 && d.line >= l) || (l > 0 && d.line <= l)) {
                        d.maxX = maxX;
                    }
                }
            }

            LedgerLineData lld;
            // check if note vert. pos. is outside current range
            // and, if so, add data for new line(s)
            if (l < minLine) {
                for (int i1 = l; i1 < minLine; i1 += 2) {
                    lld.line = i1;
                    lld.minX = minX;
                    lld.maxX = maxX;
                    lld.visible = visible;
                    lld.accidental = false;
                    vecLines.push_back(lld);
                }
                minLine = l;
            }
            if (l > maxLine) {
                for (int i1 = maxLine + 2; i1 <= l; i1 += 2) {
                    lld.line = i1;
                    lld.minX = minX;
                    lld.maxX = maxX;
                    lld.visible = visible;
                    lld.accidental = false;
                    vecLines.push_back(lld);
                }
                maxLine = l;
            }
        }
        if (minLine < 0 || maxLine > lineBelow) {
            double _spatium = spatium();
            double stepDistance = lineDistance * 0.5;
            for (auto lld : vecLines) {
                LedgerLine* h = new LedgerLine(score()->dummy());
                h->setParent(this);
                h->setTrack(track);
                h->setVisible(lld.visible && staffVisible);
                h->setLen(lld.maxX - lld.minX);
                h->setPos(lld.minX, lld.line * _spatium * stepDistance);
                h->setNext(m_ledgerLines);
                m_ledgerLines = h;
            }
        }
    }

    for (LedgerLine* ll = m_ledgerLines; ll; ll = ll->next()) {
        renderer()->layoutItem(ll);
    }
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

//---------------------------------------------------------
//   centerX
//    return x position for attributes
//---------------------------------------------------------

double Chord::centerX() const
{
    // TAB 'notes' are always centered on the stem
    const Staff* st = staff();
    const StaffType* stt = st->staffTypeForElement(this);
    if (stt->isTabStaff()) {
        return stt->chordStemPosX(this) * spatium();
    }

    const Note* note = up() ? downNote() : upNote();
    double x = note->pos().x() + note->noteheadCenterX();
    return x;
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
    if (m_tremolo) {
        func(m_tremolo);
    }
    if (includeTemporarySiblings) {
        for (LedgerLine* ll = m_ledgerLines; ll; ll = ll->next()) {
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

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Chord::setScore(Score* s)
{
    ChordRest::setScore(s);
    processSiblings([s](EngravingItem* e) { e->setScore(s); }, true);
}

// all values are in quarter spaces
int Chord::calcMinStemLength()
{
    int minStemLength = 0; // in quarter spaces
    double _spatium = spatium();

    if (m_tremolo && !m_tremolo->twoNotes()) {
        // buzz roll's height is actually half of the visual height,
        // so we need to multiply it by 2 to get the actual height
        int buzzRollMultiplier = m_tremolo->isBuzzRoll() ? 2 : 1;
        minStemLength += ceil(m_tremolo->minHeight() / intrinsicMag() * 4.0 * buzzRollMultiplier);
        int outSidePadding = style().styleMM(Sid::tremoloOutSidePadding).val() / _spatium * 4.0;
        int noteSidePadding = style().styleMM(Sid::tremoloNoteSidePadding).val() / _spatium * 4.0;

        int outsideStaffOffset = 0;
        if (!staff()->isTabStaff(tick())) {
            Note* lineNote = ldata()->up ? upNote() : downNote();
            if (lineNote->line() == INVALID_LINE) {
                lineNote->updateLine();
            }

            int line = lineNote->line();
            line *= 2; // convert to quarter spaces

            if (!ldata()->up && line < -2) {
                outsideStaffOffset = -line;
            } else if (ldata()->up && line > staff()->lines(tick()) * 4) {
                outsideStaffOffset = line - (staff()->lines(tick()) * 4) + 4;
            }
        }
        minStemLength += (outSidePadding + std::max(noteSidePadding, outsideStaffOffset));

        if (m_hook) {
            bool straightFlags = style().styleB(Sid::useStraightNoteFlags);
            double smuflAnchor = m_hook->smuflAnchor().y() * (ldata()->up ? 1 : -1);
            int hookOffset = floor((m_hook->height() / intrinsicMag() + smuflAnchor) / _spatium * 4) - (straightFlags ? 0 : 2);
            // some fonts have hooks that extend very far down (making the height of the hook very large)
            // so we constrain to a reasonable maximum for hook length
            hookOffset = std::min(hookOffset, 11);
            // TODO: when the SMuFL metadata includes a cutout for flags, replace this with that metadata
            // https://github.com/w3c/smufl/issues/203
            int cutout = up() ? 5 : 7;
            if (straightFlags) {
                // don't need cutout for straight flags (they are similar to beams)
                cutout = 0;
            } else if (beams() >= 2) {
                // beams greater than two extend outwards and thus don't factor into the cutout
                cutout -= 2;
            }

            minStemLength += hookOffset - cutout;

            // hooks with trems inside them no longer ceil (snap) to nearest 0.5sp.
            // if we want to add that back in, here is the place to do it:
            // minStemLength = ceil(minStemLength / 2.0) * 2;
        }
    }
    if (m_beam || (m_tremolo && m_tremolo->twoNotes())) {
        int beamCount = (m_beam ? beams() : 0) + ((m_tremolo && m_tremolo->twoNotes()) ? m_tremolo->lines() : 0);
        static const int minInnerStemLengths[4] = { 10, 9, 8, 7 };
        int innerStemLength = minInnerStemLengths[std::min(beamCount, 3)];
        int beamsHeight = beamCount * (style().styleB(Sid::useWideBeams) ? 4 : 3) - 1;
        int newMinStemLength = std::max(minStemLength, innerStemLength);
        newMinStemLength += beamsHeight;
        // for 4+ beams, there are a few situations where we need to lengthen the stem by 1
        int noteLine = line();
        int staffLines = staff()->lines(tick());
        bool noteInStaff = (ldata()->up && noteLine > 0) || (!ldata()->up && noteLine < (staffLines - 1) * 2);
        if (beamCount >= 4 && noteInStaff) {
            newMinStemLength++;
        }
        minStemLength = std::max(minStemLength, newMinStemLength);
    }
    return minStemLength;
}

// all values are in quarter spaces
int Chord::stemLengthBeamAddition() const
{
    if (m_hook) {
        return 0;
    }
    int beamCount = (m_beam ? beams() : 0) + ((m_tremolo && m_tremolo->twoNotes()) ? m_tremolo->lines() : 0);
    switch (beamCount) {
    case 0:
    case 1:
    case 2:
        return 0;
    case 3:
        return 2;
    default:
        return (beamCount - 3) * (style().styleB(Sid::useWideBeams) ? 4 : 3);
    }
}

int Chord::minStaffOverlap(bool up, int staffLines, int beamCount, bool hasHook, double beamSpacing, bool useWideBeams, bool isFullSize)
{
    int beamOverlap = 8;
    if (isFullSize) {
        if (beamCount == 3 && !hasHook) {
            beamOverlap = 12;
        } else if (beamCount >= 4 && !hasHook) {
            beamOverlap = (beamCount - 4) * beamSpacing + (useWideBeams ? 16 : 14);
        }
    }

    int staffOverlap = std::min(beamOverlap, (staffLines - 1) * 4);
    if (!up) {
        return staffOverlap;
    }
    return (staffLines - 1) * 4 - staffOverlap;
}

// all values are in quarter spaces
int Chord::maxReduction(int extensionOutsideStaff) const
{
    if (!style().styleB(Sid::shortenStem)) {
        return 0;
    }
    // [extensionOutsideStaff][beamCount]
    static const int maxReductions[4][5] = {
        //1sp 1.5sp 2sp 2.5sp >=3sp -- extensionOutsideStaff
        { 1, 2, 3, 4, 4 }, // 0 beams
        { 0, 1, 2, 3, 3 }, // 1 beam
        { 0, 1, 1, 1, 1 }, // 2 beams
        { 0, 0, 0, 1, 1 }, // 3 beams
    };
    int beamCount = 0;
    if (!m_hook) {
        beamCount = m_tremolo ? m_tremolo->lines() + (m_beam ? beams() : 0) : beams();
    }
    bool hasTradHook = m_hook && !style().styleB(Sid::useStraightNoteFlags);
    if (m_hook && !hasTradHook) {
        beamCount = std::min(beamCount, 2); // the straight glyphs extend outwards after 2 beams
    }
    if (beamCount >= 4) {
        return 0;
    }
    int extensionHalfSpaces = floor(extensionOutsideStaff / 2.0);
    extensionHalfSpaces = std::min(extensionHalfSpaces, 4);
    int reduction = maxReductions[beamCount][extensionHalfSpaces];
    if (intrinsicMag() < 1) {
        // there is an exception for grace-sized stems with hooks.
        // reducing by the full amount puts the hooks too low. Limit reduction to 0.5sp
        if (hasTradHook) {
            reduction = std::min(reduction, 1);
        }
    } else {
        // there are a few exceptions for normal-sized (non-grace) beams
        if (beamCount == 1 && extensionHalfSpaces < 2) {
            // 1) if the extension is less than 1sp above or below the staff, they've been adjusted
            //    already to play nicely with staff lines. Reduce by 1sp.
            reduction = 2;
        } else if (beamCount == 3 && extensionHalfSpaces == 3) {
            // 2) if there are three beams and it extends 1.5sp above or below the staff, we need to
            //    *extend* the stem rather than reduce it.
            reduction = 0;
        }
        if (hasTradHook) {
            reduction = std::min(reduction, 1);
        } else if (m_hook && beams() > 2) {
            reduction += 1;
        }
    }
    return reduction;
}

// all values are in quarter spaces
int Chord::stemOpticalAdjustment(int stemEndPosition) const
{
    if (m_hook && !m_beam) {
        return 0;
    }
    int beamCount = (m_tremolo ? m_tremolo->lines() : 0) + (m_beam ? beams() : 0);
    if (beamCount == 0 || beamCount > 2) {
        return 0;
    }
    bool isOnEvenLine = fmod(stemEndPosition + 4, 4) == 2;
    if (isOnEvenLine) {
        return 1;
    }
    return 0;
}

int Chord::calc4BeamsException(int stemLength) const
{
    int difference = 0;
    int staffLines = (staff()->lines(tick()) - 1) * 2;
    if (up() && upNote()->line() > staffLines) {
        difference = upNote()->line() - staffLines;
    } else if (!up() && downNote()->line() < 0) {
        difference = std::abs(downNote()->line());
    }
    switch (difference) {
    case 2:
        return std::max(stemLength, 21);
    case 3:
    case 4:
        return std::max(stemLength, 23);
    default:
        return stemLength;
    }
}

//-----------------------------------------------------------------------------
//   defaultStemLength
///   Get the default stem length for this chord
///   all internal calculation is done in quarter spaces
///   using integers to eliminate all possibilities for rounding errors
//-----------------------------------------------------------------------------

double Chord::calcDefaultStemLength()
{
    // returns default length even if the chord doesn't have a stem

    double _spatium = spatium();
    double lineDistance = (staff() ? staff()->lineDistance(tick()) : 1.0);

    const Staff* staffItem = staff();
    const StaffType* staffType = staffItem ? staffItem->staffTypeForElement(this) : nullptr;
    const StaffType* tab = (staffType && staffType->isTabStaff()) ? staffType : nullptr;

    bool isBesideTabStaff = tab && !tab->stemless() && !tab->stemThrough();
    if (isBesideTabStaff) {
        return tab->chordStemLength(this) * _spatium;
    }

    int defaultStemLength = style().styleD(Sid::stemLength) * 4;
    defaultStemLength += stemLengthBeamAddition();
    if (tab) {
        defaultStemLength *= 1.5;
    }
    // extraHeight represents the extra vertical distance between notehead and stem start
    // eg. slashed noteheads etc
    double extraHeight = (ldata()->up ? upNote()->stemUpSE().y() : downNote()->stemDownNW().y()) / intrinsicMag() / _spatium;
    int shortestStem = style().styleB(Sid::useWideBeams) ? 12 : (style().styleD(Sid::shortestStem) + abs(extraHeight)) * 4;
    int quarterSpacesPerLine = std::floor(lineDistance * 2);
    int chordHeight = (downLine() - upLine()) * quarterSpacesPerLine; // convert to quarter spaces
    int stemLength = defaultStemLength;

    int minStemLengthQuarterSpaces = calcMinStemLength();
    m_minStemLength = minStemLengthQuarterSpaces / 4.0 * _spatium;

    int staffLineCount = staffItem ? staffItem->lines(tick()) : 5;
    int shortStemStart = style().styleI(Sid::shortStemStartLocation) * quarterSpacesPerLine + 1;
    bool useWideBeams = style().styleB(Sid::useWideBeams);
    int beamCount = ((m_tremolo && m_tremolo->twoNotes()) ? m_tremolo->lines() : 0) + (m_beam ? beams() : 0);
    int middleLine = minStaffOverlap(ldata()->up, staffLineCount,
                                     beamCount, !!m_hook, useWideBeams ? 4 : 3,
                                     useWideBeams, !(isGrace() || isSmall()));
    if (up()) {
        int stemEndPosition = upLine() * quarterSpacesPerLine - defaultStemLength;
        double stemEndPositionMag = (double)upLine() * quarterSpacesPerLine - (defaultStemLength * intrinsicMag());
        int idealStemLength = defaultStemLength;

        if (stemEndPositionMag <= -shortStemStart) {
            int reduction = maxReduction(std::abs((int)floor(stemEndPositionMag) + shortStemStart));
            idealStemLength = std::max(idealStemLength - reduction, shortestStem);
        } else if (stemEndPosition > middleLine) {
            // this case will be taken care of below; even if we were to adjust here we'd have
            // to adjust again later if the line spacing != 1.0 or if _relativeMag != 1.0
        } else {
            idealStemLength -= stemOpticalAdjustment(stemEndPosition);
            idealStemLength = std::max(idealStemLength, shortestStem);
        }
        stemLength = std::max(idealStemLength, minStemLengthQuarterSpaces);
    } else {
        int stemEndPosition = downLine() * quarterSpacesPerLine + defaultStemLength;
        double stemEndPositionMag = (double)downLine() * quarterSpacesPerLine + (defaultStemLength * intrinsicMag());
        int idealStemLength = defaultStemLength;

        int downShortStemStart = (staffLineCount - 1) * (2 * quarterSpacesPerLine) + shortStemStart;
        if (stemEndPositionMag >= downShortStemStart) {
            int reduction = maxReduction(std::abs((int)ceil(stemEndPositionMag) - downShortStemStart));
            idealStemLength = std::max(idealStemLength - reduction, shortestStem);
        } else if (stemEndPosition < middleLine) {
            // this case will be taken care of below; even if we were to adjust here we'd have
            // to adjust again later if the line spacing != 1.0 or if _relativeMag != 1.0
        } else {
            idealStemLength -= stemOpticalAdjustment(stemEndPosition);
            idealStemLength = std::max(idealStemLength, shortestStem);
        }

        stemLength = std::max(idealStemLength, minStemLengthQuarterSpaces);
    }
    if (beamCount == 4 && !m_hook) {
        stemLength = calc4BeamsException(stemLength);
    }

    double finalStemLength = (chordHeight / 4.0 * _spatium) + ((stemLength / 4.0 * _spatium) * intrinsicMag());
    double extraLength = 0.;
    Note* startNote = ldata()->up ? downNote() : upNote();
    if (!startNote->fixed()) {
        // when the chord's magnitude is < 1, the stem length with mag can find itself below the middle line.
        // in those cases, we have to add the extra amount to it to bring it to a minimum.
        double upValue = ldata()->up ? -1. : 1.;
        double stemStart = startNote->ldata()->pos().y();
        double stemEndMag = stemStart + (finalStemLength * upValue);
        double topLine = 0.0;
        lineDistance *= _spatium;
        double bottomLine = lineDistance * (staffLineCount - 1.0);
        double target = 0.0;
        double midLine = middleLine / 4.0 * lineDistance;
        if (RealIsEqualOrMore(lineDistance / _spatium, 1.0)) {
            // need to extend to middle line, or to opposite line if staff is < 2sp tall
            if (bottomLine < 2 * _spatium) {
                target = ldata()->up ? topLine : bottomLine;
            } else {
                double twoSpIn = ldata()->up ? bottomLine - (2 * _spatium) : topLine + (2 * _spatium);
                target = RealIsEqual(lineDistance / _spatium, 1.0) ? midLine : twoSpIn;
            }
        } else {
            // need to extend to second line in staff, or to opposite line if staff has < 3 lines
            if (staffLineCount < 3) {
                target = ldata()->up ? topLine : bottomLine;
            } else {
                target = ldata()->up ? bottomLine - (2 * lineDistance) : topLine + (2 * lineDistance);
            }
        }
        extraLength = 0.0;
        if (ldata()->up && stemEndMag > target) {
            extraLength = stemEndMag - target;
        } else if (!ldata()->up && stemEndMag < target) {
            extraLength = target - stemEndMag;
        }
    }
    return finalStemLength + extraLength;
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

void Chord::setBeamExtension(double extension)
{
    if (m_stem) {
        m_stem->setBaseLength(std::max(m_stem->baseLength() + Millimetre(extension), Millimetre { 0.0 }));
        m_defaultStemLength = std::max(m_defaultStemLength + extension, m_stem->baseLength().val());
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
           && !(tremolo() && tremolo()->twoNotes());
}

void Chord::createStem()
{
    Stem* stem = Factory::createStem(this);
    stem->setParent(this);
    stem->setGenerated(true);
    //! score()->undoAddElement calls add(), which assigns this created stem to _stem
    score()->undoAddElement(stem);
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
    score()->undoAddElement(hook);
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

void Chord::cmdUpdateNotes(AccidentalState* as)
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
            std::vector<Note*> notes(ch->notes());        // we need a copy!
            for (Note* note : notes) {
                note->updateAccidental(as);
            }
            ch->sortNotes();
        }
        std::vector<Note*> lnotes(notes());      // we need a copy!
        for (Note* note : lnotes) {
            if (note->tieBack() && note->tpc() == note->tieBack()->startNote()->tpc()) {
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
        const std::vector<Chord*> gna(graceNotesAfter());
        for (Chord* ch : gna) {
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
    }

    sortNotes();
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

mu::PointF Chord::pagePos() const
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
        double staffYOffset = staff() ? staff()->staffType(tick())->yoffset().val() * spatium() : 0.0;
        p.ry() += system->staffYpage(vStaffIdx()) + staffYOffset;
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
    if (m_tremolo && (tremoloChordType() != TremoloChordType::TremoloSecondNote)) {
        func(data, m_tremolo);
    }
    const Staff* st = staff();
    if ((st && st->showLedgerLines(tick())) || !st) {       // also for palette
        for (LedgerLine* ll = m_ledgerLines; ll; ll = ll->next()) {
            func(data, ll);
        }
    }
    size_t n = m_notes.size();
    for (size_t i = 0; i < n; ++i) {
        m_notes.at(i)->scanElements(data, func, all);
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
    } else if (m_tremolo) {
        return m_tremolo->getProperty(Pid::PLAY).toBool();
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

    if (m_tremolo) {
        m_tremolo->undoChangeProperty(Pid::PLAY, isPlayable);
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
    const std::list<EngravingObject*> links = linkList();
    for (EngravingObject* linkedObject : links) {
        if (linkedObject == this) {
            score()->undo(new ChangeSpanArpeggio(this, a));
            continue;
        }
        Chord* chord = toChord(linkedObject);
        Score* score = chord->score();
        EngravingItem* linkedArp = chord->spanArpeggio();
        if (score && linkedArp) {
            score->undo(new ChangeSpanArpeggio(chord, toArpeggio(linkedArp)));
        }
    }
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

    case ElementType::TREMOLO:
    {
        Tremolo* t = toTremolo(e);
        if (t->twoNotes()) {
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
    }
        if (tremolo()) {
            bool sameType = (e->subtype() == tremolo()->subtype());
            score()->undoRemoveElement(tremolo());
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

void Chord::setColor(const mu::draw::Color& color)
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
    if (arpeggio()) {
        arpeggio()->localSpatiumChanged(oldValue, newValue);
    }
    if (m_tremolo && (tremoloChordType() != TremoloChordType::TremoloSecondNote)) {
        m_tremolo->localSpatiumChanged(oldValue, newValue);
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
    case Pid::NO_STEM:        return noStem();
    case Pid::SMALL:          return isSmall();
    case Pid::STEM_DIRECTION: return PropertyValue::fromValue<DirectionV>(stemDirection());
    case Pid::PLAY: return isChordPlayable();
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
    case Pid::NO_STEM:        return false;
    case Pid::SMALL:          return false;
    case Pid::STEM_DIRECTION: return PropertyValue::fromValue<DirectionV>(DirectionV::AUTO);
    case Pid::PLAY: return true;
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
    case Pid::SMALL:
        setSmall(v.toBool());
        break;
    case Pid::STEM_DIRECTION:
        setStemDirection(v.value<DirectionV>());
        break;
    case Pid::PLAY:
        setIsChordPlayable(v.toBool());
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
    CompatMidiRender::createPlayEvents(this->score(), this);
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

    // set stem to auto (mostly important for rhythmic notation on drum staves)
    undoChangeProperty(Pid::STEM_DIRECTION, PropertyValue::fromValue<DirectionV>(DirectionV::AUTO));

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
    } else {
        // set small
        undoChangeProperty(Pid::SMALL, true);
        // set outside the staff
        double y = 0.0;
        if (track() % 2) {
            line = staffType->bottomLine() + 1;
            y    = 0.5 * spatium();
        } else {
            line = -1;
            if (!staffType->isDrumStaff()) {
                y = -0.5 * spatium();
            }
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
        }
    }
}

//---------------------------------------------------------
//  updateEndsGlissando
//    sets/resets the chord _endsGlissando according any glissando (or more)
//    end into this chord or no.
//---------------------------------------------------------

void Chord::updateEndsGlissandoOrGuitarBend()
{
    m_endsGlissando = false;         // assume no glissando ends here
    // scan all chord notes for glissandi ending on this chord
    for (Note* note : notes()) {
        for (Spanner* sp : note->spannerBack()) {
            if (sp->type() == ElementType::GLISSANDO) {
                m_endsGlissando = true;
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
    if (tremolo() && !keepTremolo) {
        remove(tremolo());
    }
    if (arpeggio()) {
        remove(arpeggio());
    }
    DeleteAll(graceNotes());
    graceNotes().clear();
    DeleteAll(articulations());
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
        if (n2->tieBack()) {
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

Chord* Chord::nextTiedChord(bool backwards, bool sameSize)
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

bool Chord::containsTieEnd() const
{
    for (const Note* note : m_notes) {
        if (note->tieBack() && !note->tieFor()) {
            return true;
        }
    }

    return false;
}

bool Chord::containsTieStart() const
{
    for (const Note* note : m_notes) {
        if (!note->tieBack() && note->tieFor()) {
            return true;
        }
    }

    return false;
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
    if (m_tremolo && m_tremolo->twoNotes()) {
        if (m_tremolo->chord1() == this) {
            return TremoloChordType::TremoloFirstNote;
        } else if (m_tremolo->chord2() == this) {
            return TremoloChordType::TremoloSecondNote;
        } else {
            ASSERT_X(String(u"Chord::tremoloChordType(): inconsistency"));
        }
    }
    return TremoloChordType::TremoloSingle;
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
            } else if (m_tremolo) {
                return m_tremolo;
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
            } else if (m_tremolo) {
                return m_tremolo;
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
        if (m_tremolo) {
            return m_tremolo;
        }
        break;

    case ElementType::ACCIDENTAL:
        e = e->parentItem();
    // fall through

    case ElementType::NOTE: {
        if (e == m_notes.front()) {
            if (m_arpeggio) {
                return m_arpeggio;
            } else if (m_tremolo) {
                return m_tremolo;
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
            ChordRest* next = prevChordRest(this);
            if (next) {
                if (next->isChord()) {
                    return toChord(next)->notes().back();
                }
                return toRest(next);
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

                ChordRest* next = prevChordRest(this);
                if (next) {
                    if (next->isChord()) {
                        return toChord(next)->notes().back();
                    }
                    return toRest(next);
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

    case ElementType::TREMOLO:
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
    if (m_tremolo) {
        return m_tremolo;
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

    if (tremolo() && score()->selectionFilter().canSelect(tremolo())) {
        rez = String(u"%1 %2").arg(rez, tremolo()->screenReaderInfo());
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

    EngravingItem::undoChangeProperty(id, newValue, ps);
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
    if (m_startEndSlurs.startUp || m_startEndSlurs.endUp) {
        m_allowKerningAbove = false;
    }
    if (m_startEndSlurs.startDown || m_startEndSlurs.endDown) {
        m_allowKerningBelow = false;
    }
}

Ornament* Chord::findOrnament() const
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
    return nullptr;
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
        staff_idx_t staffIdx = grace->staffIdx();
        staff_idx_t vStaffIdx = grace->vStaffIdx();
        Shape& s = _appendedSegment->staffShape(staffIdx);
        s.add(grace->shape(LD_ACCESS::PASS).translated(grace->pos()));
        if (vStaffIdx != staffIdx) {
            // Cross-staff grace notes add their shape to both the origin and the destination staff
            Shape& s2 = _appendedSegment->staffShape(vStaffIdx);
            s2.add(grace->shape().translated(grace->pos()));
        }
    }
}
}
