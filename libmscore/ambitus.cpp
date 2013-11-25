//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "ambitus.h"
#include "chord.h"
#include "measure.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "stafftype.h"
#include "sym.h"
#include "system.h"
#include "utils.h"

namespace Ms {

static const NoteHeadGroup          NOTEHEADGROUP_DEFAULT   = NoteHeadGroup::HEAD_NORMAL;
static const NoteHeadType           NOTEHEADTYPE_DEFAULT    = NoteHeadType::HEAD_AUTO;
static const MScore::DirectionH     DIR_DEFAULT             = MScore::DH_AUTO;
static const bool                   HASLINE_DEFAULT         = true;
static const qreal                  LINEWIDTH_DEFAULT       = 0.12;
static const qreal                  LEDGEROFFSET_DEFAULT    = 0.25;
static const qreal                  LINEOFFSET_DEFAULT      = 0.8;      // the distance between note head and line

//---------------------------------------------------------
//   Ambitus
//---------------------------------------------------------

Ambitus::Ambitus(Score* s)
: Element(s), _topAccid(s), _bottomAccid(s)
      {
      _noteHeadGroup    = NOTEHEADGROUP_DEFAULT;
      _noteHeadType     = NOTEHEADTYPE_DEFAULT;
      _dir              = DIR_DEFAULT;
      _hasLine          = HASLINE_DEFAULT;
      _lineWidth        = LINEWIDTH_DEFAULT;
      _topPitch = _bottomPitch = -1;
      _topTpc = _bottomTpc = INVALID_TPC;
      _topAccid.setParent(this);
      _bottomAccid.setParent(this);
//      _topAccid.setFlags(0);
//      _bottomAccid.setFlags(0);
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      }

//---------------------------------------------------------
//   setTrack
//
//    when the Ambitus element is assigned a track,
//    initialize top and bottom 'notes' to top and bottom staff lines
//---------------------------------------------------------

void Ambitus::setTrack(int t)
      {
      Segment*    segm  = segment();
      Staff*      stf   = score()->staff(track2staff(t));

      Element::setTrack(t);
      // if not initialized and there is a segment and a staff,
      // initialize pitches and tpc's to first and last staff line
      // (for use in palettes)
      if (_topTpc == INVALID_TPC || _bottomTpc == INVALID_TPC) {
            if (segm && stf) {
                  updateRange();
                  _topAccid.setTrack(t);
                  _bottomAccid.setTrack(t);
                  }
            else
                  _topPitch   = _bottomPitch = _topTpc = _bottomTpc = -1;
            }
      }

//---------------------------------------------------------
//   setTop/BottomPitch
//
//    setting either pitch requires to adjust the corresponding tpc
//---------------------------------------------------------

void Ambitus::setTopPitch(int val)
      {
      int deltaPitch    = val - topPitch();
      // if deltaPitch is not an integer number of octaves, adjust tpc
      // (to avoid 'wild' tpc changes with octave changes)
      if (deltaPitch % PITCH_DELTA_OCTAVE != 0) {
            int newTpc        = topTpc() + deltaPitch * TPC_DELTA_SEMITONE;
            // reduce newTpc into acceptable range via enharmonic
            while (newTpc < TPC_MIN)
                  newTpc += TPC_DELTA_ENHARMONIC;
            while (newTpc > TPC_MAX)
                  newTpc -= TPC_DELTA_ENHARMONIC;
            _topTpc     = newTpc;
            }
      _topPitch   = val;
      normalize();
      }

void Ambitus::setBottomPitch(int val)
      {
      int deltaPitch    = val - bottomPitch();
      // if deltaPitch is not an integer number of octaves, adjust tpc
      // (to avoid 'wild' tpc changes with octave changes)
      if (deltaPitch % PITCH_DELTA_OCTAVE != 0) {
            int newTpc        = bottomTpc() + deltaPitch * TPC_DELTA_SEMITONE;
            // reduce newTpc into acceptable range via enharmonic
            while (newTpc < TPC_MIN)
                  newTpc += TPC_DELTA_ENHARMONIC;
            while (newTpc > TPC_MAX)
                  newTpc -= TPC_DELTA_ENHARMONIC;
            _bottomTpc  = newTpc;
            }
      _bottomPitch= val;
      normalize();
      }

//---------------------------------------------------------
//   setTop/BottomTpc
//
//    setting either tpc requires to adjust the corresponding pitch
//    (but remaining in the same octave)
//---------------------------------------------------------

void Ambitus::setTopTpc(int val)
      {
      int octave        = topPitch() / PITCH_DELTA_OCTAVE;
      int deltaTpc      = val - topTpc();
      // get new pitch according to tpc change
      int newPitch      = topPitch() + deltaTpc * TPC_DELTA_SEMITONE;
      // reduce pitch to the same octave as original pitch
      newPitch          = (octave * PITCH_DELTA_OCTAVE) + (newPitch % PITCH_DELTA_OCTAVE);
      _topPitch   = newPitch;
      _topTpc     = val;
      normalize();
      }

void Ambitus::setBottomTpc(int val)
      {
      int octave        = bottomPitch() / PITCH_DELTA_OCTAVE;
      int deltaTpc      = val - bottomTpc();
      // get new pitch according to tpc change
      int newPitch      = bottomPitch() + deltaTpc * TPC_DELTA_SEMITONE;
      // reduce pitch to the same octave as original pitch
      newPitch          = (octave * PITCH_DELTA_OCTAVE) + (newPitch % PITCH_DELTA_OCTAVE);
      _bottomPitch= newPitch;
      _bottomTpc  = val;
      normalize();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Ambitus::write(Xml& xml) const
      {
      xml.stag("Ambitus");
      xml.tag(P_HEAD_GROUP, int(_noteHeadGroup), int(NOTEHEADGROUP_DEFAULT));
      xml.tag(P_HEAD_TYPE,  int(_noteHeadType),  int(NOTEHEADTYPE_DEFAULT));
      xml.tag(P_MIRROR_HEAD,_dir,           DIR_DEFAULT);
      xml.tag("hasLine",    _hasLine,       true);
      xml.tag(P_LINE_WIDTH, _lineWidth,     LINEWIDTH_DEFAULT);
      xml.tag("topPitch",   _topPitch);
      xml.tag("topTpc",     _topTpc);
      xml.tag("bottomPitch",_bottomPitch);
      xml.tag("bottomTpc",  _bottomTpc);
      if (_topAccid.accidentalType() != Accidental::ACC_NONE) {
            xml.stag("topAccidental");
            _topAccid.write(xml);
            xml.etag();
            }
      if (_bottomAccid.accidentalType() != Accidental::ACC_NONE) {
            xml.stag("bottomAccidental");
            _bottomAccid.write(xml);
            xml.etag();
            }
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Ambitus::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "head")
                  setProperty(P_HEAD_GROUP, Ms::getProperty(P_HEAD_GROUP, e));
            else if (tag == "headType")
                  setProperty(P_HEAD_TYPE, Ms::getProperty(P_HEAD_TYPE, e).toInt());
            else if (tag == "mirror")
                  setProperty(P_MIRROR_HEAD, Ms::getProperty(P_MIRROR_HEAD, e).toInt());
            else if (tag == "hasLine")
                  setHasLine(e.readInt());
            else if (tag == "lineWidth")
                  setProperty(P_LINE_WIDTH, Ms::getProperty(P_LINE_WIDTH, e).toReal());
            else if (tag == "topPitch")
                  _topPitch = e.readInt();
            else if (tag == "bottomPitch")
                  _bottomPitch = e.readInt();
            else if (tag == "topTpc")
                  _topTpc = e.readInt();
            else if (tag == "bottomTpc")
                  _bottomTpc = e.readInt();
            else if (tag == "topAccidental") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "Accidental")
                              _topAccid.read(e);
                        else
                              e.skipCurrentElement();
                        }
                  }
            else if (tag == "bottomAccidental") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "Accidental")
                              _bottomAccid.read(e);
                        else
                              e.skipCurrentElement();
                        }
                  }
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Ambitus::layout()
      {
      int         bottomLine, topLine;
      ClefType    clf;
      qreal       headWdt     = headWidth();
      int         key;
      qreal       lineDist;
      int         numOfLines;
      Segment*    segm        = segment();
      qreal       _spatium    = spatium();
      Staff*      stf         = nullptr;
      if (segm && track() > -1) {
            int tick    = segm->tick();
            stf         = score()->staff(staffIdx());
            lineDist    = stf->lineDistance() * _spatium;
            numOfLines  = stf->lines();
            clf         = stf->clef(tick);
            }
      else {                              // for use in palettes
            lineDist    = _spatium;
            numOfLines  = 3;
            clf         = ClefType::G;
            }

      //
      // NOTE HEADS Y POS (if pitch == -1, set to some default - for use in palettes)
      //
      qreal xAccidOffTop    = 0;
      qreal xAccidOffBottom = 0;
      if (stf)
            key = stf->key(segm->tick()).accidentalType();
      else
            key = KEY_C;

      // top note head
      if (_topPitch == -1)
            _topPos.setY(0);                          // if uninitialized, set to top staff line
      else {
            topLine  = absStep(_topTpc, _topPitch);
            topLine  = relStep(topLine, clf);
            _topPos.setY(topLine * lineDist * 0.5);
            // compute accidental
            Accidental::AccidentalType accidType;
            // if (13 <= (tpc - key) <= 19) there is no accidental)
            if (_topTpc - key >= 13 && _topTpc - key <= 19)
                  accidType = Accidental::ACC_NONE;
            else {
                  AccidentalVal accidVal = AccidentalVal( (_topTpc - TPC_MIN) / TPC_DELTA_SEMITONE - 2 );
                  accidType = Accidental::value2subtype(accidVal);
                  if (accidType == Accidental::ACC_NONE)
                        accidType = Accidental::ACC_NATURAL;
                  }
            _topAccid.setAccidentalType(accidType);
            if (accidType != Accidental::ACC_NONE)
                  _topAccid.layout();
            else
                  _topAccid.setbbox(QRect());
            _topAccid.rypos() = _topPos.y();
            }

      // bottom note head
      if (_bottomPitch == -1)
            _bottomPos.setY( (numOfLines-1) * lineDist);          // if uninitialized, set to last staff line
      else {
            bottomLine  = absStep(_bottomTpc, _bottomPitch);
            bottomLine  = relStep(bottomLine, clf);
            _bottomPos.setY(bottomLine * lineDist * 0.5);
            // compute accidental
            Accidental::AccidentalType accidType;
            if (_bottomTpc - key >= 13 && _bottomTpc - key <= 19)
                  accidType = Accidental::ACC_NONE;
            else {
                  AccidentalVal accidVal = AccidentalVal( (_bottomTpc - TPC_MIN) / TPC_DELTA_SEMITONE - 2 );
                  accidType = Accidental::value2subtype(accidVal);
                  if (accidType == Accidental::ACC_NONE)
                        accidType = Accidental::ACC_NATURAL;
                  }
            _bottomAccid.setAccidentalType(accidType);
            if (accidType != Accidental::ACC_NONE)
                  _bottomAccid.layout();
            else
                  _bottomAccid.setbbox(QRect());
            _bottomAccid.rypos() = _bottomPos.y();
            }

      //
      // NOTE HEAD X POS
      //
      // Note: manages colliding accidentals
      //
      qreal accNoteDist = point(score()->styleS(ST_accidentalNoteDistance));
      qreal accAccDist  = 0.25 * _spatium;
      xAccidOffTop      = _topAccid.width() + accNoteDist;
      xAccidOffBottom   = _bottomAccid.width() + accNoteDist;
      qreal xAccidOff   = max(xAccidOffTop, xAccidOffBottom);
      // if top accidental extends down more than bottom accidental extends up,
      // bottom accidental needs to be displaced
      // TO DO : ADD UNDERCUT FLATS!
      bool collision = _topAccid.ipos().y() + _topAccid.bbox().y() + _topAccid.height()
                  > _bottomAccid.ipos().y() + _bottomAccid.bbox().y();
      if (collision) {
            // place bottom accid. at the left edge and top accid. right after it
            _bottomAccid.rxpos()    = 0.0;
            _topAccid.rxpos()       = xAccidOffBottom -accNoteDist + accAccDist;
            switch (_dir) {
                  case MScore::DH_AUTO:               // note heads one above the other
                        // place both note heads right after the top accidental
                        _topPos.setX(_topAccid.ipos().x() + xAccidOffTop);
                        _bottomPos.setX(_topPos.x());
                        break;
                  case MScore::DH_LEFT:               // top note head at the left of bottom note head
                        // place the top head right after the top accid. and the bottom head right after the top head
                        _topPos.setX(_topAccid.ipos().x() + xAccidOffTop);
                        _bottomPos.setX(_topPos.x() + headWdt);
                        break;
                  case MScore::DH_RIGHT:              // top note head at the right of bottom note head
                        // place the bottom head right after the top accid. and the top head right after the bottom head
                        _bottomPos.setX(_topAccid.ipos().x() + xAccidOffTop);
                        _topPos.setX(_bottomPos.x() + headWdt);
                        break;
                  }
            }
      else {
            switch (_dir) {
                  case MScore::DH_AUTO:               // note heads one above the other
                        // align accid.s to their right margin
                        _topAccid.rxpos()       = xAccidOff - xAccidOffTop;
                        _bottomAccid.rxpos()    = xAccidOff - xAccidOffBottom;
                        // place note heads right after accid. right margin
                        _topPos.setX(xAccidOff);
                        _bottomPos.setX(xAccidOff);
                        break;
                  case MScore::DH_LEFT:               // top note head at the left of bottom note head
                        // place top accid. at the left edge, top head after accid.,
                        // bottom head after top head and bottom accid before bottom head
                        _topAccid.rxpos() = 0;
                        _topPos.setX(xAccidOffTop);
                        _bottomPos.setX(xAccidOffTop + headWdt);
                        _bottomAccid.rxpos() = xAccidOffTop + headWdt - xAccidOffBottom;
                        break;
                  case MScore::DH_RIGHT:              // top note head at the right of bottom note head
                        // place bottom accid. at the left edge, bottom head after accid.,
                        // top head after bottom head and top accid before top head
                        _bottomAccid.rxpos() = 0;
                        _bottomPos.setX(xAccidOffBottom);
                        _topPos.setX(xAccidOff + headWdt);
                        _topAccid.rxpos() = xAccidOffBottom + headWdt - xAccidOffTop;
                        break;
                  }
            }

      // compute line from top note centre to bottom note centre
      QLineF fullLine(_topPos.x() + headWdt*0.5, _topPos.y(),
            _bottomPos.x() + headWdt*0.5, _bottomPos.y());
      // shorten line on each side by offsets
      qreal yDelta = _bottomPos.y() - _topPos.y();
      if (yDelta != 0.0) {
            qreal off = _spatium * LINEOFFSET_DEFAULT;
            QPointF p1 = fullLine.pointAt(off / yDelta);
            QPointF p2 = fullLine.pointAt(1 - (off / yDelta));
            _line = QLineF(p1, p2);
            }
      else
            _line = fullLine;

      QRectF headRect = QRectF(0, -0.5*_spatium, headWdt, 1*_spatium);
      setbbox(headRect.translated(_topPos).united(headRect.translated(_bottomPos))
            .united(_topAccid.bbox().translated(_topAccid.ipos()))
            .united(_bottomAccid.bbox().translated(_bottomAccid.ipos()))
            );
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Ambitus::draw(QPainter* p) const
      {
      qreal _spatium = spatium();
      qreal lw = lineWidth() * _spatium;
      p->setPen(QPen(curColor(), lw, Qt::SolidLine, Qt::RoundCap));
      drawSymbol(noteHead(), p, _topPos);
      drawSymbol(noteHead(), p, _bottomPos);
      if (_hasLine)
            p->drawLine(_line);

      // draw ledger lines (if not in a palette)
      if (segment() && track() > -1) {
            Staff* stf        = score()->staff(staffIdx());
            qreal lineDist    = stf->lineDistance();
            int numOfLines    = stf->lines();
            qreal step        = lineDist * _spatium;
            qreal stepTolerance = step * 0.1;
            qreal ledgerOffset = score()->styleS(ST_ledgerLineLength).val() * 0.5 * _spatium;
            p->setPen(QPen(curColor(), score()->styleS(ST_ledgerLineWidth).val() * _spatium,
                        Qt::SolidLine, Qt::RoundCap) );
            if (_topPos.y()-stepTolerance <= -step) {
                  qreal xMin = _topPos.x() - ledgerOffset;
                  qreal xMax = _topPos.x() + headWidth() + ledgerOffset;
                  for (qreal y = -step; y >= _topPos.y()-stepTolerance; y -= step)
                        p->drawLine(QPointF(xMin, y), QPointF(xMax, y));
                  }
            if (_bottomPos.y()+stepTolerance >= numOfLines * step) {
                  qreal xMin = _bottomPos.x() - ledgerOffset;
                  qreal xMax = _bottomPos.x() + headWidth() + ledgerOffset;
                  for (qreal y = numOfLines*step; y <= _bottomPos.y()+stepTolerance; y += step)
                        p->drawLine(QPointF(xMin, y), QPointF(xMax, y));
                  }
            }
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space Ambitus::space() const
      {
      return Space(spatium() * 0.75, width() + spatium() * 0.5);
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Ambitus::scanElements(void* data, void (*func)(void*, Element*), bool /*all*/)
      {
      func(data, this);
      if (_topAccid.accidentalType() != Accidental::ACC_NONE)
            func(data, &_topAccid);
      if (_bottomAccid.accidentalType() != Accidental::ACC_NONE)
            func(data, &_bottomAccid);
      }

//---------------------------------------------------------
//   noteHead
//---------------------------------------------------------

SymId Ambitus::noteHead() const
      {
      int hg = 1;
      NoteHeadType ht  = NoteHeadType::HEAD_QUARTER;

      if (_noteHeadType != NoteHeadType::HEAD_AUTO)
            ht = _noteHeadType;

      SymId t = Note::noteHead(hg, _noteHeadGroup, ht);
      if (t == SymId::noSym) {
            qDebug("invalid note head %d/%d", _noteHeadGroup, _noteHeadType);
            t = Note::noteHead(0, NoteHeadGroup::HEAD_NORMAL, ht);
            }
      return t;
      }

//---------------------------------------------------------
//   headWidth
//
//    returns the width of the note head symbol
//---------------------------------------------------------

qreal Ambitus::headWidth() const
      {
//      int head  = noteHead();
//      qreal val = symbols[score()->symIdx()][head].width(magS());
//      return val;
      return symWidth(noteHead());
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF Ambitus::pagePos() const
      {
      if (parent() == 0)
            return pos();
      System* system = segment()->measure()->system();
      qreal yp = y();
      if (system)
            yp += system->staff(staffIdx())->y() + system->y();
      return QPointF(pageX(), yp);
      }

//---------------------------------------------------------
//   normalize
//
//    makes sure _topPitch is not < _bottomPitch
//---------------------------------------------------------

void Ambitus::normalize()
      {
      if (_topPitch < _bottomPitch) {
            int temp    = _topPitch;
            _topPitch   = _bottomPitch;
            _bottomPitch= temp;
            temp        = _topTpc;
            _topTpc     = _bottomTpc;
            _bottomTpc  = temp;
            }
      }

//---------------------------------------------------------
//   updateRange
//
//    scans the staff contents up to next section break to update the range pitches/tpc's
//---------------------------------------------------------

void Ambitus::updateRange()
      {
      if (!segment())
            return;
      Chord* chord;
      int   firstTrack  = track();
      int   lastTrack   = firstTrack + VOICES-1;
      int   pitchTop    = -1000;
      int   pitchBottom = 1000;
      int   tpcTop, tpcBottom;
      int   trk;
      Measure* meas     = segment()->measure();
      Segment* segm     = meas->findSegment(Segment::SegChordRest, segment()->tick());
      bool     stop     = meas->sectionBreak() != nullptr;
      while (segm) {
            // moved to another measure?
            if (segm->measure() != meas) {
                  // if section break has been found, stop here
                  if (stop)
                        break;
                  // update meas and stop condition
                  meas = segm->measure();
                  stop = meas->sectionBreak() != nullptr;
                  }
            // scan all relevant tracks of this segment for chords
            for (trk=firstTrack; trk <= lastTrack; trk++)
                  if ( (chord=static_cast<Chord*>(segm->element(trk))) != nullptr
                              && chord->type() == Element::CHORD) {
                        // update pitch range (with associated tpc's)
                        foreach (Note* n, chord->notes()) {
                              if (n->pitch() > pitchTop) {
                                    pitchTop = n->pitch();
                                    tpcTop   = n->tpc();
                                    }
                              if (n->pitch() < pitchBottom) {
                                    pitchBottom = n->pitch();
                                    tpcBottom   = n->tpc();
                                    }
                              }
                        }
            segm = segm->nextCR();
            }

      if (pitchTop > -1000) {             // if something has been found, update this
            _topPitch    = pitchTop;
            _bottomPitch = pitchBottom;
            _topTpc      = tpcTop;
            _bottomTpc   = tpcBottom;
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Ambitus::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_HEAD_GROUP:
                  return int(noteHeadGroup());
            case P_HEAD_TYPE:
                  return int(noteHeadType());
            case P_MIRROR_HEAD:
                  return direction();
            case P_GHOST:                 // recycled property = _hasLine
                  return hasLine();
            case P_LINE_WIDTH:
                  return lineWidth();
            case P_TPC:
                  return topTpc();
            case P_FBPARENTHESIS1:        // recycled property = _bottomTpc
                  return bottomTpc();
            case P_PITCH:
                  return topPitch();
            case P_FBPARENTHESIS2:        // recycled property = _bottomPitch
                  return bottomPitch();
            case P_FBPARENTHESIS3:        // recycled property = octave of _topPitch
                  return topOctave();
            case P_FBPARENTHESIS4:        // recycled property = octave of _bottomPitch
                  return bottomOctave();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Ambitus::setProperty(P_ID propertyId, const QVariant& v)
      {
      bool  rv = true;

      score()->addRefresh(canvasBoundingRect());
      switch(propertyId) {
            case P_HEAD_GROUP:
                  setNoteHeadGroup( NoteHeadGroup(v.toInt()) );
                  break;
            case P_HEAD_TYPE:
                  setNoteHeadType( NoteHeadType(v.toInt()) );
                  break;
            case P_MIRROR_HEAD:
                  setDirection( MScore::DirectionH(v.toInt()) );
                  break;
            case P_GHOST:                 // recycled property = _hasLine
                  setHasLine(v.toBool());
                  break;
            case P_LINE_WIDTH:
                  setLineWidth(v.toReal());
                  break;
            case P_TPC:
                  setTopTpc(v.toInt());
                  break;
            case P_FBPARENTHESIS1:        // recycled property = _bottomTpc
                  setBottomTpc(v.toInt());
                  break;
            case P_PITCH:
                  setTopPitch(v.toInt());
                  break;
            case P_FBPARENTHESIS2:        // recycled property = _bottomPitch
                  setBottomPitch(v.toInt());
                  break;
            case P_FBPARENTHESIS3:        // recycled property = octave of _topPitch
                  setTopPitch(topPitch() % 12 + v.toInt() * 12);
                  break;
            case P_FBPARENTHESIS4:        // recycled property = octave of _bottomPitch
                  setBottomPitch(bottomPitch() % 12 + v.toInt() * 12);
                  break;
            default:
                  rv = Element::setProperty(propertyId, v);
                  break;
            }
      if (rv)
            score()->setLayoutAll(true);
      return rv;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Ambitus::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_HEAD_GROUP:      return int(NOTEHEADGROUP_DEFAULT);
            case P_HEAD_TYPE:       return int(NOTEHEADTYPE_DEFAULT);
            case P_MIRROR_HEAD:     return DIR_DEFAULT;
            case P_GHOST:           return HASLINE_DEFAULT;
            case P_LINE_WIDTH:      return LINEWIDTH_DEFAULT;
            case P_TPC:             break;      // no defaults for pitches, tpc's and octaves
            case P_FBPARENTHESIS1:  break;
            case P_PITCH:           break;
            case P_FBPARENTHESIS2:  break;
            case P_FBPARENTHESIS3:  break;
            case P_FBPARENTHESIS4:  break;
            default:                return Element::propertyDefault(id);
            }
      return QVariant();
      }

}

