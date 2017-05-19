//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __JIANPUHOOK_H__
#define __JIANPUHOOK_H__

#include "hook.h"

namespace Ms {

class Chord;

//---------------------------------------------------------
//   @@ JianpuHook for drawing duration beams in place of hooks.
//---------------------------------------------------------

class JianpuHook : public Hook {
      Q_GADGET

   private:
      std::vector<QLineF*> _durationBeams;  ///< Duration beams for hooks.

   public:
      JianpuHook(Score* = 0);
      JianpuHook(const Hook& h);
      ~JianpuHook();
      virtual JianpuHook* clone() const override { return new JianpuHook(*this); }
      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      };

}     // namespace Ms
#endif

