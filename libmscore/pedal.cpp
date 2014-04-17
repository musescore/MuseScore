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

#include "pedal.h"
#include "textline.h"
#include "sym.h"
#include "xml.h"

#include "score.h"

namespace Ms {

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void PedalSegment::layout()
      {
      TextLineSegment::layout1();
      if (parent())     // for palette
            rypos() += score()->styleS(ST_pedalY).val() * spatium();
      adjustReadPos();
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool PedalSegment::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_LINE_WIDTH:
            case P_LINE_STYLE:
                  return pedal()->setProperty(id, v);
            default:
                  return TextLineSegment::setProperty(id, v);
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant PedalSegment::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_LINE_WIDTH:
            case P_LINE_STYLE:
                  return pedal()->propertyDefault(id);
            default:
                  return TextLineSegment::propertyDefault(id);
            }
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyStyle PedalSegment::propertyStyle(P_ID id) const
      {
      switch (id) {
            case P_LINE_WIDTH:
            case P_LINE_STYLE:
                  return pedal()->propertyStyle(id);

            default:
                  return TextLineSegment::propertyStyle(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void PedalSegment::resetProperty(P_ID id)
      {
      switch (id) {
            case P_LINE_WIDTH:
            case P_LINE_STYLE:
                  return pedal()->resetProperty(id);

            default:
                  return TextLineSegment::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void PedalSegment::styleChanged()
      {
      pedal()->styleChanged();
      }

//---------------------------------------------------------
//   Pedal
//---------------------------------------------------------

Pedal::Pedal(Score* s)
   : TextLine(s)
      {
      setBeginText("<sym>keyboardPedalPed</sym>");

      setEndHook(true);
      setBeginHookHeight(Spatium(-1.2));
      setEndHookHeight(Spatium(-1.2));

      setLineWidth(score()->styleS(ST_pedalLineWidth));
      lineWidthStyle = PropertyStyle::STYLED;
      setLineStyle(Qt::PenStyle(score()->styleI(ST_pedalLineStyle)));
      lineStyleStyle = PropertyStyle::STYLED;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Pedal::read(XmlReader& e)
      {
      if (score()->mscVersion() >= 110) {
            // setBeginSymbol(SymId::noSym);
            setEndHook(false);
            }
      setId(e.intAttribute("id", -1));
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")          // obsolete
                  e.skipCurrentElement();
            else if (tag == "lineWidth") {
                  setLineWidth(Spatium(e.readDouble()));
                  lineWidthStyle = PropertyStyle::UNSTYLED;
                  }
            else if (tag == "lineStyle") {
                  setLineStyle(Qt::PenStyle(e.readInt()));
                  lineStyleStyle = PropertyStyle::UNSTYLED;
                  }
            else if (!TextLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Pedal::createLineSegment()
      {
      return new PedalSegment(score());
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Pedal::setYoff(qreal val)
      {
      rUserYoffset() += (val - score()->styleS(ST_pedalY).val()) * spatium();
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Pedal::setProperty(P_ID propertyId, const QVariant& val)
      {
      switch (propertyId) {
            case P_LINE_WIDTH:
                  lineWidthStyle = PropertyStyle::UNSTYLED;
                  TextLine::setProperty(propertyId, val);
                  break;

            case P_LINE_STYLE:
                  lineStyleStyle = PropertyStyle::UNSTYLED;
                  TextLine::setProperty(propertyId, val);
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

QVariant Pedal::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_LINE_WIDTH:
                  return score()->styleS(ST_pedalLineWidth).val();

            case P_LINE_STYLE:
                  return int(score()->styleI(ST_pedalLineStyle));

            default:
                  return TextLine::propertyDefault(propertyId);
            }
      }

//---------------------------------------------------------
//   propertyStyle
//---------------------------------------------------------

PropertyStyle Pedal::propertyStyle(P_ID id) const
      {
      switch (id) {
            case P_LINE_WIDTH:
                  return lineWidthStyle;

            case P_LINE_STYLE:
                  return lineStyleStyle;

            default:
                  return TextLine::propertyStyle(id);
            }
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Pedal::resetProperty(P_ID id)
      {
      switch (id) {
            case P_LINE_WIDTH:
                  setLineWidth(score()->styleS(ST_pedalLineWidth));
                  lineWidthStyle = PropertyStyle::STYLED;
                  break;

            case P_LINE_STYLE:
                  setLineStyle(Qt::PenStyle(score()->styleI(ST_pedalLineStyle)));
                  lineStyleStyle = PropertyStyle::STYLED;
                  break;

            default:
                  return TextLine::resetProperty(id);
            }
      }

//---------------------------------------------------------
//   styleChanged
//    reset all styled values to actual style
//---------------------------------------------------------

void Pedal::styleChanged()
      {
      if (lineWidthStyle == PropertyStyle::STYLED)
            setLineWidth(score()->styleS(ST_pedalLineWidth));
      if (lineStyleStyle == PropertyStyle::STYLED)
            setLineStyle(Qt::PenStyle(score()->styleI(ST_pedalLineStyle)));
      }

}

