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
      };

// variant with ligatures, works for both emmentaler and bravura:

static Dyn dynList[] = {
      // dynamic:
      {  -1,  true,  "other-dynamics", "" },
      {   1,  false, "pppppp", "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
      {   5,  false, "ppppp",  "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
      {  10,  false, "pppp",   "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
      {  16,  false, "ppp",    "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
      {  33,  false, "pp",     "<sym>dynamicPiano</sym><sym>dynamicPiano</sym>" },
      {  49,  false, "p",      "<sym>dynamicPiano</sym>" },
      {  64,  false, "mp",     "<sym>dynamicMezzo</sym><sym>dynamicPiano</sym>" },
      {  80,  false, "mf",     "<sym>dynamicMezzo</sym><sym>dynamicForte</sym>" },
      {  96,  false, "f",      "<sym>dynamicForte</sym>" },
      { 112,  false, "ff",     "<sym>dynamicForte</sym><sym>dynamicForte</sym>" },
      { 126,  false, "fff",    "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
      { 127,  false, "ffff",   "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
      { 127,  false, "fffff",  "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },
      { 127,  false, "ffffff", "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>" },

      // accents:
      {  0,   true,  "fp",     "<sym>dynamicForte</sym><sym>dynamicPiano</sym>"},
      {  0,   true,  "sf",     "<sym>dynamicSforzando</sym><sym>dynamicForte</sym>"},
      {  0,   true,  "sfz",    "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>"},
      {  0,   true,  "sff",    "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>"},
      {  0,   true,  "sffz",   "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>"},
      {  0,   true,  "sfp",    "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym>"},
      {  0,   true,  "sfpp",   "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>"},
      {  0,   true,  "rfz",    "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>"},
      {  0,   true,  "rf",     "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym>"},
      {  0,   true,  "fz",     "<sym>dynamicForte</sym><sym>dynamicZ</sym>"},
      {  0,   true,  "m",      "<sym>dynamicMezzo</sym>"},
      {  0,   true,  "r",      "<sym>dynamicRinforzando</sym>"},
      {  0,   true,  "s",      "<sym>dynamicSforzando</sym>"},
      {  0,   true,  "z",      "<sym>dynamicZ</sym>"},
      {  0,   true,  "n",      "<sym>dynamicNiente</sym>" }
      };

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Score* s)
   : Text(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      _velocity = -1;
      _dynRange = Range::PART;
      setTextStyleType(TextStyleType::DYNAMICS);
      _dynamicType  = Type::OTHER;
      }

Dynamic::Dynamic(const Dynamic& d)
   : Text(d)
      {
      _dynamicType = d._dynamicType;
      _velocity    = d._velocity;
      _dynRange    = d._dynRange;
      }

//---------------------------------------------------------
//   velocity
//---------------------------------------------------------

int Dynamic::velocity() const
      {
      return _velocity <= 0 ? dynList[int(dynamicType())].velocity : _velocity;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Dynamic::write(Xml& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag("Dynamic");
      xml.tag("subtype", dynamicTypeName());
      writeProperty(xml, P_ID::VELOCITY);
      writeProperty(xml, P_ID::DYNAMIC_RANGE);
      Text::writeProperties(xml, dynamicType() == Type::OTHER);
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
            else if (!Text::readProperties(e))
                  e.unknown();
            }
      if (textStyleType() == TextStyleType::DEFAULT)
            setTextStyleType(TextStyleType::DYNAMICS);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Dynamic::layout()
      {
      if (autoplace())
            setUserOff(QPointF());

      QPointF p(textStyle().offset(spatium()));
      if (placeAbove())
            p.ry() = staff()->height() - p.ry() + lineHeight();
      setPos(p);
      Text::layout1();

      Segment* s = segment();
      if (s) {
            int t = track() & ~0x3;
            for (int voice = 0; voice < VOICES; ++voice) {
                  Element* e = s->element(t + voice);
                  if (!e)
                        continue;
                  if (e->isChord()) {
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
      adjustReadPos();
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

      qreal minDistance = score()->styleP(StyleIdx::dynamicsMinDistance);
      Shape s1          = s->staffShape(staffIdx()).translated(s->pos());
      Shape s2          = shape().translated(s->pos());

      if (placeAbove()) {
            qreal d = s2.minVerticalDistance(s1);
            if (d > -minDistance)
                  rUserYoffset() = -d - minDistance;
            }
      else {
            qreal d = s1.minVerticalDistance(s2);
            if (d > -minDistance)
                  rUserYoffset() = d + minDistance;
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

QString Dynamic::dynamicTypeName() const
      {
      return dynList[int(dynamicType())].tag;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Dynamic::startEdit(MuseScoreView* v, const QPointF& p)
      {
      Text::startEdit(v, p);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Dynamic::endEdit()
      {
      Text::endEdit();
      if (xmlText() != QString::fromUtf8(dynList[int(_dynamicType)].text))
            _dynamicType = Type::OTHER;
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Dynamic::reset()
      {
      Text::reset();
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF Dynamic::drag(EditData* ed)
      {
      QRectF f = Element::drag(ed);

      //
      // move anchor
      //
      Qt::KeyboardModifiers km = qApp->keyboardModifiers();
      if (km != (Qt::ShiftModifier | Qt::ControlModifier)) {
            int si       = staffIdx();
            Segment* seg = segment();
            score()->dragPosition(ed->pos, &si, &seg);
            if (seg != segment() || staffIdx() != si) {
                  QPointF pos1(canvasPos());
                  score()->undo(new ChangeParent(this, seg, si));
                  setUserOff(QPointF());
                  layout();
                  QPointF pos2(canvasPos());
                  setUserOff(pos1 - pos2);
                  ed->startMove = pos2;
                  }
            }
      return f;
      }

//---------------------------------------------------------
//   undoSetDynRange
//---------------------------------------------------------

void Dynamic::undoSetDynRange(Range v)
      {
      undoChangeProperty(P_ID::DYNAMIC_RANGE, int(v));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Dynamic::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::DYNAMIC_RANGE:     return int(_dynRange);
            case P_ID::VELOCITY:          return velocity();
            case P_ID::SUBTYPE:           return int(_dynamicType);
            default:
                  return Text::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Dynamic::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::DYNAMIC_RANGE:
                  _dynRange = Range(v.toInt());
                  break;
            case P_ID::VELOCITY:
                  _velocity = v.toInt();
                  break;
            case P_ID::SUBTYPE:
                  _dynamicType = Type(v.toInt());
                  break;
            default:
                  if (!Text::setProperty(propertyId, v))
                        return false;
                  break;
            }
      triggerLayout();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Dynamic::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_ID::TEXT_STYLE_TYPE:
                  return int(TextStyleType::DYNAMICS);
            case P_ID::DYNAMIC_RANGE:
                  return int(Range::PART);
            case P_ID::VELOCITY:
                  return -1;
            default:
                  return Text::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Dynamic::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(this->dynamicTypeName());
      }

}

