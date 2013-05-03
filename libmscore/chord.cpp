//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
#include "stemslash.h"
#include "ledgerline.h"

//---------------------------------------------------------
//   upNote / downNote
//---------------------------------------------------------

Note* Chord::upNote() const
      {
      Note* result = _notes.back();
      if (staff() && staff()->isDrumStaff()) {
            foreach(Note*n, _notes) {
                  if (n->line() < result->line()) {
                        result = n;
                        }
                  }
            }

      return result;
      }

Note* Chord::downNote() const
      {
      Note* result = _notes.front();
      if (staff() && staff()->isDrumStaff()) {
            foreach(Note*n, _notes) {
                  if (n->line() > result->line()) {
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
            qreal stemX = static_cast<StaffTypeTablature*>(staff()->staffType())->chordStemPosX(this) * spatium();
            x += stemX;
            }
      else if (_up)
            x += upNote()->headWidth();
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
                  _el.push_back(e);
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
                  _el.remove(e);
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
//    TABlatures:
//       stems beside staves:
//          All stems are up / down according to TAB::stemsDown() setting
//       stems through staves:
//          Same rules as per pitched staves
//-----------------------------------------------------------------------------

void Chord::computeUp()
      {
      StaffTypeTablature* tab = 0;
      // TAB STAVES
      if (staff() && staff()->isTabStaff()) {
            tab = (StaffTypeTablature*)staff()->staffType();
            // if no stems or stem beside staves
            if (tab->slashStyle() || !tab->stemThrough()) {
                  // if measure has voices, set stem direction according to voice
                  if (measure()->mstaff(staffIdx())->hasVoices) {
                        switch(voice()) {
                              case 0:  _up = (score()->style(ST_stemDir1).toDirection() == MScore::UP); break;
                              case 1:  _up = (score()->style(ST_stemDir2).toDirection() == MScore::UP); break;
                              case 2:  _up = (score()->style(ST_stemDir3).toDirection() == MScore::UP); break;
                              case 3:  _up = (score()->style(ST_stemDir4).toDirection() == MScore::UP); break;
                              }
                        }
                  else                          // if only voice 1,
                        _up = !tab->stemsDown();// unconditionally set _up according to TAB stem direction
                  return;                       // (if no stems, _up does not really matter!)
                  }
            // if TAB has stems through staves, chain into standard processing
            }

      // PITCHED STAVES (or TAB with stems through staves)
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
      else if (staffMove()) {
            _up = staffMove() > 0;
            }
      else if (measure()->mstaff(staffIdx())->hasVoices) {
            switch(voice()) {
                  case 0:  _up = (score()->style(ST_stemDir1).toDirection() == MScore::UP); break;
                  case 1:  _up = (score()->style(ST_stemDir2).toDirection() == MScore::UP); break;
                  case 2:  _up = (score()->style(ST_stemDir3).toDirection() == MScore::UP); break;
                  case 3:  _up = (score()->style(ST_stemDir4).toDirection() == MScore::UP); break;
                  }
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
      for (Element* e : _el)
            e->write(xml);
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
                  f->setTextStyleType(TEXT_STYLE_FINGERING);
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
            return ((StaffTypeTablature*)staff()->staffType())->chordStemPosX(this) * spatium();

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
//   setMag
//---------------------------------------------------------

void Chord::setMag(qreal val)
      {
      ChordRest::setMag(val);
      processSiblings([val] (Element* e) { e->setMag(val); } );
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
            _hook->setHookType(hookIdx);
            }
      else if (_hook)
            score()->undoRemoveElement(_hook);
      }

//---------------------------------------------------------
//   layoutStem
///   Layout chord tremolo stem and hook.
//
//    hook: sets position
//    stem: sets length, but not position (assumed to be set in Chord::layout())
//---------------------------------------------------------

void Chord::layoutStem()
      {
      //
      // TAB
      //
      qreal _spatium = spatium();
      StaffTypeTablature* tab = 0;
      if (staff() && staff()->isTabStaff()) {
            tab = (StaffTypeTablature*)staff()->staffType();
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
                              _hook->setPos(_stem->pos().x(), _stem->pos().y() + (up() ? -_stem->len() : _stem->len()));
                              _hook->setMag(mag()*score()->styleD(ST_smallNoteMag));
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

            if (_noteType != NOTE_NORMAL) {
                  // grace notes stems are not subject to normal
                  // stem rules
                  stemLen =  qAbs(ul - dl) * .5;
                  stemLen += normalStemLen * score()->styleD(ST_graceNoteMag);
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
            if(staff())
                  stemLen *= staff()->staffType()->lineDistance().val();
            _stem->setLen(stemLen * _spatium);
            if (_hook) {
                  _hook->setPos(_stem ->hookPos());
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

      qreal lll       = 0.0;                    // space to leave at left of chord
      qreal rrr       = 0.0;                    // space to leave at right of chord
      Note* upnote    = upNote();
      qreal headWidth = symbols[score()->symIdx()][quartheadSym].width(magS());
      StaffTypeTablature* tab = 0;

      if (staff() && staff()->isTabStaff()) {
            //
            // TABLATURE STAVES
            //
            tab = (StaffTypeTablature*)staff()->staffType();
            qreal lineDist = tab->lineDistance().val();
            qreal stemX = tab->chordStemPosX(this) *_spatium;
            int n = _notes.size();
            for (int i = 0; i < n; ++i) {
                  Note* note = _notes.at(i);
                  note->layout();
                  // set headWidth to max fret text width
                  qreal fretWidth = note->bbox().width();
                  if (headWidth < fretWidth)
                        headWidth = fretWidth;
                  // centre fret string on stem
                  qreal x = stemX - fretWidth*0.5;
                  note->setPos(x, _spatium * tab->physStringToVisual(note->string()) * lineDist);
//                  note->layout2();              // needed? it is repeated later right before computing bbox

                  }
            // horiz. spacing: leave half width at each side of the (potential) stem
            qreal halfHeadWidth = headWidth * 0.5;
            if (lll < stemX - halfHeadWidth)
                  lll = stemX - halfHeadWidth;
            if (rrr < stemX + halfHeadWidth)
                  rrr = stemX + halfHeadWidth;
            setDotPosX(rrr);
            // if tab type is stemless or chord is stemless (possible when imported from MusicXML)
            // or duration longer than half (if halves have stems) or duration longer than crochet
            // remove stems
            if (tab->slashStyle() || _noStem || durationType().type() <
               (tab->minimStyle() != TAB_MINIM_NONE ? TDuration::V_HALF : TDuration::V_QUARTER) ) {
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
                        if (beam()) {
                              delete _hook;
                              _hook = 0;
                              }
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
            adjustReadPos();
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
            int lx = 0.0;

            adjustReadPos();

            qreal stemWidth5;
            qreal noteWidth = _notes.size() ? downNote()->headWidth() :
                        symbols[score()->symIdx()][quartheadSym].width(magS());
            qreal stemX = _up ? noteWidth : 0.0;
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
                  if (accidental) {
                         qreal minNoteDistance = score()->styleS(ST_minNoteDistance).val() * _spatium;
                         x = accidental->x() + x - minNoteDistance;
                        }
                  if (x < lx)
                        lx = x;
                  }
            lll = -lx;
            if (stem())
                  stem()->rypos() = (_up ? downNote() : upNote())->rypos();

            addLedgerLines(stemX, staffMove());

            for (LedgerLine* ll = _ledgerLines; ll; ll = ll->next())
                  ll->layout();
            }

      //
      // COMMON TO ALL STAVES
      //

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

      if (_glissando)
            lll += _spatium * .5;

      if (!tab) {
            int n = _notes.size();
            for (int i = 0; i < n; ++i) {
                  Note* note = _notes.at(i);
                  qreal lhw = note->headWidth();
                  qreal rr = 0.0;               // assume note is at left of stem (0 space at right)
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
            }
      if (dots()) {
            qreal x = dotPosX() + point(score()->styleS(ST_dotNoteDistance)
               + (dots()-1) * score()->styleS(ST_dotDotDistance));
            x += symbols[score()->symIdx()][dotSym].width(1.0);
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
      _space.setRw(rrr + ipos().x());

      for (Element* e : _el) {
            e->layout();
            if (e->type() == CHORDLINE) {
                  int x = bbox().translated(e->pos()).right();
                  if (x > _space.rw())
                        _space.setRw(x);
                  }
            }
      // bbox();

      for (int i = 0; i < _notes.size(); ++i)
            _notes.at(i)->layout2();

      QRectF bb;
      processSiblings([&bb] (Element* e) { bb |= e->bbox().translated(e->pos()); } );
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
      qreal h = dnote->pagePos().y() + downNote()->headHeight() - y;
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
                  e->setTrack(track());
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
//   propertyDefault
//---------------------------------------------------------

QVariant Chord::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_NO_STEM:        return false;
            case P_SMALL:          return false;
            case P_STEM_DIRECTION: return int(MScore::AUTO);
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
      qreal _spatium = spatium();
      qreal _spStaff = _spatium * staff()->lineDistance();      // scaled to staff line distance for vert. pos. within a staff

      a->layout();
      ArticulationAnchor aa = a->anchor();

      qreal chordTopY = upPos();    // note position of highest note
      qreal chordBotY = downPos();  // note position of lowest note
      qreal x         = centerX();
      qreal y = 0.0;

      ArticulationType st = a->articulationType();

      // TENUTO and STACCATO: always near the note head (or stem end if beyond a stem)
      if (st == Articulation_Tenuto || st == Articulation_Staccato) {
            bool bottom;                        // true: artic. is below chord | false: artic. is above chord
            // if there area voices, articulation is on stem side
            if ((aa == A_CHORD) && measure()->hasVoices(a->staffIdx()))
                  bottom = !up();
            // otherwise, look at specific anchor type (and at chord up/down if necessary)
            else
                  bottom = (aa == A_BOTTOM_CHORD) || (aa == A_CHORD && up());
            bool stemSide = (bottom != up()) && stem();     // true if there a stem between the nearest note and the articulation
            a->setUp(!bottom);

            QPointF pos;                        // computed articulation position
            if (stem())                         // if there is a stem, assume artic. will be beyond the stem
                  pos = stem()->hookPos();

            qreal _spatium2 = _spatium * .5;
            qreal _spStaff2 = _spStaff * .5;
            if (stemSide) {                     // if artic. is really beyond a stem,
                  qreal lineDelta = up() ? -_spStaff2 : _spStaff2;      // move it 1/2sp away from stem
                  int line = lrint((pos.y() + lineDelta) / _spStaff);   // round to nearest staff line
                  if (line >= 0 && line <= staff()->lines()-1)          // if within staff, align between staff lines
                        pos.ry() = (line * _spStaff) + (bottom ? _spStaff2 : -_spStaff2);
                  else {                                                // if outside staff, add some more space (?)
                        qreal dy = (score()->styleS(ST_beamWidth).val() + 1) * _spatium2;
                        pos.ry() += bottom ? dy : - dy;
                        }
                  }
            else {                              // if articulation is not beyond a stem
                  int line;
                  if (bottom) {                 // if below chord
                        line = downLine();                              // staff position (lines and spaces) of chord lowest note
                        int lines = (staff()->lines() - 1) * 2;         // num. of staff positions within staff
                        if (line < lines)                               // if note above staff bottom line
                              // round space pos. to line pos. above ("line & ~1") and move to 2nd space below ("+3")
                              line = (line & ~1) + 3;
                        else                                            // if note on or below staff bottom line,
                              line += 2;                                // move 1 whole space below
                        if (!staff()->isTabStaff())                     // on pitched staves, note is at left of stem:
                              pos.rx() -= upNote()->headWidth() * .5;   // move half-a-note-head to left
                        }
                  else {                        // if above chord
                        line = upLine();                                // staff position (lines and spaces) of chord highest note
                        if (line > 0)                                   // if note below staff top line
                              // round space pos. to line pos. below ("(line+1) & ~1") and move to 2nd space above ("-3")
                              line = ((line+1) & ~1) - 3;
                        else                                            // if note or or above staff top line
                              line -= 2;                                // move 1 whole space above
                        if (!staff()->isTabStaff())                     // on pitched staves, note is at right of stem:
                              pos.rx() += upNote()->headWidth() * .5;   // move half-a-note-head to right
                        }
                  pos.ry() = line * _spStaff2;                          // convert staff position to sp distance
                  }

            a->setPos(pos);
            a->adjustReadPos();
            return QPointF(pos);
            }

      // other articulations are outside of area occupied by the staff or the chord
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
                  qreal bw = score()->styleS(ST_beamWidth).val() * _spatium;
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

      qreal dist;                               // distance between occupied area and articulation
      switch(st) {
            case Articulation_Marcato:        dist = 1.0 * _spStaff; break;
            case Articulation_Sforzatoaccent: dist = 1.5 * _spStaff; break;
            default: dist = score()->styleS(ST_propertyDistance).val() * _spStaff;
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
      score()->undoChangeProperty(this, P_BEAM_MODE, int(BeamMode::AUTO));
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

