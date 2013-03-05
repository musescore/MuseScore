//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: hook.h 5362 2012-02-23 14:03:36Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __HOOK_H__
#define __HOOK_H__

#include "symbol.h"

class Chord;

//---------------------------------------------------------
//   @@ Hook
//---------------------------------------------------------

class Hook : public Symbol {
      Q_OBJECT

      int _hookType;

   public:
      Hook(Score*);
      virtual Hook* clone() const      { return new Hook(*this); }
      virtual ElementType type() const { return HOOK; }
      void setHookType(int v);
      int hookType() const             { return _hookType; }
      virtual void layout();
      Chord* chord() const             { return (Chord*)parent(); }
      };

#endif

