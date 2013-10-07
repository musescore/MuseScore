//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
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
#include "tie.h"
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
#include "mscore.h"
#include "accidental.h"
#include "page.h"
#include "icon.h"
#include "notedot.h"
#include "spanner.h"
#include "glissando.h"
#include "bagpembell.h"

namespace Ms {

//---------------------------------------------------------
//   noteHeads
//    note head groups
//---------------------------------------------------------

const SymId noteHeads[2][Note::HEAD_GROUPS][HEAD_TYPES] = {
      {     // down stem
      { wholeheadSym,         halfheadSym,         quartheadSym,      brevisheadSym        },
      { wholecrossedheadSym,  halfcrossedheadSym,  crossedheadSym,    wholecrossedheadSym  },
      { wholediamondheadSym,  halfdiamondheadSym,  diamondheadSym,    wholediamondheadSym  },
      { s0triangleHeadSym,    d1triangleHeadSym,   d2triangleHeadSym, s0triangleHeadSym    },
      { s0miHeadSym,          s1miHeadSym,         s2miHeadSym,       noSym                },
      { wholeslashheadSym,    halfslashheadSym,    quartslashheadSym, wholeslashheadSym    },
      { xcircledheadSym,      xcircledheadSym,     xcircledheadSym,   xcircledheadSym      },
      { s0doHeadSym,          d1doHeadSym,         d2doHeadSym,       noSym                },
      { s0reHeadSym,          d1reHeadSym,         d2reHeadSym,       noSym                },
      { d0faHeadSym,          d1faHeadSym,         d2faHeadSym,       noSym                },
      { s0laHeadSym,          s1laHeadSym,         s2laHeadSym,       noSym                },
      { s0tiHeadSym,          d1tiHeadSym,         d2tiHeadSym,       noSym                },
      { s0solHeadSym,         s1solHeadSym,        s2solHeadSym,      noSym                },
      { wholeheadSym,         halfheadSym,         quartheadSym,      brevisheadaltSym     },
      },
      {     // up stem
      { wholeheadSym,         halfheadSym,         quartheadSym,      brevisheadSym        },
      { wholecrossedheadSym,  halfcrossedheadSym,  crossedheadSym,    wholecrossedheadSym  },
      { wholediamondheadSym,  halfdiamondheadSym,  diamondheadSym,    wholediamondheadSym  },
      { s0triangleHeadSym,    u1triangleHeadSym,   u2triangleHeadSym, s0triangleHeadSym    },
      { s0miHeadSym,          s1miHeadSym,         s2miHeadSym,       noSym                },
      { wholeslashheadSym,    halfslashheadSym,    quartslashheadSym, wholeslashheadSym    },
      { xcircledheadSym,      xcircledheadSym,     xcircledheadSym,   xcircledheadSym      },
      { s0doHeadSym,          u1doHeadSym,         u2doHeadSym,       noSym                },
      { s0reHeadSym,          u1reHeadSym,         u2reHeadSym,       noSym                },
      { u0faHeadSym,          u1faHeadSym,         u2faHeadSym,       noSym                },
      { s0laHeadSym,          s1laHeadSym,         s2laHeadSym,       noSym                },
      { s0tiHeadSym,          u1tiHeadSym,         u2tiHeadSym,       noSym                },
      { s0solHeadSym,         s1solHeadSym,        s2solHeadSym,      noSym                },
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
      fret      = FRET_NONE;
      string    = STRING_NONE;
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
      xml.tag("name", Sym::id2name(_sym));
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
      _play              = true;
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

      _dots[0]           = 0;
      _dots[1]           = 0;
      _dots[2]           = 0;
      _playEvents.append(NoteEvent());    // add default play event
      _mark             = 0;
      }

Note::~Note()
      {
      delete _accidental;
      qDeleteAll(_el);
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
      _hidden            = n._hidden;
      _play              = n._play;
      _tuning            = n._tuning;
      _veloType          = n._veloType;
      _veloOffset        = n._veloOffset;
      _headGroup         = n._headGroup;
      _headType          = n._headType;
      _mirror            = n._mirror;
      _userMirror        = n._userMirror;
      _small             = n._small;
      _dotPosition       = n._dotPosition;
      _accidental        = 0;

      if (n._accidental)
            add(new Accidental(*(n._accidental)));

      for (const Element* e : n._el)
            add(e->clone());

      _playEvents = n._playEvents;

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
      _mark      = n._mark;
      }

//---------------------------------------------------------
//   setPitch
//---------------------------------------------------------

void Note::setPitch(int val)
      {
      _pitch = restrict(val, 0, 127);
      int pitchOffset = 0;
      if (score()) {
            Part* part = staff() ? staff()->part() : 0;
            if (part)
                  pitchOffset = score()->styleB(ST_concertPitch) ? 0 : part->instr()->transpose().chromatic;
            }
      for (int i = 0; i < _playEvents.size(); ++i)
            _playEvents[i].setPitch(pitchOffset);
      if (chord())
            chord()->lineChanged();
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
      _tpc    = pitch2tpc(_pitch, key.accidentalType(), PREFER_NEAREST);
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
      int hg, ht;
      if (chord()) {
            hg = chord()->up();
            ht = chord()->durationType().headType();
            }
      else {
            hg = 1;
            ht = HEAD_QUARTER;
            }
      if (_headType != HEAD_AUTO)
            ht = _headType;

      int t = noteHeads[hg][int(_headGroup)][ht];
      if (t == -1) {
            qDebug("invalid note head %d/%d", _headGroup, _headType);
            t = noteHeads[hg][0][ht];
            }
      return t;
      }

//---------------------------------------------------------
//   headWidth
//
//    returns the width of the note head symbol
//    or the width of the string representation of the fret mark
//---------------------------------------------------------

qreal Note::headWidth() const
      {
      int head  = noteHead();
      qreal val = symbols[score()->symIdx()][head].width(magS());
      return val;
      }

qreal Note::tabHeadWidth(StaffTypeTablature* tab) const
      {
      qreal val;
      if (tab && _fret != FRET_NONE && _string != STRING_NONE) {
            qreal mags = magS();
            QFont f = tab->fretFont();
            int size = lrint(tab->fretFontSize() * MScore::DPI / PPI);
            f.setPixelSize(size);
            QFontMetricsF fm(f);
            QString s = tab->fretString(_fret, _ghost);
            val  = fm.width(s) * mags;
            }
      else
            val = headWidth();
      return val;
      }

//---------------------------------------------------------
//   headHeight
//
//    returns the height of the note head symbol
//    or the height of the string representation of the fret mark
//---------------------------------------------------------

qreal Note::headHeight() const
      {
      return symbols[score()->symIdx()][noteHead()].height(magS());
      }

qreal Note::tabHeadHeight(StaffTypeTablature *tab) const
      {
      if(tab && _fret != FRET_NONE && _string != STRING_NONE)
            return tab->fretBoxH() * magS();
      return headHeight();
      }

//---------------------------------------------------------
//   attach
//---------------------------------------------------------

QPointF Note::attach() const
      {
      return symbols[score()->symIdx()][noteHead()].attach(magS());
      }

//---------------------------------------------------------
//   playTicks
///   Return total tick len of tied notes
//---------------------------------------------------------

int Note::playTicks() const
      {
      const Note* note = this;
      while (note->tieBack())
            note = note->tieBack()->startNote();
      int stick = note->chord()->tick();
      while (note->tieFor() && note->tieFor()->endNote())
            note = note->tieFor()->endNote();
      return note->chord()->tick() + note->chord()->actualTicks() - stick;
      }

//---------------------------------------------------------
//   addSpanner
//---------------------------------------------------------

void Note::addSpanner(Spanner* l)
      {
      Element* e = l->endElement();
      if (e)
            static_cast<Note*>(e)->addSpannerBack(l);
      addSpannerFor(l);
      }

//---------------------------------------------------------
//   removeSpanner
//---------------------------------------------------------

void Note::removeSpanner(Spanner* l)
      {
      if (!static_cast<Note*>(l->endElement())->removeSpannerBack(l)) {
            qDebug("Note::removeSpanner(%p): cannot remove spannerBack %s %p", this, l->name(), l);
            // abort();
            }
      if (!removeSpannerFor(l)) {
            qDebug("Note(%p): cannot remove spannerFor %s %p", this, l->name(), l);
            // abort();
            }
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
                  _el.push_back(e);
                  break;
            case TIE:
                  {
                  Tie* tie = static_cast<Tie*>(e);
	      	tie->setStartNote(this);
                  tie->setTrack(track());
      		setTieFor(tie);
                  if (tie->endNote())
                        tie->endNote()->setTieBack(tie);
                  int n = tie->spannerSegments().size();
                  for (int i = 0; i < n; ++i) {
                        SpannerSegment* ss = tie->spannerSegments().at(i);
                        if (ss->system())
                              ss->system()->add(ss);
                        }
                  }
                  break;
            case ACCIDENTAL:
                  _accidental = static_cast<Accidental*>(e);
                  break;
            case TEXTLINE:
                  addSpanner(static_cast<Spanner*>(e));
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
                  int n = tie->spannerSegments().size();
                  for (int i = 0; i < n; ++i) {
                        SpannerSegment* ss = tie->spannerSegments().at(i);
                        Q_ASSERT(ss->spanner() == tie);
                        if (ss->system())
                              ss->system()->remove(ss);
                        }
                  }
                  break;

            case ACCIDENTAL:
                  _accidental = 0;
                  break;

            case TEXTLINE:
                  removeSpanner(static_cast<Spanner*>(e));
                  break;

            default:
                  qDebug("Note::remove() not impl. %s\n", e->name());
                  break;
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Note::draw(QPainter* painter) const
      {
      if (_hidden)
            return;

      QColor c(curColor());
      painter->setPen(c);
      bool tablature = staff() && staff()->isTabStaff();

      // tablature

      if (tablature) {
            StaffTypeTablature* tab = (StaffTypeTablature*)staff()->staffType();
            if (tieBack() && tab->slashStyle())
                  return;
            QString s = tab->fretString(_fret, _ghost);

            // draw background, if required
            if (!tab->linesThrough() || fretConflict()) {
                  qreal d  = spatium() * .1;
                  QRectF bb = QRectF(bbox().x()-d, tab->fretMaskY(), bbox().width() + 2*d, tab->fretMaskH());
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
            qreal mag = magS();
            qreal imag = 1.0 / mag;
            painter->scale(mag, mag);
            painter->setFont(tab->fretFont());
            painter->setPen(c);
            painter->drawText(QPointF(bbox().x(), tab->fretFontYOffset()), s);
            painter->scale(imag, imag);
            }

      // NOT tablature

      else {
            // skip drawing, if second note of a cross-measure value
            if (chord()->crossMeasure() == CROSSMEASURE_SECOND)
                  return;
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
            symbols[score()->symIdx()][noteHead()].draw(painter, magS());
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
      int dots = chord() ? chord()->dots() : 0;
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
      if (_tieFor) {
            _tieFor->setId(++xml.spannerId);
            _tieFor->write(xml);
            }
      if (_tieBack)
            xml.tagE(QString("endSpanner id=\"%1\"").arg(_tieBack->id()));
      if (chord() == 0 || chord()->userPlayEvents()) {
            if (!_playEvents.isEmpty()) {
                  xml.stag("Events");
                  foreach(const NoteEvent& e, _playEvents)
                        e.write(xml);
                  xml.etag();
                  }
            }
      writeProperty(xml, P_PITCH);
      writeProperty(xml, P_TPC);
      writeProperty(xml, P_SMALL);
      writeProperty(xml, P_MIRROR_HEAD);
      writeProperty(xml, P_DOT_POSITION);
      writeProperty(xml, P_HEAD_GROUP);
      writeProperty(xml, P_VELO_OFFSET);
      writeProperty(xml, P_PLAY);
      writeProperty(xml, P_TUNING);
      writeProperty(xml, P_FRET);
      writeProperty(xml, P_STRING);
      writeProperty(xml, P_GHOST);
      writeProperty(xml, P_HEAD_TYPE);
      writeProperty(xml, P_VELO_TYPE);

      foreach (Spanner* e, _spannerFor) {
            e->setId(++xml.spannerId);
            e->write(xml);
            }
      foreach (Spanner* e, _spannerBack)
            xml.tagE(QString("endSpanner id=\"%1\"").arg(e->id()));

      _pitch = rpitch;
      _tpc   = rtpc;
      xml.etag();
      }

//---------------------------------------------------------
//   Note::read
//---------------------------------------------------------

void Note::read(XmlReader& e)
      {
      bool hasAccidental = false;                     // used for userAccidental backward compatibility

      _tpc = INVALID_TPC;

      if (e.hasAttribute("pitch"))                   // obsolete
            _pitch = e.intAttribute("pitch");
      if (e.hasAttribute("tpc"))                     // obsolete
            _tpc = e.intAttribute("tpc");

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "pitch")
                  _pitch = e.readInt();
            else if (tag == "tpc")
                  _tpc = e.readInt();
            else if (tag == "small")
                  setSmall(e.readInt());
            else if (tag == "mirror")
                  setProperty(P_MIRROR_HEAD, Ms::getProperty(P_MIRROR_HEAD, e));
            else if (tag == "dotPosition")
                  setProperty(P_DOT_POSITION, Ms::getProperty(P_DOT_POSITION, e));
            else if (tag == "onTimeOffset")
                  e.skipCurrentElement(); // TODO setOnTimeUserOffset(val.toInt());
            else if (tag == "offTimeOffset")
                  e.skipCurrentElement(); // TODO setOffTimeUserOffset(val.toInt());
            else if (tag == "head")
                  setProperty(P_HEAD_GROUP, Ms::getProperty(P_HEAD_GROUP, e));
            else if (tag == "velocity")
                  setVeloOffset(e.readInt());
            else if (tag == "play")
                  setPlay(e.readInt());
            else if (tag == "tuning")
                  setTuning(e.readDouble());
            else if (tag == "fret")
                  setFret(e.readInt());
            else if (tag == "string")
                  setString(e.readInt());
            else if (tag == "ghost")
                  setGhost(e.readInt());
            else if (tag == "headType")
                  if (score()->mscVersion() <= 114)
                        setProperty(P_HEAD_TYPE, Ms::getProperty(P_HEAD_TYPE, e).toInt() - 1);
                  else
                        setProperty(P_HEAD_TYPE, Ms::getProperty(P_HEAD_TYPE, e).toInt());
            else if (tag == "veloType")
                  setProperty(P_VELO_TYPE, Ms::getProperty(P_VELO_TYPE, e));
            else if (tag == "line")
                  _line = e.readInt();
            else if (tag == "Tie") {
                  _tieFor = new Tie(score());
                  _tieFor->setTrack(track());
                  _tieFor->read(e);
                  _tieFor->setStartNote(this);
                  score()->addSpanner(_tieFor);
                  }
            else if (tag == "Fingering" || tag == "Text") {       // Text is obsolete
                  Fingering* f = new Fingering(score());
                  f->setTextStyleType(TEXT_STYLE_FINGERING);
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
                  Image* image = new Image(score());
                  image->setTrack(track());
                  image->read(e);
                  add(image);
                  }
            else if (tag == "userAccidental") {
                  QString val(e.readElementText());
                  bool ok;
                  int k = val.toInt(&ok);
                  if (ok) {
                        // on older scores, a note could have both a <userAccidental> tag and an <Accidental> tag
                        // if a userAccidental has some other property set (like for instance offset)
                        // only construct a new accidental, if the other tag has not been read yet
                        // (<userAccidental> tag is only used in older scores: no need to check the score mscVersion)
                        if (!hasAccidental) {
                              _accidental = new Accidental(score());
                              _accidental->setParent(this);
                              }
                        // TODO: for backward compatibility
                        bool bracket = k & 0x8000;
                        k &= 0xfff;
                        Accidental::AccidentalType at = Accidental::ACC_NONE;
                        switch(k) {
                              case 0: at = Accidental::ACC_NONE; break;
                              case 1: at = Accidental::ACC_SHARP; break;
                              case 2: at = Accidental::ACC_FLAT; break;
                              case 3: at = Accidental::ACC_SHARP2; break;
                              case 4: at = Accidental::ACC_FLAT2; break;
                              case 5: at = Accidental::ACC_NATURAL; break;

                              case 6: at = Accidental::ACC_FLAT_SLASH; break;
                              case 7: at = Accidental::ACC_FLAT_SLASH2; break;
                              case 8: at = Accidental::ACC_MIRRORED_FLAT2; break;
                              case 9: at = Accidental::ACC_MIRRORED_FLAT; break;
                              case 10: at = Accidental::ACC_MIRRIRED_FLAT_SLASH; break;
                              case 11: at = Accidental::ACC_FLAT_FLAT_SLASH; break;

                              case 12: at = Accidental::ACC_SHARP_SLASH; break;
                              case 13: at = Accidental::ACC_SHARP_SLASH2; break;
                              case 14: at = Accidental::ACC_SHARP_SLASH3; break;
                              case 15: at = Accidental::ACC_SHARP_SLASH4; break;

                              case 16: at = Accidental::ACC_SHARP_ARROW_UP; break;
                              case 17: at = Accidental::ACC_SHARP_ARROW_DOWN; break;
                              case 18: at = Accidental::ACC_SHARP_ARROW_BOTH; break;
                              case 19: at = Accidental::ACC_FLAT_ARROW_UP; break;
                              case 20: at = Accidental::ACC_FLAT_ARROW_DOWN; break;
                              case 21: at = Accidental::ACC_FLAT_ARROW_BOTH; break;
                              case 22: at = Accidental::ACC_NATURAL_ARROW_UP; break;
                              case 23: at = Accidental::ACC_NATURAL_ARROW_DOWN; break;
                              case 24: at = Accidental::ACC_NATURAL_ARROW_BOTH; break;
                              case 25: at = Accidental::ACC_SORI; break;
                              case 26: at = Accidental::ACC_KORON; break;
                              }
                        _accidental->setAccidentalType(at);
                        _accidental->setHasBracket(bracket);
                        _accidental->setRole(Accidental::ACC_USER);
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
                  chord()->setStaffMove(e.readInt());
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
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "Event") {
                              NoteEvent ne;
                              ne.read(e);
                              _playEvents.append(ne);
                              }
                        else
                              e.unknown();
                        }
                  if (chord())
                        chord()->setUserPlayEvents(true);
                  }
            else if (tag == "endSpanner") {
                  int id = e.intAttribute("id");
                  Spanner* sp = score()->findSpanner(id);
                  if (sp) {
                        sp->setEndElement(this);
                        if (sp->type() == TIE)
                              _tieBack = static_cast<Tie*>(sp);
                        else
                              addSpannerBack(sp);
                        score()->removeSpanner(sp);
                        }
                  else
                        qDebug("Note::read(): cannot find spanner %d", id);
                  e.readNext();
                  }
            else if (tag == "TextLine") {
                  Spanner* sp = static_cast<Spanner*>(Element::name2Element(tag, score()));
                  sp->setTrack(track());
                  sp->read(e);
                  sp->setAnchor(Spanner::ANCHOR_NOTE);
                  sp->setStartElement(this);
                  sp->setTick(e.tick());
                  addSpannerFor(sp);
                  sp->setParent(this);
                  score()->addSpanner(sp);
                  }
            else if (tag == "onTimeType")                   // obsolete
                  e.skipCurrentElement(); // _onTimeType = readValueType(e);
            else if (tag == "offTimeType")                  // obsolete
                  e.skipCurrentElement(); // _offTimeType = readValueType(e);
            else if (tag == "tick")                         // bad input file
                  e.skipCurrentElement();
            else if (Element::readProperties(e))
                  ;
            else
                  e.unknown();
            }
      // ensure sane values:
      _pitch = restrict(_pitch, 0, 127);
      if (!tpcIsValid(_tpc))
            setTpcFromPitch();
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF Note::drag(const EditData& data)
      {
      if (staff()->isDrumStaff())
            return QRect();
      dragMode = true;
      QRectF bb(chord()->bbox());

      qreal _spatium = spatium();
      bool tab = staff()->isTabStaff();
      qreal step = _spatium * (tab ? staff()->staffType()->lineDistance().val() : 0.5);
      _lineOffset = lrint(data.pos.y() / step);
      score()->setLayoutAll(true);
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
            nFret       = staff->part()->instr()->stringData()->fret(_pitch, nString);
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
            ClefType clef    = staff->clef(tick);
            int key     = staff->key(tick).accidentalType();
            // determine new pitch of dragged note
            nPitch      = line2pitch(nLine, clef, key);
            tpc         = pitch2tpc(nPitch, key, PREFER_NEAREST);
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
         || (noteType() == NOTE_NORMAL && type == ICON && static_cast<Icon*>(e)->iconType() == ICON_ACCIACCATURA)
         || (noteType() == NOTE_NORMAL && type == ICON && static_cast<Icon*>(e)->iconType() == ICON_APPOGGIATURA)
	   || (noteType() == NOTE_NORMAL && type == ICON && static_cast<Icon*>(e)->iconType() == ICON_GRACE4)
	   || (noteType() == NOTE_NORMAL && type == ICON && static_cast<Icon*>(e)->iconType() == ICON_GRACE8B)
	   || (noteType() == NOTE_NORMAL && type == ICON && static_cast<Icon*>(e)->iconType() == ICON_GRACE16)
	   || (noteType() == NOTE_NORMAL && type == ICON && static_cast<Icon*>(e)->iconType() == ICON_GRACE32)
         || (noteType() == NOTE_NORMAL && type == BAGPIPE_EMBELLISHMENT)
         || (type == ICON && static_cast<Icon*>(e)->iconType() == ICON_SBEAM)
         || (type == ICON && static_cast<Icon*>(e)->iconType() == ICON_MBEAM)
         || (type == ICON && static_cast<Icon*>(e)->iconType() == ICON_NBEAM)
         || (type == ICON && static_cast<Icon*>(e)->iconType() == ICON_BEAM32)
         || (type == ICON && static_cast<Icon*>(e)->iconType() == ICON_BEAM64)
         || (type == ICON && static_cast<Icon*>(e)->iconType() == ICON_AUTOBEAM)
         || (type == ICON && static_cast<Icon*>(e)->iconType() == ICON_BRACKETS)
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
                  e->setParent(this);
                  score()->undoAddElement(e);
                  return e;

            case FINGERING:
                  e->setParent(this);
                  score()->undoAddElement(e);
                  {
                  // set style
                  Fingering* f = static_cast<Fingering*>(e);
                  int st = f->textStyleType();
                  if (st != TEXT_STYLE_UNKNOWN)
                        f->setTextStyleType(st);
                  }
                  return e;

            case SLUR:
                  delete e;
                  data.view->cmdAddSlur(this, 0);
                  return 0;

            case LYRICS:
                  e->setParent(ch);
                  e->setTrack(track());
                  score()->undoAddElement(e);
                  return e;

            case ACCIDENTAL:
                  score()->changeAccidental(this, static_cast<Accidental*>(e)->accidentalType());
                  break;

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
                  switch(static_cast<Icon*>(e)->iconType()) {
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
                        case ICON_BRACKETS:
                              {
                              Symbol* s = new Symbol(score(), leftparenSym);
                              s->setParent(this);
                              score()->undoAddElement(s);
                              s = new Symbol(score(), rightparenSym);
                              s->setParent(this);
                              score()->undoAddElement(s);
                              }
                              break;
                        }
                  }
                  delete e;
                  break;

            case BAGPIPE_EMBELLISHMENT:
                  {
                  BagpipeEmbellishment* b = static_cast<BagpipeEmbellishment*>(e);
                  noteList nl = b->getNoteList();
                  // add grace notes in reverse order, as setGraceNote adds a grace note
                  // before the current note
                  for (int i = nl.size() - 1; i >= 0; --i) {
                        int p = BagpipeEmbellishment::BagpipeNoteInfoList[nl.at(i)].pitch;
                        score()->setGraceNote(ch, p, NOTE_GRACE32, false, MScore::division/8);
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
                        if ((s->segmentType() == Segment::SegChordRest) && s->element(track()))
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
                  // in TAB, use straight line with no text
                  if (staff()->isTabStaff()) {
                        (static_cast<Glissando*>(e))->setGlissandoType(GlissandoType::STRAIGHT);
                        (static_cast<Glissando*>(e))->setShowText(false);
                        }
                  score()->undoAddElement(e);
                  }
                  break;

            case CHORD:
                  {
                  Chord* c      = static_cast<Chord*>(e);
                  Note* n       = c->upNote();
                  MScore::Direction dir = c->stemDirection();
                  int t         = (staff2track(staffIdx()) + n->voice());
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
            qreal w = tabHeadWidth(tab);
            bbox().setRect(0.0, tab->fretBoxY() * mags, w, tab->fretBoxH() * mags);
            }
      else {
            setbbox(symbols[score()->symIdx()][noteHead()].bbox(magS()));
            if (parent() == 0)
                  return;
            }
      // if not TAB or TAB with stems through
      if (!useTablature || ((StaffTypeTablature*)staff()->staffType())->stemThrough()) {
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
                        }
                  else if (_dots[i])
                        score()->undoRemoveElement(_dots[i]);
                  }
            }
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
            qreal d  = point(score()->styleS(ST_dotNoteDistance));
            qreal dd = point(score()->styleS(ST_dotDotDistance));
            qreal y  = 0.0;
            qreal x  = chord()->dotPosX() - pos().x() - chord()->pos().x();
            bool onLine = false;

