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

#include "tremolo.h"
#include "score.h"
#include "staff.h"
#include "style.h"
#include "chord.h"
#include "note.h"
#include "measure.h"
#include "segment.h"
#include "stem.h"
#include "sym.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   tremoloStyle
//---------------------------------------------------------

static const ElementStyle tremoloStyle {
      { Sid::tremoloStyle, Pid::TREMOLO_STYLE }
      };

//---------------------------------------------------------
//   Tremolo
//---------------------------------------------------------

static const char* tremoloName[] = {
      QT_TRANSLATE_NOOP("Tremolo", "Eighth through stem"),
      QT_TRANSLATE_NOOP("Tremolo", "16th through stem"),
      QT_TRANSLATE_NOOP("Tremolo", "32nd through stem"),
      QT_TRANSLATE_NOOP("Tremolo", "64th through stem"),
      QT_TRANSLATE_NOOP("Tremolo", "Buzz roll"),
      QT_TRANSLATE_NOOP("Tremolo", "Eighth between notes"),
      QT_TRANSLATE_NOOP("Tremolo", "16th between notes"),
      QT_TRANSLATE_NOOP("Tremolo", "32nd between notes"),
      QT_TRANSLATE_NOOP("Tremolo", "64th between notes")
      };

Tremolo::Tremolo(Score* score)
   : Element(score, ElementFlag::MOVABLE)
      {
      initElementStyle(&tremoloStyle);
      }

