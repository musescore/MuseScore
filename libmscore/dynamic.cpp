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

#include "score.h"
#include "chord.h"
#include "dynamic.h"
#include "dynamichairpingroup.h"
#include "measure.h"
#include "mscore.h"
#include "musescoreCore.h"
#include "score.h"
#include "segment.h"
#include "style.h"
#include "system.h"
#include "utils.h"
#include "undo.h"
#include "xml.h"

namespace Ms {


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
//   findInString
//---------------------------------------------------------

// find the longest first match of dynList's dynamic text in s
// used by the MusicXML export to correctly export dynamics embedded
// in spanner begin- or endtexts
// return match's position and length and the dynamic type

int Dynamic::findInString(const QString& s, int& length, QString& type)
      {
      length = 0;
      type = "";
      int matchIndex { -1 };
      const int n = sizeof(dynList)/sizeof(*dynList);

      // for all dynamics, find their text in s
      for (int i = 0; i < n; ++i) {
            const QString dynamicText = dynList[i].text;
            const int dynamicLength = dynamicText.length();
            // note: skip entries with empty text
            if (dynamicLength > 0) {
                  const auto index = s.indexOf(dynamicText);
                  if (index >= 0) {
                        // found a match, accept it if
                        // - it is the first one
                        // - or it starts a the same index but is longer ("pp" versus "p")
                        if (matchIndex == -1 || (index == matchIndex && dynamicLength > length)) {
                              matchIndex = index;
                              length = dynamicLength;
                              type = dynList[i].tag;
                              }
                        }
                  }
            }

            return matchIndex;
      }

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Score* s)
   : TextBase(s, Tid::DYNAMICS, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
      {
      _velocity    = -1;
      _dynRange    = Range::PART;
      _dynamicType = Type::OTHER;
      _changeInVelocity = 128;
      _velChangeSpeed = Speed::NORMAL;
      initElementStyle(&dynamicsStyle);
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

int Dynamic::dynamicVelocity(Dynamic::Type t)
      {
      return dynList[int(t)].velocity;
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
      auto text = xmlText();
      auto it = std::find_if(std::begin(dynList), std::end(dynList), [text](const Ms::Dyn& d) { return text == QString::fromUtf8(d.text); });
      _dynamicType = it == std::end(dynList) ? Type::OTHER : static_cast<Type>(it - std::begin(dynList));
      for (auto* e : this->linkList())
            toDynamic(e)->_dynamicType = _dynamicType;
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Dynamic::reset()
      {
      TextBase::reset();
      }

//---------------------------------------------------------
//   getDragGroup
//---------------------------------------------------------

std::unique_ptr<ElementGroup> Dynamic::getDragGroup(std::function<bool(const Element*)> isDragged)
      {
      if (auto g = HairpinWithDynamicsDragGroup::detectFor(this, isDragged))
            return g;
      if (auto g = DynamicNearHairpinsDragGroup::detectFor(this, isDragged))
            return g;
      return TextBase::getDragGroup(isDragged);
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
            score()->dragPosition(canvasPos(), &si, &seg);
            if (seg != segment() || staffIdx() != si) {
                  const QPointF oldOffset = offset();
                  QPointF pos1(canvasPos());
                  score()->undo(new ChangeParent(this, seg, si));
                  setOffset(QPointF());
                  layout();
                  QPointF pos2(canvasPos());
                  const QPointF newOffset = pos1 - pos2;
                  setOffset(newOffset);
                  ElementEditData* eed = ed.getData(this);
                  eed->initOffset += newOffset - oldOffset;
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
      return QString("%1: %2").arg(Element::accessibleInfo(), s);
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
      return QString("%1: %2").arg(Element::accessibleInfo(), s);
      }
}

