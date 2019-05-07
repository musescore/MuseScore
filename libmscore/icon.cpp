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
#include "property.h"

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

//---------------------------------------------------------
//   Icon::getProperty
//---------------------------------------------------------

QVariant Icon::getProperty(Pid pid) const
      {
      switch (pid) {
            case Pid::ACTION:
                  return action();
            default:
                  break;
            }
      return Element::getProperty(pid);
      }

//---------------------------------------------------------
//   Icon::setProperty
//---------------------------------------------------------

bool Icon::setProperty(Pid pid, const QVariant& v)
      {
      switch (pid) {
            case Pid::ACTION:
                  _action = v.toString().toLatin1();
                  triggerLayout();
                  break;
            default:
                  return Element::setProperty(pid, v);
            }
      return true;
      }
}
