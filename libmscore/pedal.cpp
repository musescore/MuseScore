//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: pedal.cpp 5149 2011-12-29 08:38:43Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "pedal.h"
#include "textline.h"
#include "sym.h"

#include "score.h"

//---------------------------------------------------------
//   Pedal
//---------------------------------------------------------

Pedal::Pedal(Score* s)
   : TextLine(s)
      {
      setBeginSymbol(pedalPedSym);
      setBeginSymbolOffset(QPointF(0.0, -.2));

      setEndHook(true);
      setBeginHookHeight(Spatium(-1.2));
      setEndHookHeight(Spatium(-1.2));

      setYoff(8.0);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Pedal::read(const QDomElement& e)
      {
      if (score()->mscVersion() >= 110) {
            setBeginSymbol(-1);
            setEndHook(false);
            }
      TextLine::read(e);
      }
