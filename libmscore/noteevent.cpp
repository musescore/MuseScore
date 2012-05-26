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

#include "noteevent.h"
#include "xml.h"

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void NoteEvent::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            int i = val.toInt();
            if (tag == "pitch")
                  _pitch = i;
            else if (tag == "ontime")
                  _ontime = i;
            else if (tag == "len")
                  _len = i;
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void NoteEvent::write(Xml& xml) const
      {
      xml.stag("Event");
      xml.tag("pitch", _pitch);
      xml.tag("ontime", _ontime);
      xml.tag("len", _len);
      xml.etag();
      }
