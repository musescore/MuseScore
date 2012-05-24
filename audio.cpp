//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "audio.h"
#include "xml.h"

//---------------------------------------------------------
//   Audio
//---------------------------------------------------------

Audio::Audio()
      {
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Audio::read(const QDomElement& ee)
      {
      for (QDomElement e = ee.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "path")
                  _path = val;
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Audio::write(Xml& xml) const
      {
      xml.stag("Audio");
      xml.tag("path", _path);
      xml.etag();
      }

