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


#include "annotation.h"
#include "sym.h"
#include "chord.h"
#include "note.h"
#include "score.h"
#include "staff.h"
#include "part.h"
#include "segment.h"
#include "property.h"
#include "element.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   Annotation
//---------------------------------------------------------

Annotation::Annotation(Score* s)
  : Element(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);
      _annotationType = AnnotationType::TEXT;
      _anchorType = AnchorType::SEGMENT;
      }
//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Annotation::write(Xml& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag("Annotation");
      Element::writeProperties(xml);
      xml.tag("AnnotationType", int(_annotationType));
      xml.tag("AnchorType", int(_anchorType));
      xml.tag("TextVal", _text->write(); );
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Annotation::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "AnnotationType")
                  _annotationType = AnnotationType(e.readInt());
            else if (tag == "AnchorType")
                  _anchorType = AnchorType(e.readInt());
            else if (tag == "TextVal")
                  _text = _text->read();
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------


void Annotation::draw() const
      {
      _text->draw();
      }


//---------------------------------------------------------
//   layout
//---------------------------------------------------------


void Annotation::layout() const
      {
      _text->layout();
      }
}

