//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: ottava.cpp 5427 2012-03-07 12:41:34Z wschweer $
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

//---------------------------------------------------------
//   Ottava
//---------------------------------------------------------

Ottava::Ottava(Score* s)
   : TextLine(s)
      {
      setSubtype(0);
      setYoff(s->styleS(ST_ottavaY).val());
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Ottava::layout()
      {
      setPos(0.0, 0.0);
      setLineWidth(score()->styleS(ST_ottavaLineWidth));
      SLine::layout();
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Ottava::setSubtype(int val)
      {
      setEndHook(true);
      _subtype = val;

      Spatium hook(score()->styleS(ST_ottavaHook));

      const TextStyle& ts = score()->textStyle(TEXT_STYLE_OTTAVA);

      switch(val) {
            case 0:
                  setBeginText("8va", ts);
                  setContinueText("(8va)", ts);
                  setEndHookHeight(hook);
                  _pitchShift = 12;
                  break;
            case 1:
                  setBeginText("15ma", ts);
                  setContinueText("(15ma)", ts);
                  setEndHookHeight(hook);
                  _pitchShift = 24;
                  break;
            case 2:
                  setBeginText("8vb", ts);
                  setContinueText("(8vb)", ts);
                  _pitchShift = -12;
                  setEndHookHeight(-hook);
                  break;
            case 3:
                  setBeginText("15mb", ts);
                  setContinueText("(15mb)", ts);
                  _pitchShift = -24;
                  setEndHookHeight(-hook);
                  break;
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
      if (oStartElement != startElement() || oEndElement != endElement()) {
            Staff* s = staff();
            int tick1 = static_cast<Segment*>(oStartElement)->tick();
            int tick2 = static_cast<Segment*>(oEndElement)->tick();
            s->pitchOffsets().remove(tick1);
            s->pitchOffsets().remove(tick2);

            tick1 = static_cast<Segment*>(startElement())->tick();
            tick2 = static_cast<Segment*>(endElement())->tick();
            s->pitchOffsets().setPitchOffset(tick1, _pitchShift);
            s->pitchOffsets().setPitchOffset(tick2, 0);

            score()->addLayoutFlags(LAYOUT_FIX_PITCH_VELO);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Ottava::write(Xml& xml) const
      {
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      xml.tag("subtype", subtype());
      TextLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Ottava::read(const QDomElement& de)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();
      setId(de.attribute("id", "-1").toInt());
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            if (tag == "subtype")
                  setSubtype(e.text().toInt());
            else if (!TextLine::readProperties(e))
                  domError(e);
            }
      }


