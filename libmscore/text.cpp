//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "text.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   defaultStyle
//---------------------------------------------------------

static const ElementStyle defaultStyle {
      { Sid::defaultSystemFlag, Pid::SYSTEM_FLAG },
      };

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

Text::Text(Score* s, Tid tid) : TextBase(s, tid)
      {
      initElementStyle(&defaultStyle);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Text::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "style") {
                  QString sn = e.readElementText();
                  if (sn == "Tuplet")          // ugly hack for compatibility
                        continue;
                  Tid s = textStyleFromName(sn);
                  initTid(s);
                  }
            else if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Text::propertyDefault(Pid id) const
      {
      switch(id) {
            case Pid::SUB_STYLE:
                  return int(Tid::DEFAULT);
            default:
                  return TextBase::propertyDefault(id);
            }
      }

}

