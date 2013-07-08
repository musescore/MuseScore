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

namespace Ms {

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void OttavaSegment::layout()
      {
      TextLineSegment::layout1();
      if (parent())     // for palette
            rypos() += score()->styleS(ST_ottavaY).val() * spatium();
      adjustReadPos();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant OttavaSegment::getProperty(P_ID id) const
      {
      switch (id) {
            case P_OTTAVA_TYPE:
                  return ottava()->getProperty(id);
            default:
                  return TextLineSegment::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool OttavaSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_OTTAVA_TYPE:
                  return ottava()->setProperty(id, v);
            default:
                  return TextLineSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant OttavaSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_OTTAVA_TYPE:
                  return ottava()->propertyDefault(id);
            default:
                  return TextLineSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   Ottava
//---------------------------------------------------------

Ottava::Ottava(Score* s)
   : TextLine(s)
      {
      _ottavaType = OttavaType(-1);
      setOttavaType(OTTAVA_8VA);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Ottava::layout()
      {
      setPos(0.0, 0.0);
      setLineWidth(score()->styleS(ST_ottavaLineWidth));
      TextLine::layout();
      }

//---------------------------------------------------------
//   setOttavaType
//---------------------------------------------------------

void Ottava::setOttavaType(OttavaType val)
      {
      if (val == _ottavaType)
            return;
      setEndHook(true);
      _ottavaType = val;

      Spatium hook(score()->styleS(ST_ottavaHook));

      const TextStyle& ts = score()->textStyle(TEXT_STYLE_OTTAVA);

      switch(val) {
            case OTTAVA_8VA:
                  setBeginText("8va", ts);
                  setContinueText("(8va)", ts);
                  setEndHookHeight(hook);
                  _pitchShift = 12;
                  break;
            case OTTAVA_15MA:
                  setBeginText("15ma", ts);
                  setContinueText("(15ma)", ts);
                  setEndHookHeight(hook);
                  _pitchShift = 24;
                  break;
            case OTTAVA_8VB:
                  setBeginText("8vb", ts);
                  setContinueText("(8vb)", ts);
                  _pitchShift = -12;
                  setEndHookHeight(-hook);
                  break;
            case OTTAVA_15MB:
                  setBeginText("15mb", ts);
                  setContinueText("(15mb)", ts);
                  _pitchShift = -24;
                  setEndHookHeight(-hook);
                  break;
            }
      foreach(SpannerSegment* s, spannerSegments()) {
            OttavaSegment* os = static_cast<OttavaSegment*>(s);
            os->clearText();
            }
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Ottava::createLineSegment()
      {
      return new OttavaSegment(score());
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Ottava::endEdit()
      {
      if (editTick != tick() || editTick2 != tick2()) {
            Staff* s = staff();
            s->pitchOffsets().remove(editTick);
            s->pitchOffsets().remove(editTick2);

            s->pitchOffsets().setPitchOffset(tick(), _pitchShift);
            s->pitchOffsets().setPitchOffset(tick2(), 0);

            score()->addLayoutFlags(LAYOUT_FIX_PITCH_VELO);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Ottava::write(Xml& xml) const
      {
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      xml.tag("subtype", ottavaType());
      TextLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Ottava::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();
      setId(e.intAttribute("id", -1));
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  setOttavaType(OttavaType(e.readInt()));
            else if (!TextLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Ottava::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_OTTAVA_TYPE:
                  return ottavaType();
            default:
                  break;
            }
      return TextLine::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Ottava::setProperty(P_ID propertyId, const QVariant& val)
      {
      switch(propertyId) {
            case P_OTTAVA_TYPE:
                  setOttavaType(OttavaType(val.toInt()));
                  break;
            default:
                  if (!TextLine::setProperty(propertyId, val))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Ottava::propertyDefault(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_OTTAVA_TYPE:
                  return 0;
            default:
                  return TextLine::propertyDefault(propertyId);
            }
      return QVariant();
      }

//---------------------------------------------------------
//   undoSetOttavaType
//---------------------------------------------------------

void Ottava::undoSetOttavaType(OttavaType val)
      {
      score()->undoChangeProperty(this, P_OTTAVA_TYPE, val);
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Ottava::setYoff(qreal val)
      {
      rUserYoffset() += (val - score()->styleS(ST_ottavaY).val()) * spatium();
      }

}

