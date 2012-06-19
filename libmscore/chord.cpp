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
      qreal l = spatium();
      QPointF p(stem->hookPos());
      qreal x = p.x() + l * .1;
      qreal y = p.y();

      if (chord()->beam()) {
            if (chord()->up()) {
                  y += l * .3;
                  h2 = l * .8;
                  }
            else {
                  y -= l * .3;
                  h2 = l * -.8;
                  }
            }
      else {
            if (chord()->up()) {
                  y += l * 1.2;
                  h2 = l * .4;
                  }
            else {
                  y -= l * 1.2;
                  h2 = l * -.4;
                  }
            }
      qreal w  = chord()->upNote()->headWidth() * .7;
      setLine(QLineF(QPointF(x + w, y - h2), QPointF(x - w, y + h2)));
      }

//---------------------------------------------------------
//   upLine
//---------------------------------------------------------

int Chord::upLine() const
      {
      return upNote()->line();
      }

//---------------------------------------------------------
//   downLine
//---------------------------------------------------------

int Chord::downLine() const
      {
      return downNote()->line();
      }

//---------------------------------------------------------
//   Chord
//---------------------------------------------------------

Chord::Chord(Score* s)
   : ChordRest(s)
      {
      _stem          = 0;
      _hook          = 0;
      _stemDirection = AUTO;
      _arpeggio      = 0;
      _tremolo       = 0;
      _tremoloChordType = TremoloSingle;
      _glissando     = 0;
      _noteType      = NOTE_NORMAL;
      _stemSlash     = 0;
      _noStem        = false;
      setFlags(ELEMENT_MOVABLE | ELEMENT_ON_STAFF);
      }

Chord::Chord(const Chord& c)
   : ChordRest(c)
      {
      foreach(Note* n, c.notes())
            add(new Note(*n));

      _stem          = 0;
      _hook          = 0;
      _glissando     = 0;
      _arpeggio      = 0;
      _stemSlash     = 0;

      _noStem = c._noStem;
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
      foreach(Note* n, notes())
            n->setScore(s);
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
      foreach(LedgerLine* h, _ledgerLines)
            delete h;
      foreach(Note* n, _notes)
            delete n;
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
      if (staff() && staff()->useTablature()) {
            qreal sp = spatium();
            return QPointF(STAFFTYPE_TAB_DEFAULTSTEMPOSX*sp, STAFFTYPE_TAB_DEFAULTSTEMPOSY*sp) +
               pagePos();
            }
      return (_up ? downNote() : upNote())->stemPos(_up);
      }

//---------------------------------------------------------
//   stemPosBeam
//    return stem position of note on beam side
//    return canvas coordinates
//---------------------------------------------------------

