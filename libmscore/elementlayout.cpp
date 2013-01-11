//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "elementlayout.h"
#include "xml.h"
#include "element.h"
#include "text.h"
#include "mscore.h"

//---------------------------------------------------------
//   ElementLayout
//---------------------------------------------------------

ElementLayout::ElementLayout()
      {
      _align      = ALIGN_LEFT | ALIGN_BASELINE;
      _offsetType = OFFSET_SPATIUM;
      }

//---------------------------------------------------------
//   offset
//---------------------------------------------------------

QPointF ElementLayout::offset(qreal spatium) const
      {
      QPointF o(_offset);
      if (_offsetType == OFFSET_SPATIUM)
            o *= spatium;
      else
            o *= MScore::DPI;
      return o;
      }

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void ElementLayout::layout(Element* e) const
      {
      QPointF o(offset(e->spatium()));
      qreal w = 0.0;
      qreal h = 0.0;
      if (e->parent()) {
            qreal pw, ph;
            if ((e->type() == Element::MARKER || e->type() == Element::JUMP) && e->parent()->parent()) {
                  pw = e->parent()->parent()->width();      // measure width
                  ph = e->parent()->parent()->height();
                  }
            else {
                  pw = e->parent()->width();
                  ph = e->parent()->height();
                  }
            o += QPointF(_reloff.x() * pw * 0.01, _reloff.y() * ph * 0.01);
            }
      bool frameText = e->type() == Element::TEXT
         && static_cast<Text*>(e)->layoutToParentWidth() && e->parent();
      QPointF p;
      if (frameText)
            h = e->parent()->height();
      else
            w = e->width();
      if (_align & ALIGN_BOTTOM)
            p.setY(h - e->height());
      else if (_align & ALIGN_VCENTER)
            p.setY((h - e->height()) * .5);
      else if (_align & ALIGN_BASELINE)
            p.setY(-e->baseLine());
      if (!frameText) {
            if (_align & ALIGN_RIGHT)
                  p.setX(-w);
            else if (_align & ALIGN_HCENTER)
                  p.setX(-(w * .5));
            }
      e->setPos(p + o);
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void ElementLayout::writeProperties(Xml& xml) const
      {
      if (_align & ALIGN_HCENTER)
            xml.tag("halign", "center");
      else if (_align & ALIGN_RIGHT)
            xml.tag("halign", "right");
      else
            xml.tag("halign", "left");
      if (_align & ALIGN_BOTTOM)
            xml.tag("valign", "bottom");
      else if (_align & ALIGN_VCENTER)
            xml.tag("valign", "center");
      else if (_align & ALIGN_BASELINE)
            xml.tag("valign", "baseline");
      else
            xml.tag("valign", "top");

      if (!_offset.isNull()) {
            QPointF pt(_offset);
            if (offsetType() == OFFSET_ABS)
                  pt *= INCH;
            xml.tag("xoffset", pt.x());         // save in spatium or metric mm
            xml.tag("yoffset", pt.y());
            }
      if (_reloff.x() != 0.0)
            xml.tag("rxoffset", _reloff.x());
      if (_reloff.y() != 0.0)
            xml.tag("ryoffset", _reloff.y());

      const char* p = 0;
      switch(_offsetType) {
            case OFFSET_SPATIUM: p = "spatium"; break;
            case OFFSET_ABS:     p = "absolute"; break;
            }
      xml.tag("offsetType", p);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool ElementLayout::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "halign") {
            const QString& val(e.readElementText());
            _align &= ~(ALIGN_HCENTER | ALIGN_RIGHT);
            if (val == "center")
                  _align |= ALIGN_HCENTER;
            else if (val == "right")
                  _align |= ALIGN_RIGHT;
            else if (val == "left")
                  ;
            else
                  qDebug("Text::readProperties: unknown alignment: <%s>\n", qPrintable(val));
            }
      else if (tag == "valign") {
            const QString& val(e.readElementText());
            _align &= ~(ALIGN_VCENTER | ALIGN_BOTTOM | ALIGN_BASELINE);
            if (val == "center")
                  _align |= ALIGN_VCENTER;
            else if (val == "bottom")
                  _align |= ALIGN_BOTTOM;
            else if (val == "baseline")
                  _align |= ALIGN_BASELINE;
            else if (val == "top")
                  ;
            else
                  qDebug("Text::readProperties: unknown alignment: <%s>\n", qPrintable(val));
            }
      else if (tag == "xoffset") {
            qreal xo = e.readDouble();
            if (offsetType() == OFFSET_ABS)
                  xo /= INCH;
            setXoff(xo);
            }
      else if (tag == "yoffset") {
            qreal yo = e.readDouble();
            if (offsetType() == OFFSET_ABS)
                  yo /= INCH;
            setYoff(yo);
            }
      else if (tag == "rxoffset")
            setRxoff(e.readDouble());
      else if (tag == "ryoffset")
            setRyoff(e.readDouble());
      else if (tag == "offsetType") {
            const QString& val(e.readElementText());
            OffsetType ot = OFFSET_ABS;
            if (val == "spatium" || val == "1")
                  ot = OFFSET_SPATIUM;
            if (ot != offsetType()) {
                  setOffsetType(ot);
                  if (ot == OFFSET_ABS)
                        _offset /= INCH;  // convert spatium -> inch
                  else
                        _offset *= INCH;  // convert inch -> spatium
                  }
            }
      else
            return false;
      return true;
      }


