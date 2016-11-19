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

/**
 \file
 Implementation of classes Chord
*/

#include "chord.h"
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

namespace Ms {

//---------------------------------------------------------
//   LedgerLineData
//---------------------------------------------------------

struct LedgerLineData {
      int   line;
      qreal minX, maxX;
      bool  visible;
      bool  accidental;
      };

//---------------------------------------------------------
//   upNote / downNote
//---------------------------------------------------------

Note* Chord::upNote() const
      {
      Q_ASSERT(!_notes.empty());

      Note* result = _notes.back();
      if (!staff())
            return result;
      if (staff()->isDrumStaff()) {
            for (Note* n : _notes) {
                  if (n->line() < result->line()) {
                        result = n;
                        }
                  }
            }
      else if (staff()->isTabStaff()) {
            StaffType* tab  = staff()->staffType();
            int        line = tab->lines() - 1;      // start at bottom line
            int noteLine;
            // scan each note: if TAB strings are not in sequential order,
            // visual order of notes might not correspond to pitch order
            for (Note* n : _notes) {
                  noteLine = tab->physStringToVisual(n->string());
                  if (noteLine < line) {
                        line = noteLine;
                        result = n;
                        }
                  }
            }
      return result;
      }

Note* Chord::downNote() const
      {
      Q_ASSERT(!_notes.empty());

      Note* result = _notes.front();
      if (!staff())
            return result;
      if (staff()->isDrumStaff()) {
            for (Note* n : _notes) {
                  if (n->line() > result->line()) {
                        result = n;
                        }
                  }
            }
      else if (staff()->isTabStaff()) {
            StaffType* tab  = staff()->staffType();
            int line        = 0;      // start at top line
            int noteLine;
            // scan each note: if TAB strings are not in sequential order,
            // visual order of notes might not correspond to pitch order
            for (Note* n : _notes) {
                  noteLine = tab->physStringToVisual(n->string());
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
      return (staff() && staff()->isTabStaff()) ? upString()*2 : upNote()->line();
      }

int Chord::downLine() const
      {
      return (staff() && staff()->isTabStaff()) ? downString()*2 : downNote()->line();
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
      if(!staff() || !staff()->isTabStaff())
            return 0;
      StaffType* tab = staff()->staffType();
      int       line = tab->lines() - 1;      // start at bottom line
      int                     noteLine;
      // scan each note: if TAB strings are not in sequential order,
      // visual order of notes might not correspond to pitch order
      int n = _notes.size();
      for (int i = 0; i < n; ++i) {
            noteLine = tab->physStringToVisual(_notes.at(i)->string());
            if (noteLine < line)
                  line = noteLine;
            }
      return line;
      }

int Chord::downString() const
      {
      if (!staff())                              // if no staff, return 0
            return 0;
      if (!staff()->isTabStaff())                // if staff not a TAB, return bottom line
            return staff()->lines()-1;
      StaffType* tab = staff()->staffType();
      int       line = 0;         // start at top line
      int        noteLine;
      int n = _notes.size();
      for (int i = 0; i < n; ++i) {
            noteLine = tab->physStringToVisual(_notes.at(i)->string());
            if (noteLine > line)
                  line = noteLine;
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
      _stemDirection    = Direction_AUTO;
      _arpeggio         = 0;
      _tremolo          = 0;
      _endsGlissando    = false;
      _noteType         = NoteType::NORMAL;
      _stemSlash        = 0;
      _noStem           = false;
      _playEventType    = PlayEventType::Auto;
      _crossMeasure     = CrossMeasure::UNKNOWN;
      _graceIndex   = 0;
      setFlags(ElementFlag::MOVABLE | ElementFlag::ON_STAFF);
      }

Chord::Chord(const Chord& c, bool link)
   : ChordRest(c, link)
      {
      if (link)
            score()->undo(new Link(const_cast<Chord*>(&c), this));
      _ledgerLines = 0;

      for (Note* onote : c._notes) {
            Note* nnote = new Note(*onote, link);
            add(nnote);
            }
      for (Chord* gn : c.graceNotes()) {
            Chord* nc = new Chord(*gn, link);
            add(nc);
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

      if (c._stem)
            add(new Stem(*(c._stem)));
      if (c._hook)
            add(new Hook(*(c._hook)));
      if (c._stemSlash)
            add(new StemSlash(*(c._stemSlash)));
      if (c._arpeggio) {
            Arpeggio* a = new Arpeggio(*(c._arpeggio));
            add(a);
            if (link)
                  score()->undo(new Link(const_cast<Arpeggio*>(c._arpeggio), a));
            }
      if (c._tremolo && !c._tremolo->twoNotes()) {
            Tremolo* t = new Tremolo(*(c._tremolo));
            add(t);
            if (link)
                  score()->undo(new Link(const_cast<Tremolo*>(c._tremolo), t));
            }

      for (Element* e : c.el()) {
            if (e->type() == Element::Type::CHORDLINE) {
                  ChordLine* cl = static_cast<ChordLine*>(e);
                  ChordLine* ncl = new ChordLine(*cl);
                  add(ncl);
                  if (link)
                        score()->undo(new Link(const_cast<ChordLine*>(ncl), cl));
                  }
            }
      }

//---------------------------------------------------------
//   undoUnlink
//---------------------------------------------------------

void Chord::undoUnlink()
      {
      ChordRest::undoUnlink();
      for (Note* n : _notes)
            n->undoUnlink();
      for (Chord* gn : graceNotes())
            gn->undoUnlink();

/*      if (_glissando)
            _glissando->undoUnlink(); */
      if (_arpeggio)
            _arpeggio->undoUnlink();
      if (_tremolo && !_tremolo->twoNotes())
            _tremolo->undoUnlink();

      for (Element* e : el()) {
            if (e->type() == Element::Type::CHORDLINE)
                  e->undoUnlink();
            }
      }

//---------------------------------------------------------
//   ~Chord
//---------------------------------------------------------

Chord::~Chord()
      {
      delete _arpeggio;
      if (_tremolo && _tremolo->chord1() == this) {
            if (_tremolo->chord2())
                  _tremolo->chord2()->setTremolo(0);
            delete _tremolo;
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
      if (_noteType != NoteType::NORMAL)
            nhw *= score()->styleD(StyleIdx::graceNoteMag);
      return nhw * mag();
      }

//---------------------------------------------------------
//   stemPosX
//    return Chord coordinates
//---------------------------------------------------------

qreal Chord::stemPosX() const
      {
      if (staff() && staff()->isTabStaff())
            return staff()->staffType()->chordStemPosX(this) * spatium();
      return _up ? noteHeadWidth() : 0.0;
      }

//---------------------------------------------------------
//   stemPos
//    return page coordinates
//---------------------------------------------------------

QPointF Chord::stemPos() const
      {
      qreal _spatium = spatium();
      QPointF p(pagePos());

      if (staff() && staff()->isTabStaff())
            return staff()->staffType()->chordStemPos(this) * _spatium + p;

      if (_up) {
            qreal nhw = noteHeadWidth();
            p.rx() += nhw;
            p.ry() += downNote()->pos().y();
            }
      else
            p.ry() += upNote()->pos().y();
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


      if (staff() && staff()->isTabStaff())
            return staff()->staffType()->chordStemPosBeam(this) * _spatium + p;

      if (_up) {
            qreal nhw = noteHeadWidth();
            p.rx() += nhw;
            p.ry() += upNote()->pos().y();
            }
      else
            p.ry() += downNote()->pos().y();

      return p;
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Chord::setSelected(bool)
      {
/*      Element::setSelected(f);
      int n = _notes.size();
      for (int i = 0; i < n; ++i)
            _notes.at(i)->setSelected(f);
      */
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Chord::add(Element* e)
      {
      e->setParent(this);
      e->setTrack(track());
      switch(e->type()) {
            case Element::Type::NOTE:
                  {
                  Note* note = toNote(e);
                  bool found = false;

                  // _notes should be sorted by line position,
                  // but it's often not yet possible since line is unknown
                  // use pitch instead, and line as a second sort critera.

                  for (unsigned idx = 0; idx < _notes.size(); ++idx) {
                        if (note->pitch() <= _notes[idx]->pitch()) {
                              if (note->pitch() == _notes[idx]->pitch() && note->line() > _notes[idx]->line())
                                    _notes.insert(_notes.begin()+idx+1, note);
                              else
                                    _notes.insert(_notes.begin()+idx, note);
                              found = true;
                              break;
                              }
                        }
                  if (!found)
                        _notes.push_back(note);
                  if (note->tieFor()) {
                        if (note->tieFor()->endNote())
                              note->tieFor()->endNote()->setTieBack(note->tieFor());
                        }
                  if (voice() && measure() && note->visible())
                        measure()->mstaff(staffIdx())->hasVoices = true;
                  }
                  break;
            case Element::Type::ARPEGGIO:
                  _arpeggio = toArpeggio(e);
                  break;
            case Element::Type::TREMOLO:
                  {
                  Tremolo* tr = static_cast<Tremolo*>(e);
                  if (tr->twoNotes()) {
                        if (!(_tremolo && _tremolo->twoNotes())) {
                              TDuration d = durationType();
                              int dots = d.dots();
                              d  = d.shift(-1);
                              d.setDots(dots);
                              if (tr->chord1())
                                    tr->chord1()->setDurationType(d);
                              if (tr->chord2())
                                    tr->chord2()->setDurationType(d);
                              }
                        tr->chord2()->setTremolo(tr);
                        }
                  _tremolo = tr;
                  }
                  break;
            case Element::Type::GLISSANDO:
                  _endsGlissando = true;
                  break;
            case Element::Type::STEM:
                  Q_ASSERT(!_stem);
                  _stem = toStem(e);
                  break;
            case Element::Type::HOOK:
                  _hook = toHook(e);
                  break;
            case Element::Type::CHORDLINE:
                  _el.push_back(e);
                  break;
            case Element::Type::STEM_SLASH:
                  Q_ASSERT(!_stemSlash);
                  _stemSlash = toStemSlash(e);
                  break;
            case Element::Type::CHORD:
                  {
                  Chord* gc = static_cast<Chord*>(e);
                  Q_ASSERT(gc->noteType() != NoteType::NORMAL);
                  int idx = gc->graceIndex();
                  gc->setFlags(ElementFlag::MOVABLE);
                  _graceNotes.insert(_graceNotes.begin() + idx, gc);
                  }
                  break;
            case Element::Type::LEDGER_LINE:
                  qFatal("Chord::add ledgerline");
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
      if (!e)
            return;

      switch (e->type()) {
            case Element::Type::NOTE:
                  {
                  Note* note = toNote(e);
                  auto i = std::find(_notes.begin(), _notes.end(), note);
                  if (i != _notes.end()) {
                        _notes.erase(i);
                        if (note->tieFor()) {
                              if (note->tieFor()->endNote())
                                    note->tieFor()->endNote()->setTieBack(0);
                              }
                        for (Spanner* s : note->spannerBack())
                              note->removeSpannerBack(s);
                        for (Spanner* s : note->spannerFor())
                              note->removeSpannerFor(s);
                        }
                  else
                        qDebug("Chord::remove() note %p not found!", e);
                  if (voice() && measure() && note->visible())
                        measure()->checkMultiVoices(staffIdx());
                  }
                  break;

            case Element::Type::ARPEGGIO:
                  _arpeggio = 0;
                  break;
            case Element::Type::TREMOLO:
                  {
                  Tremolo* tremolo = static_cast<Tremolo*>(e);
                  if (tremolo->twoNotes()) {
                        TDuration d = durationType();
                        int dots = d.dots();
                        d          = d.shift(1);
                        d.setDots(dots);
                        Fraction f = duration();
                        if (f.numerator() > 0)
                              d = TDuration(f);
                        if (tremolo->chord1())
                              tremolo->chord1()->setDurationType(d);
                        if (tremolo->chord2()) {
                              tremolo->chord2()->setDurationType(d);
                              tremolo->chord2()->setTremolo(0);
                              }
                        }
                  _tremolo = 0;
                  }
                  break;
            case Element::Type::GLISSANDO:
                  _endsGlissando = false;
                  break;
            case Element::Type::STEM:
                  _stem = 0;
                  break;
            case Element::Type::HOOK:
                  _hook = 0;
                  break;
            case Element::Type::STEM_SLASH:
                  Q_ASSERT(_stemSlash);
                  if (_stemSlash->selected() && score())
                        score()->deselect(_stemSlash);
                  _stemSlash = 0;
                  break;
            case Element::Type::CHORDLINE:
                  _el.remove(e);
                  break;
            case Element::Type::CHORD:
                  {
                  auto i = std::find(_graceNotes.begin(), _graceNotes.end(), static_cast<Chord*>(e));
                  Chord* grace = *i;
                  grace->setGraceIndex(i - _graceNotes.begin());
                  _graceNotes.erase(i);
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
      qreal hw       = 0;
      for (unsigned i = 0; i < _notes.size(); i++) {
            qreal t = _notes.at(i)->headWidth();
            if (t > hw)
                  hw = t;
            }
      return hw;
      }

//---------------------------------------------------------
//   addLedgerLines
//---------------------------------------------------------

void Chord::addLedgerLines()
      {
      // initialize for palette
      int track          = 0;                   // the track lines belong to
      // the line pos corresponding to the bottom line of the staff
      int lineBelow      = 8;                   // assuming 5-lined "staff"
      qreal lineDistance = 1;
      qreal _mag         = 1;
      bool staffVisible  = true;

      if (segment()) { //not palette
            int idx      = staffIdx() + staffMove();
            track        = staff2track(idx);
            Staff* st    = score()->staff(idx);
            lineBelow    = (st->lines() - 1) * 2;
            lineDistance = st->lineDistance();
            _mag         = staff()->mag();
            staffVisible = !staff()->invisible();
            }

      // need ledger lines?
      if (downLine() <= lineBelow + 1 && upLine() >= -1)
            return;

      LedgerLineData lld;
      // the extra length of a ledger line with respect to notehead (half of it on each side)
      qreal extraLen = score()->styleP(StyleIdx::ledgerLineLength) * _mag * 0.5;
      qreal hw = _notes[0]->headWidth();
      qreal minX, maxX;                         // note extrema in raster units
      int   minLine, maxLine;
      bool  visible = false;
      qreal x;

      // scan chord notes, collecting visibility and x and y extrema
      // NOTE: notes are sorted from bottom to top (line no. decreasing)
      // notes are scanned twice from outside (bottom or top) toward the staff
      // each pass stops at the first note without ledger lines
      int n = _notes.size();
      for (int j = 0; j < 2; j++) {             // notes are scanned twice...
            int from, delta;
            vector<LedgerLineData> vecLines;
            minX  = maxX = 0;
            minLine = 0;
            maxLine = lineBelow;
            if (j == 0) {                       // ...once from lowest up...
                  from  = 0;
                  delta = +1;
                  }
            else {
                  from = n-1;                   // ...once from highest down
                  delta = -1;
                  }
            for (int i = from; i < n && i >=0 ; i += delta) {
                  Note* note = _notes.at(i);
                  int l = note->physicalLine();

                  // if 1st pass and note not below staff or 2nd pass and note not above staff
                  if ((!j && l <= lineBelow + 1) || (j && l >= -1))
                        break;                  // stop this pass
                  // round line number to even number toward 0
                  if (l < 0)
                        l = (l + 1) & ~ 1;
                  else
                        l = l & ~ 1;

                  if (note->visible())          // if one note is visible,
                        visible = true;         // all lines between it and the staff are visible

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
                  x = note->pos().x();
                  if (x - extraLen < minX) {
                        minX  = x - extraLen;
                        // increase width of all lines between this one and the staff
                        for (auto& d : vecLines) {
                              if (!d.accidental && ((l < 0 && d.line >= l) || (l > 0 && d.line <= l)) )
                                    d.minX = minX ;
                              }
                        }
                  // same for left side
                  if (x + hw + extraLen > maxX) {
                        maxX = x + hw + extraLen;
                        for (auto& d : vecLines)
                              if ( (l < 0 && d.line >= l) || (l > 0 && d.line <= l) )
                                    d.maxX = maxX;
                        }

                  // check if note vert. pos. is outside current range
                  // and, if so, add data for new line(s)
                  if (l < minLine) {
                        for (int i = l; i < minLine; i += 2) {
                              lld.line = i;
                              if (lineDistance != 1.0)
                                    lld.line *= lineDistance;
                              lld.minX = minX;
                              lld.maxX = maxX;
                              lld.visible = visible;
                              lld.accidental = false;
                              vecLines.push_back(lld);
                              }
                        minLine = l;
                        }
                  if (l > maxLine) {
                        for (int i = maxLine+2; i <= l; i += 2) {
                              lld.line = i;
                              if (lineDistance != 1.0)
                                    lld.line *= lineDistance;
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
                  qreal stepDistance = 0.5;     // staff() ? staff()->lineDistance() * 0.5 : 0.5;
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
      for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next())
            ll->layout();
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
      StaffType* tab = 0;
      // TAB STAVES
      if (staff() && staff()->isTabStaff()) {
            tab = staff()->staffType();
            // if no stems or stem beside staves
            if (tab->slashStyle() || !tab->stemThrough()) {
                  // if measure has voices, set stem direction according to voice
                  if (measure()->mstaff(staffIdx())->hasVoices)
                        _up = !(track() % 2);
                  else                          // if only voice 1,
                        // uncondtionally set to down if not stems or according to TAB stem direction otherwise
                        // (even with no stems, stem direction controls position of slurs and ties)
                        _up = tab->slashStyle() ? false : !tab->stemsDown();
                  return;
                  }
            // if TAB has stems through staves, chain into standard processing
            }

      // PITCHED STAVES (or TAB with stems through staves)
      if (_stemDirection != Direction_AUTO) {
            _up = _stemDirection == Direction_UP;
            }
      else if (!parent())
            // hack for palette and drumset editor
            _up = upNote()->line() > 4;
      else if (_noteType != NoteType::NORMAL) {
            //
            // stem direction for grace notes
            //
            if (measure()->mstaff(staffIdx())->hasVoices)
                  _up = !(track() % 2);
            else
                  _up = true;
            }
      else if (staffMove())
            _up = staffMove() > 0;
      else if (measure()->mstaff(staffIdx())->hasVoices) {
            _up = !(track() % 2);
            }
      else {
            int   dnMaxLine   = staff()->middleLine();
            int   ud          = (tab ? upString()*2 : upNote()->line() ) - dnMaxLine;
            // standard case: if only 1 note or cross beaming
            if (_notes.size() == 1 || staffMove()) {
                  if (staffMove() > 0)
                        _up = true;
                  else if (staffMove() < 0)
                        _up = false;
                  else
                        _up = ud > 0;
                  }
            // if more than 1 note, compare extrema (topmost and bottommost notes)
            else {
                  int dd = (tab ? downString()*2 : downNote()->line() ) - dnMaxLine;
                  // if extrema symmetrical, average directions of intermediate notes
                  if (-ud == dd) {
                        int up = 0;
                        int n = _notes.size();
                        for (int i = 0; i < n; ++i) {
                              const Note* n = _notes.at(i);
                              int l = tab ? n->string()*2 : n->line();
                              if (l <= dnMaxLine)
                                    --up;
                              else
                                    ++up;
                              }
                        _up = up > 0;
                        }
                  // if extrema not symmetrical, set _up to prevailing
                  else
                        _up = dd > -ud;
                  }
            }
      }

//---------------------------------------------------------
//   selectedNote
//---------------------------------------------------------

Note* Chord::selectedNote() const
      {
      Note* note = 0;
      int n = _notes.size();
      for (int i = 0; i < n; ++i) {
            Note* n = _notes.at(i);
            if (n->selected()) {
                  if (note)
                        return 0;
                  note = n;
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
            c->writeBeam(xml);
            c->write(xml);
            }
      xml.stag("Chord");
      ChordRest::writeProperties(xml);
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

      if (_noStem)
            xml.tag("noStem", _noStem);
      else if (_stem && (_stem->isUserModified() || (_stem->userLen() != 0.0)))
            _stem->write(xml);
      if (_hook && _hook->isUserModified())
            _hook->write(xml);
      if (_stemSlash && _stemSlash->isUserModified())
            _stemSlash->write(xml);
      writeProperty(xml, P_ID::STEM_DIRECTION);
      for (Note* n : _notes)
            n->write(xml);
      if (_arpeggio)
            _arpeggio->write(xml);
      if (_tremolo && tremoloChordType() != TremoloChordType::TremoloSecondNote)
            _tremolo->write(xml);
      for (Element* e : _el)
            e->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Chord::read
//---------------------------------------------------------

void Chord::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (readProperties(e))
                  ;
            else
                  e.unknown();
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
            }
      else if (ChordRest::readProperties(e))
            ;
      else if (tag == "Stem") {
            Stem* s = new Stem(score());
            s->read(e);
            add(s);
            }
      else if (tag == "Hook") {
            _hook = new Hook(score());
            _hook->read(e);
            add(_hook);
            }
      else if (tag == "appoggiatura") {
            _noteType = NoteType::APPOGGIATURA;
            e.readNext();
            }
      else if (tag == "acciaccatura") {
            _noteType = NoteType::ACCIACCATURA;
            e.readNext();
            }
      else if (tag == "grace4") {
            _noteType = NoteType::GRACE4;
            e.readNext();
            }
      else if (tag == "grace16") {
            _noteType = NoteType::GRACE16;
            e.readNext();
            }
      else if (tag == "grace32") {
            _noteType = NoteType::GRACE32;
            e.readNext();
            }
      else if (tag == "grace8after") {
            _noteType = NoteType::GRACE8_AFTER;
            e.readNext();
            }
      else if (tag == "grace16after") {
            _noteType = NoteType::GRACE16_AFTER;
            e.readNext();
            }
      else if (tag == "grace32after") {
            _noteType = NoteType::GRACE32_AFTER;
            e.readNext();
            }
      else if (tag == "StemSlash") {
            StemSlash* ss = new StemSlash(score());
            ss->read(e);
            add(ss);
            }
      else if (tag == "StemDirection")
            readProperty(e, P_ID::STEM_DIRECTION);
      else if (tag == "noStem")
            _noStem = e.readInt();
      else if (tag == "Arpeggio") {
            _arpeggio = new Arpeggio(score());
            _arpeggio->setTrack(track());
            _arpeggio->read(e);
            _arpeggio->setParent(this);
            }
      // old glissando format, chord-to-chord, attached to its final chord
      else if (tag == "Glissando") {
            // the measure we are reading is not inserted in the score yet
            // as well as, possibly, the glissando intended initial chord;
            // then we cannot fully link the glissando right now;
            // temporarily attach the glissando to its final note as a back spanner;
            // after the whole score is read, Score::connectTies() will look for
            // the suitable initial note
            Note* finalNote = upNote();
            Glissando* gliss = new Glissando(score());
            gliss->read(e);
            gliss->setAnchor(Spanner::Anchor::NOTE);
            gliss->setStartElement(nullptr);
            gliss->setEndElement(nullptr);
            // in TAB, use straight line with no text
            if (score()->staff(e.track() >> 2)->isTabStaff()) {
                  gliss->setGlissandoType(Glissando::Type::STRAIGHT);
                  gliss->setShowText(false);
                  }
            finalNote->addSpannerBack(gliss);
            }
      else if (tag == "Tremolo") {
            _tremolo = new Tremolo(score());
            _tremolo->setTrack(track());
            _tremolo->read(e);
            _tremolo->setParent(this);
            }
      else if (tag == "tickOffset")       // obsolete
            ;
      else if (tag == "ChordLine") {
            ChordLine* cl = new ChordLine(score());
            cl->read(e);
            add(cl);
            }
      else
            return false;
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
      if (staff()->isTabStaff())
            return staff()->staffType()->chordStemPosX(this) * spatium();

      const Note* note = up() ? upNote() : downNote();
      qreal x = note->pos().x();
      x += note->headWidth() * .5;
      if (note->mirror()) {
            x += note->headWidth() * (up() ? -1.0 : 1.0);
            }
      return x;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Chord::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (_hook)
            func(data, _hook );
      if (_stem)
            func(data, _stem);
      if (_stemSlash)
            func(data, _stemSlash);
      if (_arpeggio)
            func(data, _arpeggio);
      if (_tremolo && (tremoloChordType() != TremoloChordType::TremoloSecondNote))
            func(data, _tremolo);
      Staff* st = staff();
      if ((st && st->showLedgerLines()) || !st)       // also for palette
            for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next())
                  func(data, ll);
      int n = _notes.size();
      for (int i = 0; i < n; ++i)
            _notes.at(i)->scanElements(data, func, all);
      n = _el.size();
      for (Chord* chord : _graceNotes)
            chord->scanElements(data, func, all);
      for (Element* e : _el)
            e->scanElements(data, func, all);
      ChordRest::scanElements(data, func, all);
      }

//---------------------------------------------------------
//   processSiblings
//---------------------------------------------------------

void Chord::processSiblings(std::function<void(Element*)> func) const
      {
      if (_hook)
            func(_hook);
      if (_stem)
            func(_stem);
      if (_stemSlash)
            func(_stemSlash);
      if (_arpeggio)
            func(_arpeggio);
      if (_tremolo)
            func(_tremolo);
      for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next())
            func(ll);
      for (Note* note : _notes)
            func(note);
      for (Element* e : _el)
            func(e);
      for (Chord* chord : _graceNotes)    // process grace notes last, needed for correct shape calculation
            func(chord);
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Chord::setTrack(int val)
      {
      ChordRest::setTrack(val);
      processSiblings([val] (Element* e) { e->setTrack(val); } );
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Chord::setScore(Score* s)
      {
      ChordRest::setScore(s);
      processSiblings([s] (Element* e) { e->setScore(s); } );
      }

//-----------------------------------------------------------------------------
//   defaultStemLength
///   Get the default stem length for this chord
//-----------------------------------------------------------------------------

qreal Chord::defaultStemLength()
      {
      qreal _spatium = spatium();
      Note* downnote;
      int dl, ul;
      qreal stemLen;
      int hookIdx       = durationType().hooks();
      downnote          = downNote();
      ul = upLine();
      dl = downLine();
      Staff* st = staff();
      qreal physicalLineDistance = st ? st->lineDistance() : 1.0;
      qreal logicalLineDistance = st ? st->logicalLineDistance() : 1.0;

      StaffType* tab = 0;
      if (st && st->isTabStaff()) {
            tab = st->staffType();
            // require stems only if TAB is not stemless and this chord has a stem
            if (!tab->slashStyle() && _stem) {
                  // if stems are beside staff, apply special formatting
                  if (!tab->stemThrough()) {
                        // process stem:
                        return tab->chordStemLength(this) * _spatium;
                        }
                  }
            }
      else if (logicalLineDistance != 1.0) {
            // convert to actual distance from top of staff in sp
            ul *= logicalLineDistance;
            dl *= logicalLineDistance;
            }

      if (tab && !tab->onLines()) {       // if TAB and frets above strings, move 1 position up
            --ul;
            --dl;
            }
      bool shortenStem = score()->styleB(StyleIdx::shortenStem);
      if (hookIdx >= 2 || _tremolo)
            shortenStem = false;

      Spatium progression = score()->styleS(StyleIdx::shortStemProgression);
      qreal shortest      = score()->styleS(StyleIdx::shortestStem).val();

      qreal normalStemLen = small() ? 2.5 : 3.5;
      switch(hookIdx) {
            case 3: normalStemLen += small() ? .5  : 0.75; break; //32nd notes
            case 4: normalStemLen += small() ? 1.0 : 1.5;  break; //64th notes
            case 5: normalStemLen += small() ? 1.5 : 2.25; break; //128th notes
            }
      if (_hook && tab == 0) {
            if (up() && durationType().dots()) {
                  //
                  // avoid collision of dot with hook
                  //
                  if (!(ul & 1))
                        normalStemLen += .5;
                  shortenStem = false;
                  }
            }

      if (_noteType != NoteType::NORMAL) {
            // grace notes stems are not subject to normal
            // stem rules
            stemLen =  qAbs(ul - dl) * .5;
            stemLen += normalStemLen * score()->styleD(StyleIdx::graceNoteMag);
            if (up())
                  stemLen *= -1;
            }
      else {
            // normal note (not grace)
            qreal staffHeight = st ? st->lines() - 1 : 4;
            if (physicalLineDistance != 1.0 && !tab)
                  staffHeight *= physicalLineDistance;
            qreal staffHlfHgt = staffHeight * 0.5;
            if (up()) {                   // stem up
                  qreal dy  = dl * .5;                      // note-side vert. pos.
                  qreal sel = ul * .5 - normalStemLen;      // stem end vert. pos

                  // if stem ends above top line (with some exceptions), shorten it
                  if (shortenStem && (sel < 0.0) && (hookIdx == 0 || tab || !downnote->mirror()))
                        sel -= sel  * progression.val();
                  if (sel > staffHlfHgt)                    // if stem ends below ('>') staff mid position,
                        sel = staffHlfHgt;                  // stretch it to mid position
                  stemLen = sel - dy;                       // actual stem length
                  if (-stemLen < shortest)                  // is stem too short,
                        stemLen = -shortest;                // lengthen it to shortest possible length
                  }
            else {                        // stem down
                  qreal uy  = ul * .5;                      // note-side vert. pos.
                  qreal sel = dl * .5 + normalStemLen;      // stem end vert. pos.

                  // if stem ends below bottom line (with some exceptions), shorten it
                  if (shortenStem && (sel > staffHeight) && (hookIdx == 0 || tab || downnote->mirror()))
                        sel -= (sel - staffHeight)  * progression.val();
                  if (sel < staffHlfHgt)                    // if stem ends above ('<') staff mid position,
                        sel = staffHlfHgt;                  // stretch it to mid position
                  stemLen = sel - uy;                       // actual stem length
                  if (stemLen < shortest)                   // if stem too short,
                        stemLen = shortest;                 // lengthen it to shortest possible position
                  }
            }

      // adjust stem len for tremolo
      if (_tremolo && !_tremolo->twoNotes()) {
            // hook up odd lines
            static const int tab[2][2][2][4] = {
                  { { { 0, 0, 0,  1 },  // stem - down - even - lines
                      { 0, 0, 0,  2 }   // stem - down - odd - lines
                      },
                    { { 0, 0, 0, -1 },  // stem - up - even - lines
                      { 0, 0, 0, -2 }   // stem - up - odd - lines
                      }
                    },
                  { { { 0, 0, 1, 2 },   // hook - down - even - lines
                      { 0, 0, 1, 2 }    // hook - down - odd - lines
                      },
                    { { 0, 0, -1, -2 }, // hook - up - even - lines
                      { 0, 0, -1, -2 }  // hook - up - odd - lines
                      }
                    }
                  };
            int odd = (up() ? upLine() : downLine()) & 1;
            int n = tab[_hook ? 1 : 0][up() ? 1 : 0][odd][_tremolo->lines()-1];
            stemLen += n * .5;
            }
      // TAB: scale stemLen according to staff line spacing
      if (tab)
            stemLen *= physicalLineDistance;

      return stemLen * _spatium;
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
      if (durationType().hasStem() && !(_noStem || measure()->slashStyle(staffIdx())
         || (staff() && staff()->isTabStaff() && staff()->staffType()->slashStyle()))) {
            if (!_stem) {
                  Stem* stem = new Stem(score());
                  stem->setParent(this);
                  stem->setGenerated(true);
                  score()->undoAddElement(stem);
                  }
            if ((_noteType == NoteType::ACCIACCATURA) && !(beam() && beam()->elements().front() != this)) {
                  if (!_stemSlash)
                        add(new StemSlash(score()));
                  }
            else if (_stemSlash)
                  remove(_stemSlash);

            qreal stemWidth5 = _stem->lineWidth() * .5;
            _stem->rxpos()   = stemPosX() + (up() ? -stemWidth5 : +stemWidth5);
            _stem->setLen(defaultStemLength());
            }
      else {
            if (_stem)
                  score()->undoRemoveElement(_stem);
            if (_stemSlash)
                  score()->undoRemoveElement(_stemSlash);
            }
      }

//-----------------------------------------------------------------------------
//   layoutStem
///   Layout chord tremolo stem and hook.
//
//    hook: sets position
//    stem: sets length, but not position (assumed to be set in Chord::layout())
//-----------------------------------------------------------------------------

void Chord::layoutStem()
      {
      for (Chord* c : _graceNotes)
            c->layoutStem();
      if (_beam)
            return;

      // create hooks for unbeamed chords

      int hookIdx  = durationType().hooks();

      if (hookIdx && !(noStem() || measure()->slashStyle(staffIdx()))) {
            if (!hook()) {
                  Hook* hook = new Hook(score());
                  hook->setParent(this);
                  hook->setGenerated(true);
                  score()->undoAddElement(hook);
                  }
            hook()->setHookType(up() ? hookIdx : -hookIdx);
            }
      else if (hook())
            score()->undoRemoveElement(hook());

      //
      // TAB
      //
      StaffType* tab = 0;
      if (staff() && staff()->isTabStaff()) {
            tab = staff()->staffType();
            // if stemless TAB
            if (tab->slashStyle()) {
                  // if 'grid' duration symbol of MEDIALFINAL type, it is time to compute its width
                  if (_tabDur != nullptr && _tabDur->beamGrid() == TabBeamGrid::MEDIALFINAL)
                        _tabDur->layout2();
                  // in all other stemless cases, do nothing
                  return;
                  }
            // not a stemless TAB; if stems are beside staff, apply special formatting
            if (!tab->stemThrough()) {
                  if (_stem) { // (duplicate code with defaultStemLength())
                        // process stem:
                        _stem->setLen(tab->chordStemLength(this) * spatium());
                        // process hook
                        int   hookIdx = durationType().hooks();
                        if (!up())
                              hookIdx = -hookIdx;
                        if (hookIdx && _hook) {
                              _hook->setHookType(hookIdx);
                              qreal x = _stem->pos().x() + _stem->lineWidth() * .5;;
                              qreal y = _stem->pos().y();
                              if (up()) {
                                    y -= _stem->bbox().height();
                                    x -= _stem->width();
                                    }
                              else {
                                    y += _stem->bbox().height();
                                    x -= _stem->width();
                                    }
                              _hook->setPos(x, y);
                              _hook->adjustReadPos();
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
            _stem->setLen(defaultStemLength());
            // if (isGrace())
            //      abort();
            if (_hook) {
                  _hook->layout();
                  QPointF p(_stem->hookPos());
                  if (up()) {
                        p.ry() -= _hook->bbox().top();
                        p.rx() -= _stem->width();
                        }
                  else {
                        p.ry() -= _hook->bbox().bottom();
                        p.rx() -= _stem->width();
                        }
                  _hook->setPos(p);
                  _hook->adjustReadPos();
                  }
            if (_stemSlash)
                  _stemSlash->layout();
            }

      //-----------------------------------------
      //    process tremolo
      //-----------------------------------------

      if (_tremolo)
            _tremolo->layout();
      }

//---------------------------------------------------------
//    underBeam: true, if grace note is placed under a beam.
//---------------------------------------------------------

bool Chord::underBeam() const
      {
      if(_noteType == NoteType::NORMAL)
          return false;
      const Chord* cr = static_cast<Chord*>(parent());
      Beam* beam = cr->beam();
      if(!beam || !cr->beam()->up())
            return false;
      int s = beam->elements().count();
      if(isGraceBefore()){
            if(beam->elements()[0] != cr)
                return true;
            }
      if(isGraceAfter()){
            if(beam->elements()[s - 1] != cr)
                return true;
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
      for (Chord* c : _graceNotes)
            c->layout2();

      qreal _spatium = spatium();
      qreal _mag = staff()->mag();

      //
      // Experimental:
      //    look for colliding ledger lines
      //

      const qreal minDist = _spatium * .17;

      Segment* s = segment()->prev(Segment::Type::ChordRest);
      if (s) {
            int strack = staff2track(staffIdx());
            int etrack = strack + VOICES;

            for (LedgerLine* h = _ledgerLines; h; h = h->next()) {
                  qreal len = h->len();
                  qreal y   = h->y();
                  qreal x   = h->x();
                  bool found = false;
                  qreal cx  = h->measureXPos();

                  for (int track = strack; track < etrack; ++track) {
                        Chord* e = static_cast<Chord*>(s->element(track));
                        if (!e || e->type() != Element::Type::CHORD)
                              continue;
                        for (LedgerLine* ll = e->ledgerLines(); ll; ll = ll->next()) {
                              if (ll->y() != y)
                                    continue;

                              qreal d = cx - ll->measureXPos() - ll->len();
                              if (d < minDist) {
                                    //
                                    // the ledger lines overlap
                                    //
                                    qreal shorten = (minDist - d) * .5;
                                    x   += shorten;
                                    len -= shorten;
                                    ll->setLen(ll->len() - shorten);
                                    h->setLen(len);
                                    h->setPos(x, y);
                                    }
                              found = true;
                              break;
                              }
                        if (found)
                              break;
                        }
                  }
            }

      //
      // position after-chord grace notes
      // room for them has been reserved in Chord::layout()
      //

      QVector<Chord*> gna = graceNotesAfter();
      if (!gna.empty()) {
            qreal minNoteDist = score()->styleP(StyleIdx::minNoteDistance) * _mag * score()->styleD(StyleIdx::graceNoteMag);
            // position grace notes from the rightmost to the leftmost
            // get segment (of whatever type) at the end of this chord; if none, get measure last segment
            Segment* s = measure()->tick2segment(segment()->tick() + actualTicks(), Segment::Type::All);
            if (s == nullptr)
                  s = measure()->last();
            if (s == segment())           // if our segment is the last, no adjacent segment found
                  s = nullptr;
            // start from the right (if next segment found, x of it relative to this chord;
            // chord right space otherwise)
            qreal xOff =  s ? s->pos().x() - (segment()->pos().x() + pos().x()) : _spaceRw;
            // final distance: if near to another chord, leave minNoteDist at right of last grace
            // else leave note-to-barline distance;
            xOff -= (s != nullptr && s->segmentType() != Segment::Type::ChordRest)
                  ? score()->styleP(StyleIdx::noteBarDistance) * _mag
                  : minNoteDist;
            // scan grace note list from the end
            int n = gna.size();
            for (int i = n-1; i >= 0; i--) {
                  Chord* g = gna.value(i);
                  xOff -= g->_spaceRw;                  // move to left by grace note left space (incl. grace own width)
                  g->rxpos() = xOff;
                  xOff -= minNoteDist + g->_spaceLw;    // move to left by grace note right space and inter-grace distance
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

      StaffGroup staffGroup = staff()->staffType()->group();
      if (staffGroup == StaffGroup::TAB) {
            const Instrument* instrument = part()->instrument();
            for (Chord* ch : graceNotes())
                  instrument->stringData()->fretChords(ch);
            instrument->stringData()->fretChords(this);
            return;
            }

      // PITCHED_ and PERCUSSION_STAFF can go note by note

      for (Chord* ch : graceNotesBefore()) {
            if (staffGroup != StaffGroup::PERCUSSION) {
                  std::vector<Note*> notes(ch->notes());  // we need a copy!
                  for (Note* note : notes)
                        note->updateAccidental(as);
                  }
            ch->sortNotes();
            }

      std::vector<Note*> lnotes(notes());  // we need a copy!

      if (staffGroup == StaffGroup::STANDARD) {
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
            }
      else if (staffGroup == StaffGroup::PERCUSSION) {
            const Instrument* instrument = part()->instrument();
            const Drumset* drumset = instrument->drumset();
            if (!drumset)
                  qWarning("no drumset");
            for (Note* note : lnotes) {
                  if (!drumset)
                        note->setLine(0);
                  else {
                        int pitch = note->pitch();
                        if (!drumset->isValid(pitch)) {
                              note->setLine(0);
                              qWarning("unmapped drum note %d", pitch);
                              }
                        else if (!note->fixed()) {
                              note->undoChangeProperty(P_ID::HEAD_GROUP, int(drumset->noteHead(pitch)));
                              // note->setHeadGroup(drumset->noteHead(pitch));
                              note->setLine(drumset->line(pitch));
                              }
                        }
                  }
            }

      for (Chord* ch : graceNotesAfter()) {
            if (staffGroup != StaffGroup::PERCUSSION) {
                  std::vector<Note*> notes(ch->notes());  // we need a copy!
                  for (Note* note : notes)
                        note->updateAccidental(as);
                  }
            ch->sortNotes();
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
            if (parent() == 0)
                  return p;
            p.rx() = pageX();

            const Chord* pc = static_cast<const Chord*>(parent());
            System* system = pc->segment()->system();
            if (!system)
                  return p;
            p.ry() += system->staffYpage(vStaffIdx());
            return p;
            }
      return Element::pagePos();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Chord::layout()
      {
      if (_notes.empty())
            return;
      if (staff() && staff()->isTabStaff())
            layoutTablature();
      else
            layoutPitched();
      }

//---------------------------------------------------------
//   layoutPitched
//---------------------------------------------------------

void Chord::layoutPitched()
      {
      int gi = 0;
      for (Chord* c : _graceNotes) {
            // HACK: graceIndex is not well-maintained on add & remove
            // so rebuild now
            c->setGraceIndex(gi++);
            if (c->isGraceBefore())
                  c->layoutPitched();
            }
      QVector<Chord*> graceNotesBefore = Chord::graceNotesBefore();
      int gnb = graceNotesBefore.size();

      // lay out grace notes after separately so they are processed left to right
      // (they are normally stored right to left)

      QVector<Chord*> gna = graceNotesAfter();
      for (Chord* c : gna)
            c->layoutPitched();

      qreal _spatium         = spatium();
      qreal _mag             = staff() ? staff()->mag() : 1.0;    // palette elements do not have a staff
      qreal dotNoteDistance  = score()->styleP(StyleIdx::dotNoteDistance)  * _mag;
      qreal minNoteDistance  = score()->styleP(StyleIdx::minNoteDistance)  * _mag;
      qreal minTieLength     = score()->styleP(StyleIdx::MinTieLength)     * _mag;
      qreal ledgerLineLength = score()->styleP(StyleIdx::ledgerLineLength) * _mag;

      qreal graceMag         = score()->styleD(StyleIdx::graceNoteMag);
      qreal chordX           = (_noteType == NoteType::NORMAL) ? ipos().x() : 0.0;

      while (_ledgerLines) {
            LedgerLine* l = _ledgerLines->next();
            delete _ledgerLines;
            _ledgerLines = l;
            }

      qreal lll    = 0.0;         // space to leave at left of chord
      qreal rrr    = 0.0;         // space to leave at right of chord
      qreal lhead  = 0.0;         // amount of notehead to left of chord origin
      Note* upnote = upNote();

      delete _tabDur;   // no TAB? no duration symbol! (may happen when converting a TAB into PITCHED)
      _tabDur = 0;

      if (!segment()) {
            //
            // hack for use in palette
            //
            int n = _notes.size();
            for (int i = 0; i < n; i++) {
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

      for (Note* note : _notes) {
            note->layout();

            qreal x1 = note->pos().x() + chordX;
            qreal x2 = x1 + note->headWidth();
            lll      = qMax(lll, -x1);
            rrr      = qMax(rrr, x2);
            // track amount of space due to notehead only
            lhead    = qMax(lhead, -x1);

            Accidental* accidental = note->accidental();
            if (accidental && !note->fixed()) {
                  // convert x position of accidental to segment coordinate system
                  qreal x = accidental->pos().x() + note->pos().x() + chordX;
                  // distance from accidental to note already taken into account
                  // but here perhaps we create more padding in *front* of accidental?
                  x -= score()->styleP(StyleIdx::accidentalDistance) * _mag;
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
                                    qreal snEnd = sn->x() + sn->headWidth();
                                    qreal scEnd = snEnd;
                                    for (unsigned i = 0; i < sc->notes().size(); ++i)
                                          scEnd = qMax(scEnd, sc->notes().at(i)->x() + sc->notes().at(i)->headWidth());
                                    overlap += scEnd - snEnd;
                                    }
                              else
                                    overlap -= sn->headWidth() * 0.12;
                              }
                        else
                              overlap += sn->headWidth() * 0.35;
                        if (notes().size() > 1 || (stem() && !up() && !tie->up())) {
                              // for positive offset:
                              //    use available space
                              // for negative x offset:
                              //    space is allocated elsewhere, so don't re-allocate here
                              if (note->ipos().x() != 0.0)
                                    overlap += qAbs(note->ipos().x());
                              else
                                    overlap -= note->headWidth() * 0.12;
                              }
                        else {
                              if (shortStart)
                                    overlap += note->headWidth() * 0.15;
                              else
                                    overlap += note->headWidth() * 0.35;
                              }
                        qreal d = qMax(minTieLength - overlap, 0.0);
                        lll = qMax(lll, d);
                        }
                  }
            }

      //-----------------------------------------
      //  create ledger lines
      //-----------------------------------------

      addLedgerLines();

      if (_arpeggio) {
            qreal arpeggioDistance = score()->styleP(StyleIdx::ArpeggioNoteDistance) * _mag;
            _arpeggio->layout();    // only for width() !
            lll        += _arpeggio->width() + arpeggioDistance + chordX;
            qreal y1   = upnote->pos().y() - upnote->headHeight() * .5;
            _arpeggio->setPos(-lll, y1);
            _arpeggio->adjustReadPos();
            // _arpeggio->layout() called in layoutArpeggio2()

            // handle the special case of _arpeggio->span() > 1
            // in layoutArpeggio2() after page layout has done so we
            // know the y position of the next staves
            }

      // allocate enough room for glissandi
      if (_endsGlissando) {
            if (rtick()                                     // if not at beginning of measure
                        || graceNotesBefore.size() > 0)     // or there are graces before
                  lll += _spatium * 0.5 + minTieLength;
            // special case of system-initial glissando final note is handled in Glissando::layout() itself
            }

      if (dots()) {
            qreal x = dotPosX() + dotNoteDistance
               + (dots()-1) * score()->styleP(StyleIdx::dotDotDistance) * _mag;
            x += symWidth(SymId::augmentationDot);
            rrr = qMax(rrr, x);
            }

      if (_hook) {
            if (beam())
                  score()->undoRemoveElement(_hook);
            else {
                  _hook->layout();
                  if (up() && stem()) {
                        // hook position is not set yet
                        qreal x = _hook->bbox().right() + stem()->hookPos().x() + chordX;
                        rrr = qMax(rrr, x);
                        }
                  }
            }

      if (_ledgerLines) {
            // we may need to increase distance to previous chord
            Chord* pc = 0;

            if (_noteType == NoteType::NORMAL) {
                  // normal note
                  if (gnb) {
                        // if there are grace notes before, get last
                        pc = graceNotesBefore.last();
                        }
                  else if (rtick()) {
                        // if this is not first chord of measure, get previous chord
                        Segment* s = segment()->prev(Segment::Type::ChordRest);
                        if (s) {
                              // prefer chord in same voice
                              // but if nothing there, look at other voices
                              // note this still leaves the possibility
                              // that this voice does not have conflict but another voice does
                              Element* e = s->element(track());
                              if (e && e->type() == Element::Type::CHORD)
                                    pc = static_cast<Chord*>(e);
                              else {
                                    int startTrack = staffIdx() * VOICES;
                                    int endTrack = startTrack + VOICES;
                                    for (int t = startTrack; t < endTrack; ++t) {
                                          if (t == track())  // already checked current voice
                                                continue;
                                          e = s->element(t);
                                          if (e && e->type() == Element::Type::CHORD) {
                                                pc = static_cast<Chord*>(e);
                                                // prefer chord with ledger lines
                                                if (pc->ledgerLines())
                                                      break;
                                                }
                                          }
                                    }
                              }
                        }
                  if (pc && !pc->graceNotes().empty()) {
                        // if previous chord has grace notes after, find last one
                        // which, conveniently, is stored first
                        for (Chord* c : pc->graceNotes()) {
                              if (c->isGraceAfter()) {
                                    pc = c;
                                    break;
                                    }
                              }
                        }
                  }

            else {
                  // grace note
                  Chord* mainChord = static_cast<Chord*>(parent());
                  bool before = isGraceBefore();
                  int incIdx = before ? -1 : 1;
                  int endIdx = before ? -1 : mainChord->graceNotes().size();
                  // find previous grace note of same type
                  for (int i = _graceIndex + incIdx; i != endIdx; i += incIdx) {
                        Chord* pgc = mainChord->graceNotes().at(i);
                        if (pgc->isGraceBefore() == before) {
                              pc = pgc;
                              break;
                              }
                        }
                  if (!pc) {
                        // no previous grace note found
                        if (!before){
                              // grace note after - we would use main note
                              // but it hasn't been laid out yet, so we can't be sure about its ledger lines
                              // err on the side of safety
                              lll = qMax(lll, ledgerLineLength + lhead + 0.2 * _spatium * mag());
                              }
                        else if (mainChord->rtick()) {
                              // grace note before - use previous normal note of measure
                              Segment* s = mainChord->segment()->prev(Segment::Type::ChordRest);
                              if (s && s->element(track()) && s->element(track())->type() == Element::Type::CHORD)
                                    pc = static_cast<Chord*>(s->element(track()));
                              }
                        }
                  }

            if (pc) {
                  // previous chord found
                  qreal llsp        = 0.0;
                  int pUpLine       = pc->upNote()->line();
                  int pDownLine     = pc->downNote()->line();
                  int upLine        = upNote()->line();
                  int downLine      = downNote()->line();
                  int ledgerBelow   = staff()->lines() * 2;
                  if (pc->_ledgerLines && ((pUpLine < 0 && upLine < 0) || (pDownLine >= ledgerBelow && downLine >= ledgerBelow))) {
                        // both chords have ledger lines above or below staff
                        llsp = ledgerLineLength;
                        // add space between ledger lines
                        llsp += 0.2 * _spatium * pc->mag();
                        // if any portion of note extended to left of origin
                        // we need to include that here so it is not subsumed by ledger line
                        llsp += lhead;
                        }
                  else if (pc->up() && upLine < 0) {
                        // even if no ledger lines in previous chord,
                        // we may need a little space to avoid crossing stem
                        llsp = ledgerLineLength * 0.5;
                        llsp += 0.2 * _spatium * pc->mag();
                        llsp += lhead;
                        }
                  if (_noteType == NoteType::NORMAL && pc->isGraceAfter()) {
                        // add appropriate space to right of previous (grace note after) chord
                        // this will be used in calculating its position relative to this chord
                        // it is too late to actually allocate space for this in segment of previous chord
                        // so we will allocate room in left space of this chord
                        qreal oldR  = pc->_spaceRw;
                        qreal stemX = pc->stemPosX();
                        qreal available = oldR - stemX;
                        qreal newR = stemX + qMax(available, llsp);
                        if (newR > oldR)
                              pc->_spaceRw = newR;
                        }
                  lll = qMax(llsp, lll);
                  }
            }

      _spaceLw = lll;
      _spaceRw = rrr;

      if (gnb){
              qreal xl = -(_spaceLw + minNoteDistance) - chordX;
              for (int i = gnb-1; i >= 0; --i) {
                    Chord* g = graceNotesBefore.value(i);
                    xl -= g->_spaceRw/* * 1.2*/;
                    g->setPos(xl, 0);
                    xl -= g->_spaceLw + minNoteDistance * graceMag;
                    }
              if (-xl > _spaceLw)
                    _spaceLw = -xl;
              }
       if (!gna.empty()) {
            qreal xr = _spaceRw;
            int n = gna.size();
            for (int i = 0; i <= n - 1; i++) {
                  Chord* g = gna.value(i);
                  xr += g->_spaceLw + g->_spaceRw + minNoteDistance * graceMag;
                  }
           if (xr > _spaceRw)
                 _spaceRw = xr;
           }

      for (Element* e : _el) {
            if (e->type() == Element::Type::SLUR)     // we cannot at this time as chordpositions are not fixed
                  continue;
            e->layout();
            if (e->type() == Element::Type::CHORDLINE) {
                  QRectF tbbox = e->bbox().translated(e->pos());
                  qreal lx = tbbox.left() + chordX;
                  qreal rx = tbbox.right() + chordX;
                  if (-lx > _spaceLw)
                        _spaceLw = -lx;
                  if (rx > _spaceRw)
                        _spaceRw = rx;
                  }
            }

      for (Note* note : _notes)
            note->layout2();

      QRectF bb;
      processSiblings([&bb] (Element* e) { bb |= e->bbox().translated(e->pos()); } );
      setbbox(bb.translated(_spatium*2, 0));
      }

//---------------------------------------------------------
//   layoutTablature
//---------------------------------------------------------

void Chord::layoutTablature()
      {
      qreal _spatium          = spatium();
      qreal dotNoteDistance   = score()->styleS(StyleIdx::dotNoteDistance).val() * _spatium;
      qreal minNoteDistance   = score()->styleS(StyleIdx::minNoteDistance).val() * _spatium;
      qreal minTieLength      = score()->styleS(StyleIdx::MinTieLength).val() * _spatium;

      for (Chord* c : _graceNotes)
            c->layoutTablature();

      while (_ledgerLines) {
            LedgerLine* l = _ledgerLines->next();
            delete _ledgerLines;
            _ledgerLines = l;
            }

      qreal lll         = 0.0;                  // space to leave at left of chord
      qreal rrr         = 0.0;                  // space to leave at right of chord
      Note* upnote      = upNote();
      qreal headWidth   = symWidth(SymId::noteheadBlack);
      StaffType* tab    = staff()->staffType();
      qreal lineDist    = tab->lineDistance().val() *_spatium;
      qreal stemX       = tab->chordStemPosX(this) *_spatium;
      int   ledgerLines = 0;
      qreal llY         = 0.0;

      int   numOfNotes  = _notes.size();
      qreal minY        = 1000.0;               // just a very large value
      for (int i = 0; i < numOfNotes; ++i) {
            Note* note = _notes.at(i);
            note->layout();
            // set headWidth to max fret text width
            qreal fretWidth = note->bbox().width();
            if (headWidth < fretWidth)
                  headWidth = fretWidth;
            // centre fret string on stem
            qreal x = stemX - fretWidth*0.5;
            qreal y = note->fixed() ? note->line() * lineDist / 2 : tab->physStringToYOffset(note->string()) * _spatium;
            note->setPos(x, y);
            if (y < minY)
                  minY  = y;
            int   currLedgerLines   = tab->numOfTabLedgerLines(note->string());
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
                  qreal overlap = 0.0;          // how much tie can overlap start and end notes
                  bool shortStart = false;      // whether tie should clear start note or not
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
                              }
                        else        // overlap start note (by ca. 1/3 of fret mark width)
                              overlap += startNoteWidth * 0.35;
                        // overlap into end chord (this)?
                        // if several notes or neither stem or tie are up
                        if (notes().size() > 1 || (stem() && !up() && !tie->up())) {
                              // for positive offset:
                              //    use available space
                              // for negative x offset:
                              //    space is allocated elsewhere, so don't re-allocate here
                              if (note->ipos().x() != 0.0)              // this probably does not work for TAB, as
                                    overlap += qAbs(note->ipos().x());  // _pos is used to centre the fret on the stem
                              else
                                    overlap -= fretWidth * 0.125;
                              }
                        else {
                              if (shortStart)
                                    overlap += fretWidth * 0.15;
                              else
                                    overlap += fretWidth * 0.35;
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
//            qreal extraLen    = score()->styleS(StyleIdx::ledgerLineLength).val() * _spatium;
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
            headWidth += extraLen;        // include ledger lines extra width in chord width
            }

      // horiz. spacing: leave half width at each side of the (potential) stem
      qreal halfHeadWidth = headWidth * 0.5;
      if (lll < stemX - halfHeadWidth)
            lll = stemX - halfHeadWidth;
      if (rrr < stemX + halfHeadWidth)
            rrr = stemX + halfHeadWidth;
      // align dots to the widest fret mark (not needed in all TAB styles, but harmless anyway)
      if (segment())
            segment()->setDotPosX(staffIdx(), headWidth);
      // if tab type is stemless or chord is stemless (possible when imported from MusicXML)
      // or measure is stemless
      // or duration longer than half (if halves have stems) or duration longer than crochet
      // remove stems
      if (tab->slashStyle() || _noStem || measure()->slashStyle(staffIdx()) || durationType().type() <
         (tab->minimStyle() != TablatureMinimStyle::NONE ? TDuration::DurationType::V_HALF : TDuration::DurationType::V_QUARTER) ) {
            if (_stem)
                  score()->undo(new RemoveElement(_stem));
            if (_hook)
                  score()->undo(new RemoveElement(_hook));
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
                  if (beam())
                        score()->undoRemoveElement(_hook);
                  else {
                        _hook->layout();
                        if (rrr < stemX + _hook->width())
                              rrr = stemX + _hook->width();
                        }
                  }
            }
      if (!tab->genDurations()            // if tab is not set for duration symbols
            || track2voice(track())       // or not in first voice
            || isGrace()) {               // or chord is a grace (no dur. symbols for graces
            //                            // wich are not used in hist. tabs anyway)
            // no tab duration symbols
            //
            delete _tabDur;   // delete an existing duration symbol
            _tabDur = 0;
            }
      else {
            //
            // tab duration symbols
            //
            // check duration of prev. CR segm
            ChordRest * prevCR = prevChordRest(this);
            // if no previous CR
            // OR symbol repeat set to ALWAYS
            // OR symbol repeat condition is triggered
            // OR duration type and/or number of dots is different from current CR
            // OR chord beam mode not AUTO
            // OR previous CR is a rest
            // AND no not-stem
            // set a duration symbol (trying to re-use existing symbols where existing to minimize
            // symbol creation and deletion)
            TablatureSymbolRepeat symRepeat = tab->symRepeat();
            if (  (prevCR == 0
                  || symRepeat == TablatureSymbolRepeat::ALWAYS
                  || (symRepeat == TablatureSymbolRepeat::MEASURE && measure() != prevCR->measure())
                  || (symRepeat == TablatureSymbolRepeat::SYSTEM && measure()->system() != prevCR->measure()->system())
                  || beamMode() != Beam::Mode::AUTO
                  || prevCR->durationType().type() != durationType().type()
                  || prevCR->dots() != dots()
                  || prevCR->type() == Element::Type::REST)
                        && !noStem() ) {
                  // symbol needed; if not exist, create; if exists, update duration
                  if (!_tabDur)
                        _tabDur = new TabDurationSymbol(score(), tab, durationType().type(), dots());
                  else
                        _tabDur->setDuration(durationType().type(), dots(), tab);
                  _tabDur->setParent(this);
//                  _tabDur->setMag(mag());           // useless to set grace mag: graces have no dur. symbol
                  _tabDur->layout();
                  if (minY < 0) {                     // if some fret extends above tab body (like bass strings)
                        _tabDur->rypos() += minY;     // raise duration symbol
                        _tabDur->bbox().translate(0, minY);
                        }
                  }
            else {                              // symbol not needed: if exists, delete
                  delete _tabDur;
                  _tabDur = 0;
                  }
            }                             // end of if(duration_symbols)

      if (_arpeggio) {
            qreal headHeight = upnote->headHeight();
            _arpeggio->layout();
            lll += _arpeggio->width() + _spatium * .5;
            qreal y = upNote()->pos().y() - headHeight * .5;
            qreal h = downNote()->pos().y() + downNote()->headHeight() - y;
            _arpeggio->setHeight(h);
            _arpeggio->setPos(-lll, y);
            _arpeggio->adjustReadPos();

            // handle the special case of _arpeggio->span() > 1
            // in layoutArpeggio2() after page layout has done so we
            // know the y position of the next staves
            }

      // allocate enough room for glissandi
      if (_endsGlissando) {
            if (rtick())                        // if not at beginning of measure
                  lll += (0.5 + score()->styleS(StyleIdx::MinTieLength).val()) * _spatium;
            // special case of system-initial glissando final note is handled in Glissando::layout() itself
            }

      if (_hook) {
            if (beam())
                  score()->undoRemoveElement(_hook);
            else if(tab == 0) {
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
                  if (_hook && !beam())
                        x = _hook->width() + dotNoteDistance;
                  // if not, dots should start at a fixed distance right after the stem
                  else
                        x = STAFFTYPE_TAB_DEFAULTDOTDIST_X * _spatium;
                  if (segment())
                        segment()->setDotPosX(staffIdx(), x);
                  }
            // if stems are through staff, use dot position computed above on fret mark widths
            else
                  x = dotPosX() + dotNoteDistance
                        + (dots()-1) * score()->styleS(StyleIdx::dotDotDistance).val() * _spatium;
            x += symWidth(SymId::augmentationDot);
            rrr = qMax(rrr, x);
            }

      _spaceLw = lll;
      _spaceRw = rrr;

      qreal graceMag = score()->styleD(StyleIdx::graceNoteMag);

      QVector<Chord*> graceNotesBefore = Chord::graceNotesBefore();
      int nb = graceNotesBefore.size();
      if (nb) {
              qreal xl = -(_spaceLw + minNoteDistance);
              for (int i = nb-1; i >= 0; --i) {
                    Chord* c = graceNotesBefore.value(i);
                    xl -= c->_spaceRw/* * 1.2*/;
                    c->setPos(xl, 0);
                    xl -= c->_spaceLw + minNoteDistance * graceMag;
                    }
              if (-xl > _spaceLw)
                    _spaceLw = -xl;
              }
       QVector<Chord*> gna = graceNotesAfter();
       int na = gna.size();
       if (na) {
           // get factor for start distance after main note. Values found by testing.
           qreal fc;
           switch (durationType().type()) {
                 case TDuration::DurationType::V_LONG:    fc = 3.8; break;
                 case TDuration::DurationType::V_BREVE:   fc = 3.8; break;
                 case TDuration::DurationType::V_WHOLE:   fc = 3.8; break;
                 case TDuration::DurationType::V_HALF:    fc = 3.6; break;
                 case TDuration::DurationType::V_QUARTER: fc = 2.1; break;
                 case TDuration::DurationType::V_EIGHTH:  fc = 1.4; break;
                 case TDuration::DurationType::V_16TH:    fc = 1.2; break;
                 default: fc = 1;
                 }
           qreal xr = fc * (_spaceRw + minNoteDistance);
           for (int i = 0; i <= na - 1; i++) {
                 Chord* c = gna.value(i);
                 xr += c->_spaceLw * (i == 0 ? 1.3 : 1);
                 c->setPos(xr, 0);
                 xr += c->_spaceRw + minNoteDistance * graceMag;
                 }
           if (xr > _spaceRw)
                 _spaceRw = xr;
           }
      for (Element* e : _el) {
            e->layout();
            if (e->type() == Element::Type::CHORDLINE) {
                  QRectF tbbox = e->bbox().translated(e->pos());
                  qreal lx = tbbox.left();
                  qreal rx = tbbox.right();
                  if (-lx > _spaceLw)
                        _spaceLw = -lx;
                  if (rx > _spaceRw)
                        _spaceRw = rx;
                  }
            }

      for (int i = 0; i < numOfNotes; ++i)
            _notes.at(i)->layout2();
      QRectF bb;
      processSiblings([&bb] (Element* e) { bb |= e->bbox().translated(e->pos()); } );
      if (_tabDur)
            bb |= _tabDur->bbox().translated(_tabDur->pos());
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
            CrossMeasure tempCross = CrossMeasure::NONE;  // assume no cross-measure modification
            // if chord has only one note and note is tied forward
            if (notes().size() == 1 && _notes[0]->tieFor()) {
                  Chord* tiedChord = _notes[0]->tieFor()->endNote()->chord();
                  // if tied note belongs to another measure and to a single-note chord
                  if (tiedChord->measure() != measure() && tiedChord->notes().size() == 1) {
                        // get total duration
                        std::vector<TDuration> durList = toDurationList(
                                    actualDurationType().fraction() +
                                    tiedChord->actualDurationType().fraction(), true);
                        // if duration can be expressed as a single duration
                        // apply cross-measure modification
                        if (durList.size() == 1) {
                              _crossMeasure = tempCross = CrossMeasure::FIRST;
                              _crossMeasureTDur = durList[0];
                              layoutStem1();
                              }
                        }
                  _crossMeasure = tempCross;
                  tiedChord->setCrossMeasure(tempCross == CrossMeasure::FIRST ?
                              CrossMeasure::SECOND : CrossMeasure::NONE);
                  }
            }
      }

//---------------------------------------------------------
//   layoutArpeggio2
//    called after layout of page
//---------------------------------------------------------

void Chord::layoutArpeggio2()
      {
      if (!_arpeggio)
            return;
      qreal y           = upNote()->pagePos().y() - upNote()->headHeight() * .5;
      int span          = _arpeggio->span();
      int btrack        = track() + (span - 1) * VOICES;
      ChordRest* bchord = static_cast<ChordRest*>(segment()->element(btrack));
      Note* dnote       = (bchord && bchord->type() == Element::Type::CHORD) ? static_cast<Chord*>(bchord)->downNote() : downNote();

      qreal h = dnote->pagePos().y() + dnote->headHeight() * .5 - y;
      _arpeggio->setHeight(h);
      _arpeggio->layout();

#if 0 // collect notes for arpeggio
      QList<Note*> notes;
      int n = _notes.size();
      for (int j = n - 1; j >= 0; --j) {
            Note* note = _notes[j];
            if (note->tieBack())
                  continue;
            notes.prepend(note);
            }

      for (int i = 1; i < span; ++i) {
            ChordRest* c = static_cast<ChordRest*>(segment()->element(track() + i * VOICES));
            if (c && c->type() == CHORD) {
                  QList<Note*> nl = static_cast<Chord*>(c)->notes();
                  int n = nl.size();
                  for (int j = n - 1; j >= 0; --j) {
                        Note* note = nl[j];
                        if (note->tieBack())
                              continue;
                        notes.prepend(note);
                        }
                  }
            }
#endif
      }

//---------------------------------------------------------
//   findNote
//---------------------------------------------------------

Note* Chord::findNote(int pitch) const
      {
      int n = _notes.size();
      for (int i = 0; i < n; ++i) {
            Note* n = _notes.at(i);
            if (n->pitch() == pitch)
                  return n;
            }
      return 0;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Chord::drop(const DropData& data)
      {
      Element* e = data.element;
      switch (e->type()) {
            case Element::Type::ARTICULATION:
                  {
                  Articulation* atr = static_cast<Articulation*>(e);
                  Articulation* oa = hasArticulation(atr);
                  if (oa) {
                        delete atr;
                        atr = 0;
                        // if attribute is already there, remove
                        // score()->cmdRemove(oa); // unexpected behaviour?
                        score()->select(oa, SelectType::SINGLE, 0);
                        }
                  else {
                        atr->setParent(this);
                        atr->setTrack(track());
                        score()->undoAddElement(atr);
                        }
                  return atr;
                  }

            case Element::Type::CHORDLINE:
                  e->setParent(this);
                  e->setTrack(track());
                  score()->undoAddElement(e);
                  break;

            case Element::Type::TREMOLO:
                  {
                  Tremolo* t = static_cast<Tremolo*>(e);
                  if (t->twoNotes()) {
                        Segment* s = segment()->next();
                        while (s) {
                              if (s->element(track()) && s->element(track())->type() == Element::Type::CHORD)
                                    break;
                              s = s->next();
                              }
                        if (s == 0) {
                              qDebug("no segment for second note of tremolo found");
                              delete e;
                              return 0;
                              }
                        Chord* ch2 = static_cast<Chord*>(s->element(track()));
                        if (ch2->duration() != duration()) {
                              qDebug("no matching chord for second note of tremolo found");
                              delete e;
                              return 0;
                             }
                        t->setChords(this, ch2);
                        }
                  }
                  if (tremolo())
                        score()->undoRemoveElement(tremolo());
                  e->setParent(this);
                  e->setTrack(track());
                  score()->undoAddElement(e);
                  break;

            case Element::Type::ARPEGGIO:
                  {
                  Arpeggio* a = static_cast<Arpeggio*>(e);
                  if (arpeggio())
                        score()->undoRemoveElement(arpeggio());
                  a->setTrack(track());
                  a->setParent(this);
                  a->setHeight(spatium() * 5);   //DEBUG
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

qreal Chord::dotPosX() const
      {
      if (parent())
            return segment()->dotPosX(staffIdx());
      return -1000.0;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Chord::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::NO_STEM:        return noStem();
            case P_ID::SMALL:          return small();
            case P_ID::STEM_DIRECTION: return stemDirection();
            default:
                  return ChordRest::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Chord::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::NO_STEM:        return false;
            case P_ID::SMALL:          return false;
            case P_ID::STEM_DIRECTION: return Direction_AUTO;
            default:
                  return ChordRest::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Chord::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::NO_STEM:
                  setNoStem(v.toBool());
                  break;
            case P_ID::SMALL:
                  setSmall(v.toBool());
                  break;
            case P_ID::STEM_DIRECTION:
                  setStemDirection(v.value<Direction>());
                  break;
            default:
                  return ChordRest::setProperty(propertyId, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   layoutArticulation
//    called from ChordRest()->layoutArticulations()
//    assumes there is only one articulation
//---------------------------------------------------------

QPointF Chord::layoutArticulation(Articulation* a)
      {
      qreal _spatium = spatium();
      qreal pld      = staff()->lineDistance();
      qreal lld      = staff()->logicalLineDistance();
      bool scale     = staff()->scaleNotesToLines();
      qreal _spStaff = _spatium * pld;    // scaled to physical staff line distance

      ArticulationAnchor aa = a->anchor();

      qreal chordTopY = upPos();    // note position of highest note
      qreal chordBotY = downPos();  // note position of lowest note
      qreal x         = centerX();
      qreal y         = 0.0;

      // TENUTO and STACCATO: always near the notehead (or stem end if beyond a stem)
      if ((a->isTenuto() || a->isStaccato() || a->isAccent())
         && (aa != ArticulationAnchor::TOP_STAFF
         && aa != ArticulationAnchor::BOTTOM_STAFF)) {
            bool bottom;                        // true: artic. is below chord | false: artic. is above chord
            bool alignToStem = false;
            // if there area voices, articulation is on stem side
            if ((aa == ArticulationAnchor::CHORD) && measure()->hasVoices(a->staffIdx()))
                  bottom = !up();
            // otherwise, look at specific anchor type (and at chord up/down if necessary)
            else
                  bottom = (aa == ArticulationAnchor::BOTTOM_CHORD) || (aa == ArticulationAnchor::CHORD && up());
            bool stemSide = (bottom != up()) && stem();     // true if there a stem between the nearest note and the articulation
            a->setUp(!bottom);

            QPointF pos;                        // computed articulation position
            if (stem())                         // if there is a stem, assume artic. will be beyond the stem
                  pos = stem()->hookPos();
            else {                              // otherwise, compute horizontal position as if there were a stem
                  pos.rx() = stemPosX();
                  if (!_up)
                        pos.rx() += score()->styleP(StyleIdx::stemWidth);
                  }
            a->layout();

            qreal _spatium2 = _spatium * .5;
            qreal _spStaff2 = _spStaff * .5;
            if (stemSide) {                     // if artic. is really beyond a stem,
                  qreal lineDelta = up() ? -_spStaff2 : _spStaff2;    // move it 1/2sp away from stem
                  int line = lrint((pos.y() + lineDelta) / _spStaff); // round to nearest staff line
                  if (line >= 0 && line < staff()->lines())          // if within staff, align between staff lines
                        pos.ry() = (line * _spStaff) + (bottom ? _spStaff2 : -_spStaff2);
                  else {
                        qreal dy = (score()->styleS(StyleIdx::beamWidth).val() + 1) * _spatium2;
                        pos.ry() += bottom ? dy : - dy;
                        }
                  alignToStem = a->isStaccato();
                  }
            else {                              // if articulation is not beyond a stem
                  int lline;                    // logical line of note
                  int line;                     // physical line of note
                  int staffOff;                 // offset that should account for line spacing
                  int extraOff = 0;             // offset that should not acocunt for line spacing
                  int lines = (staff()->lines() - 1) * 2;               // num. of staff positions within staff
                  int add   = a->isAccent() ? 1 : 0; // sforzato accent needs more offset
                  if (bottom) {                 // if below chord
                        lline = downLine();                             // logical line of chord lowest note
                        line = scale ? lline : lline * (lld / pld);     // corresponding physical line
                        if (line < lines)                               // if note above staff bottom line
                              staffOff = 3 - ((line - add) & 1) + add;        // round to next space below
                        else                                            // if note on or below staff bottom line,
                              staffOff = 2 + add;                             // move 1 whole space below
                        if (pld != 1.0) {
                              // on staves with non-standard line spacing
                              // we need to consider line spacing for the portion of the offset that is within staff or ledger lines
                              // but not for any offset beyond that
                              int clearLine = qMax(line, lines);              // line we need to clear
                              int headRoom = qMax(clearLine - line, 0);       // amount of room between note and line we need to clear
                              extraOff = staffOff - qMin(staffOff, headRoom); // amount by which we do not need to consider line distance
                              staffOff -= extraOff;                           // amount by which we need to consider line distance
                              if (!scale && lline > clearLine * pld)
                                    extraOff += lline - floor(line * pld);    // adjust for rounding of physical line
                              }
                        pos.ry() = -a->height() / 2;                    // symbol is below baseline, shift if a bit up
                        }
                  else {                        // if above chord
                        lline = upLine();                               // logical line of chord highest note
                        line = scale ? lline : lline * (lld / pld);     // corresponding physical line
                        if (line > 0)                                   // if note below staff top line
                              staffOff = -3 + ((line + add) & 1) - add;       // round to next space above
                        else                                            // if note or or above staff top line
                              staffOff = -2 - add;                            // move 1 whole space above
                        if (pld != 1.0) {
                              // see corresponding code above
                              int clearLine = qMin(line, 0);
                              int headRoom = qMax(line - clearLine, 0);
                              extraOff = staffOff + qMin(-staffOff, headRoom);
                              staffOff -= extraOff;
                              if (!scale && lline < clearLine * pld)
                                    extraOff += lline - ceil(line * pld);
                              }
                        pos.ry() = a->height() / 2;                     // symbol is on baseline, shift it a bit down
                        }
                  pos.ry() += (line + staffOff) * _spStaff2;            // offset that needs to account for line distance
                  pos.ry() += extraOff * _spatium2;                     // additional offset that need not account for line distance
                  }
            if (!staff()->isTabStaff() && !alignToStem) {
                  if (up())
                        pos.rx() -= upNote()->headWidth() * .5;   // move half-a-note-head to left
                  else
                        pos.rx() += upNote()->headWidth() * .5;   // move half-a-note-head to right
                  }
            a->setPos(pos);
            a->adjustReadPos();
            return QPointF(pos);
            }

      // Lute fingering are always in the middle of the space right below the fret mark,
      else if (staff() && staff()->staffGroup() == StaffGroup::TAB && a->isLuteFingering()) {
            // lute fing. glyphs are vertically registered in the middle of bottom space;
            // move down of half a space to have the glyph on the line
            y = chordBotY + _spatium * 0.5;
            if (staff()->staffType()->onLines()) {          // if fret marks are on lines
                  // move down by half the height of fret marks (extending below the line)
                  // and half of the remaing space below,
                  // to centre the symbol between the fret mark and the line below
                  // fretBoxH/2 + (spStaff - fretBoxH/2) / 2 becomes:
                  y += (staff()->staffType()->fretBoxH()*0.5 + _spStaff) * 0.5;
                  }
            else {                                          // if marks are between lines
                  // move down by half a sp to pace the glyph right below the mark,
                  // and not too far away (as it would have been, if centred in the line space below)
                  y += _spatium * 0.5;
                  }
            a->layout();
            a->setPos(x, y);
            a->adjustReadPos();
            return QPointF(x, y);
            }

      // other articulations are outside of area occupied by the staff or the chord
      // reserve space for slur
      bool botGap = false;
      bool topGap = false;

      auto si = score()->spannerMap().findOverlapping(tick(), tick());
      for (auto is : si) {
            Spanner* sp = is.value;
            if ((sp->type() != Element::Type::SLUR) || (sp->tick() != tick() && sp->tick2() != tick()))
                 continue;
            if ( sp->tick() == tick() && sp->track() != track())
                  continue;
            if ( sp->tick2() == tick() && sp->track2() != track())
                  continue;
            Slur* s = static_cast<Slur*>(sp);
            if (s->up())
                  topGap = true;
            else
                  botGap = true;
            }

      if (botGap)
            chordBotY += _spatium;
      else
            chordBotY += _spatium * .5;
      if (topGap)
            chordTopY -= _spatium;
      else
            chordTopY -= _spatium * .5;

      // avoid collisions of staff articulations with chord notes:
      // gap between note and staff articulation is distance0 + 0.5 spatium

      qreal staffTopY = 0;                      // top of occupied area
      qreal staffBotY = staff()->height();      // bottom of occupied area

      if (stem()) {                             // if there is a stem, occupied area may be larger
#if 0
            y = stem()->pos().y() + pos().y();
            if (up() && stem()->stemLen() < 0.0)
                  y += stem()->stemLen();
            else if (!up() && stem()->stemLen() > 0.0)
                  y -= stem()->stemLen();
#endif
            y = stem()->hookPos().y() + pos().y();    // vert. pos. of end of stem

            if (beam()) {                       // if there is a beam, stem end is further away
                  qreal bw = score()->styleP(StyleIdx::beamWidth);
                  y += up() ? -bw : bw;
                  }
            if (up())                           // if up chord, top is topmost between staff top and stem end
                  staffTopY = qMin(staffTopY, y);
            else                                // if chord is down, bottom is bottommost btw staff bottom and stem end
                  staffBotY = qMax(staffBotY, y);
            }

      staffTopY = qMin(staffTopY, qreal(chordTopY));  // top is topmost between staff top and chord stop
      staffBotY = qMax(staffBotY, qreal(chordBotY));  // bottom is bottom between staff bottom and chord bottom

      //
      // determine Direction
      //
      if (a->direction() != Direction_AUTO) {
            a->setUp(a->direction() == Direction_UP);
            }
      else {
            if (measure()->hasVoices(a->staffIdx())) {
                  a->setUp(up());
                  aa = up() ? ArticulationAnchor::TOP_STAFF : ArticulationAnchor::BOTTOM_STAFF;
                  }
            else {
                  if (aa == ArticulationAnchor::CHORD)
                        a->setUp(!up());
                  else
                        a->setUp(aa == ArticulationAnchor::TOP_STAFF || aa == ArticulationAnchor::TOP_CHORD);
                  }
            }

      qreal dist;  // distance between occupied area and articulation
      if (a->symId() == SymId::articMarcatoAbove || a->symId() == SymId::articMarcatoBelow)
            dist = 1.0 * _spatium;
      else if (a->isAccent())
            dist = 1.5 * _spatium;
      else
            dist = score()->styleP(StyleIdx::propertyDistance);

      if (aa == ArticulationAnchor::CHORD || aa == ArticulationAnchor::TOP_CHORD || aa == ArticulationAnchor::BOTTOM_CHORD) {
            bool bottom;
            if ((aa == ArticulationAnchor::CHORD) && measure()->hasVoices(a->staffIdx()))
                  bottom = !up();
            else
                  bottom = (aa == ArticulationAnchor::BOTTOM_CHORD) || (aa == ArticulationAnchor::CHORD && up());
            y = bottom ? chordBotY + dist : chordTopY - dist;
            }
      else if (aa == ArticulationAnchor::TOP_STAFF || aa == ArticulationAnchor::BOTTOM_STAFF) {
            y = a->up() ? staffTopY - dist : staffBotY + dist;
            }
      a->layout();
      a->setPos(x, y);
      a->adjustReadPos();
      return QPointF(x, y);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Chord::reset()
      {
      undoChangeProperty(P_ID::STEM_DIRECTION, Direction_AUTO);
      undoChangeProperty(P_ID::BEAM_MODE, int(Beam::Mode::AUTO));
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
            undoChangeProperty(P_ID::NO_STEM, false);
            undoChangeProperty(P_ID::SMALL, false);
            undoChangeProperty(P_ID::USER_OFF, QPointF());
            for (Note* n : _notes) {
                  n->undoChangeProperty(P_ID::HEAD_GROUP, int(NoteHead::Group::HEAD_NORMAL));
                  n->undoChangeProperty(P_ID::FIXED, false);
                  n->undoChangeProperty(P_ID::FIXED_LINE, 0);
                  n->undoChangeProperty(P_ID::PLAY, true);
                  n->undoChangeProperty(P_ID::VISIBLE, true);
                  if (staff()->isDrumStaff()) {
                        const Drumset* ds = part()->instrument()->drumset();
                        int pitch = n->pitch();
                        if (ds && ds->isValid(pitch)) {
                              undoChangeProperty(P_ID::STEM_DIRECTION, ds->stemDirection(pitch));
                              n->undoChangeProperty(P_ID::HEAD_GROUP, int(ds->noteHead(pitch)));
                              }
                        }
                  }
            return;
            }

      // set stem to auto (mostly important for rhythmic notation on drum staves)
      undoChangeProperty(P_ID::STEM_DIRECTION, Direction_AUTO);

      // make stemless if asked
      if (stemless)
            undoChangeProperty(P_ID::NO_STEM, true);

      // voice-dependent attributes - line, size, offset, head
      if (track() % VOICES < 2) {
            // use middle line
            line = staff()->middleLine();
            }
      else {
            // set small
            undoChangeProperty(P_ID::SMALL, true);
            // set outside the staff
            qreal y = 0.0;
            if (track() % 2) {
                  line = staff()->bottomLine() + 1;
                  y    = 0.5 * spatium();
                  }
            else {
                  line = -1;
                  if (!staff()->isDrumStaff())
                        y = -0.5 * spatium();
                  }
            // for non-drum staves, add an additional offset
            // for drum staves, no offset, but use normal head
            if (!staff()->isDrumStaff())
                  undoChangeProperty(P_ID::USER_OFF, QPointF(0.0, y));
            else
                  head = NoteHead::Group::HEAD_NORMAL;
            }

      int ns = _notes.size();
      for (int i = 0; i < ns; ++i) {
            Note* n = _notes[i];
            n->undoChangeProperty(P_ID::HEAD_GROUP, static_cast<int>(head));
            n->undoChangeProperty(P_ID::FIXED, true);
            n->undoChangeProperty(P_ID::FIXED_LINE, line);
            n->undoChangeProperty(P_ID::PLAY, false);
            // hide all but first notehead
            if (i)
                  n->undoChangeProperty(P_ID::VISIBLE, false);
            }
      }

//---------------------------------------------------------
//  updateEndsGlissando
//    sets/resets the chord _endsGlissando according any glissando (or more)
//    end into this chord or no.
//---------------------------------------------------------

void Chord::updateEndsGlissando()
      {
      _endsGlissando = false;       // assume no glissando ends here
      // scan all chord notes for glissandi ending on this chord
      for (Note* note : notes()) {
            for (Spanner* sp : note->spannerBack())
                  if (sp->type() == Element::Type::GLISSANDO) {
                        _endsGlissando = true;
                        return;
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
      if (tremolo() && !keepTremolo)
            remove(tremolo());
      if (arpeggio())
            remove(arpeggio());
      for (Element* e : el())
            remove(e);
      for (Element* e : articulations())
            remove(e);
      for (Element* e : lyrics())
            remove(e);
      for (Element* e : graceNotes())
            remove(e);
      for (Note* n : notes()) {
            for (Element* e : n->el())
                  n->remove(e);
            }
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Chord::mag() const
      {
      qreal m = staff() ? staff()->mag() : 1.0;
      if (small())
            m *= score()->styleD(StyleIdx::smallNoteMag);
      if (_noteType != NoteType::NORMAL)
            m *= score()->styleD(StyleIdx::graceNoteMag);
      return m;
      }

//---------------------------------------------------------
//   segment
//---------------------------------------------------------

Segment* Chord::segment() const
      {
      Element* e = parent();
      for (; e && e->type() != Element::Type::SEGMENT; e = e->parent())
            ;
      return static_cast<Segment*>(e);
      }

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure* Chord::measure() const
      {
      Element* e = parent();
      for (; e && e->type() != Element::Type::MEASURE; e = e->parent())
            ;
      return static_cast<Measure*>(e);
      }

//---------------------------------------------------------
//   graceNotesBefore
//---------------------------------------------------------

QVector<Chord*> Chord::graceNotesBefore() const
      {
      QVector<Chord*> cl;
      for (Chord* c : _graceNotes) {
               if (c->noteType() & (NoteType::ACCIACCATURA
                  | NoteType::APPOGGIATURA
                  | NoteType::GRACE4
                  | NoteType::GRACE16
                  | NoteType::GRACE32))
                  cl.push_back(c);
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
            if (c->noteType() & (NoteType::GRACE8_AFTER | NoteType::GRACE16_AFTER | NoteType::GRACE32_AFTER))
                  cl.push_back(c);
            }
      return cl;
      }

//---------------------------------------------------------
//   sortNotes
//---------------------------------------------------------

void Chord::sortNotes()
      {
      std::sort(notes().begin(), notes().end(),
         [](const Note* a,const Note* b)->bool { return b->line() < a->line(); }
         );
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
      Segment* nextSeg = backwards ? segment()->prev1(Segment::Type::ChordRest) : segment()->next1(Segment::Type::ChordRest);
      if (!nextSeg)
            return 0;
      ChordRest* nextCR = nextSeg->cr(track());
      if (!nextCR || !nextCR->isChord())
            return 0;
      Chord* next = toChord(nextCR);
      if (sameSize && notes().size() != next->notes().size())
            return 0; // sizes don't match so some notes can't be tied
      for (Note* n : _notes) {
            Tie* tie = backwards ? n->tieBack() : n->tieFor();
            if (!tie)
                  return 0; // not tied
            Note* nn = backwards ? tie->startNote() : tie->endNote();
            if (!nn || nn->chord() != next)
                  return 0; // tied to note in wrong voice, or tied over rest
            }
      return next; // all notes in this chord are tied to notes in next chord
      }

//---------------------------------------------------------
//   toGraceAfter
//---------------------------------------------------------

void Chord::toGraceAfter()
      {
      switch (noteType()) {
            case NoteType::APPOGGIATURA:  setNoteType(NoteType::GRACE8_AFTER);  break;
            case NoteType::GRACE16:       setNoteType(NoteType::GRACE16_AFTER); break;
            case NoteType::GRACE32:       setNoteType(NoteType::GRACE32_AFTER); break;
            default: break;
            }
      }

//---------------------------------------------------------
//   tremoloChordType
//---------------------------------------------------------

TremoloChordType Chord::tremoloChordType() const
      {
      if (_tremolo) {
            if (_tremolo->twoNotes()) {
                  if (_tremolo->chord1() == this)
                        return TremoloChordType::TremoloFirstNote;
                  else if (_tremolo->chord2() == this)
                        return TremoloChordType::TremoloSecondNote;
                  else
                        qDebug("Chord::tremoloChordType(): inconsistency %p - %p", _tremolo->chord1(), _tremolo->chord2());
                  }
            }
      return TremoloChordType::TremoloSingle;
      }


//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

Element* Chord::nextElement()
      {
      for (int v = track() + 1; staffIdx() == v/VOICES; ++v) {
            Element* e = segment()->element(v);
            if (e) {
                  if (e->type() == Element::Type::CHORD)
                        return static_cast<Chord*>(e)->notes().back();

                  return e;
                  }
            }

      return ChordRest::nextElement();
      }

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

Element* Chord::prevElement()
      {
      for (int v = track() - 1; staffIdx() == v/VOICES; --v) {
            Element* e = segment()->element(v);
            if (e) {
                  if (e->type() == Element::Type::CHORD)
                        return static_cast<Chord*>(e)->notes().front();

                  return e;
                  }
            }
      return ChordRest::prevElement();
      }

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

QString Chord::accessibleExtraInfo() const
      {
      QString rez = "";

      if (!isGrace()) {
            foreach (Chord* c, graceNotes()) {
                  if (!score()->selectionFilter().canSelect(c)) continue;
                  foreach (Note* n, c->notes()) {
                        rez = QString("%1 %2").arg(rez).arg(n->screenReaderInfo());
                        }
                  }
            }

      if (arpeggio() && score()->selectionFilter().canSelect(arpeggio()))
            rez = QString("%1 %2").arg(rez).arg(arpeggio()->screenReaderInfo());

      if (tremolo() && score()->selectionFilter().canSelect(tremolo()))
            rez = QString("%1 %2").arg(rez).arg(tremolo()->screenReaderInfo());

      foreach (Element* e, el()) {
            if (!score()->selectionFilter().canSelect(e))
                  continue;
            rez = QString("%1 %2").arg(rez).arg(e->screenReaderInfo());
            }

      return QString("%1 %2").arg(rez).arg(ChordRest::accessibleExtraInfo());
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape Chord::shape() const
      {
      Shape shape;
      processSiblings([&shape, this] (Element* e) {
            if (!e->isLedgerLine())             // dont add ledger lines to shape
                  shape.add(e->shape());
            });
      shape.add(ChordRest::shape());      // add articulation + lyrics
      return shape.translated(pos());
      }
}

