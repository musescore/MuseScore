//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2015 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "scoreElement.h"
#include "score.h"
#include "undo.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void ScoreElement::resetProperty(P_ID id)
      {
      QVariant v = propertyDefault(id);
      if (v.isValid())
            setProperty(id, v);
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void ScoreElement::undoChangeProperty(P_ID id, const QVariant& val)
      {
      score()->undoChangeProperty(this, id, val);
      }

//---------------------------------------------------------
//   undoPushProperty
//---------------------------------------------------------

void ScoreElement::undoPushProperty(P_ID id)
      {
      QVariant val = getProperty(id);
      score()->undo()->push1(new ChangeProperty(this, id, val));
      }

//---------------------------------------------------------
//   writeProperty
//---------------------------------------------------------

void ScoreElement::writeProperty(Xml& xml, P_ID id) const
      {
      xml.tag(id, getProperty(id), propertyDefault(id));
      }

}

