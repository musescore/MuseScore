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

namespace Ms {

//---------------------------------------------------------
//   upNote / downNote
//---------------------------------------------------------

Note* Chord::upNote() const
      {
      Note* result = _notes.back();
      if (!staff())
            return result;
      if (staff()->isDrumStaff()) {
            for (Note*n : _notes) {
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
            for (Note*n : _notes) {
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
      Note* result = _notes.front();
      if (!staff())
            return result;
      if (staff()->isDrumStaff()) {
            foreach(Note*n, _notes) {
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
            for (Note*n : _notes) {
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
      _stemDirection    = MScore::Direction::AUTO;
      _arpeggio         = 0;
      _tremolo          = 0;
      _glissando        = 0;
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
            linkTo((Chord*)&c);      // HACK!
      _ledgerLines = 0;
      int n = c._notes.size();
      for (int i = 0; i < n; ++i) {
            Note* onote = c._notes[i];
            Note* nnote = new Note(*onote, link);
            add(nnote);
            }

      for (Chord* gn : c.graceNotes()) {
            Chord* nc = new Chord(*gn, link);
            add(nc);
            }

      _stem          = 0;
      _hook          = 0;
      _glissando     = 0;
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

      if (c._glissando) {
            Glissando* g = new Glissando(*(c._glissando));
            add(g);
            if (link)
                  g->linkTo(c._glissando);
            }
      if (c._arpeggio) {
            Arpeggio* a = new Arpeggio(*(c._arpeggio));
            add(a);
            if (link)
                  a->linkTo(c._arpeggio);
            }
      if (c._tremolo && !c._tremolo->twoNotes()) {
            Tremolo* t = new Tremolo(*(c._tremolo));
            add(t);
            if (link)
                  t->linkTo(c._tremolo);
            }

      for (Element* e : c.el()) {
            if (e->type() == Element::Type::CHORDLINE) {
                  ChordLine* cl = static_cast<ChordLine*>(e);
                  ChordLine* ncl = new ChordLine(*cl);
                  add(ncl);
                  if (link)
                        cl->linkTo(ncl);
                  }
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
      delete _glissando;
      delete _stemSlash;
      delete _stem;
      delete _hook;
      for (LedgerLine* ll = _ledgerLines; ll;) {
            LedgerLine* llNext = ll->next();
            delete ll;
            ll = llNext;
            }
      qDeleteAll(_notes);
      }

//---------------------------------------------------------
//   setStem
//---------------------------------------------------------

void Chord::setStem(Stem* s)
      {
      delete _stem;
      _stem = s;
      if (_stem) {
            _stem->setParent(this);
            _stem->setTrack(track());
            }
      }

//---------------------------------------------------------
//   stemPosX
//    return Chord coordinates
//---------------------------------------------------------

qreal Chord::stemPosX() const
      {
      if (staff() && staff()->isTabStaff())
            return staff()->staffType()->chordStemPosX(this) * spatium();

      if (_up) {
            qreal nhw = score()->noteHeadWidth();
            if (_noteType != NoteType::NORMAL)
                 nhw *= score()->styleD(StyleIdx::graceNoteMag);
            nhw *= mag();
            return nhw;
            }
      return 0.0;
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
            qreal nhw = score()->noteHeadWidth();
            if (_noteType != NoteType::NORMAL)
                  nhw *= score()->styleD(StyleIdx::graceNoteMag);
            nhw    *= mag();
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
//    return canvas coordinates
//---------------------------------------------------------

QPointF Chord::stemPosBeam() const
      {
      qreal _spatium = spatium();
      if (staff() && staff()->isTabStaff())
            return staff()->staffType()->chordStemPosBeam(this) * _spatium + pagePos();
      QPointF p(pagePos());
      if (_up) {
            qreal nhw = score()->noteHeadWidth();
            if (_noteType != NoteType::NORMAL)
                 nhw *= score()->styleD(StyleIdx::graceNoteMag);
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

void Chord::setSelected(bool f)
      {
      Element::setSelected(f);
      int n = _notes.size();
      for (int i = 0; i < n; ++i)
            _notes.at(i)->setSelected(f);
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
                  Note* note = static_cast<Note*>(e);
                  bool found = false;
                  // _notes should be sorted by line position,
                  // but it's often not yet possible since line is unknown
                  // use pitch instead, and line as a second sort critera.
                  for (int idx = 0; idx < _notes.size(); ++idx) {
                        if (note->pitch() <= _notes[idx]->pitch()) {
                              if (note->pitch() == _notes[idx]->pitch() && note->line() > _notes[idx]->line())
                                    _notes.insert(idx+1, note);
                              else
                                    _notes.insert(idx, note);
                              found = true;
                              break;
                              }
                        }
                  if (!found)
                        _notes.append(note);
                  if (note->tieFor()) {
                        if (note->tieFor()->endNote())
                              note->tieFor()->endNote()->setTieBack(note->tieFor());
                        }
                  }
                  break;
            case Element::Type::ARPEGGIO:
                  _arpeggio = static_cast<Arpeggio*>(e);
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
                  _glissando = static_cast<Glissando*>(e);
                  break;
            case Element::Type::STEM:
                  _stem = static_cast<Stem*>(e);
                  break;
            case Element::Type::HOOK:
                  _hook = static_cast<Hook*>(e);
                  break;
            case Element::Type::CHORDLINE:
                  _el.push_back(e);
                  break;
            case Element::Type::STEM_SLASH:
                  _stemSlash = static_cast<StemSlash*>(e);
                  break;
            case Element::Type::CHORD:
                  {
                  Chord* gc = static_cast<Chord*>(e);
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
      switch(e->type()) {
            case Element::Type::NOTE:
                  {
                  Note* note = static_cast<Note*>(e);
                  if (_notes.removeOne(note)) {
                        if (note->tieFor()) {
                              if (note->tieFor()->endNote()) {
                                    note->tieFor()->endNote()->setTieBack(0);
                                    // update accidentals for endNote
                                    Chord* chord = note->tieFor()->endNote()->chord();
                                    Measure* m = chord->segment()->measure();
                                    m->cmdUpdateNotes(chord->staffIdx());
                                    }
                              }
                        }
                  else
                        qDebug("Chord::remove() note %p not found!", e);
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
                  _glissando = 0;
                  break;
            case Element::Type::STEM:
                  _stem = 0;
                  break;
            case Element::Type::HOOK:
                  _hook = 0;
                  break;
            case Element::Type::STEM_SLASH:
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
      for (int i = 0; i < _notes.size(); i++) {
            qreal t = _notes.at(i)->headWidth();
            if (t > hw)
                  hw = t;
            }
      return hw;
      }

//---------------------------------------------------------
//   addLedgerLine
///   Add a ledger line to a chord.
///   \arg track      track the ledger line belongs to
///   \arg line       vertical position of line
///   \arg visible    whether the line is visible or not
///   \arg x          start x-position
///   \arg len        line length
//---------------------------------------------------------

#if 0
void Chord::addLedgerLine(int track, int line, bool visible, qreal x, Spatium len)
      {
      qreal _spatium = spatium();
      LedgerLine* h = new LedgerLine(score());
      h->setParent(this);
      h->setTrack(track);
      h->setVisible(visible);

      //
      // Experimental:
      //  shorten ledger line to avoid collisions with accidentals
      //

      int n = _notes.size();
      for (int i = 0; i < n; ++i) {
            const Note* n = _notes.at(i);
            if (n->line() >= (line-1) && n->line() <= (line+1) && n->accidental()) {
                  x   += _spatium * .25;
                  len -= Spatium(.25);
                  break;
                  }
            }

      h->setLen(len);
      h->setPos(x, line * _spatium * .5);
      h->setNext(_ledgerLines);
      _ledgerLines = h;
      }
#endif

//---------------------------------------------------------
//   createLedgerLines
///   Creates the ledger lines fro a chord
///   \arg track      track the ledger line belongs to
///   \arg lines      a vector of LedgerLineData describing thelines to add
///   \arg visible    whether the line is visible or not
//---------------------------------------------------------

void Chord::createLedgerLines(int track, vector<LedgerLineData>& vecLines, bool visible)
      {
      qreal _spatium = spatium();
      for (auto lld : vecLines) {
            LedgerLine* h = new LedgerLine(score());
            h->setParent(this);
            h->setTrack(track);
            h->setVisible(lld.visible && visible);
            h->setLen(Spatium( (lld.maxX - lld.minX) / _spatium) );
            h->setPos(lld.minX, lld.line * _spatium * .5);
            h->setNext(_ledgerLines);
            _ledgerLines = h;
            }
      }

//---------------------------------------------------------
//   addLedgerLines
//---------------------------------------------------------

void Chord::addLedgerLines(int move)
      {
      LedgerLineData    lld;
      qreal _spatium = spatium();
      int   idx   = staffIdx() + move;
      int   track = staff2track(idx);           // the track lines belong to
      qreal hw    = _notes[0]->headWidth();
      // the line pos corresponding to the bottom line of the staff
      int   lineBelow = (score()->staff(idx)->lines()-1) * 2;
      qreal minX, maxX;                         // note extrema in raster units
//      qreal minXr, maxXr;                       // ledger line extrema in raster units
      int   minLine, maxLine;
      bool  visible = false;
      qreal x;
      // the extra length of a ledger line with respect to note head (half of it on each side)
      qreal extraLen = score()->styleS(StyleIdx::ledgerLineLength).val() * _spatium * 0.5;

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
                  const Note* note = _notes.at(i);

                  int l = note->line();
                  if ( (!j && l < lineBelow) || // if 1st pass and note not below staff
                       (j && l >= 0) )          // or 2nd pass and note not above staff
                        break;                  // stop this pass
                  // round line number to even number toward 0
                  if (l < 0)        l = (l+1) & ~ 1;
                  else              l = l & ~ 1;

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
                  x = note->pos().x();
                  if (x-extraLen < minX) {
                        minX  = x - extraLen;
//                        minXr = minX - extraLen;
                        // increase width of all lines between this one and the staff
                        for (auto& d : vecLines)
                              if (!d.accidental && ((l < 0 && d.line >= l) || (l > 0 && d.line <= l)) )
                                    d.minX = minX ;
                        }
                  // same for left side
                  if (x+hw+extraLen > maxX) {
                        maxX = x + hw + extraLen;
//                        maxXr = maxX + extraLen;
                        for (auto& d : vecLines)
                              if ( (l < 0 && d.line >= l) || (l > 0 && d.line <= l) )
                                    d.maxX = maxX;
                        }

                  // check if note vert. pos. is outside current range
                  // and, in case, add data for new line(s)
                  if (l < minLine) {
                        for (int i = l; i < minLine; i += 2) {
                              lld.line = i;
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
                              lld.minX = minX;
                              lld.maxX = maxX;
                              lld.visible = visible;
                              lld.accidental = false;
                              vecLines.push_back(lld);
                              }
                        maxLine = l;
                        }
                  }
            if (minLine < 0 || maxLine > lineBelow)
                  createLedgerLines(track, vecLines, !staff()->invisible());
            }

            return;                       // no ledger lines for this chord

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
                        _up = !tab->stemsDown();// unconditionally set _up according to TAB stem direction
                  return;                       // (if no stems, _up does not really matter!)
                  }
            // if TAB has stems through staves, chain into standard processing
            }

      // PITCHED STAVES (or TAB with stems through staves)
      if (_stemDirection != MScore::Direction::AUTO) {
            _up = _stemDirection == MScore::Direction::UP;
            }
      else if (_noteType != NoteType::NORMAL) {
            //
            // stem direction for grace notes
            //
            if (measure()->mstaff(staffIdx())->hasVoices)
                  _up = !(track() % 2);
            else
                  _up = true;
            }
      else if (staffMove()) {
            _up = staffMove() > 0;
            }
      else if (measure()->mstaff(staffIdx())->hasVoices) {
            _up = !(track() % 2);
            }
      else {
            int   dnMaxLine   = staff()->staffType()->lines() - 1;
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

void Chord::write(Xml& xml) const
      {
      for (Chord* c : _graceNotes) {
            c->writeBeam(xml);
            c->write(xml);
            }
      xml.stag("Chord");
      ChordRest::writeProperties(xml);
      switch (_noteType) {
            case NoteType::INVALID:
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
            }

      if (_noStem)
            xml.tag("noStem", _noStem);
      else if (_stem && (!_stem->userOff().isNull() || (_stem->userLen() != 0.0) || !_stem->visible() || (_stem->color() != MScore::defaultColor)))
            _stem->write(xml);
      if (_hook && (!_hook->visible() || !_hook->userOff().isNull() || (_hook->color() != MScore::defaultColor)))
            _hook->write(xml);
      switch(_stemDirection) {
            case MScore::Direction::UP:   xml.tag("StemDirection", QVariant("up")); break;
            case MScore::Direction::DOWN: xml.tag("StemDirection", QVariant("down")); break;
            case MScore::Direction::AUTO: break;
            }
      for (Note* n : _notes)
            n->write(xml);
      if (_arpeggio)
            _arpeggio->write(xml);
      if (_glissando)
            _glissando->write(xml);
      if (_tremolo && tremoloChordType() != TremoloChordType::TremoloSecondNote)
            _tremolo->write(xml);
      for (Element* e : _el)
            e->write(xml);
      for (auto i : score()->spanner()) {     // TODO: dont search whole list
            Spanner* s = i.second;
            if (s->generated() || s->type() != Element::Type::SLUR || !xml.canWrite(s))
                  continue;

            if (s->startElement() == this) {
                  int id = xml.spannerId(s);
                  xml.tagE(QString("Slur type=\"start\" id=\"%1\"").arg(id));
                  }
            else if (s->endElement() == this) {
                  int id = xml.spannerId(s);
                  xml.tagE(QString("Slur type=\"stop\" id=\"%1\"").arg(id));
                  }
            }
      xml.etag();
      }

//---------------------------------------------------------
//   Chord::read
//---------------------------------------------------------

void Chord::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
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
                  _stem = new Stem(score());
                  _stem->read(e);
                  add(_stem);
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
            else if (tag == "StemDirection") {
                  QString val(e.readElementText());
                  if (val == "up")
                        _stemDirection = MScore::Direction::UP;
                  else if (val == "down")
                        _stemDirection = MScore::Direction::DOWN;
                  else
                        _stemDirection = MScore::Direction(val.toInt());
                  }
            else if (tag == "noStem")
                  _noStem = e.readInt();
            else if (tag == "Arpeggio") {
                  _arpeggio = new Arpeggio(score());
                  _arpeggio->setTrack(track());
                  _arpeggio->read(e);
                  _arpeggio->setParent(this);
                  }
            else if (tag == "Glissando") {
                  _glissando = new Glissando(score());
                  _glissando->setTrack(track());
                  _glissando->read(e);
                  _glissando->setParent(this);
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
                  e.unknown();
            }
      if (score()->mscVersion() <= 114) { // #19988
            Note * n = upNote();
            if (n) {
                  if (notes().size() == 1) {
                        setUserOff(n->userOff() + userOff());
                        n->setUserOff(QPoint());
                        n->setReadPos(QPoint());
                        }
                  else if(!n->userOff().isNull()) {
                        if(!_stem) {
                              _stem = new Stem(score());
                              add(_stem);
                              }
                         _stem->setUserOff(n->userOff() + _stem->userOff());
                        }
                  }
            }
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
      bool slash = _noStem || (measure() && measure()->slashStyle(staffIdx()));
      if (_hook && !slash)
            func(data, _hook );
      if (_stem && !slash)
            func(data, _stem);
      if (_stemSlash)
            func(data, _stemSlash);
      if (_arpeggio)
            func(data, _arpeggio);
      if (_tremolo && (tremoloChordType() != TremoloChordType::TremoloSecondNote))
            func(data, _tremolo);
      if (_glissando)
            func(data, _glissando);
      if (staff() && staff()->showLedgerLines())
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

void Chord::processSiblings(std::function<void(Element*)> func)
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
      if (_glissando)
            func(_glissando);
      for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next())
            func(ll);
      for (int i = 0; i < _notes.size(); ++i)
            func(_notes.at(i));
      for (Chord* chord : _graceNotes)
            func(chord);
      for (Element* e : _el)
            func(e);
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

//---------------------------------------------------------
//   layoutStem1
///   Layout _stem and _stemSlash
//
//    Called before layout spacing of notes.
//    Create stem if necessary.
//---------------------------------------------------------

void Chord::layoutStem1()
      {
      bool hasStem = durationType().hasStem() && !(_noStem || measure()->slashStyle(staffIdx()));

      if (hasStem) {
            if (!_stem) {
                  Stem* stem = new Stem(score());
                  stem->setParent(this);
                  stem->setGenerated(true);
                  score()->undoAddElement(stem);
                  }
            }
      else if (_stem)
            score()->undoRemoveElement(_stem);

      if (hasStem && (_noteType == NoteType::ACCIACCATURA)) {
            if (_stemSlash == 0) {
                  StemSlash* slash = new StemSlash(score());
                  add(slash);

                  // slash->setParent(this);
                  // slash->setGenerated(true);
                  // score()->undoAddElement(slash);
                  }
            }
      else if (_stemSlash)
            // score()->undoRemoveElement(_stemSlash);
            setStemSlash(0);
      }

//---------------------------------------------------------
//   layoutHook1
///   Layout hook
//
//    Called before layout spacing of notes.
//    Create hook if necessary to get right note width for next
//    pass.
//---------------------------------------------------------

void Chord::layoutHook1()
      {
      int hookIdx  = durationType().hooks();
      if (hookIdx && !(_noStem || measure()->slashStyle(staffIdx()))) {
            if (!_hook) {
                  Hook* hook = new Hook(score());
                  hook->setParent(this);
                  hook->setGenerated(true);
                  if (score()->mscVersion() <= 114 && _stem && !_stem->visible()) {
                        hook->setVisible(false);
                        }
                  score()->undoAddElement(hook);
                  }
            _hook->setHookType(up() ? hookIdx : -hookIdx);
            }
      else if (_hook)
            score()->undoRemoveElement(_hook);
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
      if (beam())
            return;
      //
      // TAB
      //
      qreal _spatium = spatium();
      StaffType* tab = 0;
      if (staff() && staff()->isTabStaff()) {
            tab = staff()->staffType();
            // require stems only if TAB is not stemless and this chord has a stem
            if (!tab->slashStyle() && _stem) {
                  // if stems are beside staff, apply special formatting
                  if (!tab->stemThrough()) {
                        // process stem:
                        _stem->setLen(tab->chordStemLength(this) * _spatium);
                        // process hook
                        int   hookIdx = durationType().hooks();
                        if (!up())
                              hookIdx = -hookIdx;
                        if (hookIdx) {
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
                        return;
                        }
                  // if stems are through staff, use standard formatting
                  }
            }

      //
      // NON-TAB (or TAB with stems through staff)
      //
      if (segment()) {
            System* s = segment()->measure()->system();
            if (s == 0)       //DEBUG
                  return;
            }

      if (_stem) {
            Note* downnote;
            int dl, ul;
            qreal stemLen;
            int hookIdx       = durationType().hooks();
            downnote          = downNote();
            ul = upLine();
            dl = downLine();
            if (tab && !tab->onLines()) {       // if TAB and frets above strings, move 1 position up
                  --ul;
                  --dl;
                  }
            bool shortenStem = score()->styleB(StyleIdx::shortenStem);
            if (hookIdx >= 2 || _tremolo)
                  shortenStem = false;

            Spatium progression(score()->styleS(StyleIdx::shortStemProgression));
            qreal shortest(score()->styleS(StyleIdx::shortestStem).val());

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
                  qreal staffHeight = staff() ? (staff()->lines()- 1) : 4;
                  qreal staffHlfHgt = staffHeight * 0.5;
                  if (up()) {                   // stem up
                        qreal dy  = dl * .5;                      // note-side vert. pos.
                        qreal sel = ul * .5 - normalStemLen;      // stem end vert. pos

                        // if stem ends above top line (with some exceptions), shorten it
                        if (shortenStem && (sel < 0.0)
                                    && (hookIdx == 0 || tab || !downnote->mirror()))
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
                        if (shortenStem && (sel > staffHeight)
                           && (hookIdx == 0 || tab || downnote->mirror()))
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
                  int tab[2][2][2][4] = {
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
            // scale stemLen according to staff line spacing
            if (staff())
                  stemLen *= staff()->staffType()->lineDistance().val();
           _stem->setLen(stemLen * _spatium);
            // if (isGrace())
            //      abort();
            if (_hook) {
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

      if (glissando())
            glissando()->layout();
      qreal _spatium = spatium();

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
                  Spatium len(h->len());
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

                              qreal d = cx - ll->measureXPos() - (ll->len().val() * _spatium);
                              if (d < minDist) {
                                    //
                                    // the ledger lines overlap
                                    //
                                    qreal shorten = (minDist - d) * .5;
                                    x   += shorten;
                                    len -= Spatium(shorten / _spatium);
                                    ll->setLen(ll->len() - Spatium(shorten / _spatium));
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
      }

//---------------------------------------------------------
//   updateNotes
//---------------------------------------------------------

void Chord::updateNotes(AccidentalState* as)
      {
      for (Chord* c : _graceNotes)
            c->updateNotes(as);

      Drumset* drumset = 0;
      if (staff()->part()->instr()->useDrumset())
            drumset = staff()->part()->instr()->drumset();

      QList<Note*> nl(notes());
      if (drumset) {
            for (Note* note : nl) {
                  int pitch = note->pitch();
                  if (drumset->isValid(pitch)) {
                        note->setHeadGroup(drumset->noteHead(pitch));
                        note->setLine(drumset->line(pitch));
                        }
                  }
            }
      else {
            for (Note* note : nl)
                  note->layout10(as);
            }
      sortNotes();
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
            const Instrument* instrument = staff()->part()->instr();
            for (Chord* ch : graceNotes())
                  instrument->stringData()->fretChords(ch);
            instrument->stringData()->fretChords(this);
            return;
            }

      // PITCHED_ and PERCUSSION_STAFF can go note by note

      for (Chord* ch : graceNotes()) {
            QList<Note*> notes(ch->notes());  // we need a copy!
            for (Note* note : notes)
                  note->updateAccidental(as);
            ch->sortNotes();
            }

      QList<Note*> lnotes(notes());  // we need a copy!
      for (Note* note : lnotes) {
            if (staffGroup == StaffGroup::STANDARD) {
                  if (note->tieBack()) {
                        if (note->accidental() && note->tpc() == note->tieBack()->startNote()->tpc()) {
                              // TODO: remove accidental only if note is not
                              // on new system
                              score()->undoRemoveElement(note->accidental());
                              }
                        }
                  note->updateAccidental(as);
                  }
            else if (staffGroup == StaffGroup::PERCUSSION) {
                  const Instrument* instrument = staff()->part()->instr();
                  Drumset* drumset = instrument->drumset();
                  int pitch = note->pitch();
                  if (drumset) {
                        if (!drumset->isValid(pitch)) {
                              // qDebug("unmapped drum note %d", pitch);
                              }
                        else {
                              note->setHeadGroup(drumset->noteHead(pitch));
                              note->setLine(drumset->line(pitch));
                              continue;
                              }
                        }
                  }
            }
      sortNotes();
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
      QList<Chord*> graceNotesBefore;
      int gnb = getGraceNotesBefore(&graceNotesBefore);
      QList<Chord*> graceNotesAfter;
      // lay out grace notes after separately so they are processed left to right
      // (they are normally stored right to left)
      int gna = getGraceNotesAfter(&graceNotesAfter);
      if (gna) {
            for (Chord* c : graceNotesAfter)
                  c->layoutPitched();
            }

      qreal _spatium  = spatium();
      qreal dotNoteDistance = score()->styleS(StyleIdx::dotNoteDistance).val() * _spatium;
      qreal minNoteDistance = score()->styleS(StyleIdx::minNoteDistance).val() * _spatium;
      qreal minTieLength = score()->styleS(StyleIdx::MinTieLength).val() * _spatium;
      qreal ledgerLineLength = score()->styleD(StyleIdx::ledgerLineLength) * _spatium;
      qreal graceMag = score()->styleD(StyleIdx::graceNoteMag);
      qreal chordX = (_noteType == NoteType::NORMAL) ? ipos().x() : 0.0;

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
            return;
            }

      //-----------------------------------------
      //  process notes
      //-----------------------------------------

      for (int i = 0; i < _notes.size(); ++i) {
            Note* note = _notes.at(i);
            note->layout();

            qreal x1 = note->pos().x() + chordX;
            qreal x2 = x1 + note->headWidth();
            lll = qMax(lll, -x1);
            rrr = qMax(rrr, x2);
            // track amount of space due to notehead only
            lhead = qMax(lhead, -x1);

            Accidental* accidental = note->accidental();
            if (accidental) {
                  // convert x position of accidental to segment coordinate system
                  qreal x = accidental->pos().x() + note->pos().x() + chordX;
                  // distance from accidental to note already taken into account
                  // but here perhaps we create more padding in *front* of accidental?
                  x -= score()->styleS(StyleIdx::accidentalDistance).val() * _spatium;
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
                                    for (int i = 0; i < sc->notes().size(); ++i)
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
      //  stem position
      //-----------------------------------------

      // Y: only needed if there is an actual stem
      if (stem())
            stem()->rypos() = (_up ? downNote() : upNote())->rypos();

      //-----------------------------------------
      //  create ledger lines
      //-----------------------------------------

      addLedgerLines(staffMove());
      for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next())
            ll->layout();

      if (_arpeggio) {
            _arpeggio->layout();    // only for width() !
            lll        += _arpeggio->width() + _spatium * .5 + chordX;
            qreal y1   = upnote->pos().y() - upnote->headHeight() * .5;
            _arpeggio->setPos(-lll, y1);
            _arpeggio->adjustReadPos();
            // _arpeggio->layout() called in layoutArpeggio2()

            // handle the special case of _arpeggio->span() > 1
            // in layoutArpeggio2() after page layout has done so we
            // know the y position of the next staves
            }

      // allocate enough room for glissandi
      if (_glissando) {
            lll += _spatium * 0.5;
            if (rtick())
                  lll += minTieLength;
            }

      if (dots()) {
            qreal x = dotPosX() + dotNoteDistance
               + (dots()-1) * score()->styleS(StyleIdx::dotDotDistance).val() * _spatium;
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
                        if (s && s->element(track()) && s->element(track())->type() == Element::Type::CHORD)
                              pc = static_cast<Chord*>(s->element(track()));
                        }
                  if (pc && !pc->graceNotes().isEmpty()) {
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
                              // grace note after - use main note
                              pc = mainChord;
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
                        llsp += _spatium * 0.2 * pc->mag();
                        // if any portion of note extended to left of origin
                        // we need to include that here so it is not subsumed by ledger line
                        llsp += lhead;
                        }
                  else if (pc->up() && upLine < 0) {
                        // even if no ledger lines in previous chord,
                        // we may need a little space to avoid crossing stem
                        llsp = ledgerLineLength * 0.5;
                        llsp += _spatium * 0.2 * pc->mag();
                        llsp += lhead;
                        }
                  lll = qMax(llsp, lll);
                  }

            }

      _space.setLw(lll);
      _space.setRw(rrr);

      if (gnb){
              qreal xl = -(_space.lw() + minNoteDistance) - chordX;
              for (int i = gnb-1; i >= 0; --i) {
                    Chord* c = graceNotesBefore.value(i);
                    xl -= c->space().rw()/* * 1.2*/;
                    c->setPos(xl, 0);
                    xl -= c->space().lw() + minNoteDistance * graceMag;
                    }
              if (-xl > _space.lw())
                    _space.setLw(-xl);
              }
       if (gna){
           // get factor for start distance after main note. Values found by testing.
           qreal fc;
           switch (durationType().type()) {
                 case TDuration::DurationType::V_LONG:    fc = 3.8; break;
                 case TDuration::DurationType::V_BREVE:   fc = 3.8; break;
                 case TDuration::DurationType::V_WHOLE:   fc = 3.8; break;
                 case TDuration::DurationType::V_HALF:    fc = 3.6; break;
                 case TDuration::DurationType::V_QUARTER: fc = 2.1; break;
                 case TDuration::DurationType::V_EIGHT:   fc = 1.4; break;
                 case TDuration::DurationType::V_16TH:    fc = 1.2; break;
                 default: fc = 1;
                 }
           qreal xr = fc * (_space.rw() + minNoteDistance);
           for (int i = 0; i <= gna - 1; i++) {
                 Chord* c = graceNotesAfter.value(i);
                 xr += c->space().lw() * (i == 0 ? 1.3 : 1);
                 c->setPos(xr, 0);
                 xr += c->space().rw() + minNoteDistance * graceMag;
                 }
           if (xr > _space.rw())
                 _space.setRw(xr);
           }

      for (Element* e : _el) {
            if (e->type() == Element::Type::SLUR)     // we cannot at this time as chordpositions are not fixed
                  continue;
            e->layout();
            if (e->type() == Element::Type::CHORDLINE) {
                  QRectF tbbox = e->bbox().translated(e->pos());
                  qreal lx = tbbox.left() + chordX;
                  qreal rx = tbbox.right() + chordX;
                  if (-lx > _space.lw())
                        _space.setLw(-lx);
                  if (rx > _space.rw())
                        _space.setRw(rx);
                  }
            }
      for (int i = 0; i < _notes.size(); ++i)
            _notes.at(i)->layout2();
      QRectF bb;
      processSiblings([&bb] (Element* e) { bb |= e->bbox().translated(e->pos()); } );
      setbbox(bb.translated(_spatium*2, 0));
      }

//---------------------------------------------------------
//   layoutTablature
//---------------------------------------------------------

void Chord::layoutTablature()
      {
      qreal _spatium  = spatium();
      qreal dotNoteDistance = score()->styleS(StyleIdx::dotNoteDistance).val() * _spatium;
      qreal minNoteDistance = score()->styleS(StyleIdx::minNoteDistance).val() * _spatium;

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

      int numOfNotes = _notes.size();
      for (int i = 0; i < numOfNotes; ++i) {
            Note* note = _notes.at(i);
            note->layout();
            // set headWidth to max fret text width
            qreal fretWidth = note->bbox().width();
            if (headWidth < fretWidth)
                  headWidth = fretWidth;
            // centre fret string on stem
            qreal x = stemX - fretWidth*0.5;
            note->setPos(x, tab->physStringToVisual(note->string()) * lineDist);
            }
      // horiz. spacing: leave half width at each side of the (potential) stem
      qreal halfHeadWidth = headWidth * 0.5;
      if (lll < stemX - halfHeadWidth)
            lll = stemX - halfHeadWidth;
      if (rrr < stemX + halfHeadWidth)
            rrr = stemX + halfHeadWidth;
      // if tab type is stemless or chord is stemless (possible when imported from MusicXML)
      // or duration longer than half (if halves have stems) or duration longer than crochet
      // remove stems
      if (tab->slashStyle() || _noStem || durationType().type() <
         (tab->minimStyle() != TablatureMinimStyle::NONE ? TDuration::DurationType::V_HALF : TDuration::DurationType::V_QUARTER) ) {
            delete _stem;
            delete _hook;
            _stem = 0;
            _hook = 0;
            }
      // if stem is required but missing, add it;
      // set stem position (stem length is set in Chord:layoutStem() )
      else {
            if (_stem == 0)
                  setStem(new Stem(score()));
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
      // unconditionally delete grace slashes
      delete _stemSlash;
      _stemSlash = 0;

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
            // OR duration type and/or number of dots is different from current CR
            // OR previous CR is a rest
            // set a duration symbol (trying to re-use existing symbols where existing to minimize
            // symbol creation and deletion)
            if (prevCR == 0 || prevCR->durationType().type() != durationType().type()
                  || prevCR->dots() != dots()
                  || prevCR->type() == Element::Type::REST) {
                  // symbol needed; if not exist, create; if exists, update duration
                  if (!_tabDur)
                        _tabDur = new TabDurationSymbol(score(), tab, durationType().type(), dots());
                  else
                        _tabDur->setDuration(durationType().type(), dots(), tab);
                  _tabDur->setParent(this);
//                  _tabDur->setMag(mag());     // useless to set grace mag: graces have no dur. symbol
                  _tabDur->layout();
                  }
            else {                    // symbol not needed: if exists, delete
                  delete _tabDur;
                  _tabDur = 0;
                  }
            }                 // end of if(duration_symbols)

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

      if (_glissando) {
            lll += _spatium * 0.5;
            if (rtick())
                  lll += score()->styleS(StyleIdx::MinTieLength).val() * _spatium;
            }

      if (dots()) {
            qreal x = dotPosX() + dotNoteDistance
               + (dots()-1) * score()->styleS(StyleIdx::dotDotDistance).val() * _spatium;
            x += symWidth(SymId::augmentationDot);
            if (x > rrr)
                  rrr = x;
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

      _space.setLw(lll);
      _space.setRw(rrr);

      QList<Chord*> graceNotesBefore;
      qreal graceMag = score()->styleD(StyleIdx::graceNoteMag);
      int nb = getGraceNotesBefore(&graceNotesBefore);
      if (nb){
              qreal xl = -(_space.lw() + minNoteDistance);
              for (int i = nb-1; i >= 0; --i) {
                    Chord* c = graceNotesBefore.value(i);
                    xl -= c->space().rw()/* * 1.2*/;
                    c->setPos(xl, 0);
                    xl -= c->space().lw() + minNoteDistance * graceMag;
                    }
              if (-xl > _space.lw())
                    _space.setLw(-xl);
              }
       QList<Chord*> graceNotesAfter;
       getGraceNotesAfter(&graceNotesAfter);
       int na = graceNotesAfter.size();
       if (na){
           // get factor for start distance after main note. Values found by testing.
           qreal fc;
           switch (durationType().type()) {
                 case TDuration::DurationType::V_LONG:    fc = 3.8; break;
                 case TDuration::DurationType::V_BREVE:   fc = 3.8; break;
                 case TDuration::DurationType::V_WHOLE:   fc = 3.8; break;
                 case TDuration::DurationType::V_HALF:    fc = 3.6; break;
                 case TDuration::DurationType::V_QUARTER: fc = 2.1; break;
                 case TDuration::DurationType::V_EIGHT:   fc = 1.4; break;
                 case TDuration::DurationType::V_16TH:    fc = 1.2; break;
                 default: fc = 1;
                 }
           qreal xr = fc * (_space.rw() + minNoteDistance);
           for (int i = 0; i <= na - 1; i++) {
                 Chord* c = graceNotesAfter.value(i);
                 xr += c->space().lw() * (i == 0 ? 1.3 : 1);
                 c->setPos(xr, 0);
                 xr += c->space().rw() + minNoteDistance * graceMag;
                 }
           if (xr > _space.rw())
                 _space.setRw(xr);
           }
      for (Element* e : _el) {
            e->layout();
            if (e->type() == Element::Type::CHORDLINE) {
                  QRectF tbbox = e->bbox().translated(e->pos());
                  qreal lx = tbbox.left();
                  qreal rx = tbbox.right();
                  if (-lx > _space.lw())
                        _space.setLw(-lx);
                  if (rx > _space.rw())
                        _space.setRw(rx);
                  }
            }
      // bbox();

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
            if(notes().size() == 1 && _notes[0]->tieFor()) {
                  Chord* tiedChord = _notes[0]->tieFor()->endNote()->chord();
                  // if tied note belongs to another measure and to a single-note chord
                  if(tiedChord->measure() != measure() && tiedChord->notes().size() == 1) {
                        // get total duration
                        QList<TDuration> durList = toDurationList(
                                    actualDurationType().fraction() +
                                    tiedChord->actualDurationType().fraction(), true);
                        // if duration can be expressed as a single duration
                        // apply cross-measure modification
                        if(durList.size() == 1) {
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
      switch(propertyId) {
            case P_ID::NO_STEM:        return noStem();
            case P_ID::SMALL:          return small();
            case P_ID::STEM_DIRECTION: return int(stemDirection());
            default:
                  return ChordRest::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Chord::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_ID::NO_STEM:        return false;
            case P_ID::SMALL:          return false;
            case P_ID::STEM_DIRECTION: return int(MScore::Direction::AUTO);
            default:
                  return ChordRest::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Chord::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_ID::NO_STEM:
                  setNoStem(v.toBool());
                  score()->setLayoutAll(true);
                  break;
            case P_ID::SMALL:
                  setSmall(v.toBool());
                  score()->setLayoutAll(true);
                  break;
            case P_ID::STEM_DIRECTION:
                  setStemDirection(MScore::Direction(v.toInt()));
                  score()->setLayoutAll(true);
                  break;
            default:
                  return ChordRest::setProperty(propertyId, v);
            }
      return true;
      }

//---------------------------------------------------------
//   layoutArticulation
//    called from chord()->layoutArticulations()
//---------------------------------------------------------

QPointF Chord::layoutArticulation(Articulation* a)
      {
      qreal _spatium = spatium();
      qreal _spStaff = _spatium * staff()->lineDistance();      // scaled to staff line distance for vert. pos. within a staff

      ArticulationAnchor aa = a->anchor();

      qreal chordTopY = upPos();    // note position of highest note
      qreal chordBotY = downPos();  // note position of lowest note
      qreal x         = centerX();
      qreal y = 0.0;

      ArticulationType st = a->articulationType();

      // TENUTO and STACCATO: always near the note head (or stem end if beyond a stem)
      if ((st == ArticulationType::Tenuto || st == ArticulationType::Staccato || st == ArticulationType::Sforzatoaccent) && (aa != ArticulationAnchor::TOP_STAFF && aa != ArticulationAnchor::BOTTOM_STAFF)) {
            bool bottom;                        // true: artic. is below chord | false: artic. is above chord
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
                        pos.rx() += point(score()->styleS(StyleIdx::stemWidth));
                  }
            a->layout();

            qreal _spatium2 = _spatium * .5;
            qreal _spStaff2 = _spStaff * .5;
            if (stemSide) {                     // if artic. is really beyond a stem,
                  qreal lineDelta = up() ? -_spStaff2 : _spStaff2;      // move it 1/2sp away from stem
                  int line = lrint((pos.y() + lineDelta) / _spStaff);   // round to nearest staff line
                  if (line >= 0 && line <= staff()->lines()-1)          // if within staff, align between staff lines
                        pos.ry() = (line * _spStaff) + (bottom ? _spStaff2 : -_spStaff2);
                  else {                                                // if outside staff, add some more space (?)
                        qreal dy = (score()->styleS(StyleIdx::beamWidth).val() + 1) * _spatium2;
                        pos.ry() += bottom ? dy : - dy;
                        }
                  }
            else {                              // if articulation is not beyond a stem
                  int line;
                  int add = (st == ArticulationType::Sforzatoaccent ? 1 : 0); // sforzato accent needs more offset
                  if (bottom) {                 // if below chord
                        line = downLine();                              // staff position (lines and spaces) of chord lowest note
                        int lines = (staff()->lines() - 1) * 2;         // num. of staff positions within staff
                        if (line < lines)                               // if note above staff bottom line
                              // round space pos. to line pos. above ("line & ~1") and move to 2nd space below ("+3")
                              line = ((line-add) & ~1) + 3 + add*2;
                        else                                            // if note on or below staff bottom line,
                              line += 2 + add;                                // move 1 whole space below
                        if (!staff()->isTabStaff())                     // on pitched staves, note is at left of stem:
                              pos.rx() -= upNote()->headWidth() * .5;   // move half-a-note-head to left
                        pos.ry() = -a->height() / 2;                    // symbol is below baseline, shift if a bit up
                        }
                  else {                        // if above chord
                        line = upLine();                                // staff position (lines and spaces) of chord highest note
                        if (line > 0)                                   // if note below staff top line
                              // round space pos. to line pos. below ("(line+1) & ~1") and move to 2nd space above ("-3")
                              line = ((line+1+add) & ~1) - 3 - add*2;
                        else                                            // if note or or above staff top line
                              line -= 2 + add;                                // move 1 whole space above
                        if (!staff()->isTabStaff())                     // on pitched staves, note is at right of stem:
                              pos.rx() += upNote()->headWidth() * .5;   // move half-a-note-head to right
                        pos.ry() = a->height() / 2;                     // symbol is on baseline, shift it a bit down
                        }
                  pos.ry() += line * _spStaff2;                          // convert staff position to sp distance
                  }
            a->setPos(pos);
            a->adjustReadPos();
            return QPointF(pos);
            }

      // other articulations are outside of area occupied by the staff or the chord
      // reserve space for slur
      bool botGap = false;
      bool topGap = false;

      const std::vector< ::Interval<Spanner*> >& si = score()->spannerMap().findOverlapping(tick(), tick());
      for (::Interval<Spanner*> is : si) {
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
            chordBotY += _spStaff;
      else
            chordBotY += _spStaff * .5;
      if (topGap)
            chordTopY -= _spStaff;
      else
            chordTopY -= _spStaff * .5;

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
                  qreal bw = score()->styleS(StyleIdx::beamWidth).val() * _spatium;
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
      // determine MScore::Direction
      //
      if (a->direction() != MScore::Direction::AUTO) {
            a->setUp(a->direction() == MScore::Direction::UP);
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

      qreal dist;                               // distance between occupied area and articulation
      switch(st) {
            case ArticulationType::Marcato:        dist = 1.0 * _spStaff; break;
            case ArticulationType::Sforzatoaccent: dist = 1.5 * _spStaff; break;
            default: dist = score()->styleS(StyleIdx::propertyDistance).val() * _spStaff;
            }

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
      score()->undoChangeProperty(this, P_ID::STEM_DIRECTION, int(MScore::Direction::AUTO));
      score()->undoChangeProperty(this, P_ID::BEAM_MODE, int(Beam::Mode::AUTO));
      score()->createPlayEvents(this);
      ChordRest::reset();
      }

//---------------------------------------------------------
//   setStemSlash
//---------------------------------------------------------

void Chord::setStemSlash(StemSlash* s)
      {
      delete _stemSlash;
      _stemSlash = s;
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
//   getGraceNotesBefore
//---------------------------------------------------------

int Chord::getGraceNotesBefore(QList<Chord*>* graceNotesBefore)
      {
      int i = 0;
      foreach (Chord* c, _graceNotes) {
               if (c->noteType() == NoteType::ACCIACCATURA
               || c->noteType() == NoteType::APPOGGIATURA
               || c->noteType() == NoteType::GRACE4
               || c->noteType() == NoteType::GRACE16
               || c->noteType() == NoteType::GRACE32){
                    if (graceNotesBefore)
                          graceNotesBefore->append(c);
                    i++;
                    }
              }
      return i;
      }

//---------------------------------------------------------
//   getGraceNotesAfter
//---------------------------------------------------------

int Chord::getGraceNotesAfter(QList<Chord*>* graceNotesAfter)
      {
      if (_graceNotes.length() == 0)
            return 0;
      int i = 0;
      for (int j = _graceNotes.length() - 1; j >= 0; --j) {
            Chord* c = _graceNotes.at(j);
            if (c->noteType() == NoteType::GRACE8_AFTER
             || c->noteType() == NoteType::GRACE16_AFTER
             || c->noteType() == NoteType::GRACE32_AFTER){
                  if (graceNotesAfter)
                        graceNotesAfter->append(c);
                  i++;
                  }
            }
      return i;
      }

//---------------------------------------------------------
//   sortNotes
//---------------------------------------------------------

void Chord::sortNotes()
      {
      std::sort(notes().begin(), notes().end(),
         [](const Note* a,const Note* b)->bool { return b->line() < a->line(); }
         // [](const Note* a,const Note* b)->bool { return a->pitch() < b->pitch(); }
         );
     }

//---------------------------------------------------------
//   toGraceAfter
//---------------------------------------------------------

void Chord::toGraceAfter()
      {
      switch (noteType()) {
            case NoteType::APPOGGIATURA:  setNoteType(NoteType::GRACE8_AFTER); break;
            case NoteType::GRACE16:  setNoteType(NoteType::GRACE16_AFTER); break;
            case NoteType::GRACE32:  setNoteType(NoteType::GRACE32_AFTER); break;
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
                        qDebug("Chord::tremoloChordType(): inconsistency");
                  }
            }
      return TremoloChordType::TremoloSingle;
      }
}

