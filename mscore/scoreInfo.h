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

#ifndef __SCOREINFO_H__
#define __SCOREINFO_H__

namespace Ms {

//---------------------------------------------------------
//   ScoreInfo
//---------------------------------------------------------

class ScoreInfo : public QFileInfo {
      mutable QPixmap _pixmap;

   public:
      ScoreInfo() {}
      ScoreInfo(const QFileInfo& fi) : QFileInfo(fi) {}
      QPixmap pixmap() const;
      void setPixmap(const QPixmap& pm) { _pixmap = pm; }
      };

}

#endif

