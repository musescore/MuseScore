//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "xml.h"
#include "score.h"

namespace Ms {

//---------------------------------------------------------
//   read207
//---------------------------------------------------------

Score::FileError MasterScore::read207(XmlReader& e)
      {
      qDebug("read207");
      return MasterScore::read300(e);
      }

}

