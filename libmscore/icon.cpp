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

void Icon::write(XmlWriter& xml) const
      {
      xml.stag(this);
      xml.tag("subtype", int(_iconType));
      if (!_action.isEmpty())
            xml.tag("action", _action.data());
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
                  _action = e.readElementText().toLocal8Bit();
            else if (tag == "subtype")
                  _iconType = IconType(e.readInt());
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Icon::layout()
      {
      setbbox(QRectF(0, 0, _extent, _extent));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Icon::draw(QPainter* p) const
      {
      QPixmap pm(_icon.pixmap(_extent, QIcon::Normal, QIcon::On));
      p->drawPixmap(0, 0, pm);
      }

}