Tremolo::Tremolo(const Tremolo& t)
   : Element(t)
      {
      setTremoloType(t.tremoloType());
      _chord1       = t.chord1();
      _chord2       = t.chord2();
      _durationType = t._durationType;
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Tremolo::mag() const
      {
      return parent() ? parent()->mag() : 1.0;
      }

//---------------------------------------------------------
//   minHeight
//---------------------------------------------------------

qreal Tremolo::minHeight() const
      {
      const qreal sw = score()->styleS(Sid::tremoloStrokeWidth).val() * mag();
      const qreal td = score()->styleS(Sid::tremoloDistance).val() * mag();
      return (lines() - 1) * td + sw;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Tremolo::draw(QPainter* painter) const
      {
      if (isBuzzRoll()) {
            painter->setPen(curColor());
            drawSymbol(SymId::buzzRoll, painter);
            }
      else {
            painter->setBrush(QBrush(curColor()));
            painter->setPen(Qt::NoPen);
            painter->drawPath(path);
            }
      // for palette
      if (!parent() && !twoNotes()) {
            qreal x = 0.0; // bbox().width() * .25;
            QPen pen(curColor(), point(score()->styleS(Sid::stemWidth)));
            painter->setPen(pen);
            const qreal sp = spatium();
            if (isBuzzRoll())
                  painter->drawLine(QLineF(x, -sp, x, bbox().bottom() + sp));
            else
                  painter->drawLine(QLineF(x, -sp * .5, x, path.boundingRect().height() + sp));
            }
      }

//---------------------------------------------------------
//   setTremoloType
//---------------------------------------------------------

void Tremolo::setTremoloType(TremoloType t)
      {
      _tremoloType = t;
      switch (tremoloType()) {
            case TremoloType::R16:
            case TremoloType::C16:
                  _lines = 2;
                  break;
            case TremoloType::R32:
            case TremoloType::C32:
                  _lines = 3;
                  break;
            case TremoloType::R64:
            case TremoloType::C64:
                  _lines = 4;
                  break;
            default:
                  _lines = 1;
                  break;
            }
      computeShape();
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Tremolo::spatiumChanged(qreal oldValue, qreal newValue)
      {
      Element::spatiumChanged(oldValue, newValue);
      computeShape();
      }

//---------------------------------------------------------
//   localSpatiumChanged
//    the scale of a staff changed
//---------------------------------------------------------

void Tremolo::localSpatiumChanged(qreal oldValue, qreal newValue)
      {
      Element::localSpatiumChanged(oldValue, newValue);
      computeShape();
      }

//---------------------------------------------------------
//   styleChanged
//    the scale of a staff changed
//---------------------------------------------------------

void Tremolo::styleChanged()
      {
      Element::styleChanged();
      computeShape();
      }

//---------------------------------------------------------
//   basePath
//---------------------------------------------------------

QPainterPath Tremolo::basePath() const
      {
      if (isBuzzRoll())
            return QPainterPath();

      const qreal sp = spatium() * mag();

      qreal w2  = sp * score()->styleS(Sid::tremoloWidth).val() * .5;
      qreal nw2 = w2 * score()->styleD(Sid::tremoloStrokeLengthMultiplier);
      qreal lw  = sp * score()->styleS(Sid::tremoloStrokeWidth).val();
      qreal td  = sp * score()->styleS(Sid::tremoloDistance).val();

      QPainterPath ppath;
      
      // first line
      if (parent() && twoNotes() && (_style == TremoloStyle::DEFAULT))
            ppath.addRect(-nw2, 0.0, 2.0 * nw2, lw);
      else
            ppath.addRect(-w2, 0.0, 2.0 * w2, lw);

      qreal ty = td;

      // other lines
      for (int i = 1; i < _lines; i++) {
            if (parent() && twoNotes() && (_style != TremoloStyle::TRADITIONAL))
                  ppath.addRect(-nw2, ty, 2.0 * nw2, lw);
            else
                  ppath.addRect(-w2, ty, 2.0 * w2, lw);
            ty += td;
            }

      if (!parent() || !twoNotes()) {
            // for the palette or for one-note tremolos
            QTransform shearTransform;
            shearTransform.shear(0.0, -(lw / 2.0) / w2);
            ppath = shearTransform.map(ppath);
            }

      return ppath;
      }

//---------------------------------------------------------
//   computeShape
//---------------------------------------------------------

void Tremolo::computeShape()
      {
      if (parent() && twoNotes())
            return; // cannot compute shape here, should be done at layout stage

      if (isBuzzRoll())
            setbbox(symBbox(SymId::buzzRoll));
      else {
            path = basePath();
            setbbox(path.boundingRect());
            }
      }

//---------------------------------------------------------
//   layoutOneNoteTremolo
//---------------------------------------------------------

void Tremolo::layoutOneNoteTremolo(qreal x, qreal y, qreal spatium)
      {
      Q_ASSERT(!twoNotes());

      bool up = chord()->up();
      int line = up ? chord()->upLine() : chord()->downLine();

      qreal t = 0.0;
      // nearest distance between note and tremolo stroke should be no less than 3.0
      if (chord()->hook() || chord()->beam()) {
            t = up ? -3.0 * mag() - 2.0 * minHeight() : 3.0 * mag();
            }
      else {
            const qreal offset = 2.0 * score()->styleS(Sid::tremoloStrokeWidth).val();

            if (!up && !(line & 1)) // stem is down; even line
                  t = qMax((4.0 + offset) * mag() - 2.0 * minHeight(), 3.0 * mag());
            else if (!up && (line & 1)) // stem is down; odd line
                  t = qMax(5.0 * mag() - 2.0 * minHeight(), 3.0 * mag());
            else if (up && !(line & 1)) // stem is up; even line
                  t = qMin(-3.0 * mag() - 2.0 * minHeight(), (-4.0 - offset) * mag());
            else /*if ( up &&  (line & 1))*/ // stem is up; odd line
                  t = qMin(-3.0 * mag() - 2.0 * minHeight(), -5.0 * mag());
            }

      qreal yLine = line + t;
      // prevent stroke from going out of staff at the top while stem direction is down
      if (!chord()->up()) {
            yLine = qMax(yLine, 0.0);
            }
      // prevent stroke from going out of staff at the bottom while stem direction is up
      else {
            qreal height = isBuzzRoll() ? 0 : minHeight();
            yLine = qMin(yLine, (staff()->lines(tick()) - 1) * 2 - 2.0 * height);
            }

      y = yLine * .5 * spatium;

      setPos(x, y);
      }

extern std::pair<qreal, qreal> extendedStemLenWithTwoNoteTremolo(Tremolo*, qreal, qreal);

//---------------------------------------------------------
//   layoutTwoNotesTremolo
//---------------------------------------------------------

void Tremolo::layoutTwoNotesTremolo(qreal x, qreal y, qreal h, qreal spatium)
      {
      const bool defaultStyle = (!customStyleApplicable()) || (_style == TremoloStyle::DEFAULT);
      const bool isTraditionalAlternate = (_style == TremoloStyle::TRADITIONAL_ALTERNATE);

      //---------------------------------------------------
      //   Step 1: Calculate the position of the tremolo (x, y)
      //---------------------------------------------------

      y += (h - bbox().height()) * .5;

#if 0 // Needs to be done earlier, see connectTremolo in layout.cpp
      Segment* s = _chord1->segment()->next();
      while (s) {
            if (s->element(track()) && (s->element(track())->isChord()))
                  break;
            s = s->next();
            }
      if (s == 0) {
            qDebug("no second note of tremolo found");
            return;
            }

      _chord2 = toChord(s->element(track()));
      _chord2->setTremolo(this);
#endif

      Stem* stem1 = _chord1->stem();
      Stem* stem2 = _chord2->stem();

      // compute the y coordinates of the tips of the stems
      qreal y1, y2;
      qreal firstChordStaffY;

      if (stem2 && stem1) {
            // stemPageYOffset variable is used for the case when the first
            // chord is cross-staff
            firstChordStaffY = stem1->pagePos().y() - stem1->y();  // y coordinate of the staff of the first chord
            y1 = stem1->y() + stem1->p2().y();
            y2 = stem2->pagePos().y() - firstChordStaffY + stem2->p2().y();  // ->p2().y() is better than ->stemLen()
            }
      else {
            firstChordStaffY = _chord1->pagePos().y() - _chord1->y();  // y coordinate of the staff of the first chord
            const std::pair<qreal, qreal> extendedLen 
               = extendedStemLenWithTwoNoteTremolo(this, _chord1->defaultStemLength(), _chord2->defaultStemLength());
            y1 = _chord1->stemPos().y() - firstChordStaffY + extendedLen.first;
            y2 = _chord2->stemPos().y() - firstChordStaffY + extendedLen.second;
            }

      qreal lw = spatium * score()->styleS(Sid::tremoloStrokeWidth).val();
      if (_chord1->beams() == 0 && _chord2->beams() == 0) {
            // improve the case when one stem is up and another is down
            if (defaultStyle && _chord1->up() != _chord2->up() && !crossStaffBeamBetween()) {
                  qreal meanNote1Y = .5 * (_chord1->upNote()->pagePos().y() - firstChordStaffY + _chord1->downNote()->pagePos().y() - firstChordStaffY);
                  qreal meanNote2Y = .5 * (_chord2->upNote()->pagePos().y() - firstChordStaffY + _chord2->downNote()->pagePos().y() - firstChordStaffY);
                  y1 = .5 * (y1 + meanNote1Y);
                  y2 = .5 * (y2 + meanNote2Y);
                  }
            if (!defaultStyle && _chord1->up() == _chord2->up()) {
                  y1 += _chord1->up() ? -lw / 2.0 : lw / 2.0;
                  y2 += _chord1->up() ? -lw / 2.0 : lw / 2.0;
                  }
            }

      y = (y1 + y2) * .5;
      if (!_chord1->up()) {
            y -= isTraditionalAlternate ? lw * .5 : path.boundingRect().height() * .5;
            }
      if (!_chord2->up()) {
            y -= isTraditionalAlternate ? lw * .5 : path.boundingRect().height() * .5;
            }

      // compute the x coordinates of
      // the inner edge of the stems (default beam style)
      // the outer edge of the stems (non-default beam style)
      qreal x2 = _chord2->stemPosBeam().x();
      if (stem2) {
            if (defaultStyle && _chord2->up())
                  x2 -= stem2->lineWidth();
            else if (!defaultStyle && !_chord2->up())
                  x2 += stem2->lineWidth();
            }
      qreal x1 = _chord1->stemPosBeam().x();
      if (stem1) {
            if (defaultStyle && !_chord1->up())
                  x1 += stem1->lineWidth();
            else if (!defaultStyle && _chord1->up())
                  x1 -= stem1->lineWidth();
            }

      x = (x1 + x2) * .5 - _chord1->pagePos().x();

      //---------------------------------------------------
      //   Step 2: Stretch the tremolo strokes horizontally
      //    from the form of a one-note tremolo (done in basePath())
      //    to that of a two-note tremolo according to the distance between the two chords
      //---------------------------------------------------

      QTransform xScaleTransform;
      const qreal H_MULTIPLIER = score()->styleD(Sid::tremoloStrokeLengthMultiplier);
      // TODO const qreal MAX_H_LENGTH = spatium * score()->styleS(Sid::tremoloBeamLengthMultiplier).val();
      const qreal MAX_H_LENGTH = spatium * 12.0;

      const qreal defaultLength = qMin(H_MULTIPLIER * (x2 - x1), MAX_H_LENGTH);
      qreal xScaleFactor = defaultStyle ? defaultLength / H_MULTIPLIER : (x2 - x1);
      const qreal w2 = spatium * score()->styleS(Sid::tremoloWidth).val() * .5;
      xScaleFactor /= (2.0 * w2);

      xScaleTransform.scale(xScaleFactor, 1.0);
      path = xScaleTransform.map(path);

      //---------------------------------------------------
      //   Step 3: Calculate the adjustment of the position of the tremolo
      //    if the chords are connected by a beam so as not to collide with it
      //---------------------------------------------------

      qreal beamYOffset = 0.0;

      if (_chord1->beams() == _chord2->beams() && _chord1->beam()) {
            int beams = _chord1->beams();
            qreal beamHalfLineWidth = point(score()->styleS(Sid::beamWidth)) * .5 * mag();
            beamYOffset = beams * _chord1->beam()->beamDist() - beamHalfLineWidth;
            if (_chord1->up() != _chord2->up()) {  // cross-staff
                  beamYOffset = 2 * beamYOffset + beamHalfLineWidth;
                  }
            else if (!_chord1->up() && !_chord2->up()) {
                  beamYOffset = -beamYOffset;
                  }
            }

      //---------------------------------------------------
      //   Step 4: Tilt the tremolo strokes according to the stems of the chords
      //---------------------------------------------------

      QTransform shearTransform;
      qreal dy = y2 - y1;
      qreal dx = x2 - x1;
      if (_chord1->beams() == 0 && _chord2->beams() == 0) {
            if (_chord1->up() && !_chord2->up()) {
                  dy -= isTraditionalAlternate ? lw : path.boundingRect().height();
                  if (!defaultStyle)
                        dy += lw;
                  }
            else if (!_chord1->up() && _chord2->up()) {
                  dy += isTraditionalAlternate ? lw : path.boundingRect().height();
                  if (!defaultStyle)
                        dy -= lw;
                  }
            }
      // Make tremolo strokes less steep if two chords have the opposite stem directions,
      // except for two cases:
      // 1. The tremolo doesn't have the default style.
      // In this case tremolo strokes should attach to the ends of both stems, so no adjustment needed;
      // 2. The chords are on different staves and the tremolo is between them.
      // The layout should be improved by extending both stems, so changes are not needed here.
      if (_chord1->up() != _chord2->up() && defaultStyle && !crossStaffBeamBetween())
            dy = qMin(qMax(dy, -1.0 * spatium / defaultLength * dx), 1.0 * spatium / defaultLength * dx);
      qreal ds = dy / dx;
      shearTransform.shear(0.0, ds);
      path = shearTransform.map(path);

      //---------------------------------------------------
      //   Step 5: Flip the tremolo strokes if necessary
      //    By default, a TRADITIONAL_ALTERNATE tremolo has its attached-to-stem stroke be above other strokes,
      //    see basePath().
      //    But if both chords have stems facing down,
      //    the tremolo should be flipped to have the attached-to-stem stroke be below other strokes.
      //---------------------------------------------------

      if (isTraditionalAlternate && !_chord1->up() && !_chord2->up()) {
            QTransform rotateTransform;
            rotateTransform.translate(0.0, lw * .5);
            rotateTransform.rotate(180);
            rotateTransform.translate(0.0, -lw * .5);
            path = rotateTransform.map(path);
            }

      setbbox(path.boundingRect());
      setPos(x, y + beamYOffset);
      }

      
//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tremolo::layout()
      {
      path = basePath();

      _chord1 = toChord(parent());
      if (!_chord1) {
            // palette
            if (!isBuzzRoll()) {
                  const QRectF box = path.boundingRect();
                  addbbox(QRectF(box.x(), box.bottom(), box.width(), spatium()));
                  }
            return;
            }

      Note* anchor1 = _chord1->upNote();
      Stem* stem    = _chord1->stem();
      qreal x, y, h;
      if (stem) {
            x  = stem->pos().x();
            y  = stem->pos().y();
            h  = stem->stemLen();
            }
      else {
            // center tremolo above note
            x = anchor1->x() + anchor1->headWidth() * .5;
            y = anchor1->y();
            h = 2.0 * spatium() + bbox().height();
            if (anchor1->line() > 4)
                  h *= -1;
            }

      if (twoNotes())
            layoutTwoNotesTremolo(x, y, h, spatium());
      else
            layoutOneNoteTremolo(x, y, spatium());
      }

//---------------------------------------------------------
//   crossStaffBeamBetween
//    Return true if tremolo is two-note cross-staff and beams between staves
//---------------------------------------------------------

bool Tremolo::crossStaffBeamBetween() const
      {
      if (!twoNotes())
            return false;

      return ((_chord1->staffMove() > _chord2->staffMove()) && _chord1->up() && !_chord2->up())
         ||  ((_chord1->staffMove() < _chord2->staffMove()) && !_chord1->up() && _chord2->up());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tremolo::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);
      writeProperty(xml, Pid::TREMOLO_TYPE);
      writeProperty(xml, Pid::TREMOLO_STYLE);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tremolo::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setTremoloType(e.readElementText());
            // Style needs special handling other than readStyledProperty()
            // to avoid calling customStyleApplicable() in setProperty(),
            // which cannot be called now because durationType() isn't defined yet.
            else if (tag == "strokeStyle") {
                  setStyle(TremoloStyle(e.readInt()));
                  setPropertyFlags(Pid::TREMOLO_STYLE, PropertyFlags::UNSTYLED);
                  }
            else if (readStyledProperty(e, tag))
                  ;
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   tremoloTypeName
//---------------------------------------------------------

QString Tremolo::tremoloTypeName() const
      {
      return type2name(tremoloType());
      }

//---------------------------------------------------------
//   setTremoloType
//---------------------------------------------------------

void Tremolo::setTremoloType(const QString& s)
      {
      setTremoloType(name2Type(s));
      }

//---------------------------------------------------------
//   type2Name
//---------------------------------------------------------

QString Tremolo::type2name(TremoloType t)
      {
      switch(t) {
            case TremoloType::R8:  return QString("r8");
            case TremoloType::R16: return QString("r16");
            case TremoloType::R32: return QString("r32");
            case TremoloType::R64: return QString("r64");
            case TremoloType::C8:  return QString("c8");
            case TremoloType::C16: return QString("c16");
            case TremoloType::C32: return QString("c32");
            case TremoloType::C64: return QString("c64");
            case TremoloType::BUZZ_ROLL: return QString("buzzroll");
            default:
                  break;
            }
      return QString("??");
      }


//---------------------------------------------------------
//   nameToType
//---------------------------------------------------------

TremoloType Tremolo::name2Type(const QString& s)
      {
      TremoloType t = TremoloType::INVALID_TREMOLO;
      if (s == "r8")
            t = TremoloType::R8;
      else if (s == "r16")
            t = TremoloType::R16;
      else if (s == "r32")
            t = TremoloType::R32;
      else if (s == "r64")
            t = TremoloType::R64;
      else if (s == "c8")
            t = TremoloType::C8;
      else if (s == "c16")
            t = TremoloType::C16;
      else if (s == "c32")
            t = TremoloType::C32;
      else if (s == "c64")
            t = TremoloType::C64;
      else if (s == "buzzroll")
            t = TremoloType::BUZZ_ROLL;
      return t;
      }

//---------------------------------------------------------
//   tremoloLen
//---------------------------------------------------------

Fraction Tremolo::tremoloLen() const
      {
      Fraction f;
      switch(lines()) {
            case 1: f.set(1,8); break;
            case 2: f.set(1,16); break;
            case 3: f.set(1,32); break;
            case 4: f.set(1,64); break;
            }
      return f;
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString Tremolo::subtypeName() const
      {
      return qApp->translate("Tremolo", tremoloName[subtype()]);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Tremolo::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(subtypeName());
      }

//---------------------------------------------------------
//   customStyleApplicable
//---------------------------------------------------------

bool Tremolo::customStyleApplicable() const
      {
      return twoNotes()
         && (durationType().type() == TDuration::DurationType::V_HALF)
         && (staffType()->group() != StaffGroup::TAB);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Tremolo::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::TREMOLO_TYPE:
                  return int(_tremoloType);
            case Pid::TREMOLO_STYLE:
                  return int(_style);
            default:
                  break;
            }
      return Element::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Tremolo::setProperty(Pid propertyId, const QVariant& val)
      {
      switch (propertyId) {
            case Pid::TREMOLO_TYPE:
                  setTremoloType(TremoloType(val.toInt()));
                  break;
            case Pid::TREMOLO_STYLE:
                  if (customStyleApplicable())
                        setStyle(TremoloStyle(val.toInt()));
                  break;
            default:
                  return Element::setProperty(propertyId, val);
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Tremolo::propertyDefault(Pid propertyId) const 
      {
      switch (propertyId) {
            case Pid::TREMOLO_STYLE:
                  return score()->styleI(Sid::tremoloStyle);
            default:
                  return Element::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid Tremolo::propertyId(const QStringRef& name) const
      {
      if (name == "subtype")
            return Pid::TREMOLO_TYPE;
      return Element::propertyId(name);
      }

//---------------------------------------------------------
//   propertyUserValue
//---------------------------------------------------------

QString Tremolo::propertyUserValue(Pid pid) const
      {
      switch(pid) {
            case Pid::TREMOLO_TYPE:
                  return subtypeName();
            default:
                  break;
            }
      return Element::propertyUserValue(pid);
      }
}
