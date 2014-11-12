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

#ifndef __SCOREBROWSER_H__
#define __SCOREBROWSER_H__

#include "ui_scoreBrowser.h"
#include "scoreInfo.h"

namespace Ms {

//---------------------------------------------------------
//   ScoreBrowser
//---------------------------------------------------------

class ScoreBrowser : public QWidget, public Ui::ScoreBrowser
      {
      Q_OBJECT

   private slots:
      void scoreChanged(QListWidgetItem*, QListWidgetItem*);
      void scoreActivated(QListWidgetItem*);

   signals:
      void leave();

   public:
      ScoreBrowser(QWidget* parent = 0);
      void setScores(QFileInfoList&);
      };
}

#endif

