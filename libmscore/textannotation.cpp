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


#include "textannotation.h"
#include "sym.h"
#include "chord.h"
#include "note.h"
#include "score.h"
#include "staff.h"
#include "part.h"
#include "segment.h"
#include "property.h"
#include "element.h"
#include "score.h"
#include "stafftext.h"
#include "system.h"
#include "xml.h"
#include "text.h"
namespace Ms {

//---------------------------------------------------------
//   textAnnotation
//---------------------------------------------------------

TextAnnotation::TextAnnotation(Score* s)
  : Text(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      setTextStyleType(TextStyleType::ANNOTATION);
      _anchorType = textAnchorType::SEGMENT;
      }
//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextAnnotation::write(Xml& xml) const
      {
     if (!xml.canWrite(this))
            return;
      xml.stag("textAnnotation");
      xml.tag("textAnchorType", int(_anchorType));
      qDebug() << "Inside write, xml text is" << plainText();
      xml.tag("textVal",xmlText());
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextAnnotation::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "textAnchorType")
                  _anchorType = textAnchorType(e.readInt());
            else if (tag == "textVal")
                  setPlainText(e.readXml());
            else if (!Text::readProperties(e))
                  e.unknown();
            }
      }
}
