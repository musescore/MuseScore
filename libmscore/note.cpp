//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: note.cpp 5631 2012-05-15 16:04:07Z lasconic $
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
 Implementation of classes Note and ShadowNote.
*/

#include <assert.h>

#include "note.h"
#include "score.h"
#include "key.h"
#include "chord.h"
#include "sym.h"
#include "xml.h"
#include "slur.h"
#include "text.h"
#include "clef.h"
#include "staff.h"
#include "pitchspelling.h"
#include "arpeggio.h"
#include "tremolo.h"
#include "utils.h"
#include "image.h"
#include "system.h"
#include "tuplet.h"
#include "articulation.h"
#include "drumset.h"
#include "segment.h"
#include "measure.h"
#include "undo.h"
#include "part.h"
#include "stafftype.h"
#include "tablature.h"
#include "fret.h"
#include "harmony.h"
#include "fingering.h"
#include "bend.h"
#include "noteevent.h"
#include "mscore.h"
#include "accidental.h"
#include "page.h"
#include "icon.h"
#include "notedot.h"

//---------------------------------------------------------
//   noteHeads
//    note head groups
//---------------------------------------------------------

const int noteHeads[2][Note::HEAD_GROUPS][HEAD_TYPES] = {
      {     // down stem
      { wholeheadSym,         halfheadSym,         quartheadSym,      brevisheadSym        },
      { wholecrossedheadSym,  halfcrossedheadSym,  crossedheadSym,    wholecrossedheadSym  },
      { wholediamondheadSym,  halfdiamondheadSym,  diamondheadSym,    wholediamondheadSym  },
      { s0triangleHeadSym,    d1triangleHeadSym,   d2triangleHeadSym, s0triangleHeadSym    },
      { s0miHeadSym,          s1miHeadSym,         s2miHeadSym,       -1                   },
      { wholeslashheadSym,    halfslashheadSym,    quartslashheadSym, wholeslashheadSym    },
      { xcircledheadSym,      xcircledheadSym,     xcircledheadSym,   xcircledheadSym      },
      { s0doHeadSym,          d1doHeadSym,         d2doHeadSym,       -1                   },
      { s0reHeadSym,          d1reHeadSym,         d2reHeadSym,       -1                   },
      { d0faHeadSym,          d1faHeadSym,         d2faHeadSym,       -1                   },
      { s0laHeadSym,          s1laHeadSym,         s2laHeadSym,       -1                   },
      { s0tiHeadSym,          d1tiHeadSym,         d2tiHeadSym,       -1                   },
      { s0solHeadSym,         s1solHeadSym,        s2solHeadSym,      -1                   },
      { wholeheadSym,         halfheadSym,         quartheadSym,      brevisheadaltSym     },
      },
      {     // up stem
      { wholeheadSym,         halfheadSym,         quartheadSym,      brevisheadSym        },
      { wholecrossedheadSym,  halfcrossedheadSym,  crossedheadSym,    wholecrossedheadSym  },
      { wholediamondheadSym,  halfdiamondheadSym,  diamondheadSym,    wholediamondheadSym  },
      { s0triangleHeadSym,    u1triangleHeadSym,   u2triangleHeadSym, s0triangleHeadSym    },
      { s0miHeadSym,          s1miHeadSym,         s2miHeadSym,       -1                   },
      { wholeslashheadSym,    halfslashheadSym,    quartslashheadSym, wholeslashheadSym    },
      { xcircledheadSym,      xcircledheadSym,     xcircledheadSym,   xcircledheadSym      },
      { s0doHeadSym,          u1doHeadSym,         u2doHeadSym,       -1                   },
      { s0reHeadSym,          u1reHeadSym,         u2reHeadSym,       -1                   },
      { u0faHeadSym,          u1faHeadSym,         u2faHeadSym,       -1                   },
      { s0laHeadSym,          s1laHeadSym,         s2laHeadSym,       -1                   },
      { s0tiHeadSym,          u1tiHeadSym,         u2tiHeadSym,       -1                   },
      { s0solHeadSym,         s1solHeadSym,        s2solHeadSym,      -1                   },
      { wholeheadSym,         halfheadSym,         quartheadSym,      brevisheadaltSym     },
      }
      };

//---------------------------------------------------------
//   NoteVal
//---------------------------------------------------------

NoteVal::NoteVal()
      {
      pitch     = -1;
      tpc       = INVALID_TPC,
      fret      = -1;
      string    = -1;
      headGroup = 0;
      }

//---------------------------------------------------------
//   noteHeadSym
//---------------------------------------------------------

