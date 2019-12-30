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

#include "dynamic.h"
#include "xml.h"
#include "score.h"
#include "measure.h"
#include "system.h"
#include "segment.h"
#include "utils.h"
#include "style.h"
#include "mscore.h"
#include "chord.h"
#include "undo.h"
#include "sym.h"
#include "musescoreCore.h"

namespace Ms {

//-----------------------------------------------------------------------------
//   Dyn
//    see: http://en.wikipedia.org/wiki/File:Dynamic's_Note_Velocity.svg
//-----------------------------------------------------------------------------

struct Dyn {
      int velocity;      ///< associated midi velocity (0-127, -1 = none)
      bool accent;       ///< if true add velocity to current chord velocity
      const char* tag;   // name of dynamics, eg. "fff"
      const char* text;  // utf8 text of dynamic
      int changeInVelocity;
      };

// variant with ligatures, works for both emmentaler and bravura:

static Dyn dynList[] = {
      // dynamic:
      {  -1,  true,  "other-dynamics", "", 0 },
      {   1,  false, "pppppp", "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>", 0 },
      {   5,  false, "ppppp",  "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>", 0 },
      {  10,  false, "pppp",   "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>", 0 },
      {  16,  false, "ppp",    "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>", 0 },
      {  33,  false, "pp",     "<sym>dynamicPiano</sym><sym>dynamicPiano</sym>", 0 },
      {  49,  false, "p",      "<sym>dynamicPiano</sym>", 0 },
      {  64,  false, "mp",     "<sym>dynamicMezzo</sym><sym>dynamicPiano</sym>", 0 },
      {  80,  false, "mf",     "<sym>dynamicMezzo</sym><sym>dynamicForte</sym>", 0 },
      {  96,  false, "f",      "<sym>dynamicForte</sym>", 0 },
      { 112,  false, "ff",     "<sym>dynamicForte</sym><sym>dynamicForte</sym>", 0 },
      { 126,  false, "fff",    "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>", 0 },
      { 127,  false, "ffff",   "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>", 0 },
      { 127,  false, "fffff",  "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>", 0 },
      { 127,  false, "ffffff", "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>", 0 },

      // accents:
      {  96,  true,  "fp",     "<sym>dynamicForte</sym><sym>dynamicPiano</sym>", -47 },
      {  49,  true,  "pf",     "<sym>dynamicPiano</sym><sym>dynamicForte</sym>", 47 },
      {  112, true,  "sf",     "<sym>dynamicSforzando</sym><sym>dynamicForte</sym>", -18 },
      {  112, true,  "sfz",    "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>", -18 },
      {  126, true,  "sff",    "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>", -18 },
      {  126, true,  "sffz",   "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>", -18 },
      {  112, true,  "sfp",    "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym>", -47 },
      {  112, true,  "sfpp",   "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>", -79 },
      {  112, true,  "rfz",    "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>", -18 },
      {  112, true,  "rf",     "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym>", -18 },
      {  112, true,  "fz",     "<sym>dynamicForte</sym><sym>dynamicZ</sym>", -18 },
      {  96,  true,  "m",      "<sym>dynamicMezzo</sym>", -16 },
      {  112, true,  "r",      "<sym>dynamicRinforzando</sym>", -18 },
      {  112, true,  "s",      "<sym>dynamicSforzando</sym>", -18 },
      {  80,  true,  "z",      "<sym>dynamicZ</sym>", 0 },
      {  49,  true,  "n",      "<sym>dynamicNiente</sym>", -48 }
      };

//---------------------------------------------------------
//   dynamicsStyle
//---------------------------------------------------------

static const ElementStyle dynamicsStyle {
      { Sid::dynamicsPlacement, Pid::PLACEMENT },
      { Sid::dynamicsMinDistance, Pid::MIN_DISTANCE },
      };

//---------------------------------------------------------
//   changeSpeedTable
//---------------------------------------------------------

const std::vector<Dynamic::ChangeSpeedItem> Dynamic::changeSpeedTable {
      { Dynamic::Speed::NORMAL,           "normal" },
      { Dynamic::Speed::SLOW,             "slow"   },
      { Dynamic::Speed::FAST,             "fast"   },
      };

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Score* s)
   : TextBase(s, Tid::DYNAMICS, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      initElementStyle(&dynamicsStyle);
      _velocity    = -1;
      _dynRange    = Range::PART;
      _dynamicType = Type::OTHER;
      _changeInVelocity = 128;
      _velChangeSpeed = Speed::NORMAL;
      }

Dynamic::Dynamic(const Dynamic& d)
   : TextBase(d)
      {
      _dynamicType = d._dynamicType;
      _velocity    = d._velocity;
      _dynRange    = d._dynRange;
      _changeInVelocity = d._changeInVelocity;
      _velChangeSpeed = d._velChangeSpeed;
      }

//---------------------------------------------------------
//   velocity
//---------------------------------------------------------

int Dynamic::velocity() const
      {
      return _velocity <= 0 ? dynList[int(dynamicType())].velocity : _velocity;
      }

//---------------------------------------------------------
//   changeInVelocity
//---------------------------------------------------------

