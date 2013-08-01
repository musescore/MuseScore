//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "xml.h"
#include "bagpembell.h"

namespace Ms {

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void BagpipeEmbellishment::write(Xml& xml) const
      {
      xml.stag(name());
      xml.tag("subtype", _embelType);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void BagpipeEmbellishment::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  _embelType = e.readInt();
            else
                  e.unknown();
            }
      }

}

