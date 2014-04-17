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

#include "fingering.h"
#include "score.h"
#include "undo.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   Fingering
//---------------------------------------------------------

Fingering::Fingering(Score* s)
  : Text(s)
      {
      setTextStyleType(TEXT_STYLE_FINGERING);
      setFlag(ELEMENT_HAS_TAG, true);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Fingering::layout()
      {
      Text::layout();
      setPos(QPointF());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Fingering::write(Xml& xml) const
      {
      xml.stag(name());
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Fingering::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!Text::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Fingering::reset()
      {
      QPointF o(userOff());
      score()->layoutFingering(this);
      QPointF no(userOff());
      setUserOff(o);
      score()->undoChangeProperty(this, P_USER_OFF, no);
      }

}

