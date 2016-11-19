//=============================================================================
//  MuseScore
//  Music Composition & Notation
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

namespace Ms {

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void NoteEvent::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "pitch")
                  _pitch = e.readInt();
            else if (tag == "ontime")
                  _ontime = e.readInt();
            else if (tag == "len")
                  _len = e.readInt();
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void NoteEvent::write(XmlWriter& xml) const
      {
      xml.stag("Event");
      xml.tag("pitch", _pitch);
      xml.tag("ontime", _ontime);
      xml.tag("len", _len);
      xml.etag();
      }

//---------------------------------------------------------
//   NoteEventList
//---------------------------------------------------------

NoteEventList::NoteEventList()
   : QList<NoteEvent>()
      {
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool NoteEvent::operator==(const NoteEvent& e) const
      {
      return (e._pitch == _pitch) && (e._ontime == _ontime) && (e._len == _len);
      }

}