int Dynamic::changeInVelocity() const
      {
      return _changeInVelocity >= 128 ? dynList[int(dynamicType())].changeInVelocity : _changeInVelocity;
      }

//---------------------------------------------------------
//   setChangeInVelocity
//---------------------------------------------------------

void Dynamic::setChangeInVelocity(int val)
      {
      if (dynList[int(dynamicType())].changeInVelocity == val)
            _changeInVelocity = 128;
      else
            _changeInVelocity = val;
      }

//---------------------------------------------------------
//   velocityChangeLength
//    the time over which the velocity change occurs
//---------------------------------------------------------

Fraction Dynamic::velocityChangeLength() const
      {
      if (changeInVelocity() == 0)
            return Fraction::fromTicks(0);

      double ratio = double(score()->tempomap()->tempo(segment()->tick().ticks())) / double(Score::defaultTempo());
      double speedMult;
      switch (velChangeSpeed()) {
            case Dynamic::Speed::SLOW:
                  speedMult = 1.3;
                  break;
            case Dynamic::Speed::FAST:
                  speedMult = 0.5;
                  break;
            case Dynamic::Speed::NORMAL:
            default:
                  speedMult = 0.8;
                  break;
            }

      return Fraction::fromTicks(int(ratio * (speedMult * double(MScore::division))));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Dynamic::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(this);
      writeProperty(xml, Pid::DYNAMIC_TYPE);
      writeProperty(xml, Pid::VELOCITY);
      writeProperty(xml, Pid::DYNAMIC_RANGE);
      writeProperty(xml, Pid::VELO_CHANGE);
      writeProperty(xml, Pid::VELO_CHANGE_SPEED);
      TextBase::writeProperties(xml, dynamicType() == Type::OTHER);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Dynamic::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag = e.name();
            if (tag == "subtype")
                  setDynamicType(e.readElementText());
            else if (tag == "velocity")
                  _velocity = e.readInt();
            else if (tag == "dynType")
                  _dynRange = Range(e.readInt());
            else if (tag == "veloChange")
                  _changeInVelocity = e.readInt();
            else if (tag == "veloChangeSpeed")
                  _velChangeSpeed = nameToSpeed(e.readElementText());
            else if (!TextBase::readProperties(e))
                  e.unknown();
            }
      styleChanged();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Dynamic::layout()
      {
      TextBase::layout();

      Segment* s = segment();
      if (s) {
            int t = track() & ~0x3;
            for (int voice = 0; voice < VOICES; ++voice) {
                  Element* e = s->element(t + voice);
                  if (!e)
                        continue;
                  if (e->isChord() && (align() & Align::HCENTER)) {
                        Chord* c = toChord(e);
                        qreal noteHeadWidth = score()->noteHeadWidth() * c->mag();
                        if (c->stem() && !c->up())  // stem down
                              rxpos() += noteHeadWidth * .25;  // center on stem + optical correction
                        else
                              rxpos() += noteHeadWidth * .5;   // center on notehead
                        }
                  else
                        rxpos() += e->width() * .5;
                  break;
                  }
            }
      else
            setPos(QPointF());      // for palette
      }

//-------------------------------------------------------------------
//   doAutoplace
//
//    Move Dynamic up or down to avoid collisions with other elements.
//-------------------------------------------------------------------

void Dynamic::doAutoplace()
      {
      Segment* s = segment();
      if (!(s && autoplace()))
            return;

      qreal minDistance = score()->styleS(Sid::dynamicsMinDistance).val() * spatium();
      QRectF r          = bbox().translated(pos() + s->pos() + s->measure()->pos());
      qreal yOff = offset().y() - propertyDefault(Pid::OFFSET).toPointF().y();
      r.translate(0.0, -yOff);

      Skyline& sl       = s->measure()->system()->staff(staffIdx())->skyline();
      SkylineLine sk(!placeAbove());
      sk.add(r);

      if (placeAbove()) {
            qreal d = sk.minDistance(sl.north());
            if (d > -minDistance)
                  rypos() += -(d + minDistance);
            }
      else {
            qreal d = sl.south().minDistance(sk);
            if (d > -minDistance)
                  rypos() += d + minDistance;
            }
      }

//---------------------------------------------------------
//   setDynamicType
//---------------------------------------------------------

void Dynamic::setDynamicType(const QString& tag)
      {
      int n = sizeof(dynList)/sizeof(*dynList);
      for (int i = 0; i < n; ++i) {
            if (dynList[i].tag == tag || dynList[i].text == tag) {
                  setDynamicType(Type(i));
                  setXmlText(QString::fromUtf8(dynList[i].text));
                  return;
                  }
            }
      qDebug("setDynamicType: other <%s>", qPrintable(tag));
      setDynamicType(Type::OTHER);
      setXmlText(tag);
      }

//---------------------------------------------------------
//   dynamicTypeName
//---------------------------------------------------------

