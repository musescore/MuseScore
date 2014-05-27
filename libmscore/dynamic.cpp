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

#if 1

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
      {  0,   true,  "sfp",    "<sym>dynamicSforzando</sym><sym>dynamicForte</sym>"},
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

// variant with precomposed symbols, available only in bravura:

#else
static Dyn dynList[] = {
      // dynamic:
      {  -1,  true,  "other-dynamics", ""     },
      {   1,  false, "pppppp", "<sym>dynamicPPPPPP</sym>" },
      {   5,  false, "ppppp",  "<sym>dynamicPPPPP</sym>" },
      {  10,  false, "pppp",   "<sym>dynamicPPPP</sym>" },
      {  16,  false, "ppp",    "<sym>dynamicPPP</sym>" },
      {  33,  false, "pp",     "<sym>dynamicPP</sym>" },
      {  49,  false, "p",      "<sym>dynamicPiano</sym>" },
      {  64,  false, "mp",     "<sym>dynamicMP</sym>" },
      {  80,  false, "mf",     "<sym>dynamicMF</sym>" },
      {  96,  false, "f",      "<sym>dynamicForte</sym>" },
      { 112,  false, "ff",     "<sym>dynamicFF</sym>" },
      { 126,  false, "fff",    "<sym>dynamicFFF</sym>" },
      { 127,  false, "ffff",   "<sym>dynamicFFFF</sym>" },
      { 127,  false, "fffff",  "<sym>dynamicFFFFF</sym>" },
      { 127,  false, "ffffff", "<sym>dynamicFFFFFF</sym>" },

      // accents:
      {  0,   true,  "fp",     "<sym>dynamicFortePiano</sym>" },
      {  0,   true,  "sf",     "<sym>dynamicSforzando1</sym>" },
      {  0,   true,  "sfz",    "<sym>dynamicSforzato</sym>" },
      {  0,   true,  "sff",    "<sym>dynamicSforzando</sym><sym>dynamicFF</sym>" },
      {  0,   true,  "sffz",   "<sym>dynamicSforzatoFF</sym>" },
      {  0,   true,  "sfp",    "<sym>dynamicSforzandoPiano</sym>" },
      {  0,   true,  "sfpp",   "<sym>dynamicSforzandoPianissimo</sym>" },
      {  0,   true,  "rfz",    "<sym>dynamicRinforzando2</sym>" },
      {  0,   true,  "rf",     "<sym>dynamicRinforzando1</sym>" },
      {  0,   true,  "fz",     "<sym>dynamicForzando</sym>" },
      {  0,   true,  "m",      "<sym>dynamicMezzo</sym>" },
      {  0,   true,  "r",      "<sym>dynamicRinforzando</sym>" },
      {  0,   true,  "s",      "<sym>dynamicSforzando</sym>" },
      {  0,   true,  "z",      "<sym>dynamicZ</sym>" },
      {  0,   true,  "n",      "<sym>dynamicNiente</sym>" }
      };
#endif

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

Dynamic::Dynamic(Score* s)
   : Text(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      _velocity = -1;
      _dynRange = DynamicRange::PART;
      setTextStyleType(TEXT_STYLE_DYNAMICS);
      _dynamicType  = DynamicType::OTHER;
      }

Dynamic::Dynamic(const Dynamic& d)
   : Text(d)
      {
      _dynamicType = d._dynamicType;
      _velocity    = d._velocity;
      _dynRange    = d._dynRange;
      }

//---------------------------------------------------------
//   setVelocity
//---------------------------------------------------------

void Dynamic::setVelocity(int v)
      {
      _velocity = v;
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
      xml.stag("Dynamic");
      xml.tag("subtype", dynamicTypeName());
      writeProperty(xml, P_ID::VELOCITY);
      writeProperty(xml, P_ID::DYNAMIC_RANGE);
      Text::writeProperties(xml, dynamicType() == DynamicType::OTHER);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Dynamic::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag = e.name();
            if (tag == "subtype") {
                  setDynamicType(e.readElementText());
                  }
            else if (tag == "velocity")
                  _velocity = e.readInt();
            else if (tag == "dynType")
                  _dynRange = DynamicRange(e.readInt());
            else if (!Text::readProperties(e))
                  e.unknown();
            }
      if (textStyleType() == TEXT_STYLE_DEFAULT)
            setTextStyleType(TEXT_STYLE_DYNAMICS);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Dynamic::layout()
      {
      if (!readPos().isNull()) {
            if (score()->mscVersion() < 118) {
                  setReadPos(QPointF());
                  // hack: 1.2 boundingBoxes are a bit wider which results
                  // in symbols moved right
                  setUserXoffset(userOff().x() - spatium() * .6);
                  }
            }
      setPos(textStyle().offset(spatium()));
      Text::layout1();

      Segment* s = segment();
      if (!s)
            return;
      for (int voice = 0; voice < VOICES; ++voice) {
            int t = (track() & ~0x3) + voice;
            Chord* c = static_cast<Chord*>(s->element(t));
            if (!c)
                  continue;
            if (c->type() == ElementType::CHORD) {
                  qreal noteHeadWidth = score()->noteHeadWidth() * c->mag();
                  if (c->stem() && !c->up())  // stem down
                        rxpos() += noteHeadWidth * .25;  // center on stem + optical correction
                  else
                        rxpos() += noteHeadWidth * .5;   // center on note head
                  }
            else
                  rxpos() += c->width() * .5;
            break;
            }
      adjustReadPos();
      }

//---------------------------------------------------------
//   setDynamicType
//---------------------------------------------------------

void Dynamic::setDynamicType(const QString& tag)
      {
      int n = sizeof(dynList)/sizeof(*dynList);
      for (int i = 0; i < n; ++i) {
            if (dynList[i].tag == tag || dynList[i].text == tag) {
                  setDynamicType(DynamicType(i));
                  setText(QString::fromUtf8(dynList[i].text));
                  return;
                  }
            }
      qDebug("setDynamicType: other <%s>", qPrintable(tag));
      setDynamicType(DynamicType::OTHER);
      setText(tag);
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
      if (text() != QString::fromUtf8(dynList[int(_dynamicType)].text))
            _dynamicType = DynamicType::OTHER;
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Dynamic::reset()
      {
//      setDynamicType(getText());
      Text::reset();
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Dynamic::dragAnchor() const
      {
      qreal xp = 0.0;
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      qreal yp = measure()->system()->staffYpage(staffIdx());
      QPointF p(xp, yp);

      return QLineF(p, canvasPos());
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
            int si;
            Segment* seg = 0;
            _score->pos2measure(ed->pos, &si, 0, &seg, 0);
            if (seg && (seg != segment() || staffIdx() != si)) {
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

void Dynamic::undoSetDynRange(DynamicRange v)
      {
      score()->undoChangeProperty(this, P_ID::DYNAMIC_RANGE, int(v));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Dynamic::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
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
                  _dynRange = DynamicRange(v.toInt());
                  break;
            case P_ID::VELOCITY:
                  _velocity = v.toInt();
                  break;
            case P_ID::SUBTYPE:
                  _dynamicType = DynamicType(v.toInt());
                  break;
            default:
                  if (!Text::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Dynamic::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_ID::TEXT_STYLE_TYPE: return TEXT_STYLE_DYNAMICS;
            case P_ID::DYNAMIC_RANGE:   return int(DynamicRange::PART);
            case P_ID::VELOCITY:        return -1;
            default:                return Text::propertyDefault(id);
            }
      }

}

