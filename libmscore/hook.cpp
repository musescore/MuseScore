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

//---------------------------------------------------------
//   Hook
//---------------------------------------------------------

Hook::Hook(Score* s)
  : Symbol(s)
      {
      setFlag(ELEMENT_MOVABLE, false);
      }

//---------------------------------------------------------
//   setHookType
//---------------------------------------------------------

void Hook::setHookType(int i)
      {
      _hookType = i;
      switch(i) {
            case 0:    break;
            case 1:    setSym(eighthflagSym);        break;
            case 2:    setSym(sixteenthflagSym);     break;
            case 3:    setSym(thirtysecondflagSym);  break;
            case 4:    setSym(sixtyfourthflagSym);   break;
            case 5:    setSym(flag128Sym);   break;
            case -1:   setSym(deighthflagSym);       break;
            case -2:   setSym(dsixteenthflagSym);    break;
            case -3:   setSym(dthirtysecondflagSym); break;
            case -4:   setSym(dsixtyfourthflagSym);  break;
            case -5:   setSym(dflag128Sym);  break;
            default:
                  qDebug("no hook for subtype %d\n", i);
                  break;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Hook::layout()
      {
      ElementLayout::layout(this);
      setbbox(symbols[score()->symIdx()][_sym].bbox(magS()));
      }
