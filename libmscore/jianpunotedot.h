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

#ifndef __JIANPUNOTEDOT_H__
#define __JIANPUNOTEDOT_H__

#include "element.h"
#include "notedot.h"

namespace Ms {

class JianpuNote;

//---------------------------------------------------------
//   @@ JianpuNoteDot
//---------------------------------------------------------

class JianpuNoteDot : public NoteDot {

   public:
      JianpuNoteDot(Score* score = 0);
      JianpuNoteDot(const NoteDot& noteDot, bool link = false);
      JianpuNoteDot(const JianpuNoteDot& noteDot, bool link = false);
      virtual ~JianpuNoteDot();

      JianpuNoteDot& operator=(const JianpuNoteDot& note) = delete;
      virtual JianpuNoteDot* clone() const override  { return new JianpuNoteDot(*this, false); }

      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      };

}     // namespace Ms
#endif