            // do not draw dots on staff line:

            // if TAB and stems through staff
            if (staff()->isTabStaff()) {
                  // with TAB's, dotPosX is not set:
                  // get dot X from width of fret text and use TAB default spacing
                  StaffTypeTablature* tab = static_cast<StaffTypeTablature*>( staff()->staffType());
                  x = width();
                  dd = STAFFTYPE_TAB_DEFAULTDOTDIST_X * spatium();
                  d = dd * 0.5;
                  if(tab->stemThrough() ) {
                        // if fret mark on lines, use standard processing
                        if (tab->onLines())
                              onLine = true;
                        else
                        // if fret marks above lines, raise the dots by half line distance
                              y = -staff()->lineDistance() * 0.5;
                        }
                  // if stems beside staff, do nothing
                  else
                        return;
                  }
            // if not TAB, look at note line
            else
                  onLine = (_line & 1) == 0;
            // if on line, displace dots by half spatium up or down according to voice
            if (onLine) {
                  if (_dotPosition == MScore::AUTO)
                        y = (voice() & 1) == 0 ? -0.5 : 0.5;
                  else if (_dotPosition == MScore::UP)
                        y = -0.5;
                  else
                        y = 0.5;
                  }
            y *= spatium();

            // apply to dots
            for (int i = 0; i < dots; ++i) {
                  NoteDot* dot = _dots[i];
                  if (dot) {
                        dot->setPos(x + d + dd * i, y);
                        _dots[i]->adjustReadPos();
                        }
                  }
            }

      // layout elements attached to note
      for (Element* e : _el) {
            if (!score()->tagIsValid(e->tag()))
                  continue;
            e->setMag(mag());
            e->layout();
            if (e->type() == SYMBOL && static_cast<Symbol*>(e)->sym() == rightparenSym) {
                  qreal w = headWidth();
                  if (staff()->isTabStaff()) {
                        StaffTypeTablature* tab = (StaffTypeTablature*)staff()->staffType();
                        w = tabHeadWidth(tab);
                        }
                  e->setPos(w, 0.0);
                  }
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
                  StringData* stringData = staff()->part()->instr()->stringData();
                  if (stringData->convertPitch(_pitch, &string, &fret)) {
                        _fret   = fret;
                        _string = string;
                        }
                  }
            }
      else {
            _line = absStep(_tpc, _pitch);

            // calculate accidental

            Accidental::AccidentalType acci = Accidental::ACC_NONE;
            if (_accidental && _accidental->role() == Accidental::ACC_USER) {
                  acci = _accidental->accidentalType();
                  if (acci == Accidental::ACC_SHARP || acci == Accidental::ACC_FLAT) {
                        // TODO - what about double flat and double sharp?
                        KeySigEvent key = (staff() && chord()) ? staff()->key(chord()->tick()) : KeySigEvent();
                        int ntpc = pitch2tpc(_pitch, key.accidentalType(), acci == Accidental::ACC_SHARP ? PREFER_SHARPS : PREFER_FLATS);
                        if (ntpc != _tpc) {
                              qDebug("note has wrong tpc: %d, expected %d", _tpc, ntpc);
//                              setColor(QColor(255, 0, 0));
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
                              if (acci == Accidental::ACC_NONE)
                                    acci = Accidental::ACC_NATURAL;
                              }
                        }
                  }
            if (acci != Accidental::ACC_NONE && !_tieBack && !_hidden) {
                  if (_accidental == 0) {
                        _accidental = new Accidental(score());
                        _accidental->setGenerated(true);
                        add(_accidental);
                        }
                  _accidental->setAccidentalType(acci);
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

            //
            // calculate the real note line depending on clef
            //
            Staff* s = score()->staff(staffIdx() + chord()->staffMove());
            int tick = chord()->tick();
            _line    = relStep(_line, s->clef(tick));

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
      for (Element* e : _el) {
            if (score()->tagIsValid(e->tag()))
                  e->scanElements(data, func, all);
            }
      for (Spanner* sp : _spannerFor)
            sp->scanElements(data, func, all);

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
      foreach (Element* e, _el)
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
//    reset
//---------------------------------------------------------

void Note::reset()
      {
      score()->undoChangeProperty(this, P_USER_OFF, QPointF());
      score()->undoChangeProperty(chord(), P_USER_OFF, QPointF());
      score()->undoChangeProperty(chord(), P_STEM_DIRECTION, MScore::AUTO);
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Note::mag() const
      {
      qreal m = chord()->mag();
      if (_small)
            m *= score()->styleD(ST_smallNoteMag);
      return m;
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void Note::setSmall(bool val)
      {
      _small = val;
      }

//---------------------------------------------------------
//   setLine
//---------------------------------------------------------

void Note::setLine(int n)
      {
      _line = n;
      rypos() = _line * spatium() * .5;
      if (chord())
            chord()->lineChanged();
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
      return _pitch + staff()->pitchOffset(tick) + pitchOffset;
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
            score()->undoChangeProperty(ch, P_USER_OFF, ch->userOff() + userOff());
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

      // don't touch accidentals that don't concern tpc such as
      // quarter tones
      if (_accidental &&
            _accidental->accidentalType() > Accidental::ACC_NATURAL) {
            // calculate the real note line depending on clef

            Staff* s = score()->staff(staffIdx() + chord()->staffMove());
            int tick = chord()->tick();
            ClefType clef = s->clef(tick);
            _line    = relStep(_line, clef);
            return;
            }

      // calculate accidental

      Accidental::AccidentalType acci = Accidental::ACC_NONE;

      AccidentalVal accVal = tpc2alter(_tpc);
      if ((accVal != as->accidentalVal(int(_line))) || hidden() || as->tieContext(int(_line))) {
            as->setAccidentalVal(int(_line), accVal, _tieBack != 0);
            if (_tieBack)
                  acci = Accidental::ACC_NONE;
            else {
                  acci = Accidental::value2subtype(accVal);
                  if (acci == Accidental::ACC_NONE)
                        acci = Accidental::ACC_NATURAL;
                  }
            }

      if (acci != Accidental::ACC_NONE && !_tieBack && !_hidden) {
            if (_accidental == 0) {
                  Accidental* a = new Accidental(score());
                  a->setParent(this);
                  a->setAccidentalType(acci);
                  score()->undoAddElement(a);
                  }
            else if (_accidental->accidentalType() != acci) {
                  Accidental* a = _accidental->clone();
                  a->setParent(this);
                  a->setAccidentalType(acci);
                  score()->undoChangeElement(_accidental, a);
                  }
            }
      else {
            if (_accidental) {
                  // remove this if it was ACC_AUTO:
                  if (_accidental->role() == Accidental::ACC_AUTO)
                        score()->undoRemoveElement(_accidental);
                  else {
                        // keep it, but update type if needed
                        acci = Accidental::value2subtype(accVal);
                        if (acci == Accidental::ACC_NONE)
                              acci = Accidental::ACC_NATURAL;
                        if (_accidental->accidentalType() != acci) {
                              Accidental* a = _accidental->clone();
                              a->setParent(this);
                              a->setAccidentalType(acci);
                              score()->undoChangeElement(_accidental, a);
                              }
                        }
                  }
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
            case P_PLAY:
                  return play();
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
            case P_VISIBLE: {                     // P_VISIBLE requires reflecting property on dots
                  setVisible(v.toBool());
                  int dots = chord()->dots();
                  for (int i = 0; i < dots; ++i) {
                        if (_dots[i])
                              _dots[i]->setVisible(visible());
                        }
                  break;
                  }
            case P_PLAY:
                  setPlay(v.toBool());
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
//   undoSetPlay
//---------------------------------------------------------

void Note::undoSetPlay(bool val)
      {
      undoChangeProperty(P_PLAY, val);
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
//   setHeadType
//---------------------------------------------------------

void Note::setHeadType(NoteHeadType t)
      {
      _headType = t;
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
            case P_GHOST:
            case P_SMALL:
                  return false;
            case P_MIRROR_HEAD:
                  return MScore::DH_AUTO;
            case P_DOT_POSITION:
                  return MScore::AUTO;
            case P_HEAD_GROUP:
            case P_VELO_OFFSET:
                  return 0;
            case P_TUNING:
                  return 0.0;
            case P_FRET:
            case P_STRING:
                  return -1;
            case P_HEAD_TYPE:
                  return Note::HEAD_AUTO;
            case P_VELO_TYPE:
                  return MScore::OFFSET_VAL;
            case P_PLAY:
                  return true;
            default:
                  break;
            }
      return Element::propertyDefault(propertyId);
      }

//---------------------------------------------------------
//   setOnTimeOffset
//---------------------------------------------------------

void Note::setOnTimeOffset(int)
      {
      // TODO
      }

//---------------------------------------------------------
//   setOffTimeOffset
//---------------------------------------------------------

void Note::setOffTimeOffset(int)
      {
      // TODO
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Note::setScore(Score* s)
      {
      Element::setScore(s);
      if (_tieFor)
            _tieFor->setScore(s);
      }
}

