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

#ifndef __HOOK_H__
#define __HOOK_H__

#include "symbol.h"

namespace Ms {

class Chord;

//---------------------------------------------------------
//   @@ Hook
//---------------------------------------------------------

class Hook : public Symbol {
      Q_OBJECT

      int _hookType;

   public:
      Hook(Score*);
      virtual Hook* clone() const        { return new Hook(*this); }
      virtual qreal mag() const          { return parent()->mag(); }
      virtual Element::Type type() const { return Element::Type::HOOK; }
      void setHookType(int v);
      int hookType() const               { return _hookType; }
      virtual void layout();
      virtual void draw(QPainter*) const;
      Chord* chord() const               { return (Chord*)parent(); }
      };


}     // namespace Ms
#endif

