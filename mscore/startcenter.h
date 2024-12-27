//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __STARTCENTER_H__
#define __STARTCENTER_H__

#include "config.h"
#include "abstractdialog.h"
#include "ui_startcenter.h"

namespace Ms {

//---------------------------------------------------------
//   Startcenter
//---------------------------------------------------------

class Startcenter : public AbstractDialog, public Ui::Startcenter {
      Q_OBJECT
      virtual void closeEvent(QCloseEvent*);

    private slots:
      void loadScore(QString);
      void newScore();
      void openScoreClicked();

    protected:
      virtual void retranslate() { retranslateUi(this); }

    signals:
      void closed(bool);

    public:
      Startcenter(QWidget* parent);
      ~Startcenter();
      void updateRecentScores();
      void writeSettings();
      void readSettings();
      void keyPressEvent(QKeyEvent*) override;
      void keyReleaseEvent(QKeyEvent*) override;
      };
}
#endif //__STARTCENTER_H__
