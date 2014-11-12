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

#ifndef __SCOREPREVIEW_H__
#define __SCOREPREVIEW_H__

#include "ui_scorePreview.h"

namespace Ms {

//---------------------------------------------------------
//   ScorePreview
//---------------------------------------------------------


class ScorePreview : public QWidget, public Ui::ScorePreview
      {
      Q_OBJECT

   public slots:
      void setScore(const QString&);

   public:
      ScorePreview(QWidget* parent = 0);
      };
}


#endif


