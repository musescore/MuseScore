//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "hook.h"
#include "sym.h"
#include "chord.h"
#include "stem.h"
#include "score.h"

namespace Ms {

//---------------------------------------------------------
//   Hook
//---------------------------------------------------------

Hook::Hook(Score* s)
  : Symbol(s, ElementFlag::NOTHING)
      {
      setZ(int(type()) * 100);
      }

//---------------------------------------------------------
//   setHookType
//---------------------------------------------------------

void Hook::setHookType(int i)
      {
      _hookType = i;
      switch(i) {
            case 0:    break;
            case 1:    setSym(SymId::flag8thUp);     break;
            case 2:    setSym(SymId::flag16thUp);    break;
            case 3:    setSym(SymId::flag32ndUp);    break;
            case 4:    setSym(SymId::flag64thUp);    break;
            case 5:    setSym(SymId::flag128thUp);   break;
            case 6:    setSym(SymId::flag256thUp);   break;
            case 7:    setSym(SymId::flag512thUp);   break;
            case 8:    setSym(SymId::flag1024thUp);  break;

            case -1:   setSym(SymId::flag8thDown);   break;
            case -2:   setSym(SymId::flag16thDown);  break;
            case -3:   setSym(SymId::flag32ndDown);  break;
            case -4:   setSym(SymId::flag64thDown);  break;
            case -5:   setSym(SymId::flag128thDown); break;
            case -6:   setSym(SymId::flag256thDown); break;
            case -7:   setSym(SymId::flag512thDown); break;
            case -8:   setSym(SymId::flag1024thDown);break;
            default:
                  qDebug("no hook/flag for subtype %d", i);
                  break;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Hook::layout()
      {
      setbbox(symBbox(_sym));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Hook::draw(QPainter* painter) const
      {
      // hide if belonging to the second chord of a cross-measure pair
      if (chord() && chord()->crossMeasure() == CrossMeasure::SECOND)
            return;
      Symbol::draw(painter);
      }

}
