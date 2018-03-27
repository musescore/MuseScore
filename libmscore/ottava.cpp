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

#include "ottava.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "score.h"
#include "text.h"
#include "staff.h"
#include "segment.h"
#include "sym.h"

namespace Ms {

//---------------------------------------------------------
//   OttavaDefault
//---------------------------------------------------------

struct OttavaDefault {
      SymId id;
      SymId numbersOnlyId;
      QPointF offset;
      qreal  hookDirection;
      Placement place;
      int shift;
      const char* name;
      const char* numbersOnlyName;
      };

// order is important, should be the same as OttavaType
static const OttavaDefault ottavaDefault[] = {
      { SymId::ottavaAlta,        SymId::ottava,       QPointF(0.0, .7),    1.0, Placement::ABOVE,  12, "8va", "8"   },
      { SymId::ottavaBassaBa,     SymId::ottava,       QPointF(0.0, -1.0), -1.0, Placement::BELOW, -12, "8vb", "8"   },
      { SymId::quindicesimaAlta,  SymId::quindicesima, QPointF(0.0, .7),    1.0, Placement::ABOVE,  24, "15ma", "15" },
      { SymId::quindicesimaBassa, SymId::quindicesima, QPointF(0.0, -1.0), -1.0, Placement::BELOW, -24, "15mb", "15" },
      { SymId::ventiduesimaAlta,  SymId::ventiduesima, QPointF(0.0, .7),    1.0, Placement::ABOVE,  36, "22ma", "22" },
      { SymId::ventiduesimaBassa, SymId::ventiduesima, QPointF(0.0, -1.0), -1.0, Placement::BELOW, -36, "22mb", "22" }
      };

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void OttavaSegment::layout()
      {
      if (autoplace())
            setUserOff(QPointF());

      TextLineBaseSegment::layout();
      if (parent()) {
            qreal yo = score()->styleP(ottava()->placeBelow() ? StyleIdx::ottavaPosBelow : StyleIdx::ottavaPosAbove) * mag();
            rypos() += yo;
            if (autoplace()) {
                  qreal minDistance = spatium() * .7;
                  Shape s1 = shape().translated(pos());
                  if (ottava()->placeAbove()) {
                        qreal d  = system()->topDistance(staffIdx(), s1);
                        if (d > -minDistance)
                              rUserYoffset() = -d - minDistance;
                        }
                  else {
                        qreal d  = system()->bottomDistance(staffIdx(), s1);
                        if (d > -minDistance)
                              rUserYoffset() = d + minDistance;
                        }
                  }
            else
                  adjustReadPos();
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant OttavaSegment::getProperty(P_ID id) const
      {
      for (const StyledProperty* spp = spanner()->styledProperties(); spp->styleIdx != StyleIdx::NOSTYLE; ++spp) {
            if (spp->propertyIdx == id)
                  return spanner()->getProperty(id);
            }
      return TextLineBaseSegment::getProperty(id);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool OttavaSegment::setProperty(P_ID id, const QVariant& v)
      {
      for (const StyledProperty* spp = spanner()->styledProperties(); spp->styleIdx != StyleIdx::NOSTYLE; ++spp) {
            if (spp->propertyIdx == id)
                  return spanner()->setProperty(id, v);
            }
      return TextLineBaseSegment::setProperty(id, v);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant OttavaSegment::propertyDefault(P_ID id) const
      {
      for (const StyledProperty* spp = spanner()->styledProperties(); spp->styleIdx != StyleIdx::NOSTYLE; ++spp) {
            if (spp->propertyIdx == id)
                  return spanner()->propertyDefault(id);
            }
      return TextLineBaseSegment::propertyDefault(id);
      }

//---------------------------------------------------------
//   Ottava
//---------------------------------------------------------

Ottava::Ottava(Score* s)
   : TextLineBase(s, ElementFlag::ON_STAFF)
      {
      _ottavaType = OttavaType::OTTAVA_8VA;
      setBeginTextPlace(PlaceText::LEFT);
      setContinueTextPlace(PlaceText::LEFT);
      setLineVisible(true);
      initSubStyle(SubStyleId::OTTAVA);
      }

Ottava::Ottava(const Ottava& o)
   : TextLineBase(o)
      {
      setOttavaType(o._ottavaType);
      _numbersOnly = o._numbersOnly;
      _pitchShift  = o._pitchShift;
      }

//---------------------------------------------------------
//   setOttavaType
//---------------------------------------------------------

void Ottava::setOttavaType(OttavaType val)
      {
      _ottavaType = val;

      const OttavaDefault* def = &ottavaDefault[int(_ottavaType)];
      setBeginText(propertyDefault(P_ID::BEGIN_TEXT).toString());
      setContinueText(propertyDefault(P_ID::CONTINUE_TEXT).toString());

      setEndHookType(HookType::HOOK_90);
      setEndHookHeight(score()->styleS(StyleIdx::ottavaHook) * def->hookDirection);

      setPlacement(def->place);
      _pitchShift = def->shift;
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Ottava::createLineSegment()
      {
      return new OttavaSegment(score());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Ottava::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
//      writeProperty(xml, P_ID::NUMBERS_ONLY);
      xml.tag("subtype", ottavaDefault[int(ottavaType())].name);

      for (const StyledProperty* spp = styledProperties(); spp->styleIdx != StyleIdx::NOSTYLE; ++spp)
            writeProperty(xml, spp->propertyIdx);

      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Ottava::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();
      e.addSpanner(e.intAttribute("id", -1), this);
      while (e.readNextStartElement())
            readProperties(e);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Ottava::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());
      if (tag == "subtype") {
            QString s = e.readElementText();
            bool ok;
            int idx = s.toInt(&ok);
            if (!ok) {
                  idx = int(OttavaType::OTTAVA_8VA);
                  for (unsigned i = 0; i < sizeof(ottavaDefault)/sizeof(*ottavaDefault); ++i) {
                        if (s == ottavaDefault[i].name) {
                              idx = i;
                              break;
                              }
                        }
                  }
            else if (score()->mscVersion() <= 114) {
                  //subtype are now in a different order...
                  if (idx == 1)
                        idx = 2;
                  else if (idx == 2)
                        idx = 1;
                  }
            setOttavaType(OttavaType(idx));
            }
      else if (!TextLineBase::readProperties(e)) {
            e.unknown();
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   undoSetOttavaType
//---------------------------------------------------------

void Ottava::undoSetOttavaType(OttavaType val)
      {
      undoChangeProperty(P_ID::OTTAVA_TYPE, int(val));
      }

//---------------------------------------------------------
//   setYoff
//    used in musicxml import
//---------------------------------------------------------

void Ottava::setYoff(qreal val)
      {
      rUserYoffset() += val * spatium() - score()->styleP(placeAbove() ? StyleIdx::ottavaPosAbove : StyleIdx::ottavaPosBelow);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Ottava::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::OTTAVA_TYPE:
                  return int(ottavaType());
            case P_ID::NUMBERS_ONLY:
                  return _numbersOnly;
            default:
                  break;
            }
      return TextLineBase::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Ottava::setProperty(P_ID propertyId, const QVariant& val)
      {
      switch (propertyId) {
            case P_ID::OTTAVA_TYPE:
                  setOttavaType(OttavaType(val.toInt()));
                  break;

            case P_ID::PLACEMENT:
                  if (val != getProperty(propertyId)) {
                        // reverse hooks
                        // setBeginHookHeight(-beginHookHeight());
                        setEndHookHeight(-endHookHeight());
                        }
                  setPlacement(Placement(val.toInt()));
                  break;

            case P_ID::NUMBERS_ONLY:
                  setNumbersOnly(val.toBool());
                  setOttavaType(_ottavaType);
                  break;

            case P_ID::SPANNER_TICKS:
                  setTicks(val.toInt());
                  staff()->updateOttava();
                  break;

            case P_ID::SPANNER_TICK:
                  setTick(val.toInt());
                  staff()->updateOttava();
                  break;

            default:
                  if (!TextLineBase::setProperty(propertyId, val))
                        return false;
                  break;
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Ottava::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::OTTAVA_TYPE:
                  return QVariant();
            case P_ID::END_HOOK_TYPE:
                  return int(HookType::HOOK_90);
            case P_ID::PLACEMENT:
                  return int(ottavaDefault[int(_ottavaType)].place);
            case P_ID::END_HOOK_HEIGHT:
                  return score()->styleS(StyleIdx::ottavaHook) * ottavaDefault[int(_ottavaType)].hookDirection;
            case P_ID::BEGIN_TEXT:
            case P_ID::CONTINUE_TEXT: {
                  const OttavaDefault* def = &ottavaDefault[int(_ottavaType)];
                  SymId id = _numbersOnly ? def->numbersOnlyId : def->id;
                  return QString("<sym>%1</sym>").arg(Sym::id2name(id));
                  }
            default:
                  for (const StyledProperty& p : subStyle(subStyleId())) {
                        if (p.propertyIdx == propertyId)
                              return score()->styleV(p.styleIdx);
                        }
                  return getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Ottava::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(ottavaDefault[static_cast<int>(ottavaType())].name);
      }

}

