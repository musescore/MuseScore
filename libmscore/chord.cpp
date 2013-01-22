//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: chord.cpp 5658 2012-05-21 18:40:58Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 \file
 Implementation of classes Chord, LedgerLine, NoteList Stem ans StemSlash.
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
#include "slur.h"
#include "arpeggio.h"
#include "score.h"
#include "tremolo.h"
#include "glissando.h"
#include "staff.h"
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
#include "rendermidi.h"

//---------------------------------------------------------
//   StemSlash
//---------------------------------------------------------

StemSlash::StemSlash(Score* s)
   : Element(s)
      {
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StemSlash::draw(QPainter* painter) const
      {
      qreal lw = point(score()->styleS(ST_stemWidth));
      painter->setPen(QPen(curColor(), lw));
      painter->drawLine(QLineF(line.x1(), line.y1(), line.x2(), line.y2()));
      }

//---------------------------------------------------------
//   setLine
//---------------------------------------------------------

void StemSlash::setLine(const QLineF& l)
      {
      line = l;
      qreal w = point(score()->styleS(ST_stemWidth)) * .5;
      setbbox(QRectF(line.p1(), line.p2()).normalized().adjusted(-w, w, 2.0*w, 2.0*w));
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StemSlash::layout()
      {
      Stem* stem = chord()->stem();
      qreal h2;
      qreal _spatium = spatium();
      qreal l = chord()->up() ? _spatium : -_spatium;
      QPointF p(stem->hookPos());
      qreal x = p.x() + _spatium * .1;
      qreal y = p.y();

      if (chord()->beam()) {
            y += l * .3;
            h2 = l * .8;
            }
      else {
            y += l * 1.2;
            h2 = l * .4;
            }
      qreal w  = chord()->upNote()->headWidth() * .7;
      setLine(QLineF(QPointF(x + w, y - h2), QPointF(x - w, y + h2)));
      }

//---------------------------------------------------------
//   upLine / downLine
//---------------------------------------------------------

int Chord::upLine() const
      {
      return upNote()->line();
      }

int Chord::downLine() const
      {
      return downNote()->line();
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
      StaffTypeTablature*     tab = (StaffTypeTablature*) staff()->staffType();
      int                     line = tab->lines() - 1;      // start at bottom line
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
      if(!staff())                              // if no staff, return 0
            return 0;
      if(!staff()->isTabStaff())                // if staff not a TAB, return bottom line
            return staff()->lines()-1;
      StaffTypeTablature*     tab = (StaffTypeTablature*) staff()->staffType();
      int                     line = 0;         // start at top line
      int                     noteLine;
      int n = _notes.size();
      for (int i = 0; i < n; ++i) {
            noteLine = tab->physStringToVisual(_notes.at(i)->string());
            if(noteLine > line)
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
      _stemDirection    = MScore::AUTO;
      _arpeggio         = 0;
      _tremolo          = 0;
      _tremoloChordType = TremoloSingle;
      _glissando        = 0;
      _noteType         = NOTE_NORMAL;
      _stemSlash        = 0;
      _noStem           = false;
      _userPlayEvents   = false;
      setFlags(ELEMENT_MOVABLE | ELEMENT_ON_STAFF);
      }

Chord::Chord(const Chord& c)
   : ChordRest(c)
      {
      _ledgerLines = 0;
      int n = c._notes.size();
      for (int i = 0; i < n; ++i)
            add(new Note(*c._notes.at(i)));

      _stem          = 0;
      _hook          = 0;
      _glissando     = 0;
      _arpeggio      = 0;
      _stemSlash     = 0;

      _noStem         = c._noStem;
      _userPlayEvents = c._userPlayEvents;

      if (c._stem)
            add(new Stem(*(c._stem)));
      if (c._hook)
            add(new Hook(*(c._hook)));
      if (c._glissando)
            add(new Glissando(*(c._glissando)));
      if (c._arpeggio)
            add(new Arpeggio(*(c._arpeggio)));
      if (c._stemSlash)
            add(new StemSlash(*(c._stemSlash)));
      _stemDirection    = c._stemDirection;
      _tremoloChordType = TremoloSingle;
      _tremolo          = 0;
      _noteType         = c._noteType;
      }

//---------------------------------------------------------
//   linkedClone
//---------------------------------------------------------

Chord* Chord::linkedClone()
      {
      Chord* chord = new Chord(*this);
      linkTo(chord);
      int n = notes().size();
      for (int i = 0; i < n; ++i)
            _notes[i]->linkTo(chord->_notes[i]);
      return chord;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Chord::setScore(Score* s)
      {
      Element::setScore(s);
      int n = notes().size();
      for (int i = 0; i < n; ++i)
            _notes[i]->setScore(s);
      if (_stem)
           _stem->setScore(s);
      if (_hook)
            _hook->setScore(s);
      if (_glissando)
            _glissando->setScore(s);
      if (_arpeggio)
            _arpeggio->setScore(s);
      if (_stemSlash)
            _stemSlash->setScore(s);
      }

//---------------------------------------------------------
//   ~Chord
//---------------------------------------------------------

Chord::~Chord()
      {
      delete _arpeggio;
      if (_tremolo && _tremolo->chord1() == this)
            delete _tremolo;
      delete _glissando;
      delete _stemSlash;
      delete _stem;
      delete _hook;
      for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next())
            delete ll;
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
//   stemPos
//    return page coordinates
//---------------------------------------------------------

QPointF Chord::stemPos() const
      {
      if (staff() && staff()->isTabStaff())
            return (static_cast<StaffTypeTablature*>(staff()->staffType())->chordStemPos(this) * spatium()) + pagePos();
      return (_up ? downNote() : upNote())->stemPos(_up);
      }

//---------------------------------------------------------
//   stemPosX
//    return page coordinates
//---------------------------------------------------------

qreal Chord::stemPosX() const
      {
      qreal x = pageX();
      if (staff() && staff()->isTabStaff()) {
            QPointF p = static_cast<StaffTypeTablature*>(staff()->staffType())->chordStemPos(this) * spatium();
            x += p.x();
            }
      else if (_up)
            x += symbols[score()->symIdx()][quartheadSym].width(magS());
      return x;
      }

//---------------------------------------------------------
//   stemPosBeam
//    return stem position of note on beam side
//    return canvas coordinates
//---------------------------------------------------------

QPointF Chord::stemPosBeam() const
      {
      if (staff() && staff()->isTabStaff())
            return (static_cast<StaffTypeTablature*>(staff()->staffType())->chordStemPosBeam(this) * spatium()) + pagePos();
      return (_up ? upNote() : downNote())->stemPos(_up);
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
            case NOTE:
                  {
                  Note* note = static_cast<Note*>(e);
                  bool found = false;
                  for (int idx = 0; idx < _notes.size(); ++idx) {
                        if (note->pitch() < _notes[idx]->pitch()) {
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
            case ARPEGGIO:
                  _arpeggio = static_cast<Arpeggio*>(e);
                  break;
            case TREMOLO:
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
                        _tremoloChordType = TremoloFirstNote;
                        tr->chord2()->setTremolo(tr);
                        tr->chord2()->setTremoloChordType(TremoloSecondNote);
                        }
                  else
                        _tremoloChordType = TremoloSingle;
                  _tremolo = tr;
                  }
                  break;
            case GLISSANDO:
                  _glissando = static_cast<Glissando*>(e);
                  break;
            case STEM:
                  _stem = static_cast<Stem*>(e);
                  break;
            case HOOK:
                  _hook = static_cast<Hook*>(e);
                  break;
            case CHORDLINE:
                  _el.append(e);
                  break;
            case STEM_SLASH:
                  _stemSlash = static_cast<StemSlash*>(e);
                  _stemSlash->setMag(mag());
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
            case NOTE:
                  {
                  Note* note = static_cast<Note*>(e);
                  if (_notes.removeOne(note)) {
                        if (note->tieFor()) {
                              if (note->tieFor()->endNote())
                                    note->tieFor()->endNote()->setTieBack(0);
                              }
                        }
                  else
                        qDebug("Chord::remove() note %p not found!\n", e);
                  }
                  break;

            case ARPEGGIO:
                  _arpeggio = 0;
                  break;
            case TREMOLO:
                  {
                  Tremolo* tremolo = static_cast<Tremolo*>(e);
                  if (tremolo->twoNotes()) {
                        TDuration d = durationType();
                        int dots = d.dots();
                        d          = d.shift(1);
                        d.setDots(dots);
                        if (tremolo->chord1())
                              tremolo->chord1()->setDurationType(d);
                        if (tremolo->chord2())
                              tremolo->chord2()->setDurationType(d);
                        tremolo->chord2()->setTremolo(0);
                        }
                  _tremolo = 0;
                  }
                  break;
            case GLISSANDO:
                  _glissando = 0;
                  break;
            case STEM:
                  _stem = 0;
                  break;
            case HOOK:
                  _hook = 0;
                  break;
            case CHORDLINE:
                  _el.removeOne(e);
                  break;
            default:
                  ChordRest::remove(e);
                  break;
            }
      }

//---------------------------------------------------------
//   addLedgerLine
///   Add a ledger line to a chord.
///   \arg x          note head position
///   \arg staffIdx   determines the y origin
///   \arg line       vertical position of line
///   \arg lr         extend to left and/or right
//---------------------------------------------------------

void Chord::addLedgerLine(qreal x, int staffIdx, int line, int lr, bool visible)
      {
      qreal _spatium = spatium();
      qreal hw       = upNote()->headWidth();
      qreal hw2      = hw * .5;

      LedgerLine* h = new LedgerLine(score());

      h->setParent(this);
      h->setTrack(staff2track(staffIdx));
      if (staff()->invisible())
            visible = false;
      h->setVisible(visible);

      // ledger lines extend less than half a space on each side
      // of the notehead:
      //
      qreal ll = hw + score()->styleS(ST_ledgerLineLength).val() * _spatium;

      if (_noteType != NOTE_NORMAL)
            ll *= score()->style(ST_graceNoteMag).toDouble();
      x -= ll * .5;

      x += (lr & 1) ? -hw2 : hw2;
      if (lr == 3)
            ll += hw;

      Spatium len(ll / _spatium);

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

//---------------------------------------------------------
//   addLedgerLines
//---------------------------------------------------------

void Chord::addLedgerLines(qreal x, int move)
      {
      int uppos = 1000;
      int ulr   = 0;
      int idx   = staffIdx() + move;

      // make ledger lines invisible if all notes
      // are invisible
      bool visible = false;
      int n = _notes.size();
      for (int i = 0; i < n; ++i) {
            if (_notes.at(i)->visible()) {
                  visible = true;
                  break;
                  }
            }
      for (int ni = n - 1; ni >= 0; --ni) {
            const Note* note = _notes[ni];
            int l = note->line();
            if (l >= 0)
                  break;
            for (int i = (uppos+1) & ~1; i < l; i += 2)
                  addLedgerLine(x, idx, i, ulr, visible);
            ulr |= (up() ^ note->mirror()) ? 0x1 : 0x2;
            uppos = l;
            }
      for (int i = (uppos+1) & ~1; i <= -2; i += 2)
            addLedgerLine(x, idx, i, ulr, visible);

      int downpos = -1000;
      int dlr = 0;

      for (int i = 0; i < n; ++i) {
            const Note* note = _notes.at(i);
            int l = note->line();
            if (l <= 8)
                  break;
            for (int i = downpos & ~1; i > l; i -= 2)
                  addLedgerLine(x, idx, i, dlr, visible);
            dlr |= (up() ^ note->mirror()) ? 0x1 : 0x2;
            downpos = l;
            }
      for (int i = downpos & ~1; i >= 10; i -= 2)
            addLedgerLine(x, idx, i, dlr, visible);
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
//      exception: in tablatures all stems are up.
//-----------------------------------------------------------------------------

void Chord::computeUp()
      {
      // tablatures
      if (staff() && staff()->isTabStaff()) {
            _up = !((StaffTypeTablature*)staff()->staffType())->stemsDown();
            return;
            }
      // pitched staves
      if (_stemDirection != MScore::AUTO) {
            _up = _stemDirection == MScore::UP;
            }
      else if (_noteType != NOTE_NORMAL) {
            //
            // stem direction for grace notes
            //
            if (measure()->mstaff(staffIdx())->hasVoices) {
                  switch(voice()) {
                        case 0:  _up = (score()->style(ST_stemDir1).toDirection() == MScore::UP); break;
                        case 1:  _up = (score()->style(ST_stemDir2).toDirection() == MScore::UP); break;
                        case 2:  _up = (score()->style(ST_stemDir3).toDirection() == MScore::UP); break;
                        case 3:  _up = (score()->style(ST_stemDir4).toDirection() == MScore::UP); break;
                        }
                  }
            else
                  _up = true;
            }
      else if (measure()->mstaff(staffIdx())->hasVoices) {
            switch(voice()) {
                  case 0:  _up = (score()->style(ST_stemDir1).toDirection() == MScore::UP); break;
                  case 1:  _up = (score()->style(ST_stemDir2).toDirection() == MScore::UP); break;
                  case 2:  _up = (score()->style(ST_stemDir3).toDirection() == MScore::UP); break;
                  case 3:  _up = (score()->style(ST_stemDir4).toDirection() == MScore::UP); break;
                  }
            }
      else if (_notes.size() == 1 || staffMove()) {
            if (staffMove() > 0)
                  _up = true;
            else if (staffMove() < 0)
                  _up = false;
            else
                  _up = upNote()->line() > 4;
            }
      else {
            Note* un = upNote();
            Note* dn = downNote();
            int ud = un->line() - 4;
            int dd = dn->line() - 4;
            if (-ud == dd) {
                  int up = 0;
                  int n = _notes.size();
                  for (int i = 0; i < n; ++i) {
                        const Note* n = _notes.at(i);
                        int l = n->line();
                        if (l <= 4)
                              --up;
                        else
                              ++up;
                        }
                  _up = up > 0;
                  }
            else
                  _up = dd > -ud;
            }
      }

//---------------------------------------------------------
//   selectedNote
//---------------------------------------------------------

Note* Chord::selectedNote() const
      {
      Note* note = 0;
      int n = _notes.size();
      for (int i = 0; i < n; ++n) {
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
      xml.stag("Chord");
      ChordRest::writeProperties(xml);
      if (_noteType != NOTE_NORMAL) {
            switch(_noteType) {
                  case NOTE_INVALID:
                  case NOTE_NORMAL:
                        break;
                  case NOTE_ACCIACCATURA:
                        xml.tagE("acciaccatura");
                        break;
                  case NOTE_APPOGGIATURA:
                        xml.tagE("appoggiatura");
                        break;
     	            case NOTE_GRACE4:
                        xml.tagE("grace4");
                        break;
                  case NOTE_GRACE16:
                        xml.tagE("grace16");
                        break;
                  case NOTE_GRACE32:
                        xml.tagE("grace32");
                        break;
                  }
            }
      if (_noStem)
            xml.tag("noStem", _noStem);
      else if (_stem && (!_stem->userOff().isNull() || (_stem->userLen() != 0.0) || !_stem->visible() || (_stem->color() != MScore::defaultColor)))
            _stem->write(xml);
      if (_hook && (!_hook->visible() || !_hook->userOff().isNull() || (_hook->color() != MScore::defaultColor)))
            _hook->write(xml);
      switch(_stemDirection) {
            case MScore::UP:   xml.tag("StemDirection", QVariant("up")); break;
            case MScore::DOWN: xml.tag("StemDirection", QVariant("down")); break;
            case MScore::AUTO: break;
            }
      int n = _notes.size();
      for (int i = 0; i < n; ++i)
            _notes.at(i)->write(xml);
      if (_arpeggio)
            _arpeggio->write(xml);
      if (_glissando)
            _glissando->write(xml);
      if (_tremolo)
            _tremolo->write(xml);
      n = _el.size();
      for (int i = 0; i < n; ++i)
            _el.at(i)->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Chord::readNote
//---------------------------------------------------------

void Chord::readNote(XmlReader& e)
      {
      Note* note = new Note(score());
      note->setPitch(e.intAttribute("pitch"));
      if (e.hasAttribute("ticks")) {
            int ticks  = e.intAttribute("ticks");
            TDuration d;
            d.setVal(ticks);
            setDurationType(d);
            }
      int tpc = INVALID_TPC;
      if (e.hasAttribute("tpc"))
            tpc = e.intAttribute("tpc");

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            QString val(e.readElementText());
            if (tag == "StemDirection") {
                  if (val == "up")
                        _stemDirection = MScore::UP;
                  else if (val == "down")
                        _stemDirection = MScore::DOWN;
                  else
                        _stemDirection = MScore::Direction(e.readInt());
                  }
            else if (tag == "pitch")
                  note->setPitch(e.readInt());
            else if (tag == "prefix") {
                  qDebug("read Note:: prefix: TODO\n");
                  }
            else if (tag == "line")
                  note->setLine(e.readInt());
            else if (tag == "Tie") {
                  Tie* _tieFor = new Tie(score());
                  _tieFor->setTrack(track());
                  _tieFor->read(e);
                  _tieFor->setStartNote(note);
                  note->setTieFor(_tieFor);
                  }
            else if (tag == "Text") {
                  Text* f = new Text(score());
                  f->setTextStyle(score()->textStyle(TEXT_STYLE_FINGERING));
                  f->read(e);
                  f->setParent(this);
                  note->add(f);
                  }
            else if (tag == "move")
                  setStaffMove(e.readInt());
            else if (!ChordRest::readProperties(e))
                  e.unknown();
            }
      if (!tpcIsValid(tpc))
            note->setTpc(tpc);
      else
            note->setTpcFromPitch();
      add(note);
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
            else if (tag == "appoggiatura") {
                  _noteType = NOTE_APPOGGIATURA;
                  e.readNext();
                  }
            else if (tag == "acciaccatura") {
                  _noteType = NOTE_ACCIACCATURA;
                  e.readNext();
                  }
            else if (tag == "grace4") {
                  _noteType = NOTE_GRACE4;
                  e.readNext();
                  }
            else if (tag == "grace16") {
                  _noteType = NOTE_GRACE16;
                  e.readNext();
                  }
            else if (tag == "grace32") {
                  _noteType = NOTE_GRACE32;
                  e.readNext();
                  }
            else if (tag == "StemDirection") {
                  QString val(e.readElementText());
                  if (val == "up")
                        _stemDirection = MScore::UP;
                  else if (val == "down")
                        _stemDirection = MScore::DOWN;
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
            else if (tag == "ChordLine") {
                  ChordLine* cl = new ChordLine(score());
                  cl->read(e);
                  add(cl);
                  }
            else if (!ChordRest::readProperties(e))
                  e.unknown();
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
            func(data, _hook);
      if (_stem)
            func(data, _stem);
      if (_stemSlash)
            func(data, _stemSlash);
      if (_arpeggio)
            func(data, _arpeggio);
      if (_tremolo && (_tremoloChordType != TremoloSecondNote))
            func(data, _tremolo);
      if (_glissando)
            func(data, _glissando);
      for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next())
            func(data, ll);
      int n = _notes.size();
      for (int i = 0; i < n; ++i)
            _notes.at(i)->scanElements(data, func, all);
      n = _el.size();
      for (int i = 0; i < n; ++i)
            _el.at(i)->scanElements(data, func, all);
      ChordRest::scanElements(data, func, all);
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Chord::setTrack(int val)
      {
      if (_hook)
            _hook->setTrack(val);
      if (_stem)
            _stem->setTrack(val);
      if (_stemSlash)
            _stemSlash->setTrack(val);
      if (_arpeggio)
            _arpeggio->setTrack(val);
      if (_glissando)
            _glissando->setTrack(val);
      if (_tremolo)
            _tremolo->setTrack(val);

      for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next())
            ll->setTrack(val);

      int n = _notes.size();
      for (int i = 0; i < n; ++i)
            _notes.at(i)->setTrack(val);
      ChordRest::setTrack(val);
      }

//---------------------------------------------------------
//   LedgerLine
//---------------------------------------------------------

LedgerLine::LedgerLine(Score* s)
   : Line(s, false)
      {
      setZ(NOTE * 100 - 50);
      setSelectable(false);
      _next = 0;
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF LedgerLine::pagePos() const
      {
      System* system = chord()->measure()->system();
      qreal yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(pageX(), yp);
      }

//---------------------------------------------------------
//   measureXPos
//---------------------------------------------------------

qreal LedgerLine::measureXPos() const
      {
      qreal xp = x();                   // chord relative
      xp += chord()->x();                // segment relative
      xp += chord()->segment()->x();     // measure relative
      return xp;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LedgerLine::layout()
      {
      setLineWidth(score()->styleS(ST_ledgerLineWidth));
      Line::layout();
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void Chord::setMag(qreal val)
      {
      ChordRest::setMag(val);
      for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next())
            ll->setMag(val);
      if (_stem)
            _stem->setMag(val);
      if (_hook)
            _hook->setMag(val);
      if (_stemSlash)
            _stemSlash->setMag(val);
      if (_arpeggio)
            _arpeggio->setMag(val);
      if (_tremolo)
            _tremolo->setMag(val);
      if (_glissando)
            _glissando->setMag(val);
      int n = _notes.size();
      for (int i = 0; i < n; ++i)
            _notes.at(i)->setMag(val);
      }

//---------------------------------------------------------
//   layoutStem1
///   Layout chord stem and hook.
//
//    called before layout spacing of notes
//    set hook if necessary to get right note width for next
//       pass
//---------------------------------------------------------

void Chord::layoutStem1()
      {
      int istaff = staffIdx();

      //-----------------------------------------
      //  process stem
      //-----------------------------------------

      bool hasStem = durationType().hasStem() && !(_noStem || measure()->slashStyle(istaff));
      int hookIdx  = hasStem ? durationType().hooks() : 0;

      if (hasStem) {
            if (!_stem)
                  setStem(new Stem(score()));
            }
      else
            setStem(0);

      if (hasStem && (_noteType == NOTE_ACCIACCATURA)) {
            if (_stemSlash == 0)
                  add(new StemSlash(score()));
            }
      else
            setStemSlash(0);

      //-----------------------------------------
      //  process hook
      //-----------------------------------------

      if (hookIdx) {
            if (!up())
                  hookIdx = -hookIdx;
            if (!_hook) {
                  Hook* hook = new Hook(score());
                  hook->setParent(this);
                  score()->undoAddElement(hook);
                  }
            _hook->setMag(mag());
            _hook->setSubtype(hookIdx);
            }
      else if (_hook)
            score()->undoRemoveElement(_hook);
      }

//---------------------------------------------------------
//   layoutStem
///   Layout chord tremolo stem and hook.
//---------------------------------------------------------

void Chord::layoutStem()
      {
      //
      // TAB
      //
      if (staff() && staff()->isTabStaff()) {
            StaffTypeTablature* tab = (StaffTypeTablature*)staff()->staffType();
            // require stems only if not stemless and this chord has a stem
            if (!tab->slashStyle() && _stem) {
                  // process stem:
                  _stem->setPos(tab->chordStemPos(this) * spatium());
                  _stem->setLen(tab->chordStemLength(this) * spatium());
                  // process hook
                  int   hookIdx = durationType().hooks();
                  if (tab->stemsDown())
                        hookIdx = -hookIdx;
                  if (hookIdx) {
                        _hook->setSubtype(hookIdx);
                        _hook->setPos(_stem->pos().x(), _stem->pos().y() + (up() ? -_stem->len() : _stem->len()));
                        _hook->setMag(mag()*score()->styleD(ST_smallNoteMag));
                        _hook->adjustReadPos();
                        }
                  return;
                  }
            }

      //
      // NON-TAB
      //
      if (segment()) {
            System* s = segment()->measure()->system();
            if (s == 0)       //DEBUG
                  return;
            }

      if (_stem) {
            qreal _spatium   = spatium();
            qreal stemLen;
            int hookIdx      = durationType().hooks();
            Note* upnote     = upNote();
            Note* downnote   = downNote();
            int ul           = upnote->line();
            int dl           = downnote->line();
            bool shortenStem = score()->styleB(ST_shortenStem);
            if (hookIdx >= 2 || _tremolo)
                  shortenStem = false;

            Spatium progression(score()->styleS(ST_shortStemProgression));
            qreal shortest(score()->styleS(ST_shortestStem).val());

            qreal normalStemLen = small() ? 2.5 : 3.5;
            switch(hookIdx) {
                  case 3: normalStemLen += small() ? .5  : 0.75; break; //32nd notes
                  case 4: normalStemLen += small() ? 1.0 : 1.5;  break; //64th notes
                  case 5: normalStemLen += small() ? 1.5 : 2.25; break; //128th notes
                  }
            if (_hook) {
                  if (up() && durationType().dots()) {
                        //
                        // avoid collision of dot with hook
                        //
                        if (!(ul & 1))
                              normalStemLen += .5;
                        shortenStem = false;
                        }
                  }

            if (_noteType != NOTE_NORMAL) {
                  // grace notes stems are not subject to normal
                  // stem rules
                  stemLen =  qAbs(ul - dl) * .5;
                  stemLen += normalStemLen * score()->styleD(ST_graceNoteMag);
                  if (up())
                        stemLen *= -1;
                  }
            else {
                  if (up()) {
                        qreal dy  = dl * .5;
                        qreal sel = ul * .5 - normalStemLen;

                        if (shortenStem && (sel < 0.0) && (hookIdx == 0 || !downnote->mirror()))
                              sel -= sel  * progression.val();
                        if (sel > 2.0)
                              sel = 2.0;
                        stemLen = sel - dy;
                        if (-stemLen < shortest)
                              stemLen = -shortest;
                        }
                  else {
                        qreal uy  = ul * .5;
                        qreal sel = dl * .5 + normalStemLen;

                        if (shortenStem && (sel > 4.0) && (hookIdx == 0 || downnote->mirror()))
                              sel -= (sel - 4.0)  * progression.val();
                        if (sel < 2.0)
                              sel = 2.0;
                        stemLen = sel - uy;
                        if (stemLen < shortest)
                              stemLen = shortest;
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
            _stem->setLen(stemLen * _spatium);
            if (_hook) {
                  _hook->setPos(_stem->hookPos());
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
//   layout2
//    Called after horizontal positions of all elements
//    are fixed.
//---------------------------------------------------------

void Chord::layout2()
      {
      if (glissando())
            glissando()->layout();
      qreal _spatium = spatium();

      //
      // Experimental:
      //    look for colliding ledger lines
      //

      const qreal minDist = _spatium * .17;

      Segment* s = segment()->prev(Segment::SegChordRestGrace);
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
                        if (!e || e->type() != CHORD)
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
//   layout
//---------------------------------------------------------

void Chord::layout()
      {
      if (_notes.empty())
            return;

      qreal _spatium  = spatium();

      while (_ledgerLines) {
            LedgerLine* l = _ledgerLines->next();
            delete _ledgerLines;
            _ledgerLines = l;
            }

      qreal lx         = 0.0;
      Note*  upnote    = upNote();
      qreal headWidth  = symbols[score()->symIdx()][quartheadSym].width(magS());

      qreal minNoteDistance = score()->styleS(ST_minNoteDistance).val() * _spatium;
      bool  useTab     = false;
      StaffTypeTablature* tab = 0;

      if (staff() && staff()->isTabStaff()) {
            //
            // TABLATURE STAVES
            //
            useTab = true;
            tab = (StaffTypeTablature*)staff()->staffType();
            qreal lineDist = tab->lineDistance().val();
            int n = _notes.size();
            for (int i = 0; i < n; ++i) {
                  Note* note = _notes.at(i);
                  note->layout();
                  note->setPos(0.0, _spatium * tab->physStringToVisual(note->string()) * lineDist);
                  note->layout2();
                  }
            // if tab type is stemless or duration longer than half (if halves have stems) or duration longer than crochet
            // remove stems
            if (tab->slashStyle() || durationType().type() <
                        (tab->minimStyle() != TAB_MINIM_NONE ? TDuration::V_HALF : TDuration::V_QUARTER) ) {
                  delete _stem;
                  delete _hook;
                  _stem = 0;
                  _hook = 0;
                  }
            // if stem is required but missing, add it
            else if(/*durationType().hasStem() &&*/ _stem == 0)
                  setStem(new Stem(score()));
            // unconditionally delete grace slashes
            delete _stemSlash;
            _stemSlash = 0;

            if (!tab->genDurations()            // if tab is not set for duration symbols
               || (track2voice(track()))) {         // or not in first voice
                  //
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
                        || prevCR->type() == REST) {
                        // symbol needed; if not exist, create; if exists, update duration
                        if (!_tabDur)
                              _tabDur = new TabDurationSymbol(score(), tab, durationType().type(), dots());
                        else
                              _tabDur->setDuration(durationType().type(), dots(), tab);
                        _tabDur->setParent(this);
                        _tabDur->layout();
                        }
                  else {                    // symbol not needed: if exists, delete
                        delete _tabDur;
                        _tabDur = 0;
                        }
                  }                 // end of if(duration_symbols)
            }                       // end of if(isTabStaff)
      else {
            //
            // NON-TABLATURE STAVES
            //
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
            //    - position
            //-----------------------------------------

            qreal stepDistance = _spatium * .5;
            int stepOffset     = staff()->staffType()->stepOffset();

            adjustReadPos();

            qreal stemWidth5;
            qreal stemX = _up ? symbols[score()->symIdx()][quartheadSym].width(magS()) : 0.0;
            if (stem()) {
                  stemWidth5 = stem()->lineWidth() * .5;
                  _stem->rxpos() = _up ? stemX - stemWidth5 : stemWidth5;
                  }
            else
                  stemWidth5 = 0.0;

            int n = _notes.size();
            for (int i = 0; i < n; ++i) {
                  Note* note = _notes.at(i);
                  note->layout();
                  qreal x;

                  qreal hw = note->headWidth();

                  if (note->mirror())
                        if (_up)
                              x = stemX - stemWidth5 * 2;
                        else
                              x = stemX - hw + stemWidth5 * 2;
                  else {
                        if (_up)
                              x = stemX - hw;
                        else
                              x = 0.0;
                        }

                  note->rypos() = (note->line() + stepOffset) * stepDistance;
                  note->rxpos() = x;

                  Accidental* accidental = note->accidental();
                  if (accidental)
                        x = accidental->x() + x - minNoteDistance;
                  if (x < lx)
                        lx = x;
                  }
            if (stem())
                  stem()->rypos() = (_up ? _notes.front() : _notes.back())->rypos();

            addLedgerLines(stemX, staffMove());

            for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next())
                  ll->layout();
            }

      //
      // COMMON TO ALL STAVES
      //
      qreal lll = -lx;

      if (_arpeggio) {
            qreal headHeight = upnote->headHeight();
            _arpeggio->layout();
            lll += _arpeggio->width() + _spatium * .5;
            qreal y = upNote()->pos().y() - headHeight * .5;
            qreal h = downNote()->pos().y() - y;
            _arpeggio->setHeight(h);
            _arpeggio->setPos(-lll, y);

            // handle the special case of _arpeggio->span() > 1
            // in layoutArpeggio2() after page layout has done so we
            // know the y position of the next staves
            }

      if (_glissando)
            lll += _spatium * .5;

      qreal rrr = 0.0;
      int n = _notes.size();
      for (int i = 0; i < n; ++i) {
            Note* note = _notes.at(i);
            qreal lhw = useTab ? note->tabHeadWidth(tab) : note->headWidth();
            qreal rr = 0.0;
            if (note->mirror()) {
                  if (up())
                        rr = lhw * 2.0;
                  else {
                        if (lhw > lll)
                              lll = lhw;
                        }
                  }
            else
                  rr = lhw;
            if (rr > rrr)
                  rrr = rr;
            qreal xx = note->pos().x() + headWidth + pos().x();
            if (xx > dotPosX())
                  setDotPosX(xx);
            }
      if (dots()) {
            qreal x = dotPosX() + point(score()->styleS(ST_dotNoteDistance)
               + (dots()-1) * score()->styleS(ST_dotDotDistance));
            x += symbols[score()->symIdx()][dotSym].width(1.0);
            if (x > rrr)
                  rrr = x;
            }

      if (_hook) {
            _hook->layout();
            if (up() && !useTab)
                  rrr += _hook->width() + minNoteDistance;
            }

      if (_noteType != NOTE_NORMAL) {
            // qreal m = score()->styleD(ST_graceNoteMag);
            static const qreal m = .9;
            lll *= m;
            rrr *= m;
            }

      _space.setLw(lll);
      _space.setRw(rrr + ipos().x());

      n = _el.size();
      for (int i = 0; i < n; ++i) {
            Element* e = _el.at(i);
            e->layout();
            if (e->type() == CHORDLINE) {
                  int x = bbox().translated(e->pos()).right();
                  if (x > _space.rw())
                        _space.setRw(x);
                  }
            }
      // bbox();

      QRectF bb;
      n = _notes.size();
      for (int i = 0; i < n; ++i) {
            Note* note = _notes.at(i);
            note->layout2();
            bb |= note->bbox().translated(note->pos());
            }
      for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next())
            bb |= ll->bbox().translated(ll->pos());
      n = _articulations.size();
      for (int i = 0; i < n; ++i) {
            Articulation* a = _articulations.at(i);
            bb |= a->bbox().translated(a->pos());
            }
      if (_hook)
            bb |= _hook->bbox().translated(_hook->pos());
      if (_stem)
            bb |= _stem->bbox().translated(_stem->pos());
      if (_arpeggio)
            bb |= _arpeggio->bbox().translated(_arpeggio->pos());
      if (_glissando)
            bb |= _glissando->bbox().translated(_glissando->pos());
      if (_stemSlash)
            bb |= _stemSlash->bbox().translated(_stemSlash->pos());
      if (_tremolo)
            bb |= _tremolo->bbox().translated(_tremolo->pos());
      if (staff() && staff()->isTabStaff() && _tabDur)
            bb |= _tabDur->bbox().translated(_tabDur->pos());
      setbbox(bb);
      }

//---------------------------------------------------------
//   layoutArpeggio2
//    called after layout of page
//---------------------------------------------------------

void Chord::layoutArpeggio2()
      {
      if (!_arpeggio)
            return;
      Note* upnote      = upNote();
      qreal headHeight = upnote->headHeight();
      qreal y          = upNote()->pagePos().y() - headHeight * .5;
      int span          = _arpeggio->span();
      Note* dnote       = downNote();
      int btrack        = track() + (span - 1) * VOICES;
      ChordRest* bchord = static_cast<ChordRest*>(segment()->element(btrack));

      if (bchord && bchord->type() == CHORD)
            dnote = static_cast<Chord*>(bchord)->downNote();
      qreal h = dnote->pagePos().y() - y;
      _arpeggio->setHeight(h);

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
//      bool up = _arpeggio->subtype() != ARP_DOWN;
//      if (!notes.isEmpty())
//            renderArpeggio(notes, up);
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
//   noteLessThan
//---------------------------------------------------------

static bool noteLessThan(const Note* n1, const Note* n2)
      {
      return n1->pitch() <= n2->pitch();
      }

//---------------------------------------------------------
//   pitchChanged
//---------------------------------------------------------

void Chord::pitchChanged()
      {
      qSort(_notes.begin(), _notes.end(), noteLessThan);
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Chord::drop(const DropData& data)
      {
      Element* e = data.element;
      switch (e->type()) {
            case ARTICULATION:
                  {
                  Articulation* atr = static_cast<Articulation*>(e);
                  Articulation* oa = hasArticulation(atr);
                  if (oa) {
                        delete atr;
                        atr = 0;
                        // if attribute is already there, remove
                        // score()->cmdRemove(oa); // unexpected behaviour?
                        score()->select(oa, SELECT_SINGLE, 0);
                        }
                  else {
                        atr->setParent(this);
                        atr->setTrack(track());
                        score()->undoAddElement(atr);
                        }
                  return atr;
                  }

            case CHORDLINE:
                  e->setParent(this);
                  score()->undoAddElement(e);
                  break;

            case TREMOLO:
                  {
                  Tremolo* t = static_cast<Tremolo*>(e);
                  if (t->twoNotes()) {
                        Segment* s = segment()->next();
                        while (s) {
                              if (s->element(track()) && s->element(track())->type() == CHORD)
                                    break;
                              s = s->next();
                              }
                        if (s == 0) {
                              qDebug("no segment for second note of tremolo found\n");
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

            case ARPEGGIO:
                  {
                  Arpeggio* a = static_cast<Arpeggio*>(e);
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
      return segment()->dotPosX(staffIdx());
      }

//---------------------------------------------------------
//   setDotPosX
//---------------------------------------------------------

void Chord::setDotPosX(qreal val)
      {
      segment()->setDotPosX(staffIdx(), val);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Chord::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_NO_STEM:        return noStem();
            case P_SMALL:          return small();
            case P_STEM_DIRECTION: return int(stemDirection());
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
            case P_NO_STEM:
                  setNoStem(v.toBool());
                  score()->setLayoutAll(true);
                  break;
            case P_SMALL:
                  setSmall(v.toBool());
                  score()->setLayoutAll(true);
                  break;
            case P_STEM_DIRECTION:
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
      qreal _spatium  = spatium();

      a->layout();
      ArticulationAnchor aa = a->anchor();

      qreal chordTopY = upPos();    // note position of highest note
      qreal chordBotY = downPos();  // note position of lowest note
      qreal x         = centerX();
      qreal y = 0.0;

      int st = a->subtype();

      if (st == Articulation_Tenuto || st == Articulation_Staccato) {
            bool bottom;
            if ((aa == A_CHORD) && measure()->hasVoices(a->staffIdx()))
                  bottom = !up();
            else
                  bottom = (aa == A_BOTTOM_CHORD) || (aa == A_CHORD && up());
            bool stemSide = (bottom != up()) && stem();
            a->setUp(!bottom);

            QPointF pos;
            if (stem())
                  pos = stem()->hookPos();

            qreal _spatium2 = _spatium * .5;
            if (stemSide) {
                  qreal yy = up() ? -_spatium2 : _spatium2;
                  int line = lrint((pos.y() + yy) / _spatium);
                  if (line >= 0 && line <= 4)    // align between staff lines
                        pos.ry() = (line * _spatium) + (bottom ? _spatium2 : -_spatium2);
                  else {
                        qreal dy = (score()->styleS(ST_beamWidth).val() + 1) * _spatium2;
                        pos.ry() += bottom ? dy : - dy;
                        }
                  }
            else {
                  int line;
                  if (bottom) {
                        line = downLine();
                        int lines = (staff()->lines() - 1) * 2;
                        if (line < lines)
                              line = (line & ~1) + 3;
                        else
                              line += 2;
                        pos.rx() -= upNote()->headWidth() * .5;
                        }
                  else {
                        line = upLine();
                        if (line > 0)
                              line = ((line+1) & ~1) - 3;
                        else
                              line -= 2;
                        pos.rx() += upNote()->headWidth() * .5;
                        }
                  pos.ry() = line * _spatium2;
                  }

            a->setPos(pos);
            a->adjustReadPos();
            return QPointF(pos);
            }

      // reserve space for slur
      bool botGap = false;
      bool topGap = false;
      for (Spanner* sp = spannerFor(); sp; sp = sp->next()) {
            if (sp->type() != SLUR)
                 continue;
            Slur* s = static_cast<Slur*>(sp);
            if (s->up())
                  topGap = true;
            else
                  botGap = true;
            }
      for (Spanner* sp = spannerBack(); sp; sp = sp->next()) {
            if (sp->type() != SLUR)
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

      qreal staffTopY = 0;
      qreal staffBotY = staff()->height();

      if (stem()) {
#if 0
            y = stem()->pos().y() + pos().y();
            if (up() && stem()->stemLen() < 0.0)
                  y += stem()->stemLen();
            else if (!up() && stem()->stemLen() > 0.0)
                  y -= stem()->stemLen();
#endif
            y = stem()->hookPos().y() + pos().y();

            if (beam()) {
                  qreal bw = score()->styleS(ST_beamWidth).val() * _spatium;
                  y += up() ? -bw : bw;
                  }
            if (up())
                  staffTopY = qMin(staffTopY, y);
            else
                  staffBotY = qMax(staffBotY, y);
            }

      staffTopY = qMin(staffTopY, qreal(chordTopY));
      staffBotY = qMax(staffBotY, qreal(chordBotY));

      //
      // determine Direction
      //
      if (a->direction() != MScore::AUTO) {
            a->setUp(a->direction() == MScore::UP);
            }
      else {
            if (measure()->hasVoices(a->staffIdx())) {
                  a->setUp(up());
                  aa = up() ? A_TOP_STAFF : A_BOTTOM_STAFF;
                  }
            else {
                  if (aa == A_CHORD)
                        a->setUp(!up());
                  else
                        a->setUp(aa == A_TOP_STAFF || aa == A_TOP_CHORD);
                  }
            }

      qreal dist;
      switch(st) {
            case Articulation_Marcato:        dist = 1.0 * _spatium; break;
            case Articulation_Sforzatoaccent: dist = 1.5 * _spatium; break;
            default: dist = score()->styleS(ST_propertyDistance).val() * _spatium;
            }

      if (aa == A_CHORD || aa == A_TOP_CHORD || aa == A_BOTTOM_CHORD) {
            bool bottom;
            if ((aa == A_CHORD) && measure()->hasVoices(a->staffIdx()))
                  bottom = !up();
            else
                  bottom = (aa == A_BOTTOM_CHORD) || (aa == A_CHORD && up());
            y = bottom ? chordBotY + dist : chordTopY - dist;
            }
      else if (aa == A_TOP_STAFF || aa == A_BOTTOM_STAFF) {
            y = a->up() ? staffTopY - dist : staffBotY + dist;
            }
      a->setPos(x, y);
      a->adjustReadPos();
      return QPointF(x, y);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Chord::reset()
      {
      score()->undoChangeProperty(this, P_STEM_DIRECTION, int(MScore::AUTO));
      score()->undoChangeProperty(this, P_BEAM_MODE, int(BEAM_AUTO));
      createPlayEvents(this);
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

