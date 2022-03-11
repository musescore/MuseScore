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

#include "style/style.h"
#include "rw/xml.h"

#include "factory.h"
#include "note.h"
#include "segment.h"
#include "text.h"
#include "measure.h"
#include "system.h"
#include "tuplet.h"
#include "hook.h"
#include "tie.h"
#include "arpeggio.h"
#include "score.h"
#include "tremolo.h"
#include "glissando.h"
#include "staff.h"
#include "part.h"
#include "utils.h"
#include "articulation.h"
#include "undo.h"
#include "chordline.h"
#include "lyrics.h"
#include "navigate.h"
#include "stafftype.h"
#include "stem.h"
#include "mscore.h"
#include "accidental.h"
#include "noteevent.h"
#include "pitchspelling.h"
#include "stemslash.h"
#include "ledgerline.h"
#include "drumset.h"
#include "key.h"
#include "stringdata.h"
#include "beam.h"
#include "slur.h"
#include "fingering.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
//---------------------------------------------------------
//   LedgerLineData
//---------------------------------------------------------

struct LedgerLineData {
    int line;
    qreal minX, maxX;
    bool visible;
    bool accidental;
};

//---------------------------------------------------------
//   upNote
//---------------------------------------------------------

Note* Chord::upNote() const
{
    Q_ASSERT(!_notes.empty());

    Note* result = _notes.back();
    const Staff* stf = staff();
    if (!stf) {
        return result;
    }

    const StaffType* st  = stf->staffTypeForElement(this);
    if (st->isDrumStaff()) {
        for (Note* n : _notes) {
            if (n->line() < result->line()) {
                result = n;
            }
        }
    } else if (st->isTabStaff()) {
        int line = st->lines() - 1;          // start at bottom line
        int noteLine;
        // scan each note: if TAB strings are not in sequential order,
        // visual order of notes might not correspond to pitch order
        for (Note* n : _notes) {
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
    Q_ASSERT(!_notes.empty());

    Note* result = _notes.front();
    const Staff* stf = staff();
    if (!stf) {
        return result;
    }

    const StaffType* st  = stf->staffTypeForElement(this);
    if (st->isDrumStaff()) {
        for (Note* n : _notes) {
            if (n->line() > result->line()) {
                result = n;
            }
        }
    } else if (st->isTabStaff()) {
        int line        = 0;          // start at top line
        int noteLine;
        // scan each note: if TAB strings are not in sequential order,
        // visual order of notes might not correspond to pitch order
        for (Note* n : _notes) {
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
//    If staf is not a TAB, always returns TOP and BOTTOM staff lines
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
    size_t n = _notes.size();
    for (size_t i = 0; i < n; ++i) {
        noteLine = tab->physStringToVisual(_notes.at(i)->string());
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
    size_t n = _notes.size();
    for (size_t i = 0; i < n; ++i) {
        noteLine = tab->physStringToVisual(_notes.at(i)->string());
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
    Q_ASSERT(staffType);
    bool isTabStaff = staffType->isTabStaff();
    int staffMiddleLine = staffType->middleLine();

    std::vector<int> distances;
    for (Note* note : _notes) {
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
    _ledgerLines      = 0;
    _stem             = 0;
    _hook             = 0;
    _stemDirection    = DirectionV::AUTO;
    _arpeggio         = 0;
    _tremolo          = 0;
    _endsGlissando    = false;
    _noteType         = NoteType::NORMAL;
    _stemSlash        = 0;
    _noStem           = false;
    _playEventType    = PlayEventType::Auto;
    _spaceLw          = 0.;
    _spaceRw          = 0.;
    _crossMeasure     = CrossMeasure::UNKNOWN;
    _graceIndex   = 0;
}

Chord::Chord(const Chord& c, bool link)
    : ChordRest(c, link)
{
    if (link) {
        score()->undo(new Link(this, const_cast<Chord*>(&c)));
    }
    _ledgerLines = 0;

    for (Note* onote : c._notes) {
        Note* nnote = Factory::copyNote(*onote, link);
        add(nnote);
    }
    for (Chord* gn : c.graceNotes()) {
        Chord* nc = new Chord(*gn, link);
        add(nc);
    }
    for (Articulation* a : c._articulations) {      // make deep copy
        Articulation* na = new Articulation(*a);
        if (link) {
            na->linkTo(a);
        }
        na->setParent(this);
        na->setTrack(track());
        _articulations.append(na);
    }
    _stem          = 0;
    _hook          = 0;
    _endsGlissando = false;
    _arpeggio      = 0;
    _stemSlash     = 0;
    _tremolo       = 0;

    _graceIndex     = c._graceIndex;
    _noStem         = c._noStem;
    _playEventType  = c._playEventType;
    _stemDirection  = c._stemDirection;
    _noteType       = c._noteType;
    _crossMeasure   = CrossMeasure::UNKNOWN;

    if (c._stem) {
        add(Factory::copyStem(*(c._stem)));
    }
    if (c._hook) {
        add(new Hook(*(c._hook)));
    }
    if (c._stemSlash) {
        add(Factory::copyStemSlash(*(c._stemSlash)));
    }
    if (c._arpeggio) {
        Arpeggio* a = new Arpeggio(*(c._arpeggio));
        add(a);
        if (link) {
            score()->undo(new Link(a, const_cast<Arpeggio*>(c._arpeggio)));
        }
    }
    if (c._tremolo) {
        Tremolo* t = Factory::copyTremolo(*(c._tremolo));
        if (link) {
            score()->undo(new Link(t, const_cast<Tremolo*>(c._tremolo)));
        }
        if (c._tremolo->twoNotes()) {
            if (c._tremolo->chord1() == &c) {
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
            if (link) {
                score()->undo(new Link(ncl, cl));
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
    for (Note* n : _notes) {
        n->undoUnlink();
    }
    for (Chord* gn : graceNotes()) {
        gn->undoUnlink();
    }
    for (Articulation* a : qAsConst(_articulations)) {
        a->undoUnlink();
    }
/*      if (_glissando)
            _glissando->undoUnlink(); */
    if (_arpeggio) {
        _arpeggio->undoUnlink();
    }
    if (_tremolo && !_tremolo->twoNotes()) {
        _tremolo->undoUnlink();
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
    qDeleteAll(_articulations);
    delete _arpeggio;
    if (_tremolo) {
        if (_tremolo->chord1() == this) {
            Tremolo* tremoloPointer = _tremolo;       // setTremolo(0) loses reference to the current pointer
            if (_tremolo->chord2()) {
                _tremolo->chord2()->setTremolo(0);
            }
            delete tremoloPointer;
        } else if (!(_tremolo->chord1())) { // delete orphaned tremolo
            delete _tremolo;
        }
    }
    delete _stemSlash;
    delete _stem;
    delete _hook;
    for (LedgerLine* ll = _ledgerLines; ll;) {
        LedgerLine* llNext = ll->next();
        delete ll;
        ll = llNext;
    }
    qDeleteAll(_graceNotes);
    qDeleteAll(_notes);
}

//---------------------------------------------------------
//   noteHeadWidth
//---------------------------------------------------------

qreal Chord::noteHeadWidth() const
{
    qreal nhw = score()->noteHeadWidth();
    if (_noteType != NoteType::NORMAL) {
        nhw *= score()->styleD(Sid::graceNoteMag);
    }
    return nhw * mag();
}

//! Returns Chord coordinates
qreal Chord::stemPosX() const
{
    const Staff* staff = this->staff();
    const StaffType* staffType = staff ? staff->staffTypeForElement(this) : nullptr;
    if (staffType && staffType->isTabStaff()) {
        return staffType->chordStemPosX(this) * spatium();
    }
    return _up ? noteHeadWidth() : 0.0;
}

//! Returns page coordinates
PointF Chord::stemPos() const
{
    const Staff* staff = this->staff();
    const StaffType* staffType = staff ? staff->staffTypeForElement(this) : nullptr;
    if (staffType && staffType->isTabStaff()) {
        return pagePos() + staffType->chordStemPos(this) * spatium();
    }

    if (_up) {
        const Note* downNote = this->downNote();
        qreal nhw = _notes.size() == 1 ? downNote->bboxRightPos() : noteHeadWidth();
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

    if (_up) {
        qreal nhw = noteHeadWidth();
        return pagePos() + PointF(nhw, upNote()->pos().y());
    }

    return pagePos() + PointF(0, downNote()->pos().y());
}

//---------------------------------------------------------
//   rightEdge
//---------------------------------------------------------

qreal Chord::rightEdge() const
{
    qreal right = 0.0;
    for (Note* n : notes()) {
        right = qMax(right, x() + n->x() + n->bboxRightPos());
    }

    return right;
}

//---------------------------------------------------------
//   setTremolo
//---------------------------------------------------------

void Chord::setTremolo(Tremolo* tr)
{
    if (_tremolo && tr && tr == _tremolo) {
        return;
    }

    if (_tremolo) {
        if (_tremolo->twoNotes()) {
            TDuration d;
            const Fraction f = ticks();
            if (f.numerator() > 0) {
                d = TDuration(f);
            } else {
                d = _tremolo->durationType();
                const int dots = d.dots();
                d = d.shift(1);
                d.setDots(dots);
            }

            setDurationType(d);
            Chord* other = _tremolo->chord1() == this ? _tremolo->chord2() : _tremolo->chord1();
            _tremolo = nullptr;
            if (other) {
                other->setTremolo(nullptr);
            }
        } else {
            _tremolo = nullptr;
        }
    }

    if (tr) {
        if (tr->twoNotes()) {
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
            _tremolo = tr;
            if (other) {
                other->setTremolo(tr);
            }
        } else {
            _tremolo = tr;
        }
    } else {
        _tremolo = nullptr;
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

        for (unsigned idx = 0; idx < _notes.size(); ++idx) {
            if (tab) {
                // _notes should be sorted by string
                if (note->string() > _notes[idx]->string()) {
                    _notes.insert(_notes.begin() + idx, note);
                    found = true;
                    break;
                }
            } else {
                // _notes should be sorted by line position,
                // but it's often not yet possible since line is unknown
                // use pitch instead, and line as a second sort criteria.
                if (note->pitch() <= _notes[idx]->pitch()) {
                    if (note->pitch() == _notes[idx]->pitch() && note->line() >= _notes[idx]->line()) {
                        _notes.insert(_notes.begin() + idx + 1, note);
                    } else {
                        _notes.insert(_notes.begin() + idx, note);
                    }
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            _notes.push_back(note);
        }
        note->connectTiedNotes();
        if (voice() && measure() && note->visible()) {
            measure()->setHasVoices(staffIdx(), true);
        }
    }
        score()->setPlaylistDirty();
        break;
    case ElementType::ARPEGGIO:
        _arpeggio = toArpeggio(e);
        break;
    case ElementType::TREMOLO:
        setTremolo(toTremolo(e));
        break;
    case ElementType::GLISSANDO:
        _endsGlissando = true;
        break;
    case ElementType::STEM:
        Q_ASSERT(!_stem);
        _stem = toStem(e);
        break;
    case ElementType::HOOK:
        _hook = toHook(e);
        break;
    case ElementType::CHORDLINE:
    case ElementType::SLIDE:
        el().push_back(e);
        break;
    case ElementType::STEM_SLASH:
        Q_ASSERT(!_stemSlash);
        _stemSlash = toStemSlash(e);
        break;
    case ElementType::CHORD:
    {
        Chord* gc = toChord(e);
        Q_ASSERT(gc->noteType() != NoteType::NORMAL);
        int idx = gc->graceIndex();
        gc->setFlag(ElementFlag::MOVABLE, true);
        _graceNotes.insert(_graceNotes.begin() + idx, gc);
    }
    break;
    case ElementType::LEDGER_LINE:
        qFatal("Chord::add ledgerline");
        break;
    case ElementType::ARTICULATION:
    {
        Articulation* a = toArticulation(e);
        if (a->layoutCloseToNote()) {
            auto i = _articulations.begin();
            while (i != _articulations.end() && (*i)->layoutCloseToNote()) {
                i++;
            }
            _articulations.insert(i, a);
        } else {
            _articulations.push_back(a);
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
        auto i = std::find(_notes.begin(), _notes.end(), note);
        if (i != _notes.end()) {
            _notes.erase(i);
            note->disconnectTiedNotes();
            for (Spanner* s : note->spannerBack()) {
                note->removeSpannerBack(s);
            }
            for (Spanner* s : note->spannerFor()) {
                note->removeSpannerFor(s);
            }
        } else {
            qDebug("Chord::remove() note %p not found!", e);
        }
        if (voice() && measure() && note->visible()) {
            measure()->checkMultiVoices(staffIdx());
        }
        score()->setPlaylistDirty();
    }
    break;

    case ElementType::ARPEGGIO:
        _arpeggio = 0;
        break;
    case ElementType::TREMOLO:
        setTremolo(nullptr);
        break;
    case ElementType::GLISSANDO:
        _endsGlissando = false;
        break;
    case ElementType::STEM:
        _stem = 0;
        break;
    case ElementType::HOOK:
        _hook = 0;
        break;
    case ElementType::STEM_SLASH:
        Q_ASSERT(_stemSlash);
        if (_stemSlash->selected() && score()) {
            score()->deselect(_stemSlash);
        }
        _stemSlash = 0;
        break;
    case ElementType::CHORDLINE:
        el().remove(e);
        break;
    case ElementType::CHORD:
    {
        auto i = std::find(_graceNotes.begin(), _graceNotes.end(), toChord(e));
        Chord* grace = *i;
        grace->setGraceIndex(i - _graceNotes.begin());
        _graceNotes.erase(i);
    }
    break;
    case ElementType::ARTICULATION:
    {
        Articulation* a = toArticulation(e);
        if (!_articulations.removeOne(a)) {
            qDebug("ChordRest::remove(): articulation not found");
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

qreal Chord::maxHeadWidth() const
{
    // determine max head width in chord
    qreal hw = 0;
    for (const Note* n : _notes) {
        qreal t = n->headWidth();
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
    int track          = 0;                     // the track lines belong to
    // the line pos corresponding to the bottom line of the staff
    int lineBelow      = 8;                     // assuming 5-lined "staff"
    qreal lineDistance = 1;
    qreal mag         = 1;
    bool staffVisible  = true;
    int stepOffset = 0;                         // for staff type changes with a step offset

    if (segment()) {   //not palette
        Fraction tick = segment()->tick();
        int idx       = staffIdx() + staffMove();
        track         = staff2track(idx);
        Staff* st     = score()->staff(idx);
        lineBelow     = (st->lines(tick) - 1) * 2;
        lineDistance  = st->lineDistance(tick);
        mag           = staff()->staffMag(tick);
        staffVisible  = !staff()->isLinesInvisible(tick);
        stepOffset = st->staffType(tick)->stepOffset();
    }

    // need ledger lines?
    if (downLine() + stepOffset <= lineBelow + 1 && upLine() + stepOffset >= -1) {
        return;
    }

    // the extra length of a ledger line to be added on each side of the notehead
    qreal extraLen = score()->styleMM(Sid::ledgerLineLength) * mag;
    qreal hw;
    qreal minX, maxX;                           // note extrema in raster units
    int minLine, maxLine;
    bool visible = false;
    qreal x;

    // scan chord notes, collecting visibility and x and y extrema
    // NOTE: notes are sorted from bottom to top (line no. decreasing)
    // notes are scanned twice from outside (bottom or top) toward the staff
    // each pass stops at the first note without ledger lines
    size_t n = _notes.size();
    for (size_t j = 0; j < 2; j++) {               // notes are scanned twice...
        int from, delta;
        std::vector<LedgerLineData> vecLines;
        hw = 0.0;
        minX  = maxX = 0;
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
            Note* note = _notes.at(i);
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
            hw = qMax(hw, note->headWidth());

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
            qreal _spatium = spatium();
            qreal stepDistance = lineDistance * 0.5;
            for (auto lld : vecLines) {
                LedgerLine* h = new LedgerLine(score());
                h->setParent(this);
                h->setTrack(track);
                h->setVisible(lld.visible && staffVisible);
                h->setLen(lld.maxX - lld.minX);
                h->setPos(lld.minX, lld.line * _spatium * stepDistance);
                h->setNext(_ledgerLines);
                _ledgerLines = h;
            }
        }
    }
    for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next()) {
        ll->layout();
    }
}

void Chord::computeUp()
{
    Q_ASSERT(!_notes.empty());

    _usesAutoUp = false;

    bool hasCustomStemDirection = _stemDirection != DirectionV::AUTO;
    if (hasCustomStemDirection) {
        _up = _stemDirection == DirectionV::UP;
        return;
    }

    if (_beam) {
        _up = _beam->up();
        return;
    }

    if (_isUiItem) {
        _up = true;
        return;
    }

    bool staffHasMultipleVoices = measure()->hasVoices(staffIdx(), tick(), actualTicks());
    if (staffHasMultipleVoices) {
        bool isTrackEven = track() % 2 == 0;
        _up = isTrackEven;
        return;
    }

    const StaffType* tab = staff() ? staff()->staffTypeForElement(this) : 0;
    bool isTabStaff  = tab && tab->isTabStaff();
    if (isTabStaff && (tab->stemless() || !tab->stemThrough())) {
        _up = tab->stemless() ? false : !tab->stemsDown();
        return;
    }

    bool isGraceNote = _noteType != NoteType::NORMAL;
    if (isGraceNote) {
        _up = true;
        return;
    }

    bool chordIsCrossStaff = staffMove() != 0;
    if (chordIsCrossStaff) {
        _up = staffMove() > 0;
        return;
    }

    int staffLineCount = staff()->lines(tick());
    DirectionV stemDirection = score()->styleV(Sid::smallStaffStemDirection).value<DirectionV>();
    int minStaffSizeForAutoStems = score()->styleI(Sid::minStaffSizeForAutoStems);
    if (staffLineCount < minStaffSizeForAutoStems && stemDirection != DirectionV::AUTO) {
        _up = stemDirection == DirectionV::UP;
        return;
    }

    std::vector<int> distances = noteDistances();
    int direction = Chord::computeAutoStemDirection(&distances);
    _up = direction > 0;
    _usesAutoUp = direction == 0;

    if (_usesAutoUp && score()->styleB(Sid::preferStemDirectionMatchContext)) {
        _up = computeUpContext();
    }
}

// return 1 means up, 0 means in the middle, -1 means down
int Chord::computeAutoStemDirection(const std::vector<int>* noteDistances)
{
    int left = 0;
    int right = static_cast<int>(noteDistances->size()) - 1;

    while (left <= right) {
        int leftNote = noteDistances->at(left);
        int rightNote = noteDistances->at(right);
        int netDirecting = leftNote + rightNote;
        if (netDirecting == 0) {
            left++;
            right--;
            continue;
        }
        return netDirecting > 0 ? 1 : -1;
    }
    return 0;
}

bool Chord::computeUpContext()
{
    ChordRest* previous = prevChordRest(this);
    bool previousIsChord = false;
    bool previousIsUp = false;
    if (previous) {
        while (previous->isChord() && previous->usesAutoUp()) {
            if (prevChordRest(previous)) {
                previous = prevChordRest(previous);
            } else {
                break;
            }
        }
        previousIsChord = previous->isChord();
        previousIsUp = previous->isChord() && previous->up();
    }

    ChordRest* next = nextChordRest(this);
    bool nextIsChord = false;
    bool nextIsUp = false;
    if (next) {
        while (next->isChord() && next->usesAutoUp()) {
            if (nextChordRest(next)) {
                next = nextChordRest(next);
            } else {
                break;
            }
        }
        nextIsChord = next->isChord();
        nextIsUp = next->isChord() && next->up();
    }

    return (previousIsUp && nextIsUp) || (!previousIsChord && nextIsUp) || (!nextIsChord && previousIsUp);
}

//---------------------------------------------------------
//   selectedNote
//---------------------------------------------------------

Note* Chord::selectedNote() const
{
    Note* note = 0;
    size_t n = _notes.size();
    for (size_t i = 0; i < n; ++i) {
        Note* currentNote = _notes.at(i);
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
//   Chord::write
//---------------------------------------------------------

void Chord::write(XmlWriter& xml) const
{
    for (Chord* c : _graceNotes) {
        c->write(xml);
    }
    writeBeam(xml);
    xml.startObject(this);
    ChordRest::writeProperties(xml);
    for (const Articulation* a : _articulations) {
        a->write(xml);
    }
    switch (_noteType) {
    case NoteType::NORMAL:
        break;
    case NoteType::ACCIACCATURA:
        xml.tagE("acciaccatura");
        break;
    case NoteType::APPOGGIATURA:
        xml.tagE("appoggiatura");
        break;
    case NoteType::GRACE4:
        xml.tagE("grace4");
        break;
    case NoteType::GRACE16:
        xml.tagE("grace16");
        break;
    case NoteType::GRACE32:
        xml.tagE("grace32");
        break;
    case NoteType::GRACE8_AFTER:
        xml.tagE("grace8after");
        break;
    case NoteType::GRACE16_AFTER:
        xml.tagE("grace16after");
        break;
    case NoteType::GRACE32_AFTER:
        xml.tagE("grace32after");
        break;
    default:
        break;
    }

    if (_noStem) {
        xml.tag("noStem", _noStem);
    } else if (_stem && (_stem->isUserModified() || (_stem->userLength() != 0.0))) {
        _stem->write(xml);
    }
    if (_hook && _hook->isUserModified()) {
        _hook->write(xml);
    }
    if (_stemSlash && _stemSlash->isUserModified()) {
        _stemSlash->write(xml);
    }
    writeProperty(xml, Pid::STEM_DIRECTION);
    for (Note* n : _notes) {
        n->write(xml);
    }
    if (_arpeggio) {
        _arpeggio->write(xml);
    }
    if (_tremolo && tremoloChordType() != TremoloChordType::TremoloSecondNote) {
        _tremolo->write(xml);
    }
    for (EngravingItem* e : el()) {
        e->write(xml);
    }
    xml.endObject();
}

//---------------------------------------------------------
//   Chord::read
//---------------------------------------------------------

void Chord::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        if (readProperties(e)) {
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Chord::readProperties(XmlReader& e)
{
    const QStringRef& tag(e.name());

    if (tag == "Note") {
        Note* note = Factory::createNote(this);
        // the note needs to know the properties of the track it belongs to
        note->setTrack(track());
        note->setParent(this);
        note->read(e);
        add(note);
    } else if (ChordRest::readProperties(e)) {
    } else if (tag == "Stem") {
        Stem* s = Factory::createStem(this);
        s->read(e);
        add(s);
    } else if (tag == "Hook") {
        _hook = new Hook(this);
        _hook->read(e);
        add(_hook);
    } else if (tag == "appoggiatura") {
        _noteType = NoteType::APPOGGIATURA;
        e.readNext();
    } else if (tag == "acciaccatura") {
        _noteType = NoteType::ACCIACCATURA;
        e.readNext();
    } else if (tag == "grace4") {
        _noteType = NoteType::GRACE4;
        e.readNext();
    } else if (tag == "grace16") {
        _noteType = NoteType::GRACE16;
        e.readNext();
    } else if (tag == "grace32") {
        _noteType = NoteType::GRACE32;
        e.readNext();
    } else if (tag == "grace8after") {
        _noteType = NoteType::GRACE8_AFTER;
        e.readNext();
    } else if (tag == "grace16after") {
        _noteType = NoteType::GRACE16_AFTER;
        e.readNext();
    } else if (tag == "grace32after") {
        _noteType = NoteType::GRACE32_AFTER;
        e.readNext();
    } else if (tag == "StemSlash") {
        StemSlash* ss = Factory::createStemSlash(this);
        ss->read(e);
        add(ss);
    } else if (readProperty(tag, e, Pid::STEM_DIRECTION)) {
    } else if (tag == "noStem") {
        _noStem = e.readInt();
    } else if (tag == "Arpeggio") {
        _arpeggio = Factory::createArpeggio(this);
        _arpeggio->setTrack(track());
        _arpeggio->read(e);
        _arpeggio->setParent(this);
    } else if (tag == "Tremolo") {
        _tremolo = Factory::createTremolo(this);
        _tremolo->setTrack(track());
        _tremolo->read(e);
        _tremolo->setParent(this);
        _tremolo->setDurationType(durationType());
    } else if (tag == "tickOffset") {     // obsolete
    } else if (tag == "ChordLine") {
        ChordLine* cl = Factory::createChordLine(this);
        cl->read(e);
        add(cl);
    } else {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   upPos
//---------------------------------------------------------

qreal Chord::upPos() const
{
    return upNote()->pos().y();
}

//---------------------------------------------------------
//   downPos
//---------------------------------------------------------

qreal Chord::downPos() const
{
    return downNote()->pos().y();
}

//---------------------------------------------------------
//   centerX
//    return x position for attributes
//---------------------------------------------------------

qreal Chord::centerX() const
{
    // TAB 'notes' are always centered on the stem
    const Staff* st = staff();
    const StaffType* stt = st->staffTypeForElement(this);
    if (stt->isTabStaff()) {
        return stt->chordStemPosX(this) * spatium();
    }

    const Note* note = up() ? upNote() : downNote();
    qreal x = note->pos().x() + note->noteheadCenterX();
    if (note->mirror()) {
        x += (note->headBodyWidth()) * (up() ? -1.0 : 1.0);
    }
    return x;
}

//---------------------------------------------------------
//   processSiblings
//---------------------------------------------------------

void Chord::processSiblings(std::function<void(EngravingItem*)> func) const
{
    if (_hook) {
        func(_hook);
    }
    if (_stem) {
        func(_stem);
    }
    if (_stemSlash) {
        func(_stemSlash);
    }
    if (_arpeggio) {
        func(_arpeggio);
    }
    if (_tremolo) {
        func(_tremolo);
    }
    for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next()) {
        func(ll);
    }
    for (Articulation* a : _articulations) {
        func(a);
    }
    for (Note* note : _notes) {
        func(note);
    }
    for (EngravingItem* e : el()) {
        func(e);
    }
    for (Chord* chord : _graceNotes) {    // process grace notes last, needed for correct shape calculation
        func(chord);
    }
}

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Chord::setTrack(int val)
{
    ChordRest::setTrack(val);
    processSiblings([val](EngravingItem* e) { e->setTrack(val); });
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Chord::setScore(Score* s)
{
    ChordRest::setScore(s);
    processSiblings([s](EngravingItem* e) { e->setScore(s); });
}

// all values are in quarter spaces
int Chord::calcMinStemLength()
{
    int minStemLength = 0; // in quarter spaces
    qreal _spatium = spatium();

    if (_tremolo && !_tremolo->twoNotes()) {
        // buzz roll's height is actually half of the visual height,
        // so we need to multiply it by 2 to get the actual height
        int buzzRollMultiplier = _tremolo->isBuzzRoll() ? 2 : 1;
        minStemLength += ceil(_tremolo->minHeight() * 4.0 * buzzRollMultiplier);
        static const int outSidePadding = score()->styleMM(Sid::tremoloOutSidePadding).val() / _spatium * 4.0;
        static const int noteSidePadding = score()->styleMM(Sid::tremoloNoteSidePadding).val() / _spatium * 4.0;
        int line = _up ? upNote()->line() : downNote()->line();
        line *= 2; // convert to quarter spaces
        int outsideStaffOffset = 0;
        if (!_up && line < -2) {
            outsideStaffOffset = -line;
        } else if (_up && line > staff()->lines(tick()) * 4) {
            outsideStaffOffset = line - staff()->lines(tick()) * 4 + 4;
        }
        minStemLength += (outSidePadding + qMax(noteSidePadding, outsideStaffOffset));
    }

    if (_hook) {
        int hookOffset = (_hook->height() - qAbs(_hook->smuflAnchor().y())) / _spatium * 4 - 2;
        // TODO: when the SMuFL metadata includes a cutout for flags, replace this with that metadata
        // https://github.com/w3c/smufl/issues/203
        int cutout = up() ? 6 : 8;
        if (beams() >= 2) {
            cutout -= 2;
        }
        if (score()->styleB(Sid::useStraightNoteFlags)) {
            cutout = 0;
        }
        if (minStemLength < cutout) {
            minStemLength = hookOffset;
        } else {
            minStemLength += hookOffset - cutout;
        }
        // ceils to the nearest half-space (returned as pixels)
        minStemLength = ceil(minStemLength / 2.0) * 2;
    } else {
        static const int minInnerStemLengths[4] = { 10, 9, 8, 7 };
        minStemLength = qMax(minStemLength, minInnerStemLengths[qMin(beams(), 3)]);
        // add beam lengths
        minStemLength += beams() * (score()->styleB(Sid::useWideBeams) ? 4 : 3);
        if (beams() > 0) {
            minStemLength -= 1;
        }
    }
    return minStemLength;
}

// all values are in quarter spaces
int Chord::stemLengthBeamAddition() const
{
    if (_hook) {
        return 0;
    }
    int beamCount = beams();
    switch (beamCount) {
    case 0:
    case 1:
    case 2:
        return 0;
    case 3:
        return 2;
    default:
        return (beamCount - 3) * (score()->styleB(Sid::useWideBeams) ? 4 : 3);
    }
}

int Chord::minStaffOverlap(bool up, int staffLines, int beamCount, bool hasHook, qreal beamSpacing, bool useWideBeams)
{
    int beamOverlap = 8;
    if (beamCount == 3 && !hasHook) {
        beamOverlap = 12;
    } else if (beamCount >= 4 && !hasHook) {
        beamOverlap = (beamCount - 4) * beamSpacing + (useWideBeams ? 16 : 14);
    }

    int staffOverlap = qMin(beamOverlap, (staffLines - 1) * 4);
    if (!up) {
        return staffOverlap;
    }
    return (staffLines - 1) * 4 - staffOverlap;
}

// all values are in quarter spaces
int Chord::maxReduction(int extensionOutsideStaff) const
{
    // [extensionOutsideStaff][beamCount]
    static const int maxReductions[4][5] = {
        //1sp 1.5sp 2sp   2.5sp >=3sp -- extensionOutsideStaff
        { 0, 1, 2,  3, 4 }, // 0 beams
        { 2, 2, 2,  3, 3 }, // 1 beam
        { 0, 1, 1,  1, 1 }, // 2 beams
        { 0, 0, 0, -1, 1 }, // 3 beams
    };
    int beamCount = beams();
    if (beamCount >= 4) {
        return 0;
    }
    int extensionHalfSpaces = extensionOutsideStaff / 2;
    if (extensionHalfSpaces >= 5) {
        return maxReductions[beamCount][4];
    }
    if (_hook) {
        return maxReductions[0][extensionHalfSpaces];
    }
    return maxReductions[beamCount][extensionHalfSpaces];
}

// all values are in quarter spaces
int Chord::stemOpticalAdjustment(int stemEndPosition) const
{
    if (_hook) {
        return 0;
    }
    int beamCount = beams();
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
        difference = qAbs(downNote()->line());
    }
    switch (difference) {
    case 2:
        return qMax(stemLength, 21);
    case 3:
    case 4:
        return qMax(stemLength, 23);
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

qreal Chord::calcDefaultStemLength()
{
    if (!_stem) {
        return 0.0;
    }

    qreal _spatium = spatium();
    int defaultStemLength = (isSmall() ? score()->styleD(Sid::stemLengthSmall) : score()->styleD(Sid::stemLength)) * 4;
    defaultStemLength += stemLengthBeamAddition();
    int chordHeight = (downLine() - upLine()) * 2; // convert to quarter spaces
    int stemLength = defaultStemLength;

    const Staff* staffItem = staff();
    const StaffType* staffType = staffItem ? staffItem->staffTypeForElement(this) : nullptr;
    const StaffType* tab = (staffType && staffType->isTabStaff()) ? staffType : nullptr;

    bool isBesideTabStaff = tab && !tab->stemless() && !tab->stemThrough();
    if (isBesideTabStaff) {
        return tab->chordStemLength(this) * _spatium;
    } else if (tab) {
        defaultStemLength *= 1.5;
    }

    int minStemLengthQuarterSpaces = calcMinStemLength();
    _minStemLength = minStemLengthQuarterSpaces / 4.0 * _spatium;

    int staffLineCount = staffItem ? staffItem->lines(tick()) : 5;
    int shortStemStart = score()->styleI(Sid::shortStemStartLocation) * 2 + 1;
    bool useWideBeams = score()->styleB(Sid::useWideBeams);
    int middleLine = minStaffOverlap(_up, staffLineCount, beams(), !!_hook, useWideBeams ? 4 : 3, useWideBeams);
    int shortestStem = score()->styleB(Sid::useWideBeams) ? 12 : score()->styleD(Sid::shortestStem) * 4;
    if (isGrace()) {
        stemLength = qMax(static_cast<int>(defaultStemLength * score()->styleD(Sid::graceNoteMag)), minStemLengthQuarterSpaces);
    } else if (up()) {
        int stemEndPosition = upLine() * 2 - defaultStemLength;
        int idealStemLength = defaultStemLength;

        if (stemEndPosition <= -shortStemStart) {
            int reduction = maxReduction(qAbs(stemEndPosition + shortStemStart));
            if (tab) {
                reduction *= 2;
            }
            if (_hook) {
                reduction = reduction / 2 * 2; // transforms to only half steps positions
            }
            idealStemLength = qMax(idealStemLength - reduction, shortestStem);
        } else if (stemEndPosition > middleLine) {
            idealStemLength += stemEndPosition - middleLine;
        } else {
            idealStemLength -= stemOpticalAdjustment(stemEndPosition);
            idealStemLength = qMax(idealStemLength, shortestStem);
        }
        stemLength = qMax(idealStemLength, minStemLengthQuarterSpaces);
    } else {
        int stemEndPosition = downLine() * 2 + defaultStemLength;
        int idealStemLength = defaultStemLength;
        int downShortStemStart = (staffLineCount - 1) * 4 + shortStemStart;
        if (stemEndPosition >= downShortStemStart) {
            int reduction = maxReduction(stemEndPosition - downShortStemStart);
            if (tab) {
                reduction *= 2;
            }
            idealStemLength = qMax(idealStemLength - reduction, shortestStem);
        } else if (stemEndPosition < middleLine) {
            idealStemLength += middleLine - stemEndPosition;
        } else {
            idealStemLength -= stemOpticalAdjustment(stemEndPosition);
            idealStemLength = qMax(idealStemLength, shortestStem);
        }

        stemLength = qMax(idealStemLength, minStemLengthQuarterSpaces);
    }
    if (beams() == 4) {
        stemLength = calc4BeamsException(stemLength);
    }
    return (stemLength + chordHeight) / 4.0 * _spatium;
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

void Chord::setBeamExtension(qreal extension)
{
    if (_stem) {
        _stem->setBaseLength(_stem->baseLength() + Millimetre(extension));
        _defaultStemLength += extension;
    }
}

bool Chord::shouldHaveStem() const
{
    const Staff* staff = this->staff();
    const StaffType* staffType = staff ? staff->staffTypeForElement(this) : nullptr;

    return !_noStem
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
           && !beam();
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
    if (_stem) {
        score()->undoRemoveElement(_stem);
    }
    if (_hook) {
        score()->undoRemoveElement(_hook);
    }
    if (_stemSlash) {
        score()->undoRemoveElement(_stemSlash);
    }
}

void Chord::createHook()
{
    Hook* hook = new Hook(this);
    hook->setParent(this);
    hook->setGenerated(true);
    score()->undoAddElement(hook);
}

void Chord::layoutHook()
{
    if (!_hook) {
        createHook();
    }
    _hook->setHookType(up() ? durationType().hooks() : -durationType().hooks());
    _hook->layout();
}

//! May be called again when the chord is added to or removed from a beam.
void Chord::layoutStem()
{
    if (!shouldHaveStem()) {
        removeStem();
        return;
    }

    if (!_stem) {
        createStem();
    }

    // Stem needs to know hook's bbox and SMuFL anchors.
    if (shouldHaveHook()) {
        layoutHook();
    } else {
        score()->undoRemoveElement(_hook);
    }

    _stem->rxpos() = stemPosX();

    _defaultStemLength = calcDefaultStemLength();
    // This calls _stem->layout()
    _stem->setBaseLength(Millimetre(_defaultStemLength));

    // And now we need to set the position of the flag.
    if (_hook) {
        _hook->setPos(_stem->flagPosition());
    }

    // Add Stem slash
    if ((_noteType == NoteType::ACCIACCATURA) && !(beam() && beam()->elements().front() != this)) {
        if (!_stemSlash) {
            add(Factory::createStemSlash(this));
        }
        _stemSlash->layout();
    } else if (_stemSlash) {
        remove(_stemSlash);
    }
}

//---------------------------------------------------------
//    underBeam: true, if grace note is placed under a beam.
//---------------------------------------------------------

bool Chord::underBeam() const
{
    if (_noteType == NoteType::NORMAL) {
        return false;
    }
    const Chord* cr = toChord(explicitParent());
    Beam* beam = cr->beam();
    if (!beam || !cr->beam()->up()) {
        return false;
    }
    int s = beam->elements().count();
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
//   layout2
//    Called after horizontal positions of all elements
//    are fixed.
//---------------------------------------------------------

void Chord::layout2()
{
    for (Chord* c : qAsConst(_graceNotes)) {
        c->layout2();
    }

    const qreal mag = staff()->staffMag(this);

    //
    // position after-chord grace notes
    // room for them has been reserved in Chord::layout()
    //

    QVector<Chord*> gna = graceNotesAfter();
    if (!gna.empty()) {
        qreal minNoteDist = score()->styleMM(Sid::minNoteDistance) * mag * score()->styleD(Sid::graceNoteMag);
        // position grace notes from the rightmost to the leftmost
        // get segment (of whatever type) at the end of this chord; if none, get measure last segment
        Segment* s = measure()->tick2segment(segment()->tick() + actualTicks(), SegmentType::All);
        if (s == nullptr) {
            s = measure()->last();
        }
        if (s == segment()) {             // if our segment is the last, no adjacent segment found
            s = nullptr;
        }
        // start from the right (if next segment found, x of it relative to this chord;
        // chord right space otherwise)
        Chord* last = gna.last();
        qreal xOff =  s ? (s->pos().x() - s->staffShape(last->vStaffIdx()).left()) - (segment()->pos().x() + pos().x()) : _spaceRw;
        // final distance: if near to another chord, leave minNoteDist at right of last grace
        // else leave note-to-barline distance;
        xOff -= (s != nullptr && s->segmentType() != SegmentType::ChordRest)
                ? score()->styleMM(Sid::noteBarDistance) * mag
                : minNoteDist;
        // scan grace note list from the end
        int n = gna.size();
        for (int i = n - 1; i >= 0; i--) {
            Chord* g = gna.value(i);
            xOff -= g->_spaceRw;                        // move to left by grace note left space (incl. grace own width)
            g->rxpos() = xOff;
            xOff -= minNoteDist + g->_spaceLw;          // move to left by grace note right space and inter-grace distance
        }
    }
    if (_tabDur) {
        _tabDur->layout2();
    }
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
                //qWarning("unmapped drum note %d", pitch);
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
        const Instrument* instrument = part()->instrument(this->tick());
        for (Chord* ch : graceNotes()) {
            instrument->stringData()->fretChords(ch);
        }
        instrument->stringData()->fretChords(this);
        return;
    } else {
        // if not tablature, use instrument->useDrumset to set staffGroup (to allow pitched to unpitched in same staff)
        staffGroup = st->part()->instrument(this->tick())->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;
    }

    // PITCHED_ and PERCUSSION_STAFF can go note by note

    if (staffGroup == StaffGroup::STANDARD) {
        const QVector<Chord*> gnb(graceNotesBefore());
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
        const QVector<Chord*> gna(graceNotesAfter());
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
            qWarning("no drumset");
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
        qreal staffYOffset = staff() ? staff()->staffType(tick())->yoffset().val() * spatium() : 0.0;
        p.ry() += system->staffYpage(vStaffIdx()) + staffYOffset;
        return p;
    }
    return EngravingItem::pagePos();
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Chord::layout()
{
    if (_notes.empty()) {
        return;
    }
    if (onTabStaff()) {
        layoutTablature();
    } else {
        layoutPitched();
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Chord::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    for (Articulation* a : _articulations) {
        func(data, a);
    }
    if (_hook) {
        func(data, _hook);
    }
    if (_stem) {
        func(data, _stem);
    }
    if (_stemSlash) {
        func(data, _stemSlash);
    }
    if (_arpeggio) {
        func(data, _arpeggio);
    }
    if (_tremolo && (tremoloChordType() != TremoloChordType::TremoloSecondNote)) {
        func(data, _tremolo);
    }
    const Staff* st = staff();
    if ((st && st->showLedgerLines(tick())) || !st) {       // also for palette
        for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next()) {
            func(data, ll);
        }
    }
    size_t n = _notes.size();
    for (size_t i = 0; i < n; ++i) {
        _notes.at(i)->scanElements(data, func, all);
    }
    for (Chord* chord : _graceNotes) {
        chord->scanElements(data, func, all);
    }
    for (EngravingItem* e : el()) {
        e->scanElements(data, func, all);
    }
    ChordRest::scanElements(data, func, all);
}

//---------------------------------------------------------
//   layoutPitched
//---------------------------------------------------------

void Chord::layoutPitched()
{
    int gi = 0;
    for (Chord* c : qAsConst(_graceNotes)) {
        // HACK: graceIndex is not well-maintained on add & remove
        // so rebuild now
        c->setGraceIndex(gi++);
        if (c->isGraceBefore()) {
            c->layoutPitched();
        }
    }
    const QVector<Chord*> graceNotesBefore = Chord::graceNotesBefore();
    const int gnb = graceNotesBefore.size();

    // lay out grace notes after separately so they are processed left to right
    // (they are normally stored right to left)

    const QVector<Chord*> gna = graceNotesAfter();
    for (Chord* c : gna) {
        c->layoutPitched();
    }

    qreal _spatium         = spatium();
    qreal mag_             = staff() ? staff()->staffMag(this) : 1.0;      // palette elements do not have a staff
    qreal dotNoteDistance  = score()->styleMM(Sid::dotNoteDistance) * mag_;
    qreal minNoteDistance  = score()->styleMM(Sid::minNoteDistance) * mag_;
    qreal minTieLength     = score()->styleMM(Sid::MinTieLength) * mag_;

    qreal graceMag         = score()->styleD(Sid::graceNoteMag);
    qreal chordX           = (_noteType == NoteType::NORMAL) ? ipos().x() : 0.0;

    while (_ledgerLines) {
        LedgerLine* l = _ledgerLines->next();
        delete _ledgerLines;
        _ledgerLines = l;
    }

    qreal lll    = 0.0;           // space to leave at left of chord
    qreal rrr    = 0.0;           // space to leave at right of chord
    qreal lhead  = 0.0;           // amount of notehead to left of chord origin
    Note* upnote = upNote();

    delete _tabDur;     // no TAB? no duration symbol! (may happen when converting a TAB into PITCHED)
    _tabDur = 0;

    if (!segment()) {
        //
        // hack for use in palette
        //
        size_t n = _notes.size();
        for (size_t i = 0; i < n; i++) {
            Note* note = _notes.at(i);
            note->layout();
            qreal x = 0.0;
            qreal y = note->line() * _spatium * .5;
            note->setPos(x, y);
        }
        computeUp();
        layoutStem();
        addLedgerLines();
        return;
    }

    //-----------------------------------------
    //  process notes
    //-----------------------------------------

    // Keeps track if there are any accidentals in this chord.
    // Used to remove excess space in front of arpeggios.
    // See GitHub issue #8970 for more details.
    // https://github.com/musescore/MuseScore/issues/8970
    QVector<Accidental*> chordAccidentals;

    for (Note* note : _notes) {
        note->layout();

        qreal x1 = note->pos().x() + chordX;
        qreal x2 = x1 + note->headWidth();
        lll      = qMax(lll, -x1);
        rrr      = qMax(rrr, x2);
        // track amount of space due to notehead only
        lhead    = qMax(lhead, -x1);

        Accidental* accidental = note->accidental();
        if (accidental && accidental->visible()) {
            chordAccidentals.append(accidental);
        }
        if (accidental && accidental->addToSkyline() && !note->fixed()) {
            // convert x position of accidental to segment coordinate system
            qreal x = accidental->pos().x() + note->pos().x() + chordX;
            // distance from accidental to note already taken into account
            // but here perhaps we create more padding in *front* of accidental?
            x -= score()->styleMM(Sid::accidentalDistance) * mag_;
            lll = qMax(lll, -x);
        }

        // allow extra space for shortened ties
        // this code must be kept synchronized
        // with the tie positioning code in Tie::slurPos()
        // but the allocation of space needs to be performed here
        Tie* tie;
        tie = note->tieBack();
        if (tie) {
            tie->calculateDirection();
            qreal overlap = 0.0;
            bool shortStart = false;
            Note* sn = tie->startNote();
            Chord* sc = sn->chord();
            if (sc && sc->measure() == measure() && sc == prevChordRest(this)) {
                if (sc->notes().size() > 1 || (sc->stem() && sc->up() == tie->up())) {
                    shortStart = true;
                    if (sc->width() > sn->width()) {
                        // chord with second?
                        // account for noteheads further to right
                        qreal snEnd = sn->x() + sn->bboxRightPos();
                        qreal scEnd = snEnd;
                        for (unsigned i = 0; i < sc->notes().size(); ++i) {
                            scEnd = qMax(scEnd, sc->notes().at(i)->x() + sc->notes().at(i)->bboxRightPos());
                        }
                        overlap += scEnd - snEnd;
                    } else {
                        overlap -= sn->headWidth() * 0.12;
                    }
                } else {
                    overlap += sn->headWidth() * 0.35;
                }
                if (notes().size() > 1 || (stem() && !up() && !tie->up())) {
                    // for positive offset:
                    //    use available space
                    // for negative x offset:
                    //    space is allocated elsewhere, so don't re-allocate here
                    if (note->ipos().x() != 0.0) {
                        overlap += qAbs(note->ipos().x());
                    } else {
                        overlap -= note->headWidth() * 0.12;
                    }
                } else {
                    if (shortStart) {
                        overlap += note->headWidth() * 0.15;
                    } else {
                        overlap += note->headWidth() * 0.35;
                    }
                }
                qreal d = qMax(minTieLength - overlap, 0.0);
                lll = qMax(lll, d);
            }
        }

        // clear layout for note-based fingerings
        for (EngravingItem* e : note->el()) {
            if (e->isFingering()) {
                Fingering* f = toFingering(e);
                if (f->layoutType() == ElementType::NOTE) {
                    f->setPos(PointF());
                    f->setbbox(RectF());
                }
            }
        }
    }

    //-----------------------------------------
    //  create ledger lines
    //-----------------------------------------

    addLedgerLines();

    if (_arpeggio) {
        _arpeggio->layout();        // only for width() !
        _arpeggio->setHeight(0.0);

        qreal arpeggioNoteDistance = score()->styleMM(Sid::ArpeggioNoteDistance) * mag_;

        qreal gapSize = arpeggioNoteDistance;

        if (chordAccidentals.size()) {
            qreal arpeggioAccidentalDistance = score()->styleMM(Sid::ArpeggioAccidentalDistance) * mag_;
            qreal accidentalDistance = score()->styleMM(Sid::accidentalDistance) * mag_;
            gapSize = arpeggioAccidentalDistance - accidentalDistance;
            gapSize -= _arpeggio->insetDistance(chordAccidentals, mag_);
        }

        qreal extraX = _arpeggio->width() + gapSize + chordX;

        qreal y1   = upnote->pos().y() - upnote->headHeight() * .5;
        _arpeggio->setPos(-(lll + extraX), y1);
        if (_arpeggio->visible()) {
            lll += extraX;
        }
        // _arpeggio->layout() called in layoutArpeggio2()

        // handle the special case of _arpeggio->span() > 1
        // in layoutArpeggio2() after page layout has done so we
        // know the y position of the next staves
    }

    // allocate enough room for glissandi
    if (_endsGlissando) {
        for (const Note* note : notes()) {
            for (const Spanner* sp : note->spannerBack()) {
                if (sp->isGlissando()) {
                    if (toGlissando(sp)->visible()) {
                        if (!rtick().isZero()                             // if not at beginning of measure
                            || graceNotesBefore.size() > 0) {             // or there are graces before
                            lll += _spatium * 0.5 + minTieLength;
                            break;
                        }
                    }
                }
            }
        }
        // special case of system-initial glissando final note is handled in Glissando::layout() itself
    }

    if (dots()) {
        qreal x = dotPosX() + dotNoteDistance
                  + qreal(dots() - 1) * score()->styleMM(Sid::dotDotDistance) * mag_;
        x += symWidth(SymId::augmentationDot);
        rrr = qMax(rrr, x);
    }

    if (_hook) {
        if (beam()) {
            score()->undoRemoveElement(_hook);
        } else {
            _hook->layout();
            if (up() && stem()) {
                // hook position is not set yet
                qreal x = _hook->bbox().right() + stem()->flagPosition().x() + chordX;
                rrr = qMax(rrr, x);
            }
        }
    }

    _spaceLw = lll;
    _spaceRw = rrr;

    if (gnb) {
        qreal xl = -(_spaceLw + minNoteDistance) - chordX;
        for (int i = gnb - 1; i >= 0; --i) {
            Chord* g = graceNotesBefore.value(i);
            xl -= g->_spaceRw /* * 1.2*/;
            g->setPos(xl, 0);
            xl -= g->_spaceLw + minNoteDistance * graceMag;
        }
        if (-xl > _spaceLw) {
            _spaceLw = -xl;
        }
    }
    if (!gna.empty()) {
        qreal xr = _spaceRw;
        int n = gna.size();
        for (int i = 0; i <= n - 1; i++) {
            Chord* g = gna.value(i);
            xr += g->_spaceLw + g->_spaceRw + minNoteDistance * graceMag;
        }
        if (xr > _spaceRw) {
            _spaceRw = xr;
        }
    }

    for (EngravingItem* e : el()) {
        if (e->type() == ElementType::SLUR) {       // we cannot at this time as chordpositions are not fixed
            continue;
        }
        e->layout();
        if (e->type() == ElementType::CHORDLINE) {
            RectF tbbox = e->bbox().translated(e->pos());
            qreal lx = tbbox.left() + chordX;
            qreal rx = tbbox.right() + chordX;
            if (-lx > _spaceLw) {
                _spaceLw = -lx;
            }
            if (rx > _spaceRw) {
                _spaceRw = rx;
            }
        }
    }

    for (Note* note : _notes) {
        note->layout2();
    }

    // align note-based fingerings
    std::vector<Fingering*> alignNote;
    qreal xNote = 10000.0;
    for (Note* note : _notes) {
        bool leftFound = false;
        for (EngravingItem* e : note->el()) {
            if (e->isFingering() && e->autoplace()) {
                Fingering* f = toFingering(e);
                if (f->layoutType() == ElementType::NOTE && f->textStyleType() == TextStyleType::LH_GUITAR_FINGERING) {
                    alignNote.push_back(f);
                    if (!leftFound) {
                        leftFound = true;
                        qreal xf = f->ipos().x();
                        xNote = qMin(xNote, xf);
                    }
                }
            }
        }
    }
    for (Fingering* f : alignNote) {
        f->rxpos() = xNote;
    }
}

//---------------------------------------------------------
//   layoutTablature
//---------------------------------------------------------

void Chord::layoutTablature()
{
    qreal _spatium          = spatium();
    qreal mag_ = staff() ? staff()->staffMag(this) : 1.0;    // palette elements do not have a staff
    qreal dotNoteDistance = score()->styleMM(Sid::dotNoteDistance) * mag_;
    qreal minNoteDistance = score()->styleMM(Sid::minNoteDistance) * mag_;
    qreal minTieLength = score()->styleMM(Sid::MinTieLength) * mag_;

    for (Chord* c : qAsConst(_graceNotes)) {
        c->layoutTablature();
    }

    while (_ledgerLines) {
        LedgerLine* l = _ledgerLines->next();
        delete _ledgerLines;
        _ledgerLines = l;
    }

    qreal lll         = 0.0;                    // space to leave at left of chord
    qreal rrr         = 0.0;                    // space to leave at right of chord
    Note* upnote      = upNote();
    qreal headWidth   = symWidth(SymId::noteheadBlack);
    const Staff* st   = staff();
    const StaffType* tab = st->staffTypeForElement(this);
    qreal lineDist    = tab->lineDistance().val() * _spatium;
    qreal stemX       = tab->chordStemPosX(this) * _spatium;
    int ledgerLines = 0;
    qreal llY         = 0.0;

    size_t numOfNotes = _notes.size();
    qreal minY        = 1000.0;                 // just a very large value
    for (size_t i = 0; i < numOfNotes; ++i) {
        Note* note = _notes.at(i);
        note->layout();
        // set headWidth to max fret text width
        qreal fretWidth = note->bbox().width();
        if (headWidth < fretWidth) {
            headWidth = fretWidth;
        }
        // centre fret string on stem
        qreal x = stemX - fretWidth * 0.5;
        qreal y = note->fixed() ? note->line() * lineDist / 2 : tab->physStringToYOffset(note->string()) * _spatium;
        note->setPos(x, y);
        if (y < minY) {
            minY  = y;
        }
        int currLedgerLines   = tab->numOfTabLedgerLines(note->string());
        if (currLedgerLines > ledgerLines) {
            ledgerLines = currLedgerLines;
            llY         = y;
        }

        // allow extra space for shortened ties; this code must be kept synchronized
        // with the tie positioning code in Tie::slurPos()
        // but the allocation of space needs to be performed here
        Tie* tie;
        tie = note->tieBack();
        if (tie) {
            tie->calculateDirection();
            qreal overlap = 0.0;                // how much tie can overlap start and end notes
            bool shortStart = false;            // whether tie should clear start note or not
            Note* startNote = tie->startNote();
            Chord* startChord = startNote->chord();
            if (startChord && startChord->measure() == measure() && startChord == prevChordRest(this)) {
                qreal startNoteWidth = startNote->width();
                // overlap into start chord?
                // if in start chord, there are several notes or stem and tie in same direction
                if (startChord->notes().size() > 1 || (startChord->stem() && startChord->up() == tie->up())) {
                    // clear start note (1/8 of fret mark width)
                    shortStart = true;
                    overlap -= startNoteWidth * 0.125;
                } else {            // overlap start note (by ca. 1/3 of fret mark width)
                    overlap += startNoteWidth * 0.35;
                }
                // overlap into end chord (this)?
                // if several notes or neither stem or tie are up
                if (notes().size() > 1 || (stem() && !up() && !tie->up())) {
                    // for positive offset:
                    //    use available space
                    // for negative x offset:
                    //    space is allocated elsewhere, so don't re-allocate here
                    if (note->ipos().x() != 0.0) {                      // this probably does not work for TAB, as
                        overlap += qAbs(note->ipos().x());              // _pos is used to centre the fret on the stem
                    } else {
                        overlap -= fretWidth * 0.125;
                    }
                } else {
                    if (shortStart) {
                        overlap += fretWidth * 0.15;
                    } else {
                        overlap += fretWidth * 0.35;
                    }
                }
                qreal d = qMax(minTieLength - overlap, 0.0);
                lll = qMax(lll, d);
            }
        }
    }

    // create ledger lines, if required (in some historic styles)
    if (ledgerLines > 0) {
// there seems to be no need for widening 'ledger lines' beyond fret mark widths; more 'on the field'
// tests and usage will show if this depends on the metrics of the specific fonts used or not.
//            qreal extraLen    = score()->styleS(Sid::ledgerLineLength).val() * _spatium;
        qreal extraLen    = 0;
        qreal llX         = stemX - (headWidth + extraLen) * 0.5;
        for (int i = 0; i < ledgerLines; i++) {
            LedgerLine* ldgLin = new LedgerLine(score());
            ldgLin->setParent(this);
            ldgLin->setTrack(track());
            ldgLin->setVisible(visible());
            ldgLin->setLen(headWidth + extraLen);
            ldgLin->setPos(llX, llY);
            ldgLin->setNext(_ledgerLines);
            _ledgerLines = ldgLin;
            ldgLin->layout();
            llY += lineDist / ledgerLines;
        }
        headWidth += extraLen;            // include ledger lines extra width in chord width
    }

    // horiz. spacing: leave half width at each side of the (potential) stem
    qreal halfHeadWidth = headWidth * 0.5;
    if (lll < stemX - halfHeadWidth) {
        lll = stemX - halfHeadWidth;
    }
    if (rrr < stemX + halfHeadWidth) {
        rrr = stemX + halfHeadWidth;
    }
    // align dots to the widest fret mark (not needed in all TAB styles, but harmless anyway)
    if (segment()) {
        segment()->setDotPosX(staffIdx(), headWidth);
    }

    if (shouldHaveStem()) {
        // if stem is required but missing, add it;
        // set stem position (stem length is set in Chord:layoutStem() )
        if (!_stem) {
            Stem* stem = Factory::createStem(this);
            stem->setParent(this);
            stem->setGenerated(true);
            score()->undo(new AddElement(stem));
        }
        _stem->setPos(tab->chordStemPos(this) * _spatium);
        if (_hook) {
            if (beam()) {
                score()->undoRemoveElement(_hook);
            } else {
                if (rrr < stemX + _hook->width()) {
                    rrr = stemX + _hook->width();
                }

                _hook->setPos(_stem->flagPosition());
            }
        }
    } else {
        if (_stem) {
            score()->undo(new RemoveElement(_stem));
            remove(_stem);
        }
        if (_hook) {
            score()->undo(new RemoveElement(_hook));
            remove(_hook);
        }
        if (_beam) {
            score()->undo(new RemoveElement(_beam));
            remove(_beam);
        }
    }

    if (!tab->genDurations()                           // if tab is not set for duration symbols
        || track2voice(track())                        // or not in first voice
        || (isGrace()                                  // no tab duration symbols if grace notes
            && beamMode() == BeamMode::AUTO)) {      // and beammode == AUTO
        //
        delete _tabDur;       // delete an existing duration symbol
        _tabDur = 0;
    } else {
        //
        // tab duration symbols
        //
        // if no previous CR
        // OR symbol repeat set to ALWAYS
        // OR symbol repeat condition is triggered
        // OR duration type and/or number of dots is different from current CR
        // OR chord beam mode not AUTO
        // OR previous CR is a rest
        // AND no not-stem
        // set a duration symbol (trying to re-use existing symbols where existing to minimize
        // symbol creation and deletion)
        bool needTabDur = false;
        bool repeat = false;
        if (!noStem()) {
            // check duration of prev. CR segm
            ChordRest* prevCR = prevChordRest(this);
            if (prevCR == 0) {
                needTabDur = true;
            } else if (beamMode() != BeamMode::AUTO
                       || prevCR->durationType().type() != durationType().type()
                       || prevCR->dots() != dots()
                       || prevCR->tuplet() != tuplet()
                       || prevCR->type() == ElementType::REST) {
                needTabDur = true;
            } else if (tab->symRepeat() == TablatureSymbolRepeat::ALWAYS
                       || ((tab->symRepeat() == TablatureSymbolRepeat::MEASURE
                            || tab->symRepeat() == TablatureSymbolRepeat::SYSTEM)
                           && measure() != prevCR->measure())) {
                needTabDur = true;
                repeat = true;
            }
        }
        if (needTabDur) {
            // symbol needed; if not exist, create; if exists, update duration
            if (!_tabDur) {
                _tabDur = new TabDurationSymbol(this, tab, durationType().type(), dots());
            } else {
                _tabDur->setDuration(durationType().type(), dots(), tab);
            }
            _tabDur->setParent(this);
            _tabDur->setRepeat(repeat);
//                  _tabDur->setMag(mag());           // useless to set grace mag: graces have no dur. symbol
            _tabDur->layout();
            if (minY < 0) {                           // if some fret extends above tab body (like bass strings)
                _tabDur->rypos() += minY;             // raise duration symbol
                _tabDur->bbox().translate(0, minY);
            }
        } else {                                // symbol not needed: if exists, delete
            delete _tabDur;
            _tabDur = 0;
        }
    }                                     // end of if(duration_symbols)

    if (_arpeggio) {
        qreal headHeight = upnote->headHeight();
        _arpeggio->layout();
        lll += _arpeggio->width() + _spatium * .5;
        qreal y = upNote()->pos().y() - headHeight * .5;
        qreal h = downNote()->pos().y() + downNote()->headHeight() - y;
        _arpeggio->setHeight(h);
        _arpeggio->setPos(-lll, y);

        // handle the special case of _arpeggio->span() > 1
        // in layoutArpeggio2() after page layout has done so we
        // know the y position of the next staves
    }

    // allocate enough room for glissandi
    if (_endsGlissando) {
        if (!rtick().isZero()) {                          // if not at beginning of measure
            lll += _spatium * 0.5 + minTieLength;
        }
        // special case of system-initial glissando final note is handled in Glissando::layout() itself
    }

    if (_hook) {
        if (beam()) {
            score()->undoRemoveElement(_hook);
        } else if (tab == 0) {
            _hook->layout();
            if (up()) {
                // hook position is not set yet
                qreal x = _hook->bbox().right() + stem()->flagPosition().x();
                rrr = qMax(rrr, x);
            }
        }
    }

    if (dots()) {
        qreal x = 0.0;
        // if stems are beside staff, dots are placed near to stem
        if (!tab->stemThrough()) {
            // if there is an unbeamed hook, dots should start after the hook
            if (_hook && !beam()) {
                x = _hook->width() + dotNoteDistance;
            }
            // if not, dots should start at a fixed distance right after the stem
            else {
                x = STAFFTYPE_TAB_DEFAULTDOTDIST_X * _spatium;
            }
            if (segment()) {
                segment()->setDotPosX(staffIdx(), x);
            }
        }
        // if stems are through staff, use dot position computed above on fret mark widths
        else {
            x = dotPosX() + dotNoteDistance
                + (dots() - 1) * score()->styleS(Sid::dotDotDistance).val() * _spatium;
        }
        x += symWidth(SymId::augmentationDot);
        rrr = qMax(rrr, x);
    }

    _spaceLw = lll;
    _spaceRw = rrr;

    qreal graceMag = score()->styleD(Sid::graceNoteMag);

    QVector<Chord*> graceNotesBefore = Chord::graceNotesBefore();
    int nb = graceNotesBefore.size();
    if (nb) {
        qreal xl = -(_spaceLw + minNoteDistance);
        for (int i = nb - 1; i >= 0; --i) {
            Chord* c = graceNotesBefore.value(i);
            xl -= c->_spaceRw /* * 1.2*/;
            c->setPos(xl, 0);
            xl -= c->_spaceLw + minNoteDistance * graceMag;
        }
        if (-xl > _spaceLw) {
            _spaceLw = -xl;
        }
    }
    QVector<Chord*> gna = graceNotesAfter();
    int na = gna.size();
    if (na) {
        // get factor for start distance after main note. Values found by testing.
        qreal fc;
        switch (durationType().type()) {
        case DurationType::V_LONG:    fc = 3.8;
            break;
        case DurationType::V_BREVE:   fc = 3.8;
            break;
        case DurationType::V_WHOLE:   fc = 3.8;
            break;
        case DurationType::V_HALF:    fc = 3.6;
            break;
        case DurationType::V_QUARTER: fc = 2.1;
            break;
        case DurationType::V_EIGHTH:  fc = 1.4;
            break;
        case DurationType::V_16TH:    fc = 1.2;
            break;
        default: fc = 1;
        }
        qreal xr = fc * (_spaceRw + minNoteDistance);
        for (int i = 0; i <= na - 1; i++) {
            Chord* c = gna.value(i);
            xr += c->_spaceLw * (i == 0 ? 1.3 : 1);
            c->setPos(xr, 0);
            xr += c->_spaceRw + minNoteDistance * graceMag;
        }
        if (xr > _spaceRw) {
            _spaceRw = xr;
        }
    }
    for (EngravingItem* e : el()) {
        e->layout();
        if (e->type() == ElementType::CHORDLINE) {
            RectF tbbox = e->bbox().translated(e->pos());
            qreal lx = tbbox.left();
            qreal rx = tbbox.right();
            if (-lx > _spaceLw) {
                _spaceLw = -lx;
            }
            if (rx > _spaceRw) {
                _spaceRw = rx;
            }
        }
    }

    for (size_t i = 0; i < numOfNotes; ++i) {
        _notes.at(i)->layout2();
    }
    RectF bb;
    processSiblings([&bb](EngravingItem* e) { bb.unite(e->bbox().translated(e->pos())); });
    if (_tabDur) {
        bb.unite(_tabDur->bbox().translated(_tabDur->pos()));
    }
    setbbox(bb);
}

//---------------------------------------------------------
//   crossMeasureSetup
//---------------------------------------------------------

void Chord::crossMeasureSetup(bool on)
{
    if (!on) {
        if (_crossMeasure != CrossMeasure::UNKNOWN) {
            _crossMeasure = CrossMeasure::UNKNOWN;
            layoutStem();
        }
        return;
    }
    if (_crossMeasure == CrossMeasure::UNKNOWN) {
        CrossMeasure tempCross = CrossMeasure::NONE;      // assume no cross-measure modification
        // if chord has only one note and note is tied forward
        if (notes().size() == 1 && _notes[0]->tieFor()) {
            Chord* tiedChord = _notes[0]->tieFor()->endNote()->chord();
            // if tied note belongs to another measure and to a single-note chord
            if (tiedChord->measure() != measure() && tiedChord->notes().size() == 1) {
                // get total duration
                std::vector<TDuration> durList = toDurationList(
                    actualDurationType().fraction()
                    + tiedChord->actualDurationType().fraction(), true);
                // if duration can be expressed as a single duration
                // apply cross-measure modification
                if (durList.size() == 1) {
                    _crossMeasure = tempCross = CrossMeasure::FIRST;
                    _crossMeasureTDur = durList[0];
                    layoutStem();
                }
            }
            _crossMeasure = tempCross;
            tiedChord->setCrossMeasure(tempCross == CrossMeasure::FIRST
                                       ? CrossMeasure::SECOND : CrossMeasure::NONE);
        }
    }
}

//---------------------------------------------------------
//   layoutArpeggio2
//    called after layout of page
//---------------------------------------------------------

void Chord::layoutArpeggio2()
{
    if (!_arpeggio) {
        return;
    }
    qreal y           = upNote()->pagePos().y() - upNote()->headHeight() * .5;
    int span          = _arpeggio->span();
    int btrack        = track() + (span - 1) * VOICES;

    EngravingItem* element = segment()->element(btrack);
    ChordRest* bchord = element ? toChordRest(element) : nullptr;
    Note* dnote       = (bchord && bchord->type() == ElementType::CHORD) ? toChord(bchord)->downNote() : downNote();

    qreal h = dnote->pagePos().y() + dnote->headHeight() * .5 - y;
    _arpeggio->setHeight(h);
    _arpeggio->layout();
}

//---------------------------------------------------------
//   layoutNotesSpanners
//---------------------------------------------------------

void Chord::layoutSpanners()
{
    for (const Note* n : notes()) {
        Tie* tie = n->tieFor();
        if (tie) {
            tie->layout();
        }
        for (Spanner* sp : n->spannerBack()) {
            sp->layout();
        }
    }
}

void Chord::layoutSpanners(System* system, const Fraction& stick)
{
    //! REVIEW Needs explanation
    for (const Note* note : notes()) {
        Tie* t = note->tieFor();
        if (t) {
            t->layoutFor(system);
        }
        t = note->tieBack();
        if (t) {
            if (t->startNote()->tick() < stick) {
                t->layoutBack(system);
            }
        }
        for (Spanner* sp : note->spannerBack()) {
            sp->layout();
        }
    }
}

//---------------------------------------------------------
//   isChordPlayable
//   @note Now every related to chord element has it's own "PLAY" property,
//         However, there is no way to control these properties outside the scope of the chord since the new inspector.
//         So we'll use a chord as a proxy entity for "PLAY" property handling
//---------------------------------------------------------

bool Chord::isChordPlayable() const
{
    if (!_notes.empty()) {
        return _notes.front()->getProperty(Pid::PLAY).toBool();
    } else if (_tremolo) {
        return _tremolo->getProperty(Pid::PLAY).toBool();
    } else if (_arpeggio) {
        return _arpeggio->getProperty(Pid::PLAY).toBool();
    }

    return false;
}

//---------------------------------------------------------
//   setIsChordPlayable
//---------------------------------------------------------

void Chord::setIsChordPlayable(const bool isPlayable)
{
    for (Note* note : _notes) {
        note->undoChangeProperty(Pid::PLAY, isPlayable);
    }

    if (_arpeggio) {
        _arpeggio->undoChangeProperty(Pid::PLAY, isPlayable);
    }

    if (_tremolo) {
        _tremolo->undoChangeProperty(Pid::PLAY, isPlayable);
    }

    triggerLayout();
}

//---------------------------------------------------------
//   findNote
//---------------------------------------------------------

Note* Chord::findNote(int pitch, int skip) const
{
    size_t ns = _notes.size();
    for (size_t i = 0; i < ns; ++i) {
        Note* n = _notes.at(i);
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
            atr->setParent(this);
            atr->setTrack(track());
            score()->undoAddElement(atr);
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
                qDebug("no segment for second note of tremolo found");
                delete e;
                return 0;
            }
            Chord* ch2 = toChord(s->element(track()));
            if (ch2->ticks() != ticks()) {
                qDebug("no matching chord for second note of tremolo found");
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
        a->setHeight(spatium() * 5);             //DEBUG
        score()->undoAddElement(a);
    }
        return e;

    default:
        return ChordRest::drop(data);
    }
    return 0;
}

//---------------------------------------------------------
//   dotPosX
//---------------------------------------------------------

void Chord::setColor(const mu::draw::Color& color)
{
    ChordRest::setColor(color);

    for (Note* note : _notes) {
        note->undoChangeProperty(Pid::COLOR, PropertyValue::fromValue(color));
    }
}

//---------------------------------------------------------
//   dotPosX
//---------------------------------------------------------

qreal Chord::dotPosX() const
{
    if (explicitParent()) {
        return segment()->dotPosX(staffIdx());
    }
    return -1000.0;
}

//---------------------------------------------------------
//   setStemDirection
//---------------------------------------------------------

void Chord::setStemDirection(DirectionV d)
{
    _stemDirection = d;
}

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void Chord::localSpatiumChanged(qreal oldValue, qreal newValue)
{
    ChordRest::localSpatiumChanged(oldValue, newValue);
    for (EngravingItem* e : graceNotes()) {
        e->localSpatiumChanged(oldValue, newValue);
    }
    if (_hook) {
        _hook->localSpatiumChanged(oldValue, newValue);
    }
    if (_stem) {
        _stem->localSpatiumChanged(oldValue, newValue);
    }
    if (_stemSlash) {
        _stemSlash->localSpatiumChanged(oldValue, newValue);
    }
    if (arpeggio()) {
        arpeggio()->localSpatiumChanged(oldValue, newValue);
    }
    if (_tremolo && (tremoloChordType() != TremoloChordType::TremoloSecondNote)) {
        _tremolo->localSpatiumChanged(oldValue, newValue);
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
    for (Articulation* a : qAsConst(_articulations)) {
        if (a->subtype() == aa->subtype()) {
            return a;
        }
    }
    return 0;
}

void Chord::updateArticulations(const std::set<SymId>& newArticulationIds, ArticulationsUpdateMode updateMode)
{
    std::set<SymId> currentArticulationIds;
    for (Articulation* artic: _articulations) {
        currentArticulationIds.insert(artic->symId());
        score()->undoRemoveElement(artic);
    }

    std::set<SymId> articulationIds = flipArticulations(currentArticulationIds, PlacementV::ABOVE);
    articulationIds = splitArticulations(articulationIds);

    std::set<SymId> _newArticulationIds = flipArticulations(newArticulationIds, PlacementV::ABOVE);
    _newArticulationIds = splitArticulations(_newArticulationIds);

    for (const SymId& articulationId: _newArticulationIds) {
        articulationIds = Ms::updateArticulations(articulationIds, articulationId, updateMode);
    }

    std::set<SymId> result = joinArticulations(articulationIds);
    for (const SymId& articulationSymbolId: result) {
        Articulation* newArticulation = Factory::createArticulation(score()->dummy()->chord());
        newArticulation->setSymId(articulationSymbolId);
        score()->toggleArticulation(this, newArticulation);
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Chord::reset()
{
    undoChangeProperty(Pid::STEM_DIRECTION, DirectionV::AUTO);
    score()->createPlayEvents(this);
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
        for (Note* n : _notes) {
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
        qreal y = 0.0;
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
            rypos() += y;
        } else {
            head = NoteHeadGroup::HEAD_NORMAL;
        }
    }

    size_t ns = _notes.size();
    for (size_t i = 0; i < ns; ++i) {
        Note* n = _notes[i];
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

void Chord::updateEndsGlissando()
{
    _endsGlissando = false;         // assume no glissando ends here
    // scan all chord notes for glissandi ending on this chord
    for (Note* note : notes()) {
        for (Spanner* sp : note->spannerBack()) {
            if (sp->type() == ElementType::GLISSANDO) {
                _endsGlissando = true;
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
    qDeleteAll(graceNotes());
    graceNotes().clear();
    qDeleteAll(articulations());
    articulations().clear();
    for (Note* n : notes()) {
        for (EngravingItem* e : n->el()) {
            n->remove(e);
        }
    }
    ChordRest::removeMarkings(keepTremolo);
}

//---------------------------------------------------------
//   chordMag
//---------------------------------------------------------

qreal Chord::chordMag() const
{
    qreal m = 1.0;
    if (isSmall()) {
        m *= score()->styleD(Sid::smallNoteMag);
    }
    if (_noteType != NoteType::NORMAL) {
        m *= score()->styleD(Sid::graceNoteMag);
    }
    return m;
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Chord::mag() const
{
    const Staff* st = staff();
    return (st ? st->staffMag(this) : 1.0) * chordMag();
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

QVector<Chord*> Chord::graceNotesBefore() const
{
    QVector<Chord*> cl;
    for (Chord* c : _graceNotes) {
        Q_ASSERT(c->noteType() != NoteType::NORMAL && c->noteType() != NoteType::INVALID);
        if (c->noteType() & (
                NoteType::ACCIACCATURA
                | NoteType::APPOGGIATURA
                | NoteType::GRACE4
                | NoteType::GRACE16
                | NoteType::GRACE32)) {
            cl.push_back(c);
        }
    }
    return cl;
}

//---------------------------------------------------------
//   graceNotesAfter
//---------------------------------------------------------

QVector<Chord*> Chord::graceNotesAfter() const
{
    QVector<Chord*> cl;
    for (int i = _graceNotes.size() - 1; i >= 0; i--) {
        Chord* c = _graceNotes[i];
        Q_ASSERT(c->noteType() != NoteType::NORMAL && c->noteType() != NoteType::INVALID);
        if (c->noteType() & (NoteType::GRACE8_AFTER | NoteType::GRACE16_AFTER | NoteType::GRACE32_AFTER)) {
            cl.push_back(c);
        }
    }
    return cl;
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
    for (Note* n : _notes) {
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

//---------------------------------------------------------
//   tremoloChordType
//---------------------------------------------------------

TremoloChordType Chord::tremoloChordType() const
{
    if (_tremolo && _tremolo->twoNotes()) {
        if (_tremolo->chord1() == this) {
            return TremoloChordType::TremoloFirstNote;
        } else if (_tremolo->chord2() == this) {
            return TremoloChordType::TremoloSecondNote;
        } else {
            qFatal("Chord::tremoloChordType(): inconsistency %p - %p, this is %p", _tremolo->chord1(), _tremolo->chord2(), this);
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
    if (!e && !score()->selection().elements().isEmpty()) {
        e = score()->selection().elements().first();
    }

    switch (e->type()) {
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
    case ElementType::FINGERING:
    case ElementType::TEXT:
    case ElementType::BEND: {
        Note* n = toNote(e->explicitParent());
        if (n == _notes.front()) {
            if (_arpeggio) {
                return _arpeggio;
            } else if (_tremolo) {
                return _tremolo;
            }
            break;
        }
        for (auto& i : _notes) {
            if (i == n) {
                return *(&i - 1);
            }
        }
        break;
    }

    case ElementType::GLISSANDO_SEGMENT:
    case ElementType::TIE_SEGMENT: {
        SpannerSegment* s = toSpannerSegment(e);
        Spanner* sp = s->spanner();
        EngravingItem* elSt = sp->startElement();
        Q_ASSERT(elSt->type() == ElementType::NOTE);
        Note* n = toNote(elSt);
        Q_ASSERT(n != NULL);
        if (n == _notes.front()) {
            if (_arpeggio) {
                return _arpeggio;
            } else if (_tremolo) {
                return _tremolo;
            }
            break;
        }
        for (auto& i : _notes) {
            if (i == n) {
                return *(&i - 1);
            }
        }
        break;
    }
    case ElementType::ARPEGGIO:
        if (_tremolo) {
            return _tremolo;
        }
        break;

    case ElementType::ACCIDENTAL:
        e = e->parentItem();
    // fall through

    case ElementType::NOTE: {
        if (e == _notes.front()) {
            if (_arpeggio) {
                return _arpeggio;
            } else if (_tremolo) {
                return _tremolo;
            }
            break;
        }
        for (auto& i : _notes) {
            if (i == e) {
                return *(&i - 1);
            }
        }
    }
    break;

    case ElementType::CHORD:
        return _notes.back();

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
    if (!e && !score()->selection().elements().isEmpty()) {
        e = score()->selection().elements().last();
    }
    switch (e->type()) {
    case ElementType::NOTE: {
        if (e == _notes.back()) {
            break;
        }
        Note* prevNote = nullptr;
        for (auto& i : _notes) {
            if (i == e) {
                prevNote = *(&i + 1);
            }
        }
        EngravingItem* next = prevNote->lastElementBeforeSegment();
        return next;
    }

    case ElementType::CHORD:
        return _notes.front();

    case ElementType::TREMOLO:
        if (_arpeggio) {
            return _arpeggio;
        }
    // fall through

    case ElementType::ARPEGGIO: {
        Note* n = _notes.front();
        EngravingItem* elN = n->lastElementBeforeSegment();
        Q_ASSERT(elN != NULL);
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
    if (_tremolo) {
        return _tremolo;
    } else if (_arpeggio) {
        return _arpeggio;
    } else {
        Note* n = _notes.front();
        EngravingItem* elN = n->lastElementBeforeSegment();
        Q_ASSERT(elN != NULL);
        return elN;
    }
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* Chord::nextSegmentElement()
{
    for (int v = track() + 1; staffIdx() == v / VOICES; ++v) {
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
    if (!el && !score()->selection().elements().isEmpty()) {
        el = score()->selection().elements().first();
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

QString Chord::accessibleExtraInfo() const
{
    QString rez = "";

    for (const Chord* c : graceNotes()) {
        if (!score()->selectionFilter().canSelect(c)) {
            continue;
        }
        for (const Note* n : c->notes()) {
            rez = QString("%1 %2").arg(rez, n->screenReaderInfo());
        }
    }

    for (Articulation* a : articulations()) {
        if (!score()->selectionFilter().canSelect(a)) {
            continue;
        }
        rez = QString("%1 %2").arg(rez, a->screenReaderInfo());
    }

    if (arpeggio() && score()->selectionFilter().canSelect(arpeggio())) {
        rez = QString("%1 %2").arg(rez, arpeggio()->screenReaderInfo());
    }

    if (tremolo() && score()->selectionFilter().canSelect(tremolo())) {
        rez = QString("%1 %2").arg(rez, tremolo()->screenReaderInfo());
    }

    foreach (EngravingItem* e, el()) {
        if (!score()->selectionFilter().canSelect(e)) {
            continue;
        }
        rez = QString("%1 %2").arg(rez, e->screenReaderInfo());
    }

    return QString("%1 %2").arg(rez, ChordRest::accessibleExtraInfo());
}

//---------------------------------------------------------
//   shape
//    does not contain articulations
//---------------------------------------------------------

Shape Chord::shape() const
{
    Shape shape;
    if (_hook && _hook->addToSkyline()) {
        shape.add(_hook->shape().translated(_hook->pos()));
    }
    if (_stem && _stem->addToSkyline()) {
        // stem direction is not known soon enough for cross staff beamed notes
        if (!(beam() && (staffMove() || beam()->cross()))) {
            shape.add(_stem->shape().translated(_stem->pos()));
        }
    }
    if (_stemSlash && _stemSlash->addToSkyline()) {
        shape.add(_stemSlash->shape().translated(_stemSlash->pos()));
    }
    if (_arpeggio && _arpeggio->addToSkyline()) {
        shape.add(_arpeggio->shape().translated(_arpeggio->pos()));
    }
//      if (_tremolo)
//            shape.add(_tremolo->shape().translated(_tremolo->pos()));
    for (Note* note : _notes) {
        shape.add(note->shape().translated(note->pos()));
        for (EngravingItem* e : note->el()) {
            if (!e->addToSkyline()) {
                continue;
            }
            if (e->isFingering() && toFingering(e)->layoutType() == ElementType::CHORD && e->bbox().isValid()) {
                shape.add(e->bbox().translated(e->pos() + note->pos()));
            }
        }
    }
    for (EngravingItem* e : el()) {
        if (e->addToSkyline()) {
            shape.add(e->shape().translated(e->pos()));
        }
    }
    for (Chord* chord : _graceNotes) {    // process grace notes last, needed for correct shape calculation
        shape.add(chord->shape().translated(chord->pos()));
    }
    shape.add(ChordRest::shape());      // add lyrics
    for (LedgerLine* l = _ledgerLines; l; l = l->next()) {
        shape.add(l->shape().translated(l->pos()));
    }
    if (_spaceLw || _spaceRw) {
        shape.addHorizontalSpacing(Shape::SPACING_GENERAL, -_spaceLw, _spaceRw);
    }
    return shape;
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
        });
    }

    EngravingItem::undoChangeProperty(id, newValue, ps);
}

//---------------------------------------------------------
//   layoutArticulations
//    layout tenuto and staccato
//    called before layouting slurs
//---------------------------------------------------------

void Chord::layoutArticulations()
{
    for (Chord* gc : graceNotes()) {
        gc->layoutArticulations();
    }

    if (_articulations.empty()) {
        return;
    }
    const Staff* st = staff();
    const StaffType* staffType = st->staffTypeForElement(this);
    qreal mag            = (staffType->isSmall() ? score()->styleD(Sid::smallStaffMag) : 1.0) * staffType->userMag();
    qreal _spatium       = score()->spatium() * mag;
    qreal _spStaff       = _spatium * staffType->lineDistance().val();

    //
    //    determine direction
    //    place tenuto and staccato
    //

    Articulation* prevArticulation = nullptr;
    for (Articulation* a : qAsConst(_articulations)) {
        if (a->anchor() == ArticulationAnchor::CHORD) {
            if (measure()->hasVoices(a->staffIdx(), tick(), actualTicks())) {
                a->setUp(up());         // if there are voices place articulation at stem
            } else if (a->symId() >= SymId::articMarcatoAbove && a->symId() <= SymId::articMarcatoTenutoBelow) {
                a->setUp(true);         // Gould, p. 117: strong accents above staff
            } else if (isGrace() && up() && !a->layoutCloseToNote() && downNote()->line() < 6) {
                a->setUp(true);         // keep articulation close to grace note
            } else {
                a->setUp(!up());         // place articulation at note head
            }
        } else {
            a->setUp(a->anchor() == ArticulationAnchor::TOP_STAFF || a->anchor() == ArticulationAnchor::TOP_CHORD);
        }

        if (!a->layoutCloseToNote()) {
            continue;
        }

        bool bottom = !a->up();      // true: articulation is below chord;  false: articulation is above chord
        a->layout();                 // must be done after assigning direction, or else symId is not reliable

        bool headSide = bottom == up();
        qreal x = centerX();
        qreal y = 0.0;
        const qreal halfBeamSp = score()->styleS(Sid::beamWidth).val() * score()->spatium() * 0.5;
        if (bottom) {
            if (!headSide && stem()) {
                auto userLen = stem()->userLength();
                if (_up) {
                    y = downPos() - stem()->length() - userLen;
                    if (beam()) {
                        y -= halfBeamSp * beam()->mag();
                    }
                } else {
                    y = upPos() + stem()->length() - userLen;
                    if (beam()) {
                        y += halfBeamSp * beam()->mag();
                    }
                }
                int line   = lrint((y + 0.5 * _spStaff) / _spStaff);
                if (line < staffType->lines()) {        // align between staff lines
                    y = line * _spStaff + _spatium * .5;
                } else {
                    y += _spatium;
                }
                if (a->isStaccato() && articulations().size() == 1) {
                    if (_up) {
                        x = downNote()->bboxRightPos() - stem()->width() * .5;
                    } else {
                        x = stem()->width() * .5;
                    }
                }
            } else {
                int line = downLine();
                int lines = (staffType->lines() - 1) * 2;
                if (line < lines) {
                    y = ((line & ~1) + 3) * _spStaff;
                } else {
                    y = line * _spStaff + 2 * _spatium;
                }
                y *= .5;
            }
            if (prevArticulation && (prevArticulation->up() == a->up())) {
                y += _spatium;
            }
            y -= a->height() * .5;              // center symbol
        } else {
            if (!headSide && stem()) {
                auto userLen = stem()->userLength();
                if (_up) {
                    y = downPos() - stem()->length() + userLen;
                    if (beam()) {
                        y -= halfBeamSp * beam()->mag();
                    }
                } else {
                    y = upPos() + stem()->length() + userLen;
                    if (beam()) {
                        y += halfBeamSp * beam()->mag();
                    }
                }
                int line   = lrint((y - 0.5 * _spStaff) / _spStaff);
                if (line >= 0) {        // align between staff lines
                    y = line * _spStaff - _spatium * .5;
                } else {
                    y -= _spatium;
                }
                if (a->isStaccato() && articulations().size() == 1) {
                    if (_up) {
                        x = downNote()->bboxRightPos() - stem()->width() * .5;
                    } else {
                        x = stem()->width() * .5;
                    }
                }
            } else {
                int line = upLine();
                if (line > 0) {
                    y = (((line + 1) & ~1) - 3) * _spStaff;
                } else {
                    y = line * _spStaff - 2 * _spatium;
                }
                y *= .5;
            }
            if (prevArticulation && (prevArticulation->up() == a->up())) {
                y -= _spatium;
            }
            y += a->height() * .5;              // center symbol
        }
        a->setPos(x, y);
        prevArticulation = a;
//            measure()->system()->staff(a->staffIdx())->skyline().add(a->shape().translated(a->pos() + segment()->pos() + measure()->pos()));
    }
}

//---------------------------------------------------------
//   layoutArticulations2
//    Called after layouting systems
//    Tentatively layout all articulations
//    To be finished after laying out slurs
//---------------------------------------------------------

void Chord::layoutArticulations2()
{
    for (Chord* gc : graceNotes()) {
        gc->layoutArticulations2();
    }

    if (_articulations.empty()) {
        return;
    }
    qreal _spatium  = spatium();
    qreal x         = centerX();
    qreal distance0 = score()->styleMM(Sid::propertyDistance);
    qreal distance2 = score()->styleMM(Sid::propertyDistanceStem);

    qreal chordTopY = upPos();      // note position of highest note
    qreal chordBotY = downPos();    // note position of lowest note

    qreal staffTopY = -distance2;
    qreal staffBotY = staff()->height() + distance2;

    // avoid collisions of staff articulations with chord notes:
    // gap between note and staff articulation is distance0 + 0.5 spatium

    if (stem()) {
        qreal y = stem()->pos().y() + pos().y() + stem()->length();
        if (beam()) {
            qreal bw = score()->styleS(Sid::beamWidth).val() * _spatium;
            y += up() ? -bw : bw;
        }
        if (up()) {
            chordTopY = y;
        } else {
            chordBotY = y;
        }
    }

    //
    //    place all articulations with anchor at chord/rest
    //
    qreal distance1 = score()->styleMM(Sid::propertyDistanceHead);
    chordTopY -= up() ? 0.5 * _spatium : distance1;
    chordBotY += up() ? distance1 : 0.5 * _spatium;
    for (Articulation* a : qAsConst(_articulations)) {
        ArticulationAnchor aa = a->anchor();
        if (aa != ArticulationAnchor::CHORD && aa != ArticulationAnchor::TOP_CHORD && aa != ArticulationAnchor::BOTTOM_CHORD) {
            continue;
        }

        if (a->up()) {
            if (!a->layoutCloseToNote()) {
                a->layout();
                a->setPos(x, chordTopY);
                a->doAutoplace();
            }
            if (a->visible()) {
                chordTopY = a->y() - a->height() - 0.5 * _spatium;
            }
        } else {
            if (!a->layoutCloseToNote()) {
                a->layout();
                a->setPos(x, chordBotY);
                a->doAutoplace();
            }
            if (a->visible()) {
                chordBotY = a->y() + a->height() + 0.5 * _spatium;
            }
        }
    }
    //
    //    now place all articulations with staff top or bottom anchor
    //

    staffTopY = qMin(staffTopY, chordTopY - distance0 - 0.5 * _spatium);
    staffBotY = qMax(staffBotY, chordBotY + distance0 + 0.5 * _spatium);
    for (Articulation* a : qAsConst(_articulations)) {
        ArticulationAnchor aa = a->anchor();
        if (aa == ArticulationAnchor::TOP_STAFF || aa == ArticulationAnchor::BOTTOM_STAFF) {
            a->layout();
            if (a->up()) {
                a->setPos(x, staffTopY);
                if (a->visible()) {
                    staffTopY -= distance0;
                }
            } else {
                a->setPos(x, staffBotY);
                if (a->visible()) {
                    staffBotY += distance0;
                }
            }
            a->doAutoplace();
        }
    }
    for (Articulation* a : qAsConst(_articulations)) {
        if (a->addToSkyline()) {
            // the segment shape has already been calculated
            // so measure width and spacing is already determined
            // in line mode, we cannot add to segment shape without throwing this off
            // but adding to skyline is always good
            Segment* s = segment();
            Measure* m = s->measure();
            RectF r = a->bbox().translated(a->pos() + pos());
            // TODO: limit to width of chord
            // this avoids "staircase" effect due to space not having been allocated already
            // ANOTHER alternative is to allocate the space in layoutPitched() / layoutTablature()
            //qreal w = qMin(r.width(), width());
            //r.translate((r.width() - w) * 0.5, 0.0);
            //r.setWidth(w);
            if (!score()->lineMode()) {
                s->staffShape(staffIdx()).add(r);
            }
            r.translate(s->pos() + m->pos());
            m->system()->staff(vStaffIdx())->skyline().add(r);
        }
    }
}

//---------------------------------------------------------
//   layoutArticulations3
//    Called after layouting slurs
//    Fix up articulations that need to go outside the slur
//---------------------------------------------------------

void Chord::layoutArticulations3(Slur* slur)
{
    SlurSegment* ss;
    if (this == slur->startCR()) {
        ss = slur->frontSegment();
    } else if (this == slur->endCR()) {
        ss = slur->backSegment();
    } else {
        return;
    }
    Segment* s = segment();
    Measure* m = measure();
    SysStaff* sstaff = m->system() ? m->system()->staff(vStaffIdx()) : nullptr;
    for (Articulation* a : qAsConst(_articulations)) {
        if (a->layoutCloseToNote() || !a->autoplace() || !slur->addToSkyline()) {
            continue;
        }
        Shape aShape = a->shape().translated(a->pos() + pos() + s->pos() + m->pos());
        Shape sShape = ss->shape().translated(ss->pos());
        if (aShape.intersects(sShape)) {
            qreal d = score()->styleS(Sid::articulationMinDistance).val() * spatium();
            if (slur->up()) {
                d += qMax(aShape.minVerticalDistance(sShape), 0.0);
                a->rypos() -= d;
                aShape.translateY(-d);
            } else {
                d += qMax(sShape.minVerticalDistance(aShape), 0.0);
                a->rypos() += d;
                aShape.translateY(d);
            }
            if (sstaff && a->addToSkyline()) {
                sstaff->skyline().add(aShape);
            }
        }
    }
}

std::set<SymId> Chord::articulationSymbolIds() const
{
    std::set<SymId> result;
    for (const Articulation* artic: _articulations) {
        result.insert(artic->symId());
    }

    return result;
}

//---------------------------------------------------------
//   getNoteEventLists
//    Get contents of all NoteEventLists for all notes in
//    the chord.
//---------------------------------------------------------

QList<NoteEventList> Chord::getNoteEventLists()
{
    QList<NoteEventList> ell;
    if (notes().empty()) {
        return ell;
    }
    for (size_t i = 0; i < notes().size(); ++i) {
        ell.append(NoteEventList(notes()[i]->playEvents()));
    }
    return ell;
}

//---------------------------------------------------------
//   setNoteEventLists
//    Set contents of all NoteEventLists for all notes in
//    the chord.
//---------------------------------------------------------

void Chord::setNoteEventLists(QList<NoteEventList>& ell)
{
    if (notes().empty()) {
        return;
    }
    Q_ASSERT(ell.size() == int(notes().size()));
    for (size_t i = 0; int(i) < ell.size(); i++) {
        notes()[i]->setPlayEvents(ell[int(i)]);
    }
}
}