QPointF Chord::stemPosBeam() const
      {
      if (staff() && staff()->useTablature()) {
            qreal sp = spatium();
            return QPointF(STAFFTYPE_TAB_DEFAULTSTEMPOSX*sp, STAFFTYPE_TAB_DEFAULTSTEMPOSY*sp) +
               pagePos();
            }
      return (_up ? upNote() : downNote())->stemPos(_up);
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Chord::setSelected(bool f)
      {
      Element::setSelected(f);
      foreach(Note* n, _notes)
            n->setSelected(f);
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
///   \arg x          center of note head
///   \arg staffIdx   determines the y origin
///   \arg line       vertical position of line
//---------------------------------------------------------

void Chord::addLedgerLine(qreal x, int staffIdx, int line, int lr, bool visible)
      {
      qreal _spatium = spatium();
      qreal hw       = upNote()->headWidth();
      qreal hw2      = hw * .5;

      qreal y = line * _spatium * .5;

      LedgerLine* h   = new LedgerLine(score());
      h->setParent(this);
      h->setTrack(staffIdx * VOICES);
      if (staff()->invisible())
            visible = false;
      h->setVisible(visible);

      // ledger lines extend less than half a space on each side
      // of the notehead:
      //
      qreal ll = _notes[0]->headWidth() + score()->styleS(ST_ledgerLineLength).val() * _spatium;
      Spatium len(ll / _spatium);

      if (_noteType != NOTE_NORMAL)
            len *= score()->style(ST_graceNoteMag).toDouble();
      x -= len.val() * _spatium * .5;

      x += (lr & 1) ? -hw2 : hw2;
      if (lr == 3)
            len += Spatium(hw / spatium());

      //
      // Experimental:
      //  shorten ledger line to avoid collisions with accidentals
      //

      foreach(const Note* n, _notes) {
            if (n->line() >= (line-1) && n->line() <= (line+1) && n->accidental()) {
                  x   += _spatium * .25;
                  len -= Spatium(.25);
                  break;
                  }
            }
      h->setLen(len);
      h->setPos(x, y);
      _ledgerLines.push_back(h);
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
      foreach(const Note* note, _notes) {
            if (note->visible()) {
                  visible = true;
                  break;
                  }
            }
      for (int ni = _notes.size() - 1; ni >= 0; --ni) {
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
      foreach(const Note* note, _notes) {
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
      if (staff() && staff()->useTablature()) {
            _up = true;
            return;
            }
      // pitched staves
      if (_stemDirection != AUTO) {
            _up = _stemDirection == UP;
            }
      else if (_noteType != NOTE_NORMAL) {
            if (measure()->mstaff(staffIdx())->hasVoices) {
                  switch(voice()) {
                        case 0:  _up = (score()->style(ST_stemDir1).toDirection() == UP); break;
                        case 1:  _up = (score()->style(ST_stemDir2).toDirection() == UP); break;
                        case 2:  _up = (score()->style(ST_stemDir3).toDirection() == UP); break;
                        case 3:  _up = (score()->style(ST_stemDir4).toDirection() == UP); break;
                        }
                  }
            else
                  _up = true;
            }
      else if (measure()->mstaff(staffIdx())->hasVoices) {
            switch(voice()) {
                  case 0:  _up = (score()->style(ST_stemDir1).toDirection() == UP); break;
                  case 1:  _up = (score()->style(ST_stemDir2).toDirection() == UP); break;
                  case 2:  _up = (score()->style(ST_stemDir3).toDirection() == UP); break;
                  case 3:  _up = (score()->style(ST_stemDir4).toDirection() == UP); break;
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
                  foreach(const Note* n, _notes) {
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
      foreach(Note* n, _notes) {
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
            case UP:   xml.tag("StemDirection", QVariant("up")); break;
            case DOWN: xml.tag("StemDirection", QVariant("down")); break;
            case AUTO: break;
            }
      foreach (const Note* n, _notes)
            n->write(xml);
      if (_arpeggio)
            _arpeggio->write(xml);
      if (_glissando)
            _glissando->write(xml);
      if (_tremolo)
            _tremolo->write(xml);
      foreach(Element* e, _el)
            e->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Chord::readNote
//---------------------------------------------------------

void Chord::readNote(const QDomElement& de, QList<Tuplet*>* tuplets, QList<Spanner*>* spanner)
      {
      Note* note = new Note(score());
      note->setPitch(de.attribute("pitch").toInt());
      if (de.hasAttribute("ticks")) {
            int ticks  = de.attribute("ticks").toInt();
            TDuration d;
            d.setVal(ticks);
            setDurationType(d);
            }
      int tpc = INVALID_TPC;
      if (de.hasAttribute("tpc"))
            tpc = de.attribute("tpc").toInt();

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());

            if (tag == "StemDirection") {
                  if (val == "up")
                        _stemDirection = UP;
                  else if (val == "down")
                        _stemDirection = DOWN;
                  else
                        _stemDirection = Direction(val.toInt());
                  }
            else if (tag == "pitch")
                  note->setPitch(val.toInt());
            else if (tag == "prefix") {
                  qDebug("read Note:: prefix: TODO\n");
                  }
            else if (tag == "line")
                  note->setLine(val.toInt());
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
                  setStaffMove(val.toInt());
            else if (!ChordRest::readProperties(e, tuplets, spanner))
                  domError(e);
            }
      if (!tpcIsValid(tpc))
            note->setTpc(tpc);
      else
            note->setTpcFromPitch();
      add(note);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Chord::read(const QDomElement& e)
      {
      QList<Tuplet*> tl;
      QList<Spanner*> sl;
      read(e, &tl, &sl);
      }

//---------------------------------------------------------
//   Chord::read
//---------------------------------------------------------

void Chord::read(const QDomElement& de, QList<Tuplet*>* tuplets, QList<Spanner*>* spanner)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());

            if (tag == "Note") {
                  Note* note = new Note(score());
                  // the note needs to know the properties of the track it belongs to
                  note->setTrack(track());
                  note->setChord(this);
                  note->read(e);
                  add(note);
                  }
            else if (tag == "appoggiatura")
                  _noteType = NOTE_APPOGGIATURA;
            else if (tag == "acciaccatura")
                  _noteType = NOTE_ACCIACCATURA;
            else if (tag == "grace4")
                  _noteType = NOTE_GRACE4;
            else if (tag == "grace16")
                  _noteType = NOTE_GRACE16;
            else if (tag == "grace32")
                  _noteType = NOTE_GRACE32;
            else if (tag == "StemDirection") {
                  if (val == "up")
                        _stemDirection = UP;
                  else if (val == "down")
                        _stemDirection = DOWN;
                  else
                        _stemDirection = Direction(val.toInt());
                  }
            else if (tag == "noStem")
                  _noStem = val.toInt();
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
            else if (!ChordRest::readProperties(e, tuplets, spanner))
                  domError(e);
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
      foreach(LedgerLine* h, _ledgerLines)
            func(data, h);
      foreach(Note* n, _notes)
            n->scanElements(data, func, all);
      foreach(Element* e, _el)
            e->scanElements(data, func, all);
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

      foreach(LedgerLine* h, _ledgerLines)
            h->setTrack(val);

      foreach(Note* n, _notes)
            n->setTrack(val);
      ChordRest::setTrack(val);
      }

//---------------------------------------------------------
//   LedgerLine
//---------------------------------------------------------

LedgerLine::LedgerLine(Score* s)
   : Line(s, false)
      {
      setSelectable(false);
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF LedgerLine::pagePos() const
      {
      System* system = chord()->measure()->system();
      int st = track() / VOICES;
      qreal yp = y() + system->staff(st)->y() + system->y();
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
      foreach (LedgerLine* ll, _ledgerLines)
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
      foreach (Note* n, _notes)
            n->setMag(val);
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
//            if (staff()->useTablature()) {                  // in tab, all stems are up (if present)
//                  setStemDirection(UP);
//                  setUp(true);
//                  }
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
///   Layout chord stem and hook.
//---------------------------------------------------------

void Chord::layoutStem()
      {
      if (staff() && staff()->useTablature()) {
            // tablatures require stems only if not stemless
            if (!staff()->staffType()->slashStyle() && _stem) {   // if tab uses stems and this chord has one
                  // in tablatures, stem/hook setup is fixed: a simple 'comb' above the staff
                  qreal sp = spatium();
                  // process stem
                  _stem->setLen(STAFFTYPE_TAB_DEFAULTSTEMLEN*sp);
                  _stem->setPos(STAFFTYPE_TAB_DEFAULTSTEMPOSX*sp, STAFFTYPE_TAB_DEFAULTSTEMPOSY*sp);
                  // process hook
                  int hookIdx = durationType().hooks();
                  if (hookIdx) {
                        _hook->setSubtype(hookIdx);
                        _hook->setPos(_stem->hookPos());
                        _hook->setMag(mag()*score()->styleD(ST_smallNoteMag));
                        _hook->adjustReadPos();
                        }
                  }
            return;
            }

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
                  if (hookIdx >= 2)
                        shortenStem = false;
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

            QPointF npos(stemPos());
            _stem->setLen(stemLen * _spatium);
            _stem->setPos(npos - pagePos());

            if (_stemSlash) {
                  // TODO: does not work for chords
                  _stemSlash->layout();
                  }

            if (_hook) {
                  _hook->setPos(_stem->hookPos());
                  _hook->adjustReadPos();
                  }
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

      Segment* s = segment()->prev(SegChordRest | SegGrace);
      if (s) {
            int strack = staffIdx() * VOICES;
            int etrack = strack + VOICES;
            foreach (LedgerLine* h, _ledgerLines) {
                  Spatium len(h->len());
                  qreal y   = h->y();
                  qreal x   = h->x();
                  bool found = false;
                  qreal cx  = h->measureXPos();

                  for (int track = strack; track < etrack; ++track) {
                        Element* e = s->element(track);
                        if (!e || e->type() != CHORD)
                              continue;
                        foreach (LedgerLine* ll, *static_cast<Chord*>(e)->ledgerLines()) {
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

      foreach(const LedgerLine* l, _ledgerLines)
            delete l;
      _ledgerLines.clear();

      qreal lx         = 0.0;
      Note*  upnote     = upNote();
      qreal headWidth  = upnote->headWidth();
      qreal minNoteDistance = score()->styleS(ST_minNoteDistance).val() * _spatium;

      if (staff() && staff()->useTablature()) {
            //
            // TABLATURE STAVES
            //
            StaffTypeTablature * tab = (StaffTypeTablature*)staff()->staffType();
            qreal lineDist = tab->lineDistance().val();
            foreach(Note* note, _notes) {
                  note->layout();
                  note->setPos(0.0, _spatium * (tab->upsideDown() ? tab->lines()-note->string()-1 : note->string()) * lineDist);
                  note->layout2();
                  }
            // if tab type is stemless or duration longer than crochet
            // remove stems
            if (tab->slashStyle() || durationType().type() < TDuration::V_QUARTER) {
                  delete _stem;
                  delete _hook;
                  _stem = 0;
                  _hook = 0;
                  }
            // unconditionally delete grace slashes
            delete _stemSlash;
            _stemSlash = 0;

            if (!tab->genDurations()            // if tab is not set for duration symbols
               || (track() % VOICES)) {         // or not in first voice
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
                  // if no previous CR or duration type and/or number of dots is different from current CR
                  // set a duration symbol (trying to re-use existing symbols where existing to minimize
                  // symbol creation and deletion)
                  if (prevCR == 0 || prevCR->durationType().type() != durationType().type()
                     || prevCR->dots() != dots()) {
                        // symbol needed; if not exist, create
                        // if exists, update duration
                        if (!_tabDur)
                              _tabDur = new TabDurationSymbol(score(), tab, durationType().type(), dots());
                        else
                              _tabDur->setDuration(durationType().type(), dots());
                        _tabDur->setParent(this);
                        // needed?        _tabDur->setTrack(track());
                        }
                  else {                    // symbol not needed: if exists, delete
                        delete _tabDur;
                        _tabDur = 0;
                        }
                  }                 // end of if(duration_symbols)
            }                       // end of if(useTablature)
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
                  foreach(Note* note, _notes) {
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
            foreach(Note* note, _notes) {
                  note->layout();
                  qreal x = 0.0;

                  bool stemUp;
                  if (staffMove() < 0)
                        stemUp = false;
                  else if (staffMove() > 0)
                        stemUp = true;
                  else
                        stemUp = up();

                  if (note->mirror())
                        x += stemUp ? note->headWidth() : -note->headWidth();

                  if (note->small() && _up)
                        x += (headWidth - note->headWidth());

                  note->setPos(x, (note->line() + stepOffset) * stepDistance);
                  Accidental* accidental = note->accidental();
                  if (accidental)
                        x = accidental->x() + note->x() - minNoteDistance;
                  if (x < lx)
                        lx = x;
                  }

            qreal x  = upnote->ipos().x();

            if (up() ^ upnote->mirror())
                  x += headWidth;

            addLedgerLines(x, staffMove());

            foreach(LedgerLine* l, _ledgerLines)
                  l->layout();
            }

      //
      // COMMON TO ALL STAVES
      //
      renderPlayback();
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
      foreach(const Note* note, _notes) {
            qreal lhw = note->headWidth();
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
            if (up())
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

      foreach(Element* e, _el) {
            e->layout();
            if (e->type() == CHORDLINE) {
                  int x = bbox().translated(e->pos()).right();
                  if (x > _space.rw())
                        _space.setRw(x);
                  }
            }

      QRectF _bbox;
      foreach (Note* note, _notes) {
            note->layout2();
            _bbox |= note->bbox().translated(note->pos());
            }
      foreach(const LedgerLine* l, _ledgerLines)
            _bbox |= l->bbox().translated(l->pos());
      foreach(Articulation* a, articulations)
            _bbox |= a->bbox().translated(a->pos());
      if (_hook)
            _bbox |= _hook->bbox().translated(_hook->pos());
      if (_stem)
            _bbox |= _stem->bbox().translated(_stem->pos());
      if (_arpeggio)
            _bbox |= _arpeggio->bbox().translated(_arpeggio->pos());
      if (_glissando)
            _bbox |= _glissando->bbox().translated(_glissando->pos());
      if (_stemSlash)
            _bbox |= _stemSlash->bbox().translated(_stemSlash->pos());
      if (_tremolo)
            _bbox |= _tremolo->bbox().translated(_tremolo->pos());
      setbbox(_bbox);
      }

//---------------------------------------------------------
//   renderArpeggio
//---------------------------------------------------------

static void renderArpeggio(QList<Note*> notes, bool up)
      {

      if (notes.isEmpty())
            return;

      int minLen = 1000*1000;

      foreach(Note* note, notes) {
            int len = note->playTicks();
            if (len < minLen)
                  minLen = len;
            }
      int arpOffset = minLen / notes.size();

      int start, end, step;
      if (up) {
            start = 0;
            end   = notes.size();
            step  = 1;
            }
      else {
            start = notes.size() - 1;
            end   = -1;
            step  = -1;
            }
      int ctick = 0;
      for (int i = start; i != end; i += step) {
            Note* note = notes[i];
            note->setOnTimeOffset(ctick);
            ctick += arpOffset;
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
      bool up = _arpeggio->subtype() != ARP_DOWN;
      if (!notes.isEmpty())
            renderArpeggio(notes, up);
      }

//---------------------------------------------------------
//   renderPlayback
//---------------------------------------------------------

void Chord::renderPlayback()
      {
      //-----------------------------------------
      //  Layout acciaccatura and appoggiatura
      //-----------------------------------------

      if (segment()->subtype() == SegChordRest) {
            QList<Chord*> gl;
            Segment* s = segment();
            while (s->prev()) {
                  s = s->prev();
                  if (s->subtype() != SegGrace)
                        break;
                  Element* cr = s->element(track());
                  if (cr && cr->type() == CHORD)
                        gl.prepend(static_cast<Chord*>(cr));
                  }

            if (!gl.isEmpty()) {
                  int nticks = 0;
                  foreach(Chord* c, gl)
                        nticks += c->actualTicks();
                  int t = nticks;
                  if (gl.front()->noteType() == NOTE_ACCIACCATURA)
                        t /= 2;
                  if (t >= (actualTicks() / 2))
                        t = actualTicks() / 2;

                  int rt = 0;
                  foreach(Chord* c, gl) {
                        int len   = c->actualTicks() * t / nticks;
                        int etick = rt + len - c->actualTicks();
                        foreach(Note* n, c->notes()) {
                              n->setOnTimeOffset(rt);
                              n->setOffTimeOffset(etick);
                              }
                        rt += len;
                        }
                  foreach(Note* n, notes())
                        n->setOnTimeOffset(rt);
                  }
            }
      }

//---------------------------------------------------------
//   findNote
//---------------------------------------------------------

Note* Chord::findNote(int pitch) const
      {
      foreach(Note* n, _notes) {
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
                        renderArticulation(atr->articulationType());
                        }
                  return atr;
                  }
            case CHORDLINE:
                  e->setParent(this);
                  score()->undoAddElement(e);
                  break;
            default:
                  return ChordRest::drop(data);
            }
      return 0;
      }

//---------------------------------------------------------
//   renderArticulation
//---------------------------------------------------------

void Chord::renderArticulation(ArticulationType type)
      {
      int key  = staff()->key(segment()->tick()).accidentalType();
      QList<NoteEvent*> events;
      int pitch     = upNote()->ppitch();
      int pitchDown = diatonicUpDown(key, pitch, -1);
      int pitchUp   = diatonicUpDown(key, pitch, 1);

      switch (type) {
            case Articulation_Mordent:
                  //
                  // create default playback for Mordent
                  //
                  events.append(new NoteEvent(0, 0, 125));
                  events.append(new NoteEvent(pitchUp - pitch, 125, 125));
                  events.append(new NoteEvent(0, 250, 750));
                  break;
            case Articulation_Prall:
                  //
                  // create default playback events for PrallSym
                  //
                  events.append(new NoteEvent(0, 0, 125));
                  events.append(new NoteEvent(pitchDown - pitch, 125, 125));
                  events.append(new NoteEvent(0, 250, 750));
                  break;
            default:
                  return;
            }
      if (!events.isEmpty()) {
            foreach(Note* note, _notes)
                  note->setPlayEvents(events);
            foreach(NoteEvent* e, events)
                  delete e;
            }
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
                  setStemDirection(Direction(v.toInt()));
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
                  int line = lrint((pos.y() + 0.5 * _spatium) / _spatium);
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
      foreach(Spanner* sp, spannerFor()) {
            if (sp->type() != SLUR)
                 continue;
            Slur* s = static_cast<Slur*>(sp);
            if (s->up())
                  topGap = true;
            else
                  botGap = true;
            }
      foreach(Spanner* sp, spannerBack()) {
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
      if (a->direction() != AUTO) {
            a->setUp(a->direction() == UP);
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
