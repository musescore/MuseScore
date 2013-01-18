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

void Audio::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "path")
                  _path = e.readElementText();
            else
                  e.unknown();
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

