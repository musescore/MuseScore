//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "chord.h"

#include <cmath>

#include "note.h"
#include "xml.h"
#include "style.h"
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
#include "sym.h"
#include "stringdata.h"
#include "beam.h"
#include "slur.h"
#include "fingering.h"

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
//   Chord
//---------------------------------------------------------

Chord::Chord(Score* s)
    : ChordRest(s)
{
    _ledgerLines      = 0;
    _stem             = 0;
    _hook             = 0;
    _stemDirection    = Direction::AUTO;
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
        Note* nnote = new Note(*onote, link);
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
        add(new Stem(*(c._stem)));
    }
    if (c._hook) {
        add(new Hook(*(c._hook)));
    }
    if (c._stemSlash) {
        add(new StemSlash(*(c._stemSlash)));
    }
    if (c._arpeggio) {
        Arpeggio* a = new Arpeggio(*(c._arpeggio));
        add(a);
        if (link) {
            score()->undo(new Link(a, const_cast<Arpeggio*>(c._arpeggio)));
        }
    }
    if (c._tremolo) {
        Tremolo* t = new Tremolo(*(c._tremolo));
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

    for (Element* e : c.el()) {
        if (e->isChordLine()) {
            ChordLine* cl = toChordLine(e);
            ChordLine* ncl = new ChordLine(*cl);
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

    for (Element* e : el()) {
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

//---------------------------------------------------------
//   stemPosX
//    return Chord coordinates. Based on nominal notehead
//---------------------------------------------------------

qreal Chord::stemPosX() const
{
    const Staff* stf = staff();
    const StaffType* st = stf ? stf->staffTypeForElement(this) : 0;
    if (st && st->isTabStaff()) {
        return st->chordStemPosX(this) * spatium();
    }
    return _up ? noteHeadWidth() : 0.0;
}

//---------------------------------------------------------
//   stemPos
//    return page coordinates
//---------------------------------------------------------

QPointF Chord::stemPos() const
{
    QPointF p(pagePos());

    const Staff* stf = staff();
    const StaffType* st = stf ? stf->staffTypeForElement(this) : 0;
    if (st && st->isTabStaff()) {
        return st->chordStemPos(this) * spatium() + p;
    }

    if (_up) {
        qreal nhw = _notes.size() == 1 ? downNote()->bboxRightPos() : noteHeadWidth();
        p.rx() += nhw;
        p.ry() += downNote()->pos().y();
    } else {
        p.ry() += upNote()->pos().y();
    }
    return p;
}

//---------------------------------------------------------
//   stemPosBeam
//    return stem position of note on beam side
//    return page coordinates
//---------------------------------------------------------

QPointF Chord::stemPosBeam() const
{
    qreal _spatium = spatium();
    QPointF p(pagePos());

    const Staff* stf = staff();
    const StaffType* st = stf ? stf->staffTypeForElement(this) : 0;

    if (st && st->isTabStaff()) {
        return st->chordStemPosBeam(this) * _spatium + p;
    }

    if (_up) {
        qreal nhw = noteHeadWidth();
        p.rx() += nhw;
        p.ry() += upNote()->pos().y();
    } else {
        p.ry() += downNote()->pos().y();
    }

    return p;
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

void Chord::add(Element* e)
{
    e->setParent(this);
    e->setTrack(track());
    switch (e->type()) {
    case ElementType::NOTE:
    {
        Note* note = toNote(e);
        bool found = false;

        // _notes should be sorted by line position,
        // but it's often not yet possible since line is unknown
        // use pitch instead, and line as a second sort criteria.

        for (unsigned idx = 0; idx < _notes.size(); ++idx) {
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
        break;
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Chord::remove(Element* e)
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
        break;
    }
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
        staffVisible  = !staff()->invisible(tick);
        stepOffset = st->staffType(tick)->stepOffset();
    }

    // need ledger lines?
    if (downLine() + stepOffset <= lineBelow + 1 && upLine() + stepOffset >= -1) {
        return;
    }

    // the extra length of a ledger line to be added on each side of the notehead
    qreal extraLen = score()->styleP(Sid::ledgerLineLength) * mag;
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
            if (x - extraLen < minX) {
                minX  = x - extraLen;
                // increase width of all lines between this one and the staff
                for (auto& d : vecLines) {
                    if (!d.accidental && ((l < 0 && d.line >= l) || (l > 0 && d.line <= l))) {
                        d.minX = minX;
                    }
                }
            }
            // same for left side
            if (x + hw + extraLen > maxX) {
                maxX = x + hw + extraLen;
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

//-----------------------------------------------------------------------------
//   computeUp
//    rules:
//      single note:
//          All notes beneath the middle line: upward stems
//          All notes on or above the middle line: downward stems
//      two notes:
//          If the interval above the middle line is greater than the interval
//             below the middle line: downward stems
//          If the interval below the middle line is greater than the interval
//             above the middle line: upward stems
//          If the two notes are the same distance from the middle line:
//             stem can go in either direction. but most engravers prefer
//             downward stems
//      > two notes:
//          If the interval of the highest note above the middle line is greater
//             than the interval of the lowest note below the middle line:
//             downward stems
//          If the interval of the lowest note below the middle line is greater
//             than the interval of the highest note above the middle line:
//             upward stem
//          If the highest and the lowest notes are the same distance from
//          the middle line:, use these rules to determine stem direction:
//             - If the majority of the notes are above the middle:
//               downward stems
//             - If the majority of the notes are below the middle:
//               upward stems
//    TABlatures:
//       stems beside staves:
//          All stems are up / down according to TAB::stemsDown() setting
//       stems through staves:
//          Same rules as per pitched staves
//-----------------------------------------------------------------------------

void Chord::computeUp()
{
    Q_ASSERT(!_notes.empty());
    const Staff* st = staff();
    const StaffType* tab = st ? st->staffTypeForElement(this) : 0;
    bool tabStaff  = tab && tab->isTabStaff();
    // TAB STAVES
    if (tabStaff) {
        // if no stems or stem beside staves
        if (tab->stemless() || !tab->stemThrough()) {
            // if measure has voices, set stem direction according to voice
            if (measure()->hasVoices(staffIdx(), tick(), actualTicks())) {
                _up = !(track() % 2);
            } else {                            // if only voice 1,
                // unconditionally set to down if not stems or according to TAB stem direction otherwise
                // (even with no stems, stem direction controls position of slurs and ties)
                _up = tab->stemless() ? false : !tab->stemsDown();
            }
            return;
        }
        // if TAB has stems through staves, chain into standard processing
    }

    // PITCHED STAVES (or TAB with stems through staves)
    if (_stemDirection != Direction::AUTO) {
        _up = _stemDirection == Direction::UP;
    } else if (!parent()) {
        // hack for palette and drumset editor
        _up = upNote()->line() > 4;
    } else if (_noteType != NoteType::NORMAL) {
        //
        // stem direction for grace notes
        //
        if (measure()->hasVoices(staffIdx(), tick(), actualTicks())) {
            _up = !(track() % 2);
        } else {
            _up = true;
        }
    } else if (staffMove()) {
        _up = staffMove() > 0;
    } else if (measure()->hasVoices(staffIdx(), tick(), actualTicks())) {
        _up = !(track() % 2);
    } else {
        int dnMaxLine   = staff()->middleLine(tick());
        int ud          = (tabStaff ? upString() * 2 : upNote()->line()) - dnMaxLine;
        // standard case: if only 1 note or cross beaming
        if (_notes.size() == 1 || staffMove()) {
            if (staffMove() > 0) {
                _up = true;
            } else if (staffMove() < 0) {
                _up = false;
            } else {
                _up = ud > 0;
            }
        }
        // if more than 1 note, compare extrema (topmost and bottommost notes)
        else {
            int dd = (tabStaff ? downString() * 2 : downNote()->line()) - dnMaxLine;
            // if extrema symmetrical, average directions of intermediate notes
            if (-ud == dd) {
                int up = 0;
                size_t n = _notes.size();
                for (size_t i = 0; i < n; ++i) {
                    const Note* currentNote = _notes.at(i);
                    int l = tabStaff ? currentNote->string() * 2 : currentNote->line();
                    if (l <= dnMaxLine) {
                        --up;
                    } else {
                        ++up;
                    }
                }
                _up = up > 0;
            }
            // if extrema not symmetrical, set _up to prevailing
            else {
                _up = dd > -ud;
            }
        }
    }
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
    xml.stag(this);
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
    } else if (_stem && (_stem->isUserModified() || (_stem->userLen() != 0.0))) {
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
    for (Element* e : el()) {
        e->write(xml);
    }
    xml.etag();
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
        Note* note = new Note(score());
        // the note needs to know the properties of the track it belongs to
        note->setTrack(track());
        note->setChord(this);
        note->read(e);
        add(note);
    } else if (ChordRest::readProperties(e)) {
    } else if (tag == "Stem") {
        Stem* s = new Stem(score());
        s->read(e);
        add(s);
    } else if (tag == "Hook") {
        _hook = new Hook(score());
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
        StemSlash* ss = new StemSlash(score());
        ss->read(e);
        add(ss);
    } else if (readProperty(tag, e, Pid::STEM_DIRECTION)) {
    } else if (tag == "noStem") {
        _noStem = e.readInt();
    } else if (tag == "Arpeggio") {
        _arpeggio = new Arpeggio(score());
        _arpeggio->setTrack(track());
        _arpeggio->read(e);
        _arpeggio->setParent(this);
    } else if (tag == "Tremolo") {
        _tremolo = new Tremolo(score());
        _tremolo->setTrack(track());
        _tremolo->read(e);
        _tremolo->setParent(this);
        _tremolo->setDurationType(durationType());
    } else if (tag == "tickOffset") {     // obsolete
    } else if (tag == "ChordLine") {
        ChordLine* cl = new ChordLine(score());
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

void Chord::processSiblings(std::function<void(Element*)> func) const
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
    for (Element* e : el()) {
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
    processSiblings([val](Element* e) { e->setTrack(val); });
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Chord::setScore(Score* s)
{
    ChordRest::setScore(s);
    processSiblings([s](Element* e) { e->setScore(s); });
}

//-----------------------------------------------------------------------------
//   defaultStemLength
///   Get the default stem length for this chord
//-----------------------------------------------------------------------------

qreal Chord::defaultStemLength() const
{
    Note* downnote;
    qreal stemLen;
    qreal _spatium     = spatium();
    int hookIdx        = durationType().hooks();
    downnote           = downNote();
    int ul             = upLine();
    int dl             = downLine();
    const Staff* st    = staff();
    qreal lineDistance = st ? st->lineDistance(tick()) : 1.0;

    const StaffType* stt = st ? st->staffTypeForElement(this) : nullptr;
    const StaffType* tab = (stt && stt->isTabStaff()) ? stt : nullptr;
    if (tab) {
        // require stems only if TAB is not stemless and this chord has a stem
        if (!tab->stemless() && _stem) {
            // if stems are beside staff, apply special formatting
            if (!tab->stemThrough()) {
                // process stem:
                return tab->chordStemLength(this) * _spatium;
            }
        }
    } else if (lineDistance != 1.0) {
        // convert to actual distance from top of staff in sp
        // ul *= lineDistance;
        // dl *= lineDistance;
    }

    if (tab && !tab->onLines()) {         // if TAB and frets above strings, move 1 position up
        --ul;
        --dl;
    }
    bool shortenStem = score()->styleB(Sid::shortenStem);
    if (hookIdx >= 2 || _tremolo) {
        shortenStem = false;
    }

    Spatium progression = score()->styleS(Sid::shortStemProgression);
    qreal shortest      = score()->styleS(Sid::shortestStem).val();
    if (hookIdx) {
        if (up()) {
            shortest = qMax(shortest, small() ? 2.0 : 3.0);
        } else {
            shortest = qMax(shortest, small() ? 2.25 : 3.5);
        }
    }

    qreal normalStemLen = small() ? 2.5 : 3.5;
    if (hookIdx && tab == 0) {
        if (up() && durationType().dots()) {
            //
            // avoid collision of dot with hook
            //
            if (!(ul & 1)) {
                normalStemLen += .5;
            }
            shortenStem = false;
        }
    }

    if (isGrace()) {
        // grace notes stems are not subject to normal
        // stem rules
        stemLen =  qAbs(ul - dl) * .5;
        stemLen += normalStemLen * score()->styleD(Sid::graceNoteMag);
        if (up()) {
            stemLen *= -1;
        }
    } else {
        // normal note (not grace)
        qreal staffHeight = st ? st->lines(tick()) - 1 : 4;
        if (!tab) {
            staffHeight *= lineDistance;
        }
        qreal staffHlfHgt = staffHeight * 0.5;
        if (up()) {                       // stem up
            qreal dy  = dl * .5;                            // note-side vert. pos.
            qreal sel = ul * .5 - normalStemLen;            // stem end vert. pos

            // if stem ends above top line (with some exceptions), shorten it
            if (shortenStem && (sel < 0.0) && (hookIdx == 0 || tab || !downnote->mirror())) {
                sel -= sel * progression.val();
            }
            if (sel > staffHlfHgt) {                        // if stem ends below ('>') staff mid position,
                sel = staffHlfHgt;                          // stretch it to mid position
            }
            stemLen = sel - dy;                             // actual stem length
            qreal exposedLen = sel - ul * .5;               // portion extending above top note of chord
            if (-exposedLen < shortest) {                   // if stem too short,
                qreal diff = shortest + exposedLen;
                stemLen -= diff;                            // lengthen it to shortest possible length
            }
        } else {                          // stem down
            qreal uy  = ul * .5;                            // note-side vert. pos.
            qreal sel = dl * .5 + normalStemLen;            // stem end vert. pos.

            // if stem ends below bottom line (with some exceptions), shorten it
            if (shortenStem && (sel > staffHeight) && (hookIdx == 0 || tab || downnote->mirror())) {
                sel -= (sel - staffHeight) * progression.val();
            }
            if (sel < staffHlfHgt) {                        // if stem ends above ('<') staff mid position,
                sel = staffHlfHgt;                          // stretch it to mid position
            }
            stemLen = sel - uy;                             // actual stem length
            qreal exposedLen = sel - dl * .5;               // portion extending below bottom note of chord
            if (exposedLen < shortest) {                    // if stem too short,
                qreal diff = shortest - exposedLen;
                stemLen += diff;                            // lengthen it to shortest possible length
            }
        }
    }

    if (tab) {
        stemLen *= lineDistance;
    }

    const qreal sgn = up() ? -1.0 : 1.0;
    qreal stemLenPoints = point(Spatium(stemLen));
    const qreal minAbsStemLen = minAbsStemLength();
    if (sgn * stemLenPoints < minAbsStemLen) {
        stemLenPoints = sgn * minAbsStemLen;
    }

    return stemLenPoints;
}

//---------------------------------------------------------
//   minAbsStemLength
//    get minimum stem length with tremolo
//---------------------------------------------------------

qreal Chord::minAbsStemLength() const
{
    if (!_tremolo) {
        return 0.0;
    }

    const qreal sw = score()->styleS(Sid::tremoloStrokeWidth).val() * chordMag();
    const qreal td = score()->styleS(Sid::tremoloDistance).val() * chordMag();
    int beamLvl = beams();
    const qreal beamDist = beam() ? beam()->beamDist() : (sw * spatium());

    // single-note tremolo
    if (!_tremolo->twoNotes()) {
        _tremolo->layout();     // guarantee right "height value"

        // distance between tremolo stroke(s) and chord
        // choose the furthest/nearest note to calculate for unbeamed/beamed chords
        // this is due to special layout mechanisms regarding beamed chords
        // may be changed if beam layout code is improved/rewritten
        qreal height = 0.0;
        if (up()) {
            height = (beam() ? upPos() : downPos()) - _tremolo->pos().y();
        } else {
            height = _tremolo->pos().y() + _tremolo->minHeight() * spatium() - (beam() ? downPos() : upPos());
        }
        const bool hasHook = beamLvl && !beam();
        if (hasHook) {
            beamLvl += (up() ? 3 : 2); // reserve more space for stem with both hook and tremolo
        }
        const qreal addHeight1 = beamLvl ? 0 : sw* spatium();
        // buzz roll needs to have additional space so as not to collide with the beam/hook
        const qreal addHeight2 = (_tremolo->isBuzzRoll() && beamLvl) ? sw * spatium() : 0;

        return height + beamLvl * beamDist + addHeight1 + addHeight2;
    }
    // two-note tremolo
    else {
        if (_tremolo->chord2() && _tremolo->chord1()->up() == _tremolo->chord2()->up()) {
            const qreal tremoloMinHeight = _tremolo->minHeight() * spatium();
            return tremoloMinHeight + beamLvl * beamDist + 2 * td * spatium();
        }
        return 0.0;
    }
}

//---------------------------------------------------------
//   layoutStem1
///   Layout _stem and _stemSlash
//
//    Called before layout spacing of notes.
//    Create stem if necessary.
//---------------------------------------------------------

void Chord::layoutStem1()
{
    const Staff* stf = staff();
    const StaffType* st = stf ? stf->staffTypeForElement(this) : 0;
    if (durationType().hasStem()
        && !(_noStem || (measure() && measure()->stemless(staffIdx())) || (st && st->isTabStaff() && st->stemless()))) {
        if (!_stem) {
            Stem* stem = new Stem(score());
            stem->setParent(this);
            stem->setGenerated(true);
            score()->undoAddElement(stem);
        }
        if ((_noteType == NoteType::ACCIACCATURA) && !(beam() && beam()->elements().front() != this)) {
            if (!_stemSlash) {
                add(new StemSlash(score()));
            }
        } else if (_stemSlash) {
            remove(_stemSlash);
        }

        qreal stemWidth5 = _stem->lineWidth() * .5 * mag();
        _stem->rxpos()   = stemPosX() + (up() ? -stemWidth5 : +stemWidth5);
        _stem->setLen(defaultStemLength());
    } else {
        if (_stem) {
            score()->undoRemoveElement(_stem);
        }
        if (_stemSlash) {
            score()->undoRemoveElement(_stemSlash);
        }
    }
}

//-----------------------------------------------------------------------------
//   layoutStem
///   Layout chord tremolo stem and hook.
//
//    hook: sets position
//-----------------------------------------------------------------------------

void Chord::layoutStem()
{
    for (Chord* c : qAsConst(_graceNotes)) {
        c->layoutStem();
    }
    if (_beam) {
        return;
    }

    // create hooks for unbeamed chords

    int hookIdx  = durationType().hooks();

    if (hookIdx && !(noStem() || measure()->stemless(staffIdx()))) {
        if (!hook()) {
            Hook* hook = new Hook(score());
            hook->setParent(this);
            hook->setGenerated(true);
            score()->undoAddElement(hook);
        }
        hook()->setHookType(up() ? hookIdx : -hookIdx);
    } else if (hook()) {
        score()->undoRemoveElement(hook());
    }

    //
    // TAB
    //
    const Staff* st = staff();
    const StaffType* tab = st ? st->staffTypeForElement(this) : 0;
    if (tab && tab->isTabStaff()) {
        // if stemless TAB
        if (tab->stemless()) {
            // if 'grid' duration symbol of MEDIALFINAL type, it is time to compute its width
            if (_tabDur != nullptr && _tabDur->beamGrid() == TabBeamGrid::MEDIALFINAL) {
                _tabDur->layout2();
            }
            // in all other stemless cases, do nothing
            return;
        }
        // not a stemless TAB; if stems are beside staff, apply special formatting
        if (!tab->stemThrough()) {
            if (_stem) {       // (duplicate code with defaultStemLength())
                // process stem:
                _stem->setLen(tab->chordStemLength(this) * spatium());
                // process hook
                hookIdx = durationType().hooks();
                if (!up()) {
                    hookIdx = -hookIdx;
                }
                if (hookIdx && _hook) {
                    _hook->setHookType(hookIdx);
#if 0
                    _hook->layout();
                    QPointF p(_stem->hookPos());
                    p.rx() -= _stem->width();
                    _hook->setPos(p);
#endif
                }
            }
            return;
        }
        // if stems are through staff, use standard formatting
    }

    //
    // NON-TAB (or TAB with stems through staff)
    //
    if (_stem) {
        if (_hook) {
            _hook->layout();
            QPointF p(_stem->hookPos());
            p.rx() -= _stem->width();
            _hook->setPos(p);
        }
        if (_stemSlash) {
            _stemSlash->layout();
        }
    }

    //-----------------------------------------
    //    process tremolo
    //-----------------------------------------

//      if (_tremolo)
//            _tremolo->layout();
}

//---------------------------------------------------------
//    underBeam: true, if grace note is placed under a beam.
//---------------------------------------------------------

bool Chord::underBeam() const
{
    if (_noteType == NoteType::NORMAL) {
        return false;
    }
    const Chord* cr = toChord(parent());
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
        qreal minNoteDist = score()->styleP(Sid::minNoteDistance) * mag * score()->styleD(Sid::graceNoteMag);
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
                ? score()->styleP(Sid::noteBarDistance) * mag
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
                qWarning("unmapped drum note %d", pitch);
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

QPointF Chord::pagePos() const
{
    if (isGrace()) {
        QPointF p(pos());
        if (parent() == 0) {
            return p;
        }
        p.rx() = pageX();

        const Chord* pc = static_cast<const Chord*>(parent());
        System* system = pc->segment()->system();
        if (!system) {
            return p;
        }
        qreal staffYOffset = staff() ? staff()->staffType(tick())->yoffset().val() * spatium() : 0.0;
        p.ry() += system->staffYpage(vStaffIdx()) + staffYOffset;
        return p;
    }
    return Element::pagePos();
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
    qreal dotNoteDistance  = score()->styleP(Sid::dotNoteDistance) * mag_;
    qreal minNoteDistance  = score()->styleP(Sid::minNoteDistance) * mag_;
    qreal minTieLength     = score()->styleP(Sid::MinTieLength) * mag_;

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
        layoutStem1();
        if (_stem) {     //false when dragging notes from drum palette
            qreal stemWidth5 = _stem->lineWidth() * .5;
            _stem->rxpos()   = up() ? (upNote()->headBodyWidth() - stemWidth5) : stemWidth5;
        }
        addLedgerLines();
        return;
    }

    //-----------------------------------------
    //  process notes
    //-----------------------------------------

    for (Note* note : _notes) {
        note->layout();

        qreal x1 = note->pos().x() + chordX;
        qreal x2 = x1 + note->headWidth();
        lll      = qMax(lll, -x1);
        rrr      = qMax(rrr, x2);
        // track amount of space due to notehead only
        lhead    = qMax(lhead, -x1);

        Accidental* accidental = note->accidental();
        if (accidental && accidental->addToSkyline() && !note->fixed()) {
            // convert x position of accidental to segment coordinate system
            qreal x = accidental->pos().x() + note->pos().x() + chordX;
            // distance from accidental to note already taken into account
            // but here perhaps we create more padding in *front* of accidental?
            x -= score()->styleP(Sid::accidentalDistance) * mag_;
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
        for (Element* e : note->el()) {
            if (e->isFingering()) {
                Fingering* f = toFingering(e);
                if (f->layoutType() == ElementType::NOTE) {
                    f->setPos(QPointF());
                    f->setbbox(QRectF());
                }
            }
        }
    }

    //-----------------------------------------
    //  create ledger lines
    //-----------------------------------------

    addLedgerLines();

    if (_arpeggio) {
        qreal arpeggioDistance = score()->styleP(Sid::ArpeggioNoteDistance) * mag_;
        _arpeggio->layout();        // only for width() !
        _arpeggio->setHeight(0.0);
        qreal extraX = _arpeggio->width() + arpeggioDistance + chordX;
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
                  + (dots() - 1) * score()->styleP(Sid::dotDotDistance) * mag_;
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
                qreal x = _hook->bbox().right() + stem()->hookPos().x() + chordX;
                rrr = qMax(rrr, x);
            }
        }
    }

#if 0
    if (!_articulations.isEmpty()) {
        // TODO: allocate space to avoid "staircase" effect
        // but we would need to determine direction in order to get correct symid & bbox
        // another alternative is to limit the width contribution of the articulation in layoutArticulations2()
        //qreal aWidth = 0.0;
        for (Articulation* a : articulations()) {
            a->layout();            // aWidth = qMax(aWidth, a->width());
        }
        //qreal w = width();
        //qreal aExtra = (qMax(aWidth, w) - w) * 0.5;
        //lll = qMax(lll, aExtra);
        //rrr = qMax(rrr, aExtra);
    }
#endif

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

    for (Element* e : el()) {
        if (e->type() == ElementType::SLUR) {       // we cannot at this time as chordpositions are not fixed
            continue;
        }
        e->layout();
        if (e->type() == ElementType::CHORDLINE) {
            QRectF tbbox = e->bbox().translated(e->pos());
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
        for (Element* e : note->el()) {
            if (e->isFingering() && e->autoplace()) {
                Fingering* f = toFingering(e);
                if (f->layoutType() == ElementType::NOTE && f->tid() == Tid::LH_GUITAR_FINGERING) {
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
    qreal dotNoteDistance   = score()->styleP(Sid::dotNoteDistance);
    qreal minNoteDistance   = score()->styleP(Sid::minNoteDistance);
    qreal minTieLength      = score()->styleP(Sid::MinTieLength);

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
    // if tab type is stemless or chord is stemless (possible when imported from MusicXML)
    // or measure is stemless
    // or duration longer than half (if halves have stems) or duration longer than crochet
    // remove stems
    if (tab->stemless() || _noStem || measure()->stemless(staffIdx()) || durationType().type()
        < (tab->minimStyle() != TablatureMinimStyle::NONE ? TDuration::DurationType::V_HALF : TDuration::DurationType::V_QUARTER)) {
        if (_stem) {
            score()->undo(new RemoveElement(_stem));
        }
        if (_hook) {
            score()->undo(new RemoveElement(_hook));
        }
        if (_beam) {
            score()->undo(new RemoveElement(_beam));
        }
    }
    // if stem is required but missing, add it;
    // set stem position (stem length is set in Chord:layoutStem() )
    else {
        if (_stem == 0) {
            Stem* stem = new Stem(score());
            stem->setParent(this);
            score()->undo(new AddElement(stem));
        }
        _stem->setPos(tab->chordStemPos(this) * _spatium);
        if (_hook) {
            if (beam()) {
                score()->undoRemoveElement(_hook);
            } else {
                _hook->layout();
                if (rrr < stemX + _hook->width()) {
                    rrr = stemX + _hook->width();
                }

                QPointF p(_stem->hookPos());
                p.rx() -= _stem->width();
                _hook->setPos(p);
            }
        }
    }
    if (!tab->genDurations()                           // if tab is not set for duration symbols
        || track2voice(track())                        // or not in first voice
        || (isGrace()                                  // no tab duration symbols if grace notes
            && beamMode() == Beam::Mode::AUTO)) {      // and beammode == AUTO
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
            } else if (beamMode() != Beam::Mode::AUTO
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
                _tabDur = new TabDurationSymbol(score(), tab, durationType().type(), dots());
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
            lll += (0.5 + score()->styleS(Sid::MinTieLength).val()) * _spatium;
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
                qreal x = _hook->bbox().right() + stem()->hookPos().x();
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

#if 0
    if (!_articulations.isEmpty()) {
        // TODO: allocate space? see layoutPitched()
        for (Articulation* a : articulations()) {
            a->layout();
        }
    }
#endif

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
        case TDuration::DurationType::V_LONG:    fc = 3.8;
            break;
        case TDuration::DurationType::V_BREVE:   fc = 3.8;
            break;
        case TDuration::DurationType::V_WHOLE:   fc = 3.8;
            break;
        case TDuration::DurationType::V_HALF:    fc = 3.6;
            break;
        case TDuration::DurationType::V_QUARTER: fc = 2.1;
            break;
        case TDuration::DurationType::V_EIGHTH:  fc = 1.4;
            break;
        case TDuration::DurationType::V_16TH:    fc = 1.2;
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
    for (Element* e : el()) {
        e->layout();
        if (e->type() == ElementType::CHORDLINE) {
            QRectF tbbox = e->bbox().translated(e->pos());
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
    QRectF bb;
    processSiblings([&bb](Element* e) { bb |= e->bbox().translated(e->pos()); });
    if (_tabDur) {
        bb |= _tabDur->bbox().translated(_tabDur->pos());
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
            layoutStem1();
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
                    layoutStem1();
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

    Element* element = segment()->element(btrack);
    ChordRest* bchord = element ? toChordRest(element) : nullptr;
    Note* dnote       = (bchord && bchord->type() == ElementType::CHORD) ? toChord(bchord)->downNote() : downNote();

    qreal h = dnote->pagePos().y() + dnote->headHeight() * .5 - y;
    _arpeggio->setHeight(h);
    _arpeggio->layout();

#if 0 // collect notes for arpeggio
    QList<Note*> notes;
    int n = _notes.size();
    for (int j = n - 1; j >= 0; --j) {
        Note* note = _notes[j];
        if (note->tieBack()) {
            continue;
        }
        notes.prepend(note);
    }

    for (int i = 1; i < span; ++i) {
        ChordRest* c = toChordRest(segment()->element(track() + i * VOICES));
        if (c && c->type() == CHORD) {
            QList<Note*> nl = toChord(c)->notes();
            int n = nl.size();
            for (int j = n - 1; j >= 0; --j) {
                Note* note = nl[j];
                if (note->tieBack()) {
                    continue;
                }
                notes.prepend(note);
            }
        }
    }
#endif
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

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Chord::drop(EditData& data)
{
    Element* e = data.dropElement;
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

void Chord::setColor(const QColor& color)
{
    ChordRest::setColor(color);

    for (Note* note : _notes) {
        note->undoChangeProperty(Pid::COLOR, color);
    }
}

//---------------------------------------------------------
//   dotPosX
//---------------------------------------------------------

qreal Chord::dotPosX() const
{
    if (parent()) {
        return segment()->dotPosX(staffIdx());
    }
    return -1000.0;
}

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void Chord::localSpatiumChanged(qreal oldValue, qreal newValue)
{
    ChordRest::localSpatiumChanged(oldValue, newValue);
    for (Element* e : graceNotes()) {
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
    for (Element* e : articulations()) {
        e->localSpatiumChanged(oldValue, newValue);
    }
    for (Note* note : notes()) {
        note->localSpatiumChanged(oldValue, newValue);
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Chord::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::NO_STEM:        return noStem();
    case Pid::SMALL:          return small();
    case Pid::STEM_DIRECTION: return QVariant::fromValue<Direction>(stemDirection());
    case Pid::PLAY: return isChordPlayable();
    default:
        return ChordRest::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Chord::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::NO_STEM:        return false;
    case Pid::SMALL:          return false;
    case Pid::STEM_DIRECTION: return QVariant::fromValue<Direction>(Direction::AUTO);
    case Pid::PLAY: return true;
    default:
        return ChordRest::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Chord::setProperty(Pid propertyId, const QVariant& v)
{
    switch (propertyId) {
    case Pid::NO_STEM:
        setNoStem(v.toBool());
        break;
    case Pid::SMALL:
        setSmall(v.toBool());
        break;
    case Pid::STEM_DIRECTION:
        setStemDirection(v.value<Direction>());
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
    for (Articulation* articulation: _articulations) {
        currentArticulationIds.insert(articulation->symId());
        score()->undoRemoveElement(articulation);
    }

    std::set<SymId> articulationIds = flipArticulations(currentArticulationIds, Placement::ABOVE);
    articulationIds = splitArticulations(articulationIds);

    std::set<SymId> _newArticulationIds = flipArticulations(newArticulationIds, Placement::ABOVE);
    _newArticulationIds = splitArticulations(_newArticulationIds);

    for (const SymId& articulationId: _newArticulationIds) {
        articulationIds = Ms::updateArticulations(articulationIds, articulationId, updateMode);
    }

    std::set<SymId> result = joinArticulations(articulationIds);
    for (const SymId& articulationSymbolId: result) {
        Articulation* newArticulation = new Articulation(score());
        newArticulation->setSymId(articulationSymbolId);
        score()->addArticulation(this, newArticulation);
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Chord::reset()
{
    undoChangeProperty(Pid::STEM_DIRECTION, QVariant::fromValue<Direction>(Direction::AUTO));
    undoChangeProperty(Pid::BEAM_MODE, int(Beam::Mode::AUTO));
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
    NoteHead::Group head = NoteHead::Group::HEAD_SLASH;

    if (!flag) {
        // restore to normal
        undoChangeProperty(Pid::NO_STEM, false);
        undoChangeProperty(Pid::SMALL, false);
        undoChangeProperty(Pid::OFFSET, QPointF());
        for (Note* n : _notes) {
            n->undoChangeProperty(Pid::HEAD_GROUP, int(NoteHead::Group::HEAD_NORMAL));
            n->undoChangeProperty(Pid::FIXED, false);
            n->undoChangeProperty(Pid::FIXED_LINE, 0);
            n->undoChangeProperty(Pid::PLAY, true);
            n->undoChangeProperty(Pid::VISIBLE, true);
            if (staff()->isDrumStaff(tick())) {
                const Drumset* ds = part()->instrument(tick())->drumset();
                int pitch = n->pitch();
                if (ds && ds->isValid(pitch)) {
                    undoChangeProperty(Pid::STEM_DIRECTION, QVariant::fromValue<Direction>(ds->stemDirection(pitch)));
                    n->undoChangeProperty(Pid::HEAD_GROUP, int(ds->noteHead(pitch)));
                }
            }
        }
        return;
    }

    // set stem to auto (mostly important for rhythmic notation on drum staves)
    undoChangeProperty(Pid::STEM_DIRECTION, QVariant::fromValue<Direction>(Direction::AUTO));

    // make stemless if asked
    if (stemless) {
        undoChangeProperty(Pid::NO_STEM, true);
        undoChangeProperty(Pid::BEAM_MODE, int(Beam::Mode::NONE));
    }

    // voice-dependent attributes - line, size, offset, head
    if (track() % VOICES < 2) {
        // use middle line
        line = staff()->middleLine(tick());
    } else {
        // set small
        undoChangeProperty(Pid::SMALL, true);
        // set outside the staff
        qreal y = 0.0;
        if (track() % 2) {
            line = staff()->bottomLine(tick()) + 1;
            y    = 0.5 * spatium();
        } else {
            line = -1;
            if (!staff()->isDrumStaff(tick())) {
                y = -0.5 * spatium();
            }
        }
        // for non-drum staves, add an additional offset
        // for drum staves, no offset, but use normal head
        if (!staff()->isDrumStaff(tick())) {
            // undoChangeProperty(Pid::OFFSET, QPointF(0.0, y));
            rypos() += y;
        } else {
            head = NoteHead::Group::HEAD_NORMAL;
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
        for (Element* e : n->el()) {
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
    if (small()) {
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
    Element* e = parent();
    for (; e && e->type() != ElementType::SEGMENT; e = e->parent()) {
    }
    return toSegment(e);
}

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure* Chord::measure() const
{
    Element* e = parent();
    for (; e && e->type() != ElementType::MEASURE; e = e->parent()) {
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

Element* Chord::nextElement()
{
    Element* e = score()->selection().element();
    if (!e && !score()->selection().elements().isEmpty()) {
        e = score()->selection().elements().first();
    }

    switch (e->type()) {
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
    case ElementType::FINGERING:
    case ElementType::TEXT:
    case ElementType::BEND: {
        Note* n = toNote(e->parent());
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
        Element* elSt = sp->startElement();
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
        e = e->parent();
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

Element* Chord::prevElement()
{
    Element* e = score()->selection().element();
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
        Element* next = prevNote->lastElementBeforeSegment();
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
        Element* elN = n->lastElementBeforeSegment();
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

Element* Chord::lastElementBeforeSegment()
{
    if (_tremolo) {
        return _tremolo;
    } else if (_arpeggio) {
        return _arpeggio;
    } else {
        Note* n = _notes.front();
        Element* elN = n->lastElementBeforeSegment();
        Q_ASSERT(elN != NULL);
        return elN;
    }
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

Element* Chord::nextSegmentElement()
{
    for (int v = track() + 1; staffIdx() == v / VOICES; ++v) {
        Element* e = segment()->element(v);
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

Element* Chord::prevSegmentElement()
{
    Element* el = score()->selection().element();
    if (!el && !score()->selection().elements().isEmpty()) {
        el = score()->selection().elements().first();
    }
    Element* e = segment()->lastInPrevSegments(el->staffIdx());
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

    foreach (Element* e, el()) {
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
        for (Element* e : note->el()) {
            if (!e->addToSkyline()) {
                continue;
            }
            if (e->isFingering() && toFingering(e)->layoutType() == ElementType::CHORD && e->bbox().isValid()) {
                shape.add(e->bbox().translated(e->pos() + note->pos()));
            }
        }
    }
    for (Element* e : el()) {
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

void Chord::undoChangeProperty(Pid id, const QVariant& newValue)
{
    undoChangeProperty(id, newValue, propertyFlags(id));
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void Chord::undoChangeProperty(Pid id, const QVariant& newValue, PropertyFlags ps)
{
    if (id == Pid::VISIBLE) {
        processSiblings([=](Element* element) {
            element->undoChangeProperty(id, newValue, ps);
        });
    }

    Element::undoChangeProperty(id, newValue, ps);
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
    qreal mag            = (staffType->small() ? score()->styleD(Sid::smallStaffMag) : 1.0) * staffType->userMag();
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

        if (bottom) {
            if (!headSide && stem()) {
                y = upPos() + stem()->stemLen();
                if (beam()) {
                    y += score()->styleS(Sid::beamWidth).val() * _spatium * .5;
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
                y = downPos() + stem()->stemLen();
                if (beam()) {
                    y -= score()->styleS(Sid::beamWidth).val() * _spatium * .5;
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
    qreal distance0 = score()->styleP(Sid::propertyDistance);
    qreal distance2 = score()->styleP(Sid::propertyDistanceStem);

    qreal chordTopY = upPos();      // note position of highest note
    qreal chordBotY = downPos();    // note position of lowest note

    qreal staffTopY = -distance2;
    qreal staffBotY = staff()->height() + distance2;

    // avoid collisions of staff articulations with chord notes:
    // gap between note and staff articulation is distance0 + 0.5 spatium

    if (stem()) {
        qreal y = stem()->pos().y() + pos().y() + stem()->stemLen();
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
    qreal distance1 = score()->styleP(Sid::propertyDistanceHead);
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
            QRectF r = a->bbox().translated(a->pos() + pos());
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
    for (const Articulation* articulation: _articulations) {
        result.insert(articulation->symId());
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