Sym* noteHeadSym(bool up, int group, int type)
      {
      return &symbols[0][noteHeads[up][group][type]];
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void NoteHead::write(Xml& xml) const
      {
      xml.stag("NoteHead");
      xml.tag("name", symbols[0][_sym].name());
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

Note::Note(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      dragMode           = false;
      _pitch             = 0;
      _ppitch            = 0;
      _tuning            = 0.0;
      _accidental        = 0;
      _mirror            = false;
      _userMirror        = MScore::DH_AUTO;
      _small             = false;
      _dotPosition       = MScore::AUTO;
      _line              = 0;
      _fret              = -1;
      _string            = -1;
      _fretConflict      = false;
      _ghost             = false;
      _lineOffset        = 0;
      _tieFor            = 0;
      _tieBack           = 0;
      _tpc               = INVALID_TPC;
      _headGroup         = HEAD_NORMAL;
      _headType          = HEAD_AUTO;

      _hidden            = false;
      _subchannel        = 0;

      _veloType          = MScore::OFFSET_VAL;
      _veloOffset        = 0;

      _onTimeOffset      = 0;
      _onTimeUserOffset  = 0;

      _offTimeOffset     = 0;
      _offTimeUserOffset = 0;
      _dots[0]           = 0;
      _dots[1]           = 0;
      _dots[2]           = 0;
      }

Note::~Note()
      {
      delete _accidental;
      foreach(Element* e, _el)
            delete e;
      foreach(NoteEvent* e, _playEvents)
            delete e;
      delete _tieFor;
      delete _dots[0];
      delete _dots[1];
      delete _dots[2];
      }

Note::Note(const Note& n)
   : Element(n)
      {
      _subchannel        = n._subchannel;
      _line              = n._line;
      _fret              = n._fret;
      _string            = n._string;
      _fretConflict      = n._fretConflict;
      _ghost             = n._ghost;
      dragMode           = n.dragMode;
      _pitch             = n._pitch;
      _tpc               = n._tpc;
      _ppitch            = n._ppitch;
      _hidden            = n._hidden;
      _tuning            = n._tuning;
      _veloType          = n._veloType;
      _veloOffset        = n._veloOffset;
      _onTimeOffset      = n._onTimeOffset;
      _onTimeUserOffset  = n._onTimeUserOffset;
      _offTimeOffset     = n._offTimeOffset;
      _offTimeUserOffset = n._offTimeUserOffset;
      _headGroup         = n._headGroup;
      _headType          = n._headType;
      _mirror            = n._mirror;
      _userMirror        = n._userMirror;
      _small             = n._small;
      _dotPosition       = n._dotPosition;
      _accidental        = 0;
      if (n._accidental)
            add(new Accidental(*(n._accidental)));

      foreach(Element* e, n._el)
            add(e->clone());
      _playEvents.clear();
      foreach(NoteEvent* e, n._playEvents)
            _playEvents.append(new NoteEvent(*e));

      if (n._tieFor) {
            _tieFor = new Tie(*n._tieFor);
            _tieFor->setStartNote(this);
            _tieFor->setEndNote(0);
            }
      else
            _tieFor = 0;
      _tieBack  = 0;
      for (int i = 0; i < 3; ++i) {
            _dots[i] = 0;
            if (n._dots[i])
                  add(new NoteDot(*n._dots[i]));
            }
      _lineOffset = n._lineOffset;
      }

//---------------------------------------------------------
//   setPitch
//---------------------------------------------------------

void Note::setPitch(int val)
      {
      _pitch = restrict(val, 0, 127);
      int pitchOffset = 0;
      if (score()) {
            Part* part = score()->part(staffIdx());
            if (part)
                  pitchOffset = score()->styleB(ST_concertPitch) ? 0 : part->instr()->transpose().chromatic;
            }
      _ppitch = _pitch + pitchOffset;
      if (chord())
            chord()->pitchChanged();
      }

void Note::setPitch(int a, int b)
      {
      setPitch(a);
      _tpc = b;
      }

//---------------------------------------------------------
//   undoSetPitch
//---------------------------------------------------------

void Note::undoSetPitch(int p)
      {
      score()->undoChangeProperty(this, P_PITCH, p);
      }

//---------------------------------------------------------
//   setTpcFromPitch
//---------------------------------------------------------

void Note::setTpcFromPitch()
      {
      KeySigEvent key = (staff() && chord()) ? staff()->key(chord()->tick()) : KeySigEvent();
      _tpc    = pitch2tpc(_pitch, key.accidentalType());
// qDebug("setTpcFromPitch pitch %d tick %d key %d tpc %d\n", pitch(), chord()->tick(), key.accidentalType(), _tpc);
      }

//---------------------------------------------------------
//   setTpc
//---------------------------------------------------------

void Note::setTpc(int v)
      {
      if (!tpcIsValid(v)) {
            qDebug("Note::setTpc: bad tpc %d\n", v);
            abort();
            }
      _tpc = v;
      }

//---------------------------------------------------------
//   undoSetTpc
//---------------------------------------------------------

void Note::undoSetTpc(int tpc)
      {
      score()->undoChangeProperty(this, P_TPC, tpc);
      }

//---------------------------------------------------------
//   noteHead
//---------------------------------------------------------

int Note::noteHead() const
      {
      int hg = 1;
      int ht = 2;       // default quarter head
      if (chord()) {
            hg = chord()->up();
            ht = chord()->durationType().headType();
            }
      switch(_headType) {
            default:
            case HEAD_AUTO:
                  return noteHeads[hg][int(_headGroup)][ht];
            case HEAD_WHOLE:
                  return noteHeads[hg][int(_headGroup)][0];
            case HEAD_HALF:
                  return noteHeads[hg][int(_headGroup)][1];
            case HEAD_QUARTER:
                  return noteHeads[hg][int(_headGroup)][2];
            case HEAD_BREVIS:
                  return noteHeads[hg][int(_headGroup)][3];
            }
      }

//---------------------------------------------------------
//   headWidth
//---------------------------------------------------------

qreal Note::headWidth() const
      {
      qreal val = symbols[score()->symIdx()][noteHead()].width(magS());
      if (_small)
            val *= score()->styleD(ST_smallNoteMag);
      return val;
      }

//---------------------------------------------------------
//   headHeight
//---------------------------------------------------------

qreal Note::headHeight() const
      {
      return symbols[score()->symIdx()][noteHead()].height(magS());
      }

//---------------------------------------------------------
//   playTicks
//---------------------------------------------------------

/**
 Return total tick len of tied notes
*/

int Note::playTicks() const
      {
      const Note* note = this;
      while (note->tieBack())
            note = note->tieBack()->startNote();
      int len = 0;
      while (note->tieFor() && note->tieFor()->endNote()) {
            len += note->chord()->actualTicks();
            note = note->tieFor()->endNote();
            }
      len += note->chord()->actualTicks();
      return len;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Note::add(Element* e)
      {
	e->setParent(this);
      e->setTrack(track());

      switch(e->type()) {
            case NOTEDOT:
                  {
                  NoteDot* dot = static_cast<NoteDot*>(e);
                  _dots[dot->idx()] = dot;
                  }
                  break;
            case SYMBOL:
            case IMAGE:
            case FINGERING:
            case TEXT:
            case BEND:
                  _el.append(e);
                  break;
            case TIE:
                  {
                  Tie* tie = static_cast<Tie*>(e);
	      	tie->setStartNote(this);
                  tie->setTrack(track());
      		setTieFor(tie);
                  if (tie->endNote())
                        tie->endNote()->setTieBack(tie);
                  foreach(SpannerSegment* ss, tie->spannerSegments()) {
                        if (ss->system())
                              ss->system()->add(ss);
                        }
                  }
                  break;
            case ACCIDENTAL:
                  _accidental = static_cast<Accidental*>(e);
                  break;
            default:
                  qDebug("Note::add() not impl. %s\n", e->name());
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Note::remove(Element* e)
      {
      switch(e->type()) {
            case NOTEDOT:
                  for (int i = 0; i < 3; ++i) {
                        if (_dots[i] == e) {
                              _dots[i] = 0;
                              break;
                              }
                        }
                  break;

            case TEXT:
            case SYMBOL:
            case IMAGE:
            case FINGERING:
            case BEND:
                  if (!_el.remove(e))
                        qDebug("Note::remove(): cannot find %s\n", e->name());
                  break;
	      case TIE:
                  {
                  Tie* tie = static_cast<Tie*>(e);
                  setTieFor(0);
                  if (tie->endNote())
                        tie->endNote()->setTieBack(0);
                  foreach(SpannerSegment* ss, tie->spannerSegments()) {
                        Q_ASSERT(ss->spanner() == tie);
                        if (ss->system())
                              ss->system()->remove(ss);
                        }
                  }
                  break;

            case ACCIDENTAL:
                  _accidental = 0;
                  break;

            default:
                  qDebug("Note::remove() not impl. %s\n", e->name());
                  break;
            }
      }

//---------------------------------------------------------
//   stemPos
//    return in page coordinates
//---------------------------------------------------------

QPointF Note::stemPos(bool upFlag) const
      {
      QPointF pt(pos() + chord()->pagePos());
      if (_mirror)
            upFlag = !upFlag;
      if (upFlag)
            pt.rx() += headWidth();
      return pt;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Note::draw(QPainter* painter) const
      {
      if (_hidden)
            return;
      painter->setPen(curColor());
      bool tablature = staff() && staff()->isTabStaff();
      if (tablature) {
            if (tieBack())
                  return;
            StaffTypeTablature* tab = (StaffTypeTablature*)staff()->staffType();
            qreal mag = magS();
            qreal imag = 1.0 / mag;

            painter->scale(mag, mag);
            painter->setFont(tab->fretFont());

            // when using letters, "+(_fret > 8)" skips 'j'
            QString s = _ghost ? "X" :
                    ( tab->useNumbers() ? QString::number(_fret) : QString('a' + _fret + (_fret > 8)) );

            qreal currSpatium = spatium();
            qreal d  = currSpatium * .2;
            QRectF bb = bbox().adjusted(-d, 2*d, d, -2*d);

            // draw background, if required
            if (!tab->linesThrough() || fretConflict()) {
                  // we do not know which viewer did this draw() call
                  // so update all:
                  foreach(MuseScoreView* view, score()->getViewer())
                        view->drawBackground(painter, bb);

                  if (fretConflict()) {          //on fret conflict, draw on red background
                        painter->save();
                        painter->setPen(Qt::red);
                        painter->setBrush(QBrush(QColor(Qt::red)));
                        painter->drawRect(bb);
                        painter->restore();
                        }
                  }
            painter->setPen(curColor());
            painter->drawText(QPointF(bbox().x(), tab->fretFontYOffset()), s);
            painter->scale(imag, imag);
            }
      else {      // if not tablature
            //
            // warn if pitch extends usable range of instrument
            // by coloring the note head
            //
            if (chord() && chord()->segment() && staff() && !selected()
               && !score()->printing() && MScore::warnPitchRange) {
                  const Instrument* in = staff()->part()->instr();
                  int i = ppitch();
                  if (i < in->minPitchP() || i > in->maxPitchP())
                        painter->setPen(Qt::red);
                  else if (i < in->minPitchA() || i > in->maxPitchA())
                        painter->setPen(Qt::darkYellow);
                  }
            qreal mag = magS();
            if (_small)
                  mag *= score()->styleD(ST_smallNoteMag);
            symbols[score()->symIdx()][noteHead()].draw(painter, mag);
            }
      }

//--------------------------------------------------
//   Note::write
//---------------------------------------------------------

void Note::write(Xml& xml) const
      {
      xml.stag("Note");
      Element::writeProperties(xml);
      //
      // get real pitch for clipboard (copy/paste)
      //
      int rpitch = pitch();
      int rtpc   = tpc();

      if (staff()) {
            const Interval& interval = staff()->part()->instr()->transpose();
            if (xml.clipboardmode && !score()->styleB(ST_concertPitch) && interval.chromatic)
                  transposeInterval(rpitch, rtpc, &_pitch, &_tpc, interval, true);
            }

      if (_accidental)
            _accidental->write(xml);
      _el.write(xml);
      int dots = chord()->dots();
      if (dots) {
            bool hasUserModifiedDots = false;
            for (int i = 0; i < dots; ++i) {
                  if (_dots[i] && (!_dots[i]->userOff().isNull() || !_dots[i]->visible()
                     || _dots[i]->color() != Qt::black)) {
                        hasUserModifiedDots = true;
                        break;
                        }
                  }
            if (hasUserModifiedDots) {
                  for (int i = 0; i < dots; ++i)
                        _dots[i]->write(xml);
                  }
            }
      if (_tieFor)
            _tieFor->write(xml);
      if (!_playEvents.isEmpty()) {
            xml.stag("Events");
            foreach(const NoteEvent* e, _playEvents)
                  e->write(xml);
            xml.etag();
            }
      writeProperty(xml, P_PITCH);
      writeProperty(xml, P_TPC);
      writeProperty(xml, P_SMALL);
      writeProperty(xml, P_MIRROR_HEAD);
      writeProperty(xml, P_DOT_POSITION);
      writeProperty(xml, P_ONTIME_OFFSET);
      writeProperty(xml, P_OFFTIME_OFFSET);
      writeProperty(xml, P_HEAD_GROUP);
      writeProperty(xml, P_VELO_OFFSET);
      writeProperty(xml, P_TUNING);
      writeProperty(xml, P_FRET);
      writeProperty(xml, P_STRING);
      writeProperty(xml, P_GHOST);
      writeProperty(xml, P_HEAD_TYPE);
      writeProperty(xml, P_VELO_TYPE);

      _pitch = rpitch;
      _tpc   = rtpc;
      xml.etag();
      }

//---------------------------------------------------------
//   Note::read
//---------------------------------------------------------

void Note::read(const QDomElement& de)
      {
      bool hasAccidental = false;                     // used for userAccidental backward compatibility

      _tpc = INVALID_TPC;

      if (de.hasAttribute("pitch"))                   // obsolete
            _pitch = de.attribute("pitch").toInt();
      if (de.hasAttribute("tpc"))                     // obsolete
            _tpc = de.attribute("tpc").toInt();

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "pitch")
                  _pitch = val.toInt();
            else if (tag == "tpc")
                  _tpc = val.toInt();
            else if (tag == "small")
                  setSmall(val.toInt());
            else if (tag == "mirror")
                  setProperty(P_MIRROR_HEAD, ::getProperty(P_MIRROR_HEAD, e));
            else if (tag == "dotPosition")
                  setProperty(P_DOT_POSITION, ::getProperty(P_DOT_POSITION, e));
            else if (tag == "onTimeOffset")
                  setOnTimeUserOffset(val.toInt());
            else if (tag == "offTimeOffset")
                  setOffTimeUserOffset(val.toInt());
            else if (tag == "head")
                  setProperty(P_HEAD_GROUP, ::getProperty(P_HEAD_GROUP, e));
            else if (tag == "velocity")
                  setVeloOffset(val.toInt());
            else if (tag == "tuning")
                  setTuning(val.toDouble());
            else if (tag == "fret")
                  setFret(val.toInt());
            else if (tag == "string")
                  setString(val.toInt());
            else if (tag == "ghost")
                  setGhost(val.toInt());
            else if (tag == "headType")
                  setProperty(P_HEAD_TYPE, ::getProperty(P_HEAD_TYPE, e));
            else if (tag == "veloType")
                  setProperty(P_VELO_TYPE, ::getProperty(P_VELO_TYPE, e));
            else if (tag == "line")
                  _line = val.toInt();
            else if (tag == "Tie") {
                  _tieFor = new Tie(score());
                  _tieFor->setTrack(track());
                  _tieFor->read(e);
                  _tieFor->setStartNote(this);
                  }
            else if (tag == "Fingering" || tag == "Text") {       // Text is obsolete
                  Fingering* f = new Fingering(score());
                  f->setTextStyle(score()->textStyle(TEXT_STYLE_FINGERING));
                  f->read(e);
                  add(f);
                  }
            else if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->setTrack(track());
                  s->read(e);
                  add(s);
                  }
            else if (tag == "Image") {
                  // look ahead for image type
                  QString path;
                  QDomElement ee = e.firstChildElement("path");
                  if (!ee.isNull())
                        path = ee.text();
                  Image* image = 0;
                  QString s(path.toLower());
                  if (s.endsWith(".svg"))
                        image = new SvgImage(score());
                  else
                        if (s.endsWith(".jpg")
                     || s.endsWith(".png")
                     || s.endsWith(".gif")
                     || s.endsWith(".xpm")
                        ) {
                        image = new RasterImage(score());
                        }
                  else {
                        qDebug("unknown image format <%s>\n", qPrintable(path));
                        }
                  if (image) {
                        image->setTrack(track());
                        image->read(e);
                        add(image);
                        }
                  }
            else if (tag == "userAccidental") {
                  bool ok;
                  int k = val.toInt(&ok);
                  if (ok) {
                        // on older scores, a note could have both a <userAccidental> tag and an <Accidental> tag
                        // if a userAccidental has some other property set (like for instance offset)
                        // only costruct a new accidental, if the other tag has not been read yet
                        // (<userAccidental> tag is only used in older scores: no need to check the score mscVersion)
                        if (!hasAccidental) {
                              _accidental = new Accidental(score());
                              _accidental->setParent(this);
                              }
                        // TODO: for backward compatibility
                        bool bracket = k & 0x8000;
                        k &= 0xfff;
                        AccidentalType at = ACC_NONE;
                        switch(k) {
                              case 0: at = ACC_NONE; break;
                              case 1:
                              case 11: at = ACC_SHARP; break;
                              case 2:
                              case 12: at = ACC_FLAT; break;
                              case 3:
                              case 13: at = ACC_SHARP2; break;
                              case 4:
                              case 14: at = ACC_FLAT2; break;
                              case 5:
                              case 15: at = ACC_NATURAL; break;

                              case 6:  at = ACC_SHARP; bracket = true; break;
                              case 7:  at = ACC_FLAT; bracket = true; break;
                              case 8:  at = ACC_SHARP2; bracket = true; break;
                              case 9:  at = ACC_FLAT2; bracket = true; break;
                              case 10: at = ACC_NATURAL; bracket = true; break;

                              case 16: at = ACC_FLAT_SLASH; break;
                              case 17: at = ACC_FLAT_SLASH2; break;
                              case 18: at = ACC_MIRRORED_FLAT2; break;
                              case 19: at = ACC_MIRRORED_FLAT; break;
                              case 20: at = ACC_MIRRIRED_FLAT_SLASH; break;
                              case 21: at = ACC_FLAT_FLAT_SLASH; break;

                              case 22: at = ACC_SHARP_SLASH; break;
                              case 23: at = ACC_SHARP_SLASH2; break;
                              case 24: at = ACC_SHARP_SLASH3; break;
                              case 25: at = ACC_SHARP_SLASH4; break;

                              case 26: at = ACC_SHARP_ARROW_UP; break;
                              case 27: at = ACC_SHARP_ARROW_DOWN; break;
                              case 28: at = ACC_SHARP_ARROW_BOTH; break;
                              case 29: at = ACC_FLAT_ARROW_UP; break;
                              case 30: at = ACC_FLAT_ARROW_DOWN; break;
                              case 31: at = ACC_FLAT_ARROW_BOTH; break;
                              case 32: at = ACC_NATURAL_ARROW_UP; break;
                              case 33: at = ACC_NATURAL_ARROW_DOWN; break;
                              case 34: at = ACC_NATURAL_ARROW_BOTH; break;
                              }
                        _accidental->setSubtype(at);
                        _accidental->setHasBracket(bracket);
                        _accidental->setRole(ACC_USER);
                        hasAccidental = true;   // we now have an accidental
                        }
                  }
            else if (tag == "Accidental") {
                  // on older scores, a note could have both a <userAccidental> tag and an <Accidental> tag
                  // if a userAccidental has some other property set (like for instance offset)
                  Accidental* a;
                  if (hasAccidental)            // if the other tag has already been read,
                        a = _accidental;        // re-use the accidental it constructed
                  else
                        a = new Accidental(score());
                  // the accidental needs to know the properties of the
                  // track it belongs to (??)
                  a->setTrack(track());
                  a->read(e);
                  if (!hasAccidental)           // only the new accidental, if it has been added previously
                        add(a);
                  if (score()->mscVersion() < 117)
                        hasAccidental = true;   // we now have an accidental
                  }
            else if (tag == "move")             // obsolete
                  chord()->setStaffMove(val.toInt());
            else if (tag == "Bend") {
                  Bend* b = new Bend(score());
                  b->setTrack(track());
                  b->read(e);
                  add(b);
                  }
            else if (tag == "NoteDot") {
                  NoteDot* dot = new NoteDot(score());
                  dot->read(e);
                  for (int i = 0; i < 3; ++i) {
                        if (_dots[i] == 0) {
                              dot->setIdx(i);
                              add(dot);
                              dot = 0;
                              break;
                              }
                        }
                  if (dot) {
                        qDebug("Note: too many dots\n");
                        delete dot;
                        }
                  }
            else if (tag == "Events") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        const QString& tag(ee.tagName());
                        if (tag == "Event") {
                              NoteEvent* ne = new NoteEvent;
                              ne->read(ee);
                              _playEvents.append(ne);
                              }
                        else
                              domError(ee);
                        }
                  }
            else if (tag == "onTimeType")                   // obsolete
                  ; // _onTimeType = readValueType(e);
            else if (tag == "offTimeType")                  // obsolete
                  ; // _offTimeType = readValueType(e);
            else if (tag == "tick")                         // bad input file
                  ;
            else if (Element::readProperties(e))
                  ;
            else
                  domError(e);
            }
      // ensure sane values:
      _pitch = restrict(_pitch, 0, 127);
      if (!tpcIsValid(_tpc))
            setTpcFromPitch();
      _ppitch = _pitch;
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF Note::drag(const EditData& data)
      {
      dragMode = true;
      QRectF bb(chord()->bbox());

      qreal _spatium = spatium();
      bool tab = staff()->isTabStaff();
      qreal step = _spatium * (tab ? staff()->staffType()->lineDistance().val() : 0.5);
      _lineOffset = lrint(data.pos.y() / step);
      score()->setLayout(chord()->measure());
      return bb.translated(chord()->pagePos());
      }

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void Note::endDrag()
      {
      dragMode     = false;
      if (_lineOffset == 0)
            return;
      int nLine;
      int staffIdx = chord()->staffIdx() + chord()->staffMove();
      Staff* staff = score()->staff(staffIdx);
      int nPitch;
      int tpc;
      int nString;
      int nFret;
      if (staff->isTabStaff()) {
            // on TABLATURE staves, dragging a note keeps same pitch on a different string (if possible)
            // determine new string of dragged note (if tablature is upside down, invert _lineOffset)
            nString = _string + (static_cast<StaffTypeTablature*>(staff->staffType())->upsideDown() ?
                        -_lineOffset : _lineOffset);
            _lineOffset = 0;
            // get a fret number for same pitch on new string
            nFret       = staff->part()->instr()->tablature()->fret(_pitch, nString);
            if (nFret < 0)                      // no fret?
                  return;                       // no party!
            score()->undoChangeProperty(this, P_FRET, nFret);
            score()->undoChangeProperty(this, P_STRING, nString);
            }
      else {
            // on PITCHED / PERCUSSION staves, dragging a note changes the note pitch
            nLine       = _line + _lineOffset;
            _lineOffset = 0;
            // get note context
            int tick    = chord()->tick();
            int clef    = staff->clef(tick);
            int key     = staff->key(tick).accidentalType();
            // determine new pitch of dragged note
            nPitch      = line2pitch(nLine, clef, key);
            tpc         = pitch2tpc(nPitch, key);
            // undefined for non-tablature staves
            nString     = -1;
            nFret       = -1;
//            }
      Note* n = this;
      while (n->tieBack())
            n = n->tieBack()->startNote();
      for (; n; n = n->tieFor() ? n->tieFor()->endNote() : 0)
            score()->undoChangePitch(n, nPitch, tpc, nLine/*, nFret, nString*/);
            }
      score()->select(this, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Note::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      int type = e->type();
      return (type == ARTICULATION
         || type == CHORDLINE
         || type == TEXT
         || type == REHEARSAL_MARK
         || type == FINGERING
         || type == ACCIDENTAL
         || type == BREATH
         || type == ARPEGGIO
         || type == NOTEHEAD
         || type == NOTE
         || type == TREMOLO
         || type == STAFF_STATE
         || type == INSTRUMENT_CHANGE
         || type == IMAGE
         || type == CHORD
         || type == HARMONY
         || type == DYNAMIC
         || (noteType() == NOTE_NORMAL && type == ICON && static_cast<Icon*>(e)->subtype() == ICON_ACCIACCATURA)
         || (noteType() == NOTE_NORMAL && type == ICON && static_cast<Icon*>(e)->subtype() == ICON_APPOGGIATURA)
	   || (noteType() == NOTE_NORMAL && type == ICON && static_cast<Icon*>(e)->subtype() == ICON_GRACE4)
	   || (noteType() == NOTE_NORMAL && type == ICON && static_cast<Icon*>(e)->subtype() == ICON_GRACE8B)
	   || (noteType() == NOTE_NORMAL && type == ICON && static_cast<Icon*>(e)->subtype() == ICON_GRACE16)
	   || (noteType() == NOTE_NORMAL && type == ICON && static_cast<Icon*>(e)->subtype() == ICON_GRACE32)
         || (type == ICON && static_cast<Icon*>(e)->subtype() == ICON_SBEAM)
         || (type == ICON && static_cast<Icon*>(e)->subtype() == ICON_MBEAM)
         || (type == ICON && static_cast<Icon*>(e)->subtype() == ICON_NBEAM)
         || (type == ICON && static_cast<Icon*>(e)->subtype() == ICON_BEAM32)
         || (type == ICON && static_cast<Icon*>(e)->subtype() == ICON_BEAM64)
         || (type == ICON && static_cast<Icon*>(e)->subtype() == ICON_AUTOBEAM)
         || (type == SYMBOL)
         || (type == CLEF)
         || (type == BAR_LINE)
         || (type == GLISSANDO)
         || (type == SLUR)
         || (type == STAFF_TEXT)
         || (type == TEMPO_TEXT)
         || (type == BEND && (staff()->isTabStaff()))
         || (type == FRET_DIAGRAM));
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Note::drop(const DropData& data)
      {
      Element* e = data.element;

      Chord* ch = chord();
      switch(e->type()) {
            case REHEARSAL_MARK:
                  return ch->drop(data);

            case SYMBOL:
            case IMAGE:
            case FINGERING:
                  e->setParent(this);
                  score()->undoAddElement(e);
                  return e;

            case SLUR:
                  delete e;
                  data.view->cmdAddSlur(this, 0);
                  return 0;

            case LYRICS:
                  e->setParent(ch->segment());
                  e->setTrack((track() / VOICES) * VOICES);
                  score()->undoAddElement(e);
                  return e;

            case ACCIDENTAL:
                  score()->changeAccidental(this, static_cast<Accidental*>(e)->subtype());
                  if (_accidental)
                        return e;
                  break;

            case ARPEGGIO:
                  {
                  Arpeggio* a = static_cast<Arpeggio*>(e);
                  a->setParent(ch);
                  a->setHeight(spatium() * 5);   //DEBUG
                  score()->undoAddElement(a);
                  }
                  return e;

            case BEND:
                  {
                  Bend* b = static_cast<Bend*>(e);
                  b->setParent(this);
                  b->setTrack(track());
                  score()->undoAddElement(b);
                  }
                  return e;

            case NOTEHEAD:
                  {
                  Symbol* s = (Symbol*)e;
                  NoteHeadGroup group = HEAD_INVALID;

                  for (int i = 0; i < HEAD_GROUPS; ++i) {
                        if (noteHeads[0][i][1] == s->sym() || noteHeads[0][i][3] == s->sym()) {
                              group = (NoteHeadGroup)i;
                              break;
                              }
                        }
                  if (group == HEAD_INVALID) {
                        qDebug("unknown note head\n");
                        group = HEAD_NORMAL;
                        }
                  delete s;

                  if (group != _headGroup) {
                        if (links()) {
                              foreach(Element* e, *links()) {
                                    e->score()->undoChangeProperty(e, P_HEAD_GROUP, group);
                                    Note* note = static_cast<Note*>(e);
                                    if (note->staff() && note->staff()->isTabStaff()
                                       && group == HEAD_CROSS) {
                                          e->score()->undoChangeProperty(e, P_GHOST, true);
                                          }
                                    }
                              }
                        else
                              score()->undoChangeProperty(this, P_HEAD_GROUP, group);
                        score()->select(this);
                        }
                  }
                  break;

            case ICON:
                  {
                  switch(static_cast<Icon*>(e)->subtype()) {
                        case ICON_ACCIACCATURA:
                              score()->setGraceNote(ch, pitch(), NOTE_ACCIACCATURA, false, MScore::division/2);
                              break;
                        case ICON_APPOGGIATURA:
                              score()->setGraceNote(ch, pitch(), NOTE_APPOGGIATURA, false, MScore::division/2);
                              break;
                        case ICON_GRACE4:
                              score()->setGraceNote(ch, pitch(), NOTE_GRACE4, false, MScore::division);
                              break;
                        case ICON_GRACE8B:
                              score()->setGraceNote(ch, pitch(), NOTE_APPOGGIATURA, true, MScore::division/2);
                              break;
                        case ICON_GRACE16:
                              score()->setGraceNote(ch, pitch(), NOTE_GRACE16, false, MScore::division/4);
                              break;
                        case ICON_GRACE32:
                              score()->setGraceNote(ch, pitch(), NOTE_GRACE32, false, MScore::division/8);
                              break;
                        case ICON_SBEAM:
                        case ICON_MBEAM:
                        case ICON_NBEAM:
                        case ICON_BEAM32:
                        case ICON_BEAM64:
                        case ICON_AUTOBEAM:
                              return ch->drop(data);
                              break;
                        }
                  }
                  delete e;
                  break;

            case NOTE:
                  {
                  Chord* ch = chord();
                  if (ch->noteType() != NOTE_NORMAL) {
                        delete e;
                        return 0;
                        }
                  e->setParent(ch);
                  score()->undoRemoveElement(this);
                  score()->undoAddElement(e);
                  }
                  break;

            case GLISSANDO:
                  {
                  Segment* s = ch->segment();
                  s = s->next1();
                  while (s) {
                        if (s->subtype() == Segment::SegChordRest && s->element(track()))
                              break;
                        s = s->next1();
                        }
                  if (s == 0) {
                        qDebug("no segment for second note of glissando found\n");
                        delete e;
                        return 0;
                        }
                  ChordRest* cr1 = static_cast<ChordRest*>(s->element(track()));
                  if (cr1 == 0 || cr1->type() != CHORD) {
                        qDebug("no second note for glissando found, track %d\n", track());
                        delete e;
                        return 0;
                        }
                  e->setTrack(track());
                  e->setParent(cr1);
                  score()->undoAddElement(e);
                  score()->setLayout(cr1->measure());
                  }
                  break;

            case TREMOLO:
                  {
                  Tremolo* tremolo = static_cast<Tremolo*>(e);
                  if (tremolo->twoNotes()) {
                        Segment* s = ch->segment()->next();
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
                        tremolo->setChords(ch, ch2);
                        }
                  e->setParent(ch);
                  e->setTrack(track());
                  if (ch->tremolo())
                        score()->undoRemoveElement(ch->tremolo());
                  score()->undoAddElement(e);
                  }
                  break;

            case CHORD:
                  {
                  Chord* c      = static_cast<Chord*>(e);
                  Note* n       = c->upNote();
                  MScore::Direction dir = c->stemDirection();
                  int t         = (staffIdx() * VOICES) + (n->voice() % VOICES);
                  score()->select(0, SELECT_SINGLE, 0);
                  NoteVal nval;
                  nval.pitch = n->pitch();
                  nval.headGroup = n->headGroup();
                  Segment* seg = score()->setNoteRest(chord()->segment(), t, nval,
                     score()->inputState().duration().fraction(), dir);
                  ChordRest* cr = static_cast<ChordRest*>(seg->element(t));
                  if (cr)
                        score()->nextInputPos(cr, true);
                  delete e;
                  }
                  break;

            default:
                  return ch->drop(data);
            }
      return 0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Note::layout()
      {
      bool useTablature = staff() && staff()->isTabStaff();
      if (useTablature) {
            StaffTypeTablature* tab = (StaffTypeTablature*)staff()->staffType();
            qreal mags = magS();
            QFont f(tab->fretFontName());
            int size = lrint(tab->fretFontSize() * MScore::DPI / PPI);
            f.setPixelSize(size);
            QFontMetricsF fm(f);
            // when using letters, "+(_fret > 8)" skips 'j'
            QString s = _ghost ? "X" :
                        ( tab->useNumbers() ? QString::number(_fret) : QString('a' + _fret + (_fret > 8)) );
            qreal w  = fm.width(s) * mags;
            // center string name to note head
            qreal xo = (headWidth() - w) * .5;
            setbbox(QRectF(xo, tab->fretBoxY() * mags, w, tab->fretBoxH() * mags));
            }
      else {
            setbbox(symbols[score()->symIdx()][noteHead()].bbox(magS()));
            if (parent() == 0)
                  return;
            int dots = chord()->dots();
            for (int i = 0; i < 3; ++i) {
                  if (i < dots) {
                        if (_dots[i] == 0) {
                              NoteDot* dot = new NoteDot(score());
                              dot->setIdx(i);
                              dot->setParent(this);
                              dot->setTrack(track());  // needed to know the staff it belongs to (and detect tablature)
                              score()->undoAddElement(dot); // move dot to _dots[i]
                              }
                        _dots[i]->layout();
                        _dots[i]->setVisible(visible());
                        }
                  else if (_dots[i])
                        score()->undoRemoveElement(_dots[i]);
                  }
            }
// layout2();
      }

//---------------------------------------------------------
//   layout2
//    called after final position of note is set
//---------------------------------------------------------

void Note::layout2()
      {
      adjustReadPos();

      int dots = chord()->dots();
      if (dots) {
            qreal _spatium = spatium();
            qreal d  = point(score()->styleS(ST_dotNoteDistance));
            qreal dd = point(score()->styleS(ST_dotDotDistance));
            qreal y  = 0.0;
            qreal x  = chord()->dotPosX() - pos().x() - chord()->pos().x();

            // do not draw dots on staff line
            if ((_line & 1) == 0) {
                  qreal up;
                  if (_dotPosition == MScore::AUTO)
                        up = (voice() == 0 || voice() == 2) ? -1.0 : 1.0;
                  else if (_dotPosition == MScore::UP)
                        up = -1.0;
                  else
                        up = 1.0;
                  y += .5 * _spatium * up;
                  }
            for (int i = 0; i < dots; ++i) {
                  NoteDot* dot = _dots[i];
                  if (dot) {
                        dot->setPos(x + d + dd * i, y);
                        dot->setMag(mag());
                        _dots[i]->adjustReadPos();
                        }
                  }
            }

      foreach (Element* e, _el) {
            if (!score()->tagIsValid(e->tag()))
                  continue;
            e->setMag(mag());
            e->layout();
            if (e->type() == SYMBOL && static_cast<Symbol*>(e)->sym() == rightparenSym)
                  e->setPos(headWidth(), 0.0);
            }
      }

//---------------------------------------------------------
//   dotIsUp
//---------------------------------------------------------

bool Note::dotIsUp() const
      {
      if (_dots[0] == 0)
            return true;
      return _dots[0]->y() < spatium() * .1;
      }

//---------------------------------------------------------
//   layout10
//    compute actual accidental and line
//---------------------------------------------------------

void Note::layout10(AccidentalState* as)
      {
      if (staff()->isTabStaff()) {
            if (_accidental) {
                  delete _accidental;
                  _accidental = 0;
                  }
            if (_fret < 0) {
                  int string, fret;
                  Tablature* tab = staff()->part()->instr()->tablature();
                  if (tab->convertPitch(_pitch, &string, &fret)) {
                        _fret   = fret;
                        _string = string;
                        }
                  }
            }
      else {
            _line = absStep(_tpc, _pitch);

            // calculate accidental

            AccidentalType acci = ACC_NONE;
            if (_accidental && _accidental->role() == ACC_USER) {
                  acci = _accidental->subtype();
                  if (acci == ACC_SHARP || acci == ACC_FLAT) {
                        int ntpc = pitch2tpc2(_pitch, acci == ACC_SHARP);
                        if (ntpc != _tpc) {
                              qDebug("note has wrong tpc: %d, expected %d", _tpc, ntpc);
                              setColor(QColor(255, 0, 0));
                              _tpc = ntpc;
                              _line = absStep(_tpc, _pitch);
                              }
                        }
                  }
            else  {
                  AccidentalVal accVal = tpc2alter(_tpc);

                  if ((accVal != as->accidentalVal(int(_line))) || hidden() || as->tieContext(int(_line))) {
                        as->setAccidentalVal(int(_line), accVal, _tieBack != 0);
                        if (!_tieBack) {
                              acci = Accidental::value2subtype(accVal);
                              if (acci == ACC_NONE)
                                    acci = ACC_NATURAL;
                              }
                        }
                  }
            if (acci != ACC_NONE && !_tieBack && !_hidden) {
                  if (_accidental == 0) {
                        _accidental = new Accidental(score());
                        _accidental->setGenerated(true);
                        add(_accidental);
                        }
                  _accidental->setSubtype(acci);
                  }
            else {
                  if (_accidental) {
                        if (_accidental->selected()) {
                              score()->deselect(_accidental);
                              }
                        delete _accidental;
                        _accidental = 0;
                        }
                  }
            if (tieBack())
                  _line = tieBack()->startNote()->line();
            else {
                  //
                  // calculate the real note line depending on clef
                  //
                  Staff* s = score()->staff(staffIdx() + chord()->staffMove());
                  int tick = chord()->tick();
                  _line    = relStep(_line, s->clef(tick));
                  }
            }
      }

//---------------------------------------------------------
//   noteType
//---------------------------------------------------------

NoteType Note::noteType() const
      {
      return chord()->noteType();
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF Note::pagePos() const
      {
      if (parent() == 0)
            return pos();

      return parent()->pagePos() + pos();
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Note::canvasPos() const
      {
      if (parent() == 0)
            return pos();
      return parent()->canvasPos() + pos();
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Note::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      func(data, this);
      // tie segments are collected from System
      //      if (_tieFor && !staff()->isTabStaff())  // no ties in tablature
      //            _tieFor->scanElements(data, func, all);
      foreach(Element* e, _el) {
            if (score()->tagIsValid(e->tag()))
                  e->scanElements(data, func, all);
            }
      if (!dragMode && _accidental)
            func(data, _accidental);
      if (chord()) {
            for (int i = 0; i < chord()->dots(); ++i) {
                  if (_dots[i])
                        func(data, _dots[i]);
                  }
            }
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Note::setTrack(int val)
      {
      Element::setTrack(val);
      if (_tieFor) {
            _tieFor->setTrack(val);
            foreach(SpannerSegment* seg, _tieFor->spannerSegments())
                  seg->setTrack(val);
            }
      foreach(Element* e, _el)
            e->setTrack(val);
      if (_accidental)
            _accidental->setTrack(val);
      if (!chord())     // if note is dragged with shift+ctrl
            return;
      for (int i = 0; i < chord()->dots(); ++i) {
            if (_dots[i])
                  _dots[i]->setTrack(val);
            }
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Note::toDefault()
      {
      score()->undoChangeUserOffset(this, QPointF());
      score()->undoChangeUserOffset(chord(), QPointF());
      score()->undoChangeProperty(chord(), P_STEM_DIRECTION, MScore::AUTO);
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void Note::setMag(qreal val)
      {
      Element::setMag(val);
      if (_accidental)
            _accidental->setMag(val);
      foreach(Element* e, _el)
            e->setMag(val);
      }

//---------------------------------------------------------
//   setLine
//---------------------------------------------------------

void Note::setLine(int n)
      {
      _line = n;
      rypos() = _line * spatium() * .5;
      }

//---------------------------------------------------------
//   setString
//---------------------------------------------------------

void Note::setString(int val)
      {
      _string = val;
      rypos() = _string * spatium() * 1.5;
      }

//---------------------------------------------------------
//   setHeadGroup
//---------------------------------------------------------

void Note::setHeadGroup(NoteHeadGroup val)
      {
      Q_ASSERT(val >= 0 && val < HEAD_GROUPS);
      _headGroup = val;
      }

//---------------------------------------------------------
//   ppitch
//    playback pitch
//    honours ottava and transposing instruments
//---------------------------------------------------------

int Note::ppitch() const
      {
      int tick        = chord()->segment()->tick();
      int pitchOffset = score()->styleB(ST_concertPitch) ? 0 : staff()->part()->instr()->transpose().chromatic;
      return _pitch + staff()->pitchOffsets().pitchOffset(tick) + pitchOffset;
      }

//---------------------------------------------------------
//   customizeVelocity
//    Input is the global velocity determined by dynamic
//    signs and crescende/decrescendo etc.
//    Returns the actual play velocity for this note
//    modified by veloOffset
//---------------------------------------------------------

int Note::customizeVelocity(int velo) const
      {
      if (veloType() == MScore::OFFSET_VAL)
            velo = velo + (velo * veloOffset()) / 100;
      else if (veloType() == MScore::USER_VAL)
            velo = veloOffset();
      return restrict(velo, 1, 127);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Note::endEdit()
      {
      Chord* ch = chord();
      if (ch->notes().size() == 1) {
            score()->undoChangeUserOffset(ch, ch->userOff() + userOff());
            setUserOff(QPointF());
            score()->setLayoutAll(true);
            }
      }

//---------------------------------------------------------
//   updateAccidental
//---------------------------------------------------------

void Note::updateAccidental(AccidentalState* as)
      {
      _line = absStep(_tpc, _pitch);

      // calculate accidental

      AccidentalType acci = ACC_NONE;
      if (_accidental && _accidental->role() == ACC_USER)
            acci = _accidental->subtype();
      else  {
            AccidentalVal accVal = tpc2alter(_tpc);
            if ((accVal != as->accidentalVal(int(_line))) || hidden() || as->tieContext(int(_line))) {
                  as->setAccidentalVal(int(_line), accVal, _tieBack != 0);
                  if (_tieBack)
                        acci = ACC_NONE;
                  else {
                        acci = Accidental::value2subtype(accVal);
                        if (acci == ACC_NONE)
                              acci = ACC_NATURAL;
                        }
                  }
            }
      if (acci != ACC_NONE && !_tieBack && !_hidden) {
            if (_accidental == 0) {
                  Accidental* a = new Accidental(score());
                  a->setParent(this);
                  a->setSubtype(acci);
                  score()->undoAddElement(a);
                  }
            else if (_accidental->subtype() != acci) {
                  Accidental* a = new Accidental(score());
                  a->setParent(this);
                  a->setSubtype(acci);
                  score()->undoChangeElement(_accidental, a);
                  }
            }
      else {
            if (_accidental)
                  score()->undoRemoveElement(_accidental);
            }
      //
      // calculate the real note line depending on clef
      //
      Staff* s = score()->staff(staffIdx() + chord()->staffMove());
      int tick = chord()->tick();
      ClefType clef = s->clef(tick);
      _line    = relStep(_line, clef);
      }

//---------------------------------------------------------
//   updateLine
//---------------------------------------------------------

void Note::updateLine()
      {
      Staff* s      = score()->staff(staffIdx() + chord()->staffMove());
      ClefType clef = s->clef(chord()->tick());
      _line         = relStep(_pitch, _tpc, clef);
      }

//---------------------------------------------------------
//   setNval
//---------------------------------------------------------

void Note::setNval(NoteVal nval)
      {
      setPitch(nval.pitch);
      _fret      = nval.fret;
      _string    = nval.string;
      if (nval.tpc != INVALID_TPC)
            _tpc = nval.tpc;

      _headGroup = NoteHeadGroup(nval.headGroup);
      }

//---------------------------------------------------------
//   setPlayEvents
//---------------------------------------------------------

void Note::setPlayEvents(const QList<NoteEvent*>& v)
      {
      foreach(NoteEvent* e, _playEvents)
            delete e;
      _playEvents.clear();
      foreach(NoteEvent* e, v)
            _playEvents.append(new NoteEvent(*e));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Note::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_PITCH:
                  return pitch();
            case P_TPC:
                  return tpc();
            case P_SMALL:
                  return small();
            case P_MIRROR_HEAD:
                  return userMirror();
            case P_DOT_POSITION:
                  return dotPosition();
            case P_ONTIME_OFFSET:
                  return onTimeUserOffset();
            case P_OFFTIME_OFFSET:
                  return offTimeUserOffset();
            case P_HEAD_GROUP:
                  return headGroup();
            case P_VELO_OFFSET:
                  return veloOffset();
            case P_TUNING:
                  return tuning();
            case P_FRET:
                  return fret();
            case P_STRING:
                  return string();
            case P_GHOST:
                  return ghost();
            case P_HEAD_TYPE:
                  return headType();
            case P_VELO_TYPE:
                  return veloType();
            default:
                  break;
            }
      return Element::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Note::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_PITCH:
                  setPitch(v.toInt());
                  break;
            case P_TPC:
                  setTpc(v.toInt());
                  break;
            case P_SMALL:
                  setSmall(v.toBool());
                  break;
            case P_MIRROR_HEAD:
                  setUserMirror(MScore::DirectionH(v.toInt()));
                  break;
            case P_DOT_POSITION:
                  setDotPosition(MScore::Direction(v.toInt()));
                  break;
            case P_ONTIME_OFFSET:
                  setOnTimeUserOffset(v.toInt());
                  break;
            case P_OFFTIME_OFFSET:
                  setOffTimeUserOffset(v.toInt());
                  break;
            case P_HEAD_GROUP:
                  setHeadGroup(NoteHeadGroup(v.toInt()));
                  break;
            case P_VELO_OFFSET:
                  setVeloOffset(v.toInt());
                  break;
            case P_TUNING:
                  setTuning(v.toDouble());
                  break;
            case P_FRET:
                  setFret(v.toInt());
                  break;
            case P_STRING:
                  setString(v.toInt());
                  break;
            case P_GHOST:
                  setGhost(v.toBool());
                  break;
            case P_HEAD_TYPE:
                  setHeadType(NoteHeadType(v.toInt()));
                  break;
            case P_VELO_TYPE:
                  setVeloType(MScore::ValueType(v.toInt()));
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   undoSetFret
//---------------------------------------------------------

void Note::undoSetFret(int val)
      {
      undoChangeProperty(P_FRET, val);
      }

//---------------------------------------------------------
//   undoSetString
//---------------------------------------------------------

void Note::undoSetString(int val)
      {
      undoChangeProperty(P_STRING, val);
      }

//---------------------------------------------------------
//   undoSetGhost
//---------------------------------------------------------

void Note::undoSetGhost(bool val)
      {
      undoChangeProperty(P_GHOST, val);
      }

//---------------------------------------------------------
//   undoSetSmall
//---------------------------------------------------------

void Note::undoSetSmall(bool val)
      {
      undoChangeProperty(P_SMALL, val);
      }

//---------------------------------------------------------
//   undoSetTuning
//---------------------------------------------------------

void Note::undoSetTuning(qreal val)
      {
      undoChangeProperty(P_TUNING, val);
      }

//---------------------------------------------------------
//   undoSetVeloType
//---------------------------------------------------------

void Note::undoSetVeloType(MScore::ValueType val)
      {
      undoChangeProperty(P_VELO_TYPE, val);
      }

//---------------------------------------------------------
//   undoSetVeloOffset
//---------------------------------------------------------

void Note::undoSetVeloOffset(int val)
      {
      undoChangeProperty(P_VELO_OFFSET, val);
      }

//---------------------------------------------------------
//   undoSetOnTimeUserOffset
//---------------------------------------------------------

void Note::undoSetOnTimeUserOffset(int val)
      {
      undoChangeProperty(P_ONTIME_OFFSET, val);
      }

//---------------------------------------------------------
//   undoSetOffTimeUserOffset
//---------------------------------------------------------

void Note::undoSetOffTimeUserOffset(int val)
      {
      undoChangeProperty(P_OFFTIME_OFFSET, val);
      }

//---------------------------------------------------------
//   undoSetUserMirror
//---------------------------------------------------------

void Note::undoSetUserMirror(MScore::DirectionH val)
      {
      undoChangeProperty(P_MIRROR_HEAD, val);
      }

//---------------------------------------------------------
//   undoSetDotPosition
//---------------------------------------------------------

void Note::undoSetDotPosition(MScore::Direction val)
      {
      undoChangeProperty(P_DOT_POSITION, val);
      }

//---------------------------------------------------------
//   undoSetHeadGroup
//---------------------------------------------------------

void Note::undoSetHeadGroup(NoteHeadGroup val)
      {
      undoChangeProperty(P_HEAD_GROUP, val);
      }

//---------------------------------------------------------
//   undoSetHeadType
//---------------------------------------------------------

void Note::undoSetHeadType(NoteHeadType val)
      {
      undoChangeProperty(P_HEAD_TYPE, val);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Note::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_SMALL:
                  return false;
            case P_MIRROR_HEAD:
                  return MScore::DH_AUTO;
            case P_DOT_POSITION:
                  return MScore::AUTO;
            case P_ONTIME_OFFSET:
            case P_OFFTIME_OFFSET:
            case P_HEAD_GROUP:
            case P_VELO_OFFSET:
                  return 0;
            case P_TUNING:
                  return 0.0;
            case P_FRET:
            case P_STRING:
                  return -1;
            case P_GHOST:
                  return false;
            case P_HEAD_TYPE:
                  return Note::HEAD_AUTO;
            case P_VELO_TYPE:
                  return MScore::OFFSET_VAL;
            default:
                  break;
            }
      return QVariant();
      }

