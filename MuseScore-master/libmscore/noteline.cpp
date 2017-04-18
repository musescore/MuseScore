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

#include "noteline.h"

namespace Ms {

NoteLine::NoteLine(Score* s)
   : TextLine(s)
      {
      }

NoteLine::NoteLine(const NoteLine& nl)
   : TextLine(nl)
      {
      }


}     // namespace Ms

