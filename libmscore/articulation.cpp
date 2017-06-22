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

#include "articulation.h"
#include "score.h"
#include "chordrest.h"
#include "system.h"
#include "measure.h"
#include "staff.h"
#include "stafftype.h"
#include "undo.h"
#include "page.h"
#include "barline.h"
#include "sym.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   Articulation
//---------------------------------------------------------

Articulation::Articulation(Score* s)
   : Element(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      _symId         = SymId::noSym;
      _anchor        = ArticulationAnchor::TOP_STAFF;
      _direction     = Direction::AUTO;
      _up            = true;
      _timeStretch   = 1.0;
      _ornamentStyle = MScore::OrnamentStyle::DEFAULT;
      setPlayArticulation(true);
      }

Articulation::Articulation(SymId id, Score* s)
   : Articulation(s)
      {
      setSymId(id);
      }

//---------------------------------------------------------
//   setSymId
//---------------------------------------------------------

void Articulation::setSymId(SymId id)
      {
      _symId  = id;
      _anchor = ArticulationAnchor(propertyDefault(P_ID::ARTICULATION_ANCHOR).toInt());
      }

//---------------------------------------------------------
//   setUp
//---------------------------------------------------------

void Articulation::setUp(bool val)
      {
      if (val != _up) {
            QString s = Sym::id2name(_symId);
            if (s.endsWith(_up ? "Above" : "Below")) {
                  QString s2 = s.left(s.size() - 5) + (val ? "Above" : "Below");
                  _symId = Sym::name2id(s2);
                  }
            _up = val;
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Articulation::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

extern SymId oldArticulationNames2SymId(const QString&);

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Articulation::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "subtype") {
            QString s = e.readElementText();
            SymId id = Sym::name2id(s);
            if (id == SymId::noSym)
                  id = oldArticulationNames2SymId(s);       // compatibility hack for "old" 3.0 scores
            setSymId(id);
            }
      else if (tag == "channel") {
            _channelName = e.attribute("name");
            e.readNext();
            }
      else if (tag == "anchor")
            _anchor = ArticulationAnchor(e.readInt());
      else if (readProperty(tag, e, P_ID::DIRECTION))
            ;
      else if ( tag == "ornamentStyle")
            setProperty(P_ID::ORNAMENT_STYLE, Ms::getProperty(P_ID::ORNAMENT_STYLE, e));
      else if ( tag == "play")
            setPlayArticulation(e.readBool());
      else if (tag == "timeStretch")
            _timeStretch = e.readDouble();
      else if (tag == "offset") {
            if (score()->mscVersion() > 114)
                  Element::readProperties(e);
            else
                  e.skipCurrentElement(); // ignore manual layout in older scores
            }
      else if (Element::readProperties(e))
            ;
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Articulation::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag("Articulation");
      if (!_channelName.isEmpty())
            xml.tagE(QString("channel name=\"%1\"").arg(_channelName));
      writeProperty(xml, P_ID::DIRECTION);
      xml.tag("subtype", Sym::id2name(_symId));
      writeProperty(xml, P_ID::TIME_STRETCH);
      writeProperty(xml, P_ID::PLAY);
      writeProperty(xml, P_ID::ORNAMENT_STYLE);
      Element::writeProperties(xml);
      writeProperty(xml, P_ID::ARTICULATION_ANCHOR);
      xml.etag();
      }

//---------------------------------------------------------
//   userName
//---------------------------------------------------------

QString Articulation::userName() const
      {
      return Sym::id2userName(symId());
      }

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Articulation::draw(QPainter* painter) const
      {
#if 0
      SymId sym = symId();
      ArticulationShowIn flags = articulationList[int(articulationType())].flags;
      if (staff()) {
            if (staff()->staffGroup() == StaffGroup::TAB) {
                  if (!(flags & ArticulationShowIn::TABLATURE))
                        return;
                  }
            else {
                  if (!(flags & ArticulationShowIn::PITCHED_STAFF))
                        return;
                  }
            }
#endif
      painter->setPen(curColor());
      drawSymbol(_symId, painter, QPointF(-0.5 * width(), 0.0));
      }

//---------------------------------------------------------
//   chordRest
//---------------------------------------------------------

ChordRest* Articulation::chordRest() const
      {
      if (parent() && parent()->isChordRest())
            return static_cast<ChordRest*>(parent());
      return 0;
      }

Segment* Articulation::segment() const
      {
      ChordRest* cr = chordRest();
      if (!cr)
            return 0;

      Segment* s = 0;
      if (cr->isGrace()) {
            if (cr->parent())
                  s = toSegment(cr->parent()->parent());
            }
      else
            s = toSegment(cr->parent());

      return s;
      }

Measure* Articulation::measure() const
      {
      Segment* s = segment();
      return toMeasure(s ? s->parent() : 0);
      }

System* Articulation::system() const
      {
      Measure* m = measure();
      return toSystem(m ? m->parent() : 0);
      }

