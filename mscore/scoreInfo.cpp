//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "scoreInfo.h"
#include "icons.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   pixmap
//---------------------------------------------------------

QPixmap ScoreInfo::pixmap() const
      {
      if (_pixmap.isNull()) {
            // load or generate an actual thumbnail for the score
            _pixmap = mscore->extractThumbnail(filePath());
            if (_pixmap.isNull()) {
                  // couldn't load/generate thumbnail so display generic icon
                  _pixmap = icons[int(Icons::file_ICON)]->pixmap(QSize(50,60));
                  }
            }
      return _pixmap;
      }

}


