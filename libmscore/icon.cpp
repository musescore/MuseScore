//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: element.cpp 4385 2011-06-15 13:26:41Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include <string.h>
#include "xml.h"
#include "icon.h"

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Icon::write(Xml& xml) const
      {
      xml.stag(name());
      xml.tag("subtype", _subtype);
      if (_action)
            xml.tag("action", _action);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Icon::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            if (tag == "action")
                  _action = strdup(e.text().toLatin1().data());
            else if (tag == "subtype")
                  _subtype = e.text().toInt();
            else
                  domError(e);
            }
      }