Page* Articulation::page() const
      {
      System* s = system();
      return static_cast<Page*>(s ? s->parent() : 0);
      }

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void Articulation::layout()
      {
      QRectF b(symBbox(_symId));
      setbbox(b.translated(-0.5 * b.width(), 0.0));
      }

//---------------------------------------------------------
//   setDirection
//---------------------------------------------------------

void Articulation::setDirection(Direction d)
      {
      _direction = d;
//      if (d != Direction::AUTO)
//            _up = (d == Direction::UP);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Articulation::reset()
      {
#if 0
      if (_direction != Direction::AUTO)
            undoChangeProperty(P_ID::DIRECTION, Direction::AUTO);
      ArticulationAnchor a = score()->style()->articulationAnchor(int(articulationType()));
      if (_anchor != a)
            undoChangeProperty(P_ID::ARTICULATION_ANCHOR, int(a));
#endif
      Element::reset();
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Articulation::dragAnchor() const
      {
      return QLineF(canvasPos(), parent()->canvasPos());
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Articulation::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::DIRECTION:           return QVariant::fromValue<Direction>(direction());
            case P_ID::ARTICULATION_ANCHOR: return int(anchor());
            case P_ID::TIME_STRETCH:        return timeStretch();
            case P_ID::ORNAMENT_STYLE:      return int(ornamentStyle());
            case P_ID::PLAY:                return bool(playArticulation());
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Articulation::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::DIRECTION:
                  setDirection(v.value<Direction>());
                  break;
            case P_ID::ARTICULATION_ANCHOR:
                  setAnchor(ArticulationAnchor(v.toInt()));
                  break;
            case P_ID::PLAY:
                  setPlayArticulation(v.toBool());
                  break;
            case P_ID::ORNAMENT_STYLE:
                  setOrnamentStyle(MScore::OrnamentStyle(v.toInt()));
                  break;
            case P_ID::TIME_STRETCH:
                  setTimeStretch(v.toDouble());
                  score()->fixTicks();
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

QVariant Articulation::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::DIRECTION:
                  return QVariant::fromValue<Direction>(Direction::AUTO);

            case P_ID::ARTICULATION_ANCHOR:
                  switch (_symId) {
                        case SymId::articAccentAbove:
                        case SymId::articAccentBelow:
                        case SymId::articStaccatoAbove:
                        case SymId::articStaccatoBelow:
                        case SymId::articStaccatissimoAbove:
                        case SymId::articStaccatissimoBelow:
                        case SymId::articTenutoAbove:
                        case SymId::articTenutoBelow:
                        case SymId::articTenutoStaccatoAbove:
                        case SymId::articTenutoStaccatoBelow:
                        case SymId::articMarcatoAbove:
                        case SymId::articMarcatoBelow:

                        case SymId::articAccentStaccatoAbove:
                        case SymId::articAccentStaccatoBelow:
                        case SymId::articLaissezVibrerAbove:
                        case SymId::articLaissezVibrerBelow:
                        case SymId::articMarcatoStaccatoAbove:
                        case SymId::articMarcatoStaccatoBelow:
                        case SymId::articMarcatoTenutoAbove:
                        case SymId::articMarcatoTenutoBelow:
                        case SymId::articStaccatissimoStrokeAbove:
                        case SymId::articStaccatissimoStrokeBelow:
                        case SymId::articStaccatissimoWedgeAbove:
                        case SymId::articStaccatissimoWedgeBelow:
                        case SymId::articStressAbove:
                        case SymId::articStressBelow:
                        case SymId::articTenutoAccentAbove:
                        case SymId::articTenutoAccentBelow:
                        case SymId::articUnstressAbove:
                        case SymId::articUnstressBelow:

                        case SymId::articSoftAccentAbove:
                        case SymId::articSoftAccentBelow:
                        case SymId::articSoftAccentStaccatoAbove:
                        case SymId::articSoftAccentStaccatoBelow:
                        case SymId::articSoftAccentTenutoAbove:
                        case SymId::articSoftAccentTenutoBelow:
                        case SymId::articSoftAccentTenutoStaccatoAbove:
                        case SymId::articSoftAccentTenutoStaccatoBelow:

                        case SymId::guitarFadeIn:
                        case SymId::guitarFadeOut:
                        case SymId::guitarVolumeSwell:
                        case SymId::wiggleSawtooth:
                        case SymId::wiggleSawtoothWide:
                        case SymId::wiggleVibratoLargeFaster:
                        case SymId::wiggleVibratoLargeSlowest:
                              return int(ArticulationAnchor::CHORD);

                        case SymId::luteFingeringRHThumb:
                        case SymId::luteFingeringRHFirst:
                        case SymId::luteFingeringRHSecond:
                        case SymId::luteFingeringRHThird:
                              return int(ArticulationAnchor::BOTTOM_CHORD);

                        default:
                              return int(ArticulationAnchor::TOP_STAFF);
                        }

            case P_ID::TIME_STRETCH:
                  return 1.0; // articulationList[int(articulationType())].timeStretch;

            case P_ID::ORNAMENT_STYLE:
                  //return int(score()->style()->ornamentStyle(_ornamentStyle));
                  return int(MScore::OrnamentStyle::DEFAULT);

            case P_ID::PLAY:
                  return true;

            default:
                  break;
            }
      return Element::propertyDefault(propertyId);
      }

//---------------------------------------------------------
//   articulationName
//---------------------------------------------------------

const char* Articulation::articulationName() const
      {
      switch (_symId) {
            case SymId::articStaccatissimoAbove:
            case SymId::articStaccatissimoBelow:
            case SymId::articStaccatissimoStrokeAbove:
            case SymId::articStaccatissimoStrokeBelow:
            case SymId::articStaccatissimoWedgeAbove:
            case SymId::articStaccatissimoWedgeBelow:
                  return "staccatissimo";

            case SymId::articStaccatoAbove:
            case SymId::articStaccatoBelow:
            case SymId::articAccentStaccatoAbove:
            case SymId::articAccentStaccatoBelow:
            case SymId::articMarcatoStaccatoAbove:
            case SymId::articMarcatoStaccatoBelow:
                  return "staccato";

            case SymId::articTenutoStaccatoAbove:
            case SymId::articTenutoStaccatoBelow:
                  return "portato";

            case SymId::articTenutoAbove:
            case SymId::articTenutoBelow:
                  return "tenuto";

            case SymId::articMarcatoAbove:
            case SymId::articMarcatoBelow:
                  return "marcato";

            case SymId::articAccentAbove:
            case SymId::articAccentBelow:
                  return "sforzato";

            default:
                  return "---";
            }
      }


//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyFlags Articulation::propertyFlags(P_ID id) const
      {
      switch (id) {
            case P_ID::DIRECTION:
            case P_ID::TIME_STRETCH:
            case P_ID::ARTICULATION_ANCHOR:
                  return PropertyFlags::NOSTYLE;

            default:
                  break;
            }
      return Element::propertyFlags(id);
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

StyleIdx Articulation::getPropertyStyle(P_ID id) const
      {
      switch (id) {
            default:
                  break;
            }
      return StyleIdx::NOSTYLE;
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Articulation::resetProperty(P_ID id)
      {
      switch (id) {
            case P_ID::DIRECTION:
            case P_ID::TIME_STRETCH:
            case P_ID::ORNAMENT_STYLE:
                  setProperty(id, propertyDefault(id));
                  return;
            case P_ID::ARTICULATION_ANCHOR:
                  setProperty(id, propertyDefault(id));
                  return;

            default:
                  break;
            }
      Element::resetProperty(id);
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Articulation::mag() const
      {
      return parent() ? parent()->mag() * score()->styleD(StyleIdx::articulationMag): 1.0;
      }

//---------------------------------------------------------
//   isFermata
//---------------------------------------------------------

bool Articulation::isFermata() const
      {
      return _symId == SymId::fermataAbove           || _symId == SymId::fermataBelow
          || _symId == SymId::fermataLongAbove       || _symId == SymId::fermataLongBelow
          || _symId == SymId::fermataLongHenzeAbove  || _symId == SymId::fermataLongHenzeBelow
          || _symId == SymId::fermataShortAbove      || _symId == SymId::fermataShortBelow
          || _symId == SymId::fermataShortHenzeAbove || _symId == SymId::fermataShortHenzeBelow
          || _symId == SymId::fermataVeryLongAbove   || _symId == SymId::fermataVeryLongBelow
          || _symId == SymId::fermataVeryShortAbove  || _symId == SymId::fermataVeryShortBelow;      }

bool Articulation::isTenuto() const
      {
      return _symId == SymId::articTenutoAbove   || _symId == SymId::articTenutoBelow;
      }

bool Articulation::isStaccato() const
      {
      return _symId == SymId::articStaccatoAbove        || _symId == SymId::articStaccatoBelow
          || _symId == SymId::articMarcatoStaccatoAbove || _symId == SymId::articMarcatoStaccatoBelow
          || _symId == SymId::articAccentStaccatoAbove  || _symId == SymId::articAccentStaccatoBelow;
      }

bool Articulation::isAccent() const
      {
      return _symId == SymId::articAccentAbove          || _symId == SymId::articAccentBelow
          || _symId == SymId::articAccentStaccatoAbove  || _symId == SymId::articAccentStaccatoBelow;
      }

//---------------------------------------------------------
//   isLuteFingering
//---------------------------------------------------------

bool Articulation::isLuteFingering() const
      {
      return _symId == SymId::stringsThumbPosition
          || _symId == SymId::luteFingeringRHThumb
          || _symId == SymId::luteFingeringRHFirst
          || _symId == SymId::luteFingeringRHSecond
          || _symId == SymId::luteFingeringRHThird;
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Articulation::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(userName());
      }

}



