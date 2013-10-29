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

#include "rangesymbol.h"
#include "chord.h"
#include "measure.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "stafftype.h"
#include "system.h"
#include "utils.h"

namespace Ms {

static const Note::NoteHeadGroup    NOTEHEADGROUP_DEFAULT   = Note::HEAD_NORMAL;
static const Note::NoteHeadType     NOTEHEADTYPE_DEFAULT    = Note::HEAD_AUTO;
static const MScore::DirectionH     DIR_DEFAULT             = MScore::DH_AUTO;
static const bool                   HASLINE_DEFAULT         = true;
static const qreal                  LINEWIDTH_DEFAULT       = 0.18;
static const qreal                  LEDGEROFFSET_DEFAULT    = 0.25;

//---------------------------------------------------------
//   Range
//---------------------------------------------------------

Range::Range(Score* s)
  : Element(s)
      {
      _noteHeadGroup    = NOTEHEADGROUP_DEFAULT;
      _noteHeadType     = NOTEHEADTYPE_DEFAULT;
      _dir              = DIR_DEFAULT;
      _hasLine          = HASLINE_DEFAULT;
      _lineWidth        = LINEWIDTH_DEFAULT;
      _topPitch = _bottomPitch = -1;
      _topTpc = _bottomTpc = INVALID_TPC;
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      }

//---------------------------------------------------------
//   setTrack
//
//    when the range element is assigned a track,
//    initialize top and bottom 'notes' to top and bottom staff lines
//---------------------------------------------------------

void Range::setTrack(int t)
      {
      Segment*    segm  = segment();
      Staff*      stf   = score()->staff(track2staff(t));

      Element::setTrack(t);
      // if not initialized and there is a segment and a staff,
      // initialize pitches and tpc's to first and last staff line
      if (_topTpc == INVALID_TPC || _bottomTpc == INVALID_TPC) {
            if (segm && stf) {
/*
                  int numOfLines    = stf->lines();
                  int tick          = segm->tick();
                  ClefType clf      = stf->clef(tick);
                  int key           = stf->key(tick).accidentalType();
                  _topPitch         = line2pitch(0, clf, key);
                  _topTpc           = pitch2tpc(_topPitch, key, PREFER_NEAREST);
                  _bottomPitch      = line2pitch( (numOfLines-1) * 2, clf, key);
                  _bottomTpc        = pitch2tpc(_bottomPitch, key, PREFER_NEAREST);
*/
                  updateRange();
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

void Range::setTopPitch(int val)
      {
      int deltaPitch    = val - topPitch();
      // if deltaPitch is not an integer number of octaves, adjust tpc
      // (to avoid 'wild' tpc changes)
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

void Range::setBottomPitch(int val)
      {
      int deltaPitch    = val - bottomPitch();
      // if deltaPitch is not an integer number of octaves, adjust tpc
      // (to avoid 'wild' tpc changes)
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

void Range::setTopTpc(int val)
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

void Range::setBottomTpc(int val)
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

void Range::write(Xml& xml) const
      {
      xml.stag("Range");
      xml.tag(P_HEAD_GROUP, _noteHeadGroup, NOTEHEADGROUP_DEFAULT);
      xml.tag(P_HEAD_TYPE,  _noteHeadType,  NOTEHEADTYPE_DEFAULT);
      xml.tag(P_MIRROR_HEAD,_dir,           DIR_DEFAULT);
      xml.tag("hasLine",    _hasLine,       true);
      xml.tag(P_LINE_WIDTH, _lineWidth,     LINEWIDTH_DEFAULT);
      xml.tag("topPitch",   _topPitch);
      xml.tag("topTpc",     _topTpc);
      xml.tag("bottomPitch",_bottomPitch);
      xml.tag("bottomTpc",  _bottomTpc);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Range::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "head")
                  setProperty(P_HEAD_GROUP, Ms::getProperty(P_HEAD_GROUP, e));
            else if (tag == "headType")
                  setProperty(P_HEAD_TYPE, Ms::getProperty(P_HEAD_TYPE, e).toInt());
            else if (e.name() == "mirror")
                  setProperty(P_MIRROR_HEAD, Ms::getProperty(P_MIRROR_HEAD, e).toInt());
            else if (e.name() == "hasLine")
                  setHasLine(e.readInt());
            else if (e.name() == "lineWidth")
                  setProperty(P_LINE_WIDTH, Ms::getProperty(P_LINE_WIDTH, e).toReal());
            else if (e.name() == "topPitch")
                  _topPitch = e.readInt();
            else if (e.name() == "bottomPitch")
                  _bottomPitch = e.readInt();
            else if (e.name() == "topTpc")
                  _topTpc = e.readInt();
            else if (e.name() == "bottomTpc")
                  _bottomTpc = e.readInt();
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Range::layout()
      {
      ClefType clf;
      qreal headWdt     = headWidth();
      int   line;
      qreal lineDist;
      int   numOfLines;
      Segment* segm     = segment();
      if (segm && track() > -1) {
            int tick    = segm->tick();
            Staff* stf  = score()->staff(staffIdx());
            lineDist    = stf->lineDistance();
            numOfLines  = stf->lines();
            clf         = stf->clef(tick);
            }
      else {                              // for use in palettes
            lineDist    = 1;
            numOfLines  = 3;
            clf         = ClefType::G;
            }
      qreal _spatium    = spatium();

      // set top and bottom note Y pos (if pitch == -1, set to some default)
      if (_topPitch == -1)
            _topPos.setY(0);                          // if uninitialized, set to top staff line
      else {
            line  = absStep(_topTpc, _topPitch);
            line  = relStep(line, clf);
            _topPos.setY(line * lineDist * _spatium * 0.5);
            }
      if (_bottomPitch == -1)
            _bottomPos.setY( (numOfLines-1) * lineDist * _spatium); // if uninitialized, set to last staff line
      else {
            line  = absStep(_bottomTpc, _bottomPitch);
            line  = relStep(line, clf);
            _bottomPos.setY(line * lineDist * _spatium * 0.5);
            }

      // set top and bottom note X pos
      if (_dir == MScore::DH_AUTO || _dir == MScore::DH_LEFT) {
            _topPos.setX(0);
            _bottomPos.setX(_dir == MScore::DH_AUTO ? 0 : headWdt);
            }
      else {
            _topPos.setX(headWdt);
            _bottomPos.setX(0);
            }

      // compute line from top to bottom note
      _line.setLine(_topPos.x() + headWdt*0.5, _topPos.y() + 0.75*_spatium,
           _bottomPos.x() + headWdt*0.5, _bottomPos.y() - 0.75*_spatium);

      QRectF headRect = QRectF(0, -0.5*_spatium, headWdt, 1*_spatium);
      setbbox(headRect.translated(_topPos).united(headRect.translated(_bottomPos)));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Range::draw(QPainter* p) const
      {
      qreal _spatium = spatium();
      qreal lw = lineWidth() * _spatium;
      p->setPen(QPen(curColor(), lw, Qt::SolidLine, Qt::RoundCap));
      symbols[score()->symIdx()][noteHead()].draw(p, magS(), _topPos);
      symbols[score()->symIdx()][noteHead()].draw(p, magS(), _bottomPos);
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

Space Range::space() const
      {
      return Space(spatium() * 0.75, width() + spatium() * 0.5);
      }

//---------------------------------------------------------
//   noteHead
//---------------------------------------------------------

int Range::noteHead() const
      {
      int hg, ht;
            hg = 1;
            ht = Note::HEAD_QUARTER;
      if (_noteHeadType != Note::HEAD_AUTO)
            ht = _noteHeadType;

      int t = noteHeads[hg][int(_noteHeadGroup)][ht];
      if (t == -1) {
            qDebug("invalid note head %d/%d", _noteHeadGroup, _noteHeadType);
            t = noteHeads[hg][0][ht];
            }
      return t;
      }

//---------------------------------------------------------
//   headWidth
//
//    returns the width of the note head symbol
//---------------------------------------------------------

qreal Range::headWidth() const
      {
      int head  = noteHead();
      qreal val = symbols[score()->symIdx()][head].width(magS());
      return val;
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF Range::pagePos() const
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

void Range::normalize()
      {
      int temp;
      if (_topPitch < _bottomPitch) {
            temp        = _topPitch;
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
//    scans the staff contents up to next section break to update the range pitches
//---------------------------------------------------------

void Range::updateRange()
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

QVariant Range::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_HEAD_GROUP:
                  return noteHeadGroup();
            case P_HEAD_TYPE:
                  return noteHeadType();
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

bool Range::setProperty(P_ID propertyId, const QVariant& v)
      {
      bool  rv = true;

      score()->addRefresh(canvasBoundingRect());
      switch(propertyId) {
            case P_HEAD_GROUP:
                  setNoteHeadGroup( Note::NoteHeadGroup(v.toInt()) );
                  break;
            case P_HEAD_TYPE:
                  setNoteHeadType( Note::NoteHeadType(v.toInt()) );
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
            // changing either tpc requires adjusting corresponding pitch
            case P_TPC:
                  setTopTpc(v.toInt());
                  break;
            case P_FBPARENTHESIS1:        // recycled property = _bottomTpc
                  setBottomTpc(v.toInt());
                  break;
            // changing either pitch requires adjusting corresponding tpc
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

QVariant Range::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_HEAD_GROUP:      return NOTEHEADGROUP_DEFAULT;
            case P_HEAD_TYPE:       return NOTEHEADTYPE_DEFAULT;
            case P_MIRROR_HEAD:     return DIR_DEFAULT;
            case P_GHOST:           return HASLINE_DEFAULT;
            case P_LINE_WIDTH:      return LINEWIDTH_DEFAULT;
            case P_TPC:             break;
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

