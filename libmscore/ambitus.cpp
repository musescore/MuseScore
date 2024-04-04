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
#include "read206.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "stafftype.h"
#include "sym.h"
#include "system.h"
#include "utils.h"
#include "xml.h"

namespace Ms {

static const NoteHead::Group  NOTEHEADGROUP_DEFAULT = NoteHead::Group::HEAD_NORMAL;
static const NoteHead::Type   NOTEHEADTYPE_DEFAULT  = NoteHead::Type::HEAD_AUTO;
static const MScore::DirectionH     DIR_DEFAULT     = MScore::DirectionH::AUTO;
static const bool           HASLINE_DEFAULT         = true;
static const Spatium          LINEWIDTH_DEFAULT(0.12);
#if 0 // yet(?) unused
static const qreal          LEDGEROFFSET_DEFAULT    = 0.25;
#endif
static const qreal          LINEOFFSET_DEFAULT      = 0.8;      // the distance between notehead and line

//---------------------------------------------------------
//   Ambitus
//---------------------------------------------------------

Ambitus::Ambitus(Score* s)
   : Element(s, ElementFlag::MOVABLE), _topAccid(s), _bottomAccid(s)
      {
      _noteHeadGroup    = NOTEHEADGROUP_DEFAULT;
      _noteHeadType     = NOTEHEADTYPE_DEFAULT;
      _dir              = DIR_DEFAULT;
      _hasLine          = HASLINE_DEFAULT;
      _lineWidth        = LINEWIDTH_DEFAULT;
      _topPitch         = INVALID_PITCH;
      _bottomPitch      = INVALID_PITCH;
      _topTpc           = Tpc::TPC_INVALID;
      _bottomTpc        = Tpc::TPC_INVALID;
      _topAccid.setParent(this);
      _bottomAccid.setParent(this);
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Ambitus::mag() const
      {
      return staff() ? staff()->mag(tick()) : 1.0;
      }

//---------------------------------------------------------
//   initFrom
//---------------------------------------------------------

void Ambitus::initFrom(Ambitus* a)
      {
      _noteHeadGroup   = a->_noteHeadGroup;
      _noteHeadType    = a->_noteHeadType;
      _dir             = a->_dir;
      _hasLine         = a->_hasLine;
      _lineWidth       = a->_lineWidth;
      _topPitch        = a->_topPitch;
      _bottomPitch     = a->_bottomPitch;
      _topTpc          = a->_topTpc;
      _bottomTpc       = a->_bottomTpc;
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
      if (_topPitch == INVALID_PITCH || _topTpc == Tpc::TPC_INVALID
            || _bottomPitch == INVALID_PITCH ||_bottomTpc == Tpc::TPC_INVALID) {
            if (segm && stf) {
                  updateRange();
                  _topAccid.setTrack(t);
                  _bottomAccid.setTrack(t);
                  }
//            else {
//                  _topPitch = _bottomPitch = INVALID_PITCH;
//                  _topTpc   = _bottomTpc   = Tpc::TPC_INVALID;
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
            while (newTpc < Tpc::TPC_MIN)
                  newTpc += TPC_DELTA_ENHARMONIC;
            while (newTpc > Tpc::TPC_MAX)
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
            while (newTpc < Tpc::TPC_MIN)
                  newTpc += TPC_DELTA_ENHARMONIC;
            while (newTpc > Tpc::TPC_MAX)
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

void Ambitus::write(XmlWriter& xml) const
      {
      xml.stag(this);
      xml.tag(Pid::HEAD_GROUP, int(_noteHeadGroup), int(NOTEHEADGROUP_DEFAULT));
      xml.tag(Pid::HEAD_TYPE,  int(_noteHeadType),  int(NOTEHEADTYPE_DEFAULT));
      xml.tag(Pid::MIRROR_HEAD,int(_dir),           int(DIR_DEFAULT));
      xml.tag("hasLine",    _hasLine,       true);
      xml.tag(Pid::LINE_WIDTH_SPATIUM, _lineWidth, LINEWIDTH_DEFAULT);
      xml.tag("topPitch",   _topPitch);
      xml.tag("topTpc",     _topTpc);
      xml.tag("bottomPitch",_bottomPitch);
      xml.tag("bottomTpc",  _bottomTpc);
      if (_topAccid.accidentalType() != AccidentalType::NONE) {
            xml.stag("topAccidental");
            _topAccid.write(xml);
            xml.etag();
            }
      if (_bottomAccid.accidentalType() != AccidentalType::NONE) {
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
            if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Ambitus::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());
      if (tag == "head")
            readProperty(e, Pid::HEAD_GROUP);
      else if (tag == "headType")
            readProperty(e, Pid::HEAD_TYPE);
      else if (tag == "mirror")
            readProperty(e, Pid::MIRROR_HEAD);
      else if (tag == "hasLine")
            setHasLine(e.readInt());
      else if (tag == "lineWidth")
            readProperty(e, Pid::LINE_WIDTH_SPATIUM);
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
                  if (e.name() == "Accidental") {
                        if (score()->mscVersion() < 301)
                              readAccidental206(&_topAccid, e);
                        else
                              _topAccid.read(e);
                        }
                  else
                        e.skipCurrentElement();
                  }
            }
      else if (tag == "bottomAccidental") {
            while (e.readNextStartElement()) {
                  if (e.name() == "Accidental") {
                        if (score()->mscVersion() < 301)
                              readAccidental206(&_bottomAccid, e);
                        else
                              _bottomAccid.read(e);
                        }
                  else
                        e.skipCurrentElement();
                  }
            }
      else if (Element::readProperties(e))
            ;
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Ambitus::layout()
      {
      int         bottomLine, topLine;
      ClefType    clf;
      qreal       headWdt     = headWidth();
      Key         key;
      qreal       lineDist;
      int         numOfLines;
      Segment*    segm        = segment();
      qreal       _spatium    = spatium();
      Staff*      stf         = nullptr;
      if (segm && track() > -1) {
            Fraction tick    = segm->tick();
            stf         = score()->staff(staffIdx());
            lineDist    = stf->lineDistance(tick) * _spatium;
            numOfLines  = stf->lines(tick);
            clf         = stf->clef(tick);
            }
      else {                              // for use in palettes
            lineDist    = _spatium;
            numOfLines  = 3;
            clf         = ClefType::G;
            }

      //
      // NOTEHEADS Y POS
      //
      // if pitch == INVALID_PITCH oor tpc == INALID_TPC, set to some default:
      // for use in palettes and when actual range cannot be calculated (new ambitus or no notes in staff)
      //
      qreal xAccidOffTop    = 0;
      qreal xAccidOffBottom = 0;
      if (stf)
            key = stf->key(segm->tick());
      else
            key = Key::C;

      // top notehead
      if (_topPitch == INVALID_PITCH || _topTpc == Tpc::TPC_INVALID)
            _topPos.setY(0);                          // if uninitialized, set to top staff line
      else {
            topLine  = absStep(_topTpc, _topPitch);
            topLine  = relStep(topLine, clf);
            _topPos.setY(topLine * lineDist * 0.5);
            // compute accidental
            AccidentalType accidType;
            // if (13 <= (tpc - key) <= 19) there is no accidental)
            if (_topTpc - int(key) >= 13 && _topTpc - int(key) <= 19)
                  accidType = AccidentalType::NONE;
            else {
                  AccidentalVal accidVal = tpc2alter(_topTpc);
                  accidType = Accidental::value2subtype(accidVal);
                  if (accidType == AccidentalType::NONE)
                        accidType = AccidentalType::NATURAL;
                  }
            _topAccid.setAccidentalType(accidType);
            if (accidType != AccidentalType::NONE)
                  _topAccid.layout();
            else
                  _topAccid.setbbox(QRect());
            _topAccid.rypos() = _topPos.y();
            }

      // bottom notehead
      if (_bottomPitch == INVALID_PITCH || _bottomTpc == Tpc::TPC_INVALID)
            _bottomPos.setY( (numOfLines-1) * lineDist);          // if uninitialized, set to last staff line
      else {
            bottomLine  = absStep(_bottomTpc, _bottomPitch);
            bottomLine  = relStep(bottomLine, clf);
            _bottomPos.setY(bottomLine * lineDist * 0.5);
            // compute accidental
            AccidentalType accidType;
            if (_bottomTpc - int(key) >= 13 && _bottomTpc - int(key) <= 19)
                  accidType = AccidentalType::NONE;
            else {
                  AccidentalVal accidVal = tpc2alter(_bottomTpc);
                  accidType = Accidental::value2subtype(accidVal);
                  if (accidType == AccidentalType::NONE)
                        accidType = AccidentalType::NATURAL;
                  }
            _bottomAccid.setAccidentalType(accidType);
            if (accidType != AccidentalType::NONE)
                  _bottomAccid.layout();
            else
                  _bottomAccid.setbbox(QRect());
            _bottomAccid.rypos() = _bottomPos.y();
            }

      //
      // NOTEHEAD X POS
      //
      // Note: manages colliding accidentals
      //
      qreal accNoteDist = point(score()->styleS(Sid::accidentalNoteDistance));
      xAccidOffTop      = _topAccid.width() + accNoteDist;
      xAccidOffBottom   = _bottomAccid.width() + accNoteDist;

      // if top accidental extends down more than bottom accidental extends up,
      // AND ambitus is not leaning right, bottom accidental needs to be displaced
      bool collision =
            (_topAccid.ipos().y() + _topAccid.bbox().y() + _topAccid.height()
                   > _bottomAccid.ipos().y() + _bottomAccid.bbox().y() )
            && _dir != MScore::DirectionH::RIGHT;
      if (collision) {
            // displace bottom accidental (also attempting to 'undercut' flats)
            xAccidOffBottom = xAccidOffTop +
                  ((_bottomAccid.accidentalType() == AccidentalType::FLAT
                        || _bottomAccid.accidentalType() == AccidentalType::FLAT2
                        || _bottomAccid.accidentalType() == AccidentalType::NATURAL)
                  ? _bottomAccid.width() * 0.5 : _bottomAccid.width());
            }

      switch (_dir) {
            case MScore::DirectionH::AUTO:               // noteheads one above the other
                  // left align noteheads and right align accidentals 'hanging' on the left
                  _topPos.setX(0.0);
                  _bottomPos.setX(0.0);
                  _topAccid.rxpos()       = - xAccidOffTop;
                  _bottomAccid.rxpos()    = - xAccidOffBottom;
                  break;
            case MScore::DirectionH::LEFT:               // top notehead at the left of bottom notehead
                  // place top notehead at left margin; bottom notehead at right of top head;
                  // top accid. 'hanging' on left of top head and bottom accid. 'hanging' at left of bottom head
                  _topPos.setX(0.0);
                  _bottomPos.setX(headWdt);
                  _topAccid.rxpos() = - xAccidOffTop;
                  _bottomAccid.rxpos() = collision ? - xAccidOffBottom : headWdt - xAccidOffBottom;
                  break;
            case MScore::DirectionH::RIGHT:              // top notehead at the right of bottom notehead
                  // bottom notehead at left margin; top notehead at right of bottomnotehead
                  // top accid. 'hanging' on left of top head and bottom accid. 'hanging' at left of bottom head
                  _bottomPos.setX(0.0);
                  _topPos.setX(headWdt);
                  _bottomAccid.rxpos() = - xAccidOffBottom;
                  _topAccid.rxpos() = headWdt - xAccidOffTop;
                  break;
            }

      // compute line from top note centre to bottom note centre
      QLineF fullLine(_topPos.x() + headWdt*0.5, _topPos.y(),
            _bottomPos.x() + headWdt*0.5, _bottomPos.y());
      // shorten line on each side by offsets
      qreal yDelta = _bottomPos.y() - _topPos.y();
      if (!qFuzzyIsNull(yDelta)) {
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
      qreal lw = lineWidth().val() * _spatium;
      p->setPen(QPen(curColor(), lw, Qt::SolidLine, Qt::FlatCap));
      drawSymbol(noteHead(), p, _topPos);
      drawSymbol(noteHead(), p, _bottomPos);
      if (_hasLine)
            p->drawLine(_line);

      // draw ledger lines (if not in a palette)
      if (segment() && track() > -1) {
            Fraction tick          = segment()->tick();
            Staff* stf        = score()->staff(staffIdx());
            qreal lineDist    = stf->lineDistance(tick);
            int numOfLines    = stf->lines(tick);
            qreal step        = lineDist * _spatium;
            qreal stepTolerance = step * 0.1;
            qreal ledgerOffset = score()->styleS(Sid::ledgerLineLength).val() * _spatium;
            p->setPen(QPen(curColor(), score()->styleS(Sid::ledgerLineWidth).val() * _spatium,
                        Qt::SolidLine, Qt::FlatCap) );
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
//   scanElements
//---------------------------------------------------------

void Ambitus::scanElements(void* data, void (*func)(void*, Element*), bool /*all*/)
      {
      func(data, this);
      if (_topAccid.accidentalType() != AccidentalType::NONE)
            func(data, &_topAccid);
      if (_bottomAccid.accidentalType() != AccidentalType::NONE)
            func(data, &_bottomAccid);
      }

//---------------------------------------------------------
//   noteHead
//---------------------------------------------------------

SymId Ambitus::noteHead() const
      {
      int hg = 1;
      NoteHead::Type ht  = NoteHead::Type::HEAD_QUARTER;

      if (_noteHeadType != NoteHead::Type::HEAD_AUTO)
            ht = _noteHeadType;

      SymId t = Note::noteHead(hg, _noteHeadGroup, ht);
      if (t == SymId::noSym) {
            qDebug("invalid notehead %d/%d", int(_noteHeadGroup), int(_noteHeadType));
            t = Note::noteHead(0, NoteHead::Group::HEAD_NORMAL, ht);
            }
      return t;
      }

//---------------------------------------------------------
//   headWidth
//
//    returns the width of the notehead symbol
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
      int   tpcTop      = 0;  // Initialized to prevent warning
      int   tpcBottom   = 0;  // Initialized to prevent warning
      int   trk;
      Measure* meas     = segment()->measure();
      Segment* segm     = meas->findSegment(SegmentType::ChordRest, segment()->tick());
      bool     stop     = meas->sectionBreak();
      while (segm) {
            // moved to another measure?
            if (segm->measure() != meas) {
                  // if section break has been found, stop here
                  if (stop)
                        break;
                  // update meas and stop condition
                  meas = segm->measure();
                  stop = meas->sectionBreak();
                  }
            // scan all relevant tracks of this segment for chords
            for (trk = firstTrack; trk <= lastTrack; trk++) {
                  Element* e = segm->element(trk);
                  if (!e || !e->isChord())
                        continue;
                  chord = toChord(e);
                  // update pitch range (with associated tpc's)
                  for (Note* n : chord->notes()) {
                        if (!n->play())         // skip notes which are not to be played
                              continue;
                        int pitch = n->epitch();
                        if (pitch > pitchTop) {
                              pitchTop = pitch;
                              tpcTop   = n->tpc();
                              }
                        if (pitch < pitchBottom) {
                              pitchBottom = pitch;
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

void Ambitus::remove(Element* e)
      {
      if (e->type() == ElementType::ACCIDENTAL) {
            //! NOTE Do nothing (removing _topAccid or _bottomAccid)
            return;
            }

      Element::remove(e);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Ambitus::getProperty(Pid propertyId) const
      {
      switch(propertyId) {
            case Pid::HEAD_GROUP:
                  return int(noteHeadGroup());
            case Pid::HEAD_TYPE:
                  return int(noteHeadType());
            case Pid::MIRROR_HEAD:
                  return int(direction());
            case Pid::GHOST:                 // recycled property = _hasLine
                  return hasLine();
            case Pid::LINE_WIDTH_SPATIUM:
                  return lineWidth();
            case Pid::TPC1:
                  return topTpc();
            case Pid::FBPARENTHESIS1:        // recycled property = _bottomTpc
                  return bottomTpc();
            case Pid::PITCH:
                  return topPitch();
            case Pid::FBPARENTHESIS2:        // recycled property = _bottomPitch
                  return bottomPitch();
            case Pid::FBPARENTHESIS3:        // recycled property = octave of _topPitch
                  return topOctave();
            case Pid::FBPARENTHESIS4:        // recycled property = octave of _bottomPitch
                  return bottomOctave();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Ambitus::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::HEAD_GROUP:
                  setNoteHeadGroup( NoteHead::Group(v.toInt()) );
                  break;
            case Pid::HEAD_TYPE:
                  setNoteHeadType( NoteHead::Type(v.toInt()) );
                  break;
            case Pid::MIRROR_HEAD:
                  setDirection(MScore::DirectionH(v.toInt()) );
                  break;
            case Pid::GHOST:                 // recycled property = _hasLine
                  setHasLine(v.toBool());
                  break;
            case Pid::LINE_WIDTH_SPATIUM:
                  setLineWidth(v.value<Spatium>());
                  break;
            case Pid::TPC1:
                  setTopTpc(v.toInt());
                  break;
            case Pid::FBPARENTHESIS1:        // recycled property = _bottomTpc
                  setBottomTpc(v.toInt());
                  break;
            case Pid::PITCH:
                  setTopPitch(v.toInt());
                  break;
            case Pid::FBPARENTHESIS2:        // recycled property = _bottomPitch
                  setBottomPitch(v.toInt());
                  break;
            case Pid::FBPARENTHESIS3:        // recycled property = octave of _topPitch
                  setTopPitch(topPitch() % 12 + (v.toInt() + 1) * 12);
                  break;
            case Pid::FBPARENTHESIS4:        // recycled property = octave of _bottomPitch
                  setBottomPitch(bottomPitch() % 12 + (v.toInt() + 1) * 12);
                  break;
            default:
                  return Element::setProperty(propertyId, v);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Ambitus::propertyDefault(Pid id) const
      {
      switch(id) {
            case Pid::HEAD_GROUP:
                  return int(NOTEHEADGROUP_DEFAULT);
            case Pid::HEAD_TYPE:
                  return int(NOTEHEADTYPE_DEFAULT);
            case Pid::MIRROR_HEAD:
                  return int(DIR_DEFAULT);
            case Pid::GHOST:
                  return HASLINE_DEFAULT;
            case Pid::LINE_WIDTH_SPATIUM:
                  return Spatium(LINEWIDTH_DEFAULT);
            case Pid::TPC1:                  // no defaults for pitches, tpc's and octaves
            case Pid::FBPARENTHESIS1:
            case Pid::PITCH:
            case Pid::FBPARENTHESIS2:
            case Pid::FBPARENTHESIS3:
            case Pid::FBPARENTHESIS4:
                  break;
            default:
                  return Element::propertyDefault(id);
            }
      return QVariant();
      }

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

Element* Ambitus::nextSegmentElement()
      {
      return segment()->firstInNextSegments(staffIdx());
      }

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

Element* Ambitus::prevSegmentElement()
      {
      return segment()->lastInPrevSegments(staffIdx());
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Ambitus::accessibleInfo() const
      {
      return QObject::tr("%1; Top pitch: %2%3; Bottom pitch: %4%5").arg(Element::accessibleInfo(),
                                                               tpc2name(topTpc(), NoteSpellingType::STANDARD, NoteCaseType::AUTO, false),
                                                               QString::number(topOctave()),
                                                               tpc2name(bottomTpc(), NoteSpellingType::STANDARD, NoteCaseType::AUTO, false),
                                                               QString::number(bottomOctave()));
      }
}

