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
#include "musescoreCore.h"

namespace Ms {

//---------------------------------------------------------
//   OttavaDefault
//---------------------------------------------------------

struct OttavaDefault {
      OttavaType type;
      int shift;
      const char* name;
      };

// order is important, should be the same as OttavaType
static const OttavaDefault ottavaDefault[] = {
      { OttavaType::OTTAVA_8VA,  12,  "8va"   },
      { OttavaType::OTTAVA_8VB,  -12, "8vb"   },
      { OttavaType::OTTAVA_15MA, 24,  "15ma"  },
      { OttavaType::OTTAVA_15MB, -24, "15mb"  },
      { OttavaType::OTTAVA_22MA, 36,  "22ma"  },
      { OttavaType::OTTAVA_22MB, -36, "22mb"  }
      };

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void OttavaSegment::layout()
      {
      TextLineBaseSegment::layout();
      if (parent()) {
            qreal y;
            if (placeAbove()) {
                  y = score()->styleP(Sid::ottavaPosAbove);
                  }
            else {
                  qreal sh = ottava()->staff() ? ottava()->staff()->height() : 0;
                  y = score()->styleP(Sid::ottavaPosBelow) + sh;
                  }
            rypos() = y;
            if (autoplace()) {
                  setUserOff(QPointF());
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
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant OttavaSegment::getProperty(Pid id) const
      {
      for (const StyledProperty* spp = spanner()->styledProperties(); spp->sid != Sid::NOSTYLE; ++spp) {
            if (spp->pid == id)
                  return spanner()->getProperty(id);
            }
      switch (id) {
            case Pid::OTTAVA_TYPE:
            case Pid::NUMBERS_ONLY:
                  return spanner()->getProperty(id);
            default:
                  return TextLineBaseSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool OttavaSegment::setProperty(Pid id, const QVariant& v)
      {
      for (const StyledProperty* spp = spanner()->styledProperties(); spp->sid != Sid::NOSTYLE; ++spp) {
            if (spp->pid == id)
                  return spanner()->setProperty(id, v);
            }
      switch (id) {
            case Pid::OTTAVA_TYPE:
            case Pid::NUMBERS_ONLY:
                  return spanner()->setProperty(id, v);
            default:
                  return TextLineBaseSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant OttavaSegment::propertyDefault(Pid id) const
      {
      for (const StyledProperty* spp = spanner()->styledProperties(); spp->sid != Sid::NOSTYLE; ++spp) {
            if (spp->pid == id)
                  return spanner()->propertyDefault(id);
            }
      switch (id) {
            case Pid::OTTAVA_TYPE:
            case Pid::NUMBERS_ONLY:
                  return spanner()->propertyDefault(id);
            default:
                  return TextLineBaseSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   updateStyledProperties
//    some properties change styling
//---------------------------------------------------------

void Ottava::updateStyledProperties()
      {
      Q_ASSERT(int(OttavaType::OTTAVA_22MB) - int(OttavaType::OTTAVA_8VA) == 5);

      static const Sid ss[24] = {
            Sid::ottava8VAPlacement,
            Sid::ottava8VAnoText,
            Sid::ottava8VBPlacement,
            Sid::ottava8VBnoText,
            Sid::ottava15MAPlacement,
            Sid::ottava15MAnoText,
            Sid::ottava15MBPlacement,
            Sid::ottava15MBnoText,
            Sid::ottava22MAPlacement,
            Sid::ottava22MAnoText,
            Sid::ottava22MBPlacement,
            Sid::ottava22MBnoText,

            Sid::ottava8VAPlacement,
            Sid::ottava8VAText,
            Sid::ottava8VBPlacement,
            Sid::ottava8VBText,
            Sid::ottava15MAPlacement,
            Sid::ottava15MAText,
            Sid::ottava15MBPlacement,
            Sid::ottava15MBText,
            Sid::ottava22MAPlacement,
            Sid::ottava22MAText,
            Sid::ottava22MBPlacement,
            Sid::ottava22MBText,
            };

      // switch right substyles depending on _ottavaType and _numbersOnly

      StyledProperty* spl = _styledProperties.data();
      int idx    = int(_ottavaType) * 2 + (_numbersOnly ? 0 : 12);
      spl[0].sid = ss[idx];         // PLACEMENT
      spl[2].sid = ss[idx+1];       // BEGIN_TEXT
      spl[3].sid = ss[idx+1];       // CONTINUE_TEXT
      if (isStyled(Pid::PLACEMENT))
            spl[4].sid = score()->styleI(ss[idx]) == int(Placement::ABOVE) ? Sid::ottavaHookAbove : Sid::ottavaHookBelow;
      else
            spl[4].sid = placeAbove() ? Sid::ottavaHookAbove : Sid::ottavaHookBelow;
      styleChanged();   // this changes all styled properties with flag STYLED
      MuseScoreCore::mscoreCore->updateInspector();
      }

//---------------------------------------------------------
//   setOttavaType
//---------------------------------------------------------

void Ottava::setOttavaType(OttavaType val)
      {
      _ottavaType = val;
      updateStyledProperties();
      }

//---------------------------------------------------------
//   setNumbersOnly
//---------------------------------------------------------

void Ottava::setNumbersOnly(bool val)
      {
      _numbersOnly = val;
      updateStyledProperties();
      }

//---------------------------------------------------------
//   setPlacement
//---------------------------------------------------------

void Ottava::setPlacement(Placement p)
      {
      TextLineBase::setPlacement(p);
      updateStyledProperties();
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void OttavaSegment::undoChangeProperty(Pid id, const QVariant& v, PropertyFlags ps)
      {
      if (id == Pid::OTTAVA_TYPE || id == Pid::NUMBERS_ONLY || id == Pid::PLACEMENT) {
            ScoreElement::undoChangeProperty(id, v, ps);
            ottava()->updateStyledProperties();
            }
      else {
            ScoreElement::undoChangeProperty(id, v, ps);
            }
      }

//---------------------------------------------------------
//   Ottava
//---------------------------------------------------------

Ottava::Ottava(Score* s)
   : TextLineBase(s, ElementFlag::ON_STAFF | ElementFlag::MOVABLE)
      {
      _ottavaType = OttavaType::OTTAVA_8VA;
      _styledProperties = ottavaStyle;       // make copy

      setBeginTextPlace(PlaceText::LEFT);
      setContinueTextPlace(PlaceText::LEFT);
      setEndHookType(HookType::HOOK_90);
      setLineVisible(true);

      initSubStyle(SubStyleId::OTTAVA);
      }

Ottava::Ottava(const Ottava& o)
   : TextLineBase(o)
      {
      _styledProperties = o._styledProperties;
      setOttavaType(o._ottavaType);
      _numbersOnly = o._numbersOnly;
      }

//---------------------------------------------------------
//   pitchShift
//---------------------------------------------------------

int Ottava::pitchShift() const
      {
      return ottavaDefault[int(_ottavaType)].shift;
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
      xml.tag("subtype", ottavaDefault[int(ottavaType())].name);

      for (const StyledProperty* spp = styledProperties(); spp->sid != Sid::NOSTYLE; ++spp)
            writeProperty(xml, spp->pid);

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
      updateStyledProperties();
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
                  _ottavaType = OttavaType::OTTAVA_8VA;
                  for (unsigned i = 0; i < sizeof(ottavaDefault)/sizeof(*ottavaDefault); ++i) {
                        if (s == ottavaDefault[i].name) {
                              _ottavaType = ottavaDefault[i].type;
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
                  _ottavaType = OttavaType(idx);
                  }
            }
      else  if (readStyledProperty(e, tag))
            return true;
      else if (!TextLineBase::readProperties(e)) {
            e.unknown();
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   setYoff
//    used in musicxml import
//---------------------------------------------------------

void Ottava::setYoff(qreal val)
      {
      rUserYoffset() += val * spatium() - score()->styleP(placeAbove() ? Sid::ottavaPosAbove : Sid::ottavaPosBelow);
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Ottava::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::OTTAVA_TYPE:
                  return int(ottavaType());

            case Pid::NUMBERS_ONLY:
                  return _numbersOnly;

            default:
                  break;
            }
      return TextLineBase::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Ottava::setProperty(Pid propertyId, const QVariant& val)
      {
      switch (propertyId) {
            case Pid::OTTAVA_TYPE:
                  setOttavaType(OttavaType(val.toInt()));
                  break;

            case Pid::NUMBERS_ONLY:
                  _numbersOnly = val.toBool();
                  break;

            case Pid::SPANNER_TICKS:
                  setTicks(val.toInt());
                  staff()->updateOttava();
                  break;

            case Pid::SPANNER_TICK:
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

QVariant Ottava::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::OTTAVA_TYPE:
                  return QVariant();
            case Pid::END_HOOK_TYPE:
                  return int(HookType::HOOK_90);
            case Pid::LINE_VISIBLE:
                  return true;
            default:
                  QVariant v = ScoreElement::styledPropertyDefault(propertyId);
                  if (v.isValid())
                        return v;
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

