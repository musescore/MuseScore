//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __REHEARSALMARK_H__
#define __REHEARSALMARK_H__

#include "text.h"

//---------------------------------------------------------
//   RehearsalMark
//---------------------------------------------------------

class RehearsalMark : public Text  {
      Q_OBJECT


   public:
      RehearsalMark(Score* score);
      virtual RehearsalMark* clone() const { return new RehearsalMark(*this); }
      virtual ElementType type() const { return REHEARSAL_MARK; }
      };

#endif

