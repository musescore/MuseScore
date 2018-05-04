//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "bracketItem.h"
#include "property.h"
#include "staff.h"

namespace Ms {

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant BracketItem::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::SYSTEM_BRACKET:
                  return int(_bracketType);
            case P_ID::BRACKET_COLUMN:
                  return _column;
            case P_ID::BRACKET_SPAN:
                  return _bracketSpan;
                  break;
            default:
                  return QVariant();
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool BracketItem::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::SYSTEM_BRACKET:
                  // staff()->setBracketType(level(), BracketType(v.toInt()));   // change bracket type global
                  break;
            case P_ID::BRACKET_COLUMN:
                  _column = v.toInt();
                  break;
            case P_ID::BRACKET_SPAN:
                  _bracketSpan = v.toInt();
                  break;
            default:
                  // return Element::setProperty(id, v);
                  break;
            }
      score()->setLayoutAll();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant BracketItem::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::SYSTEM_BRACKET:
                  return int(BracketType::NORMAL);
            case P_ID::BRACKET_COLUMN:
                  return 0;
            default:
                  return QVariant();
            }
      }

}

