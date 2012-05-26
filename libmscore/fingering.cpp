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

#include "fingering.h"
#include "score.h"
#include "undo.h"

//---------------------------------------------------------
//   Fingering
//---------------------------------------------------------

Fingering::Fingering(Score* s)
  : Text(s)
      {
      setTextStyle(s->textStyle(TEXT_STYLE_FINGERING));
      setFlag(ELEMENT_HAS_TAG, true);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Fingering::layout()
      {
      Text::layout();
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

void Fingering::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!Text::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Fingering::toDefault()
      {
      QPointF o(userOff());
      score()->layoutFingering(this);
      QPointF no(userOff());
      setUserOff(o);
      score()->undoChangeUserOffset(this, no);
      }
