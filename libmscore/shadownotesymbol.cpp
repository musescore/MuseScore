//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "shadownotesymbol.h"
#include "sym.h"

namespace Ms {


void ShadowNoteSymbol::setSymbols(TDuration::DurationType type, SymId noteSymbol)
    {

    clearSymbols();
    symNotehead=noteSymbol;
    if(symNotehead != SymId::noSym)
    {
        bNoteSymSet = true;
    }




    switch(type) {
          case TDuration::DurationType::V_LONG:
                symFlag=SymId::flagInternalUp;
                break;
          case TDuration::DurationType::V_BREVE:
                symFlag = SymId::noSym;
                break;
          case TDuration::DurationType::V_WHOLE:
                symFlag = SymId::noSym;
                break;
          case TDuration::DurationType::V_HALF:
                symFlag=SymId::flagInternalUp;
                break;
          case TDuration::DurationType::V_QUARTER:
                symFlag= SymId::flagInternalUp;
                break;
          case TDuration::DurationType::V_EIGHTH:
                symFlag=SymId::flag8thUp;
                break;
          case TDuration::DurationType::V_16TH:
                symFlag=SymId::flag16thUp;
                break;
          case TDuration::DurationType::V_32ND:
                symFlag=SymId::flag32ndUp;
                break;
          case TDuration::DurationType::V_64TH:
                symFlag=SymId::flag64thUp;
                break;
          case TDuration::DurationType::V_128TH:
                symFlag=SymId::flag128thUp;
                break;
          default:
                symFlag = SymId::noSym;
          }

        if(symFlag != SymId::noSym)
        {
            bFlagSymSet = true;
        }
    }

void ShadowNoteSymbol::clearSymbols()
{
    symNotehead=SymId::noSym;
    symFlag=SymId::noSym;
    bNoteSymSet = false;
    bFlagSymSet = false;
}



ShadowNoteSymbol::~ShadowNoteSymbol()
      {

      }

}
