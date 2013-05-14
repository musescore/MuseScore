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

#include "xml.h"
#include "icon.h"

namespace Ms {

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Icon::write(Xml& xml) const
      {
      xml.stag(name());
      xml.tag("subtype", _iconType);
      if (_action)
            xml.tag("action", _action);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Icon::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "action")
                  _action = strdup(e.readElementText().toLatin1().data());
            else if (tag == "subtype")
                  _iconType = e.readInt();
            else
                  e.unknown();
            }
      }

}