QString Dynamic::dynamicTypeName(Dynamic::Type type)
      {
      return dynList[int(type)].tag;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Dynamic::startEdit(EditData& ed)
      {
      TextBase::startEdit(ed);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Dynamic::endEdit(EditData& ed)
      {
      TextBase::endEdit(ed);
      if (xmlText() != QString::fromUtf8(dynList[int(_dynamicType)].text))
            _dynamicType = Type::OTHER;
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Dynamic::reset()
      {
      TextBase::reset();
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF Dynamic::drag(EditData& ed)
      {
      QRectF f = Element::drag(ed);

      //
      // move anchor
      //
      Qt::KeyboardModifiers km = qApp->keyboardModifiers();
      if (km != (Qt::ShiftModifier | Qt::ControlModifier)) {
            int si       = staffIdx();
            Segment* seg = segment();
            score()->dragPosition(ed.pos, &si, &seg);
            if (seg != segment() || staffIdx() != si) {
                  QPointF pos1(canvasPos());
                  score()->undo(new ChangeParent(this, seg, si));
                  setOffset(QPointF());
                  layout();
                  QPointF pos2(canvasPos());
                  setOffset(pos1 - pos2);
                  ed.startMove = pos2;
                  }
            }
      return f;
      }

//---------------------------------------------------------
//   undoSetDynRange
//---------------------------------------------------------

void Dynamic::undoSetDynRange(Range v)
      {
      undoChangeProperty(Pid::DYNAMIC_RANGE, int(v));
      }

//---------------------------------------------------------
//   speedToName
//---------------------------------------------------------

QString Dynamic::speedToName(Speed speed)
      {
      for (auto i : Dynamic::changeSpeedTable) {
            if (i.speed == speed)
                  return i.name;
            }
      qFatal("Unrecognised change speed!");
      return "none"; // silence a compiler warning
      }


//---------------------------------------------------------
//   nameToSpeed
//---------------------------------------------------------

Dynamic::Speed Dynamic::nameToSpeed(QString name)
      {
      for (auto i : Dynamic::changeSpeedTable) {
            if (i.name == name)
                  return i.speed;
            }
      return Speed::NORMAL;   // default
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Dynamic::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::DYNAMIC_TYPE:
                  return QVariant::fromValue(_dynamicType);
            case Pid::DYNAMIC_RANGE:
                  return int(_dynRange);
            case Pid::VELOCITY:
                  return velocity();
            case Pid::SUBTYPE:
                  return int(_dynamicType);
            case Pid::VELO_CHANGE:
                  return changeInVelocity();
            case Pid::VELO_CHANGE_SPEED:
                  return int(_velChangeSpeed);
            default:
                  return TextBase::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Dynamic::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::DYNAMIC_TYPE:
                  _dynamicType = v.value<Dynamic::Type>();
                  break;
            case Pid::DYNAMIC_RANGE:
                  _dynRange = Range(v.toInt());
                  break;
            case Pid::VELOCITY:
                  _velocity = v.toInt();
                  break;
            case Pid::SUBTYPE:
                  _dynamicType = Type(v.toInt());
                  break;
            case Pid::VELO_CHANGE:
                  setChangeInVelocity(v.toInt());
                  break;
            case Pid::VELO_CHANGE_SPEED:
                  _velChangeSpeed = Speed(v.toInt());
                  break;
            default:
                  if (!TextBase::setProperty(propertyId, v))
                        return false;
                  break;
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Dynamic::propertyDefault(Pid id) const
      {
      switch(id) {
            case Pid::SUB_STYLE:
                  return int(Tid::DYNAMICS);
            case Pid::DYNAMIC_RANGE:
                  return int(Range::PART);
            case Pid::VELOCITY:
                  return -1;
            case Pid::VELO_CHANGE:
                  return dynList[int(dynamicType())].changeInVelocity;
            case Pid::VELO_CHANGE_SPEED:
                  return int(Speed::NORMAL);
            default:
                  return TextBase::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid Dynamic::propertyId(const QStringRef& name) const
      {
      if (name == propertyName(Pid::DYNAMIC_TYPE))
            return Pid::DYNAMIC_TYPE;
      return TextBase::propertyId(name);
      }

//---------------------------------------------------------
//   propertyUserValue
//---------------------------------------------------------

QString Dynamic::propertyUserValue(Pid pid) const
      {
      switch(pid) {
            case Pid::DYNAMIC_TYPE:
                  return dynamicTypeName();
            default:
                  break;
            }
      return TextBase::propertyUserValue(pid);
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid Dynamic::getPropertyStyle(Pid pid) const
      {
      if (pid == Pid::OFFSET)
            return placeAbove() ? Sid::dynamicsPosAbove : Sid::dynamicsPosBelow;
      return TextBase::getPropertyStyle(pid);
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Dynamic::accessibleInfo() const
      {
      QString s;

      if (dynamicType() == Dynamic::Type::OTHER) {
            s = plainText().simplified();
            if (s.length() > 20) {
                  s.truncate(20);
                  s += "…";
                  }
            }
      else {
            s = dynamicTypeName();
            }
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(s);
      }

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

QString Dynamic::screenReaderInfo() const
      {
      QString s;

      if (dynamicType() == Dynamic::Type::OTHER)
            s = plainText().simplified();
      else {
            s = dynamicTypeName();
            }
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(s);
      }
}

