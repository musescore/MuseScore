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

class ScoreItem;

//---------------------------------------------------------
//   ScoreListWidget
//---------------------------------------------------------

class ScoreListWidget : public QListWidget
      {
      Q_OBJECT

      virtual QSize sizeHint() const override;

   public:
      ScoreListWidget(QWidget* parent = 0) : QListWidget(parent) {}
      };

//---------------------------------------------------------
//   ScoreBrowser
//---------------------------------------------------------

class ScoreBrowser : public QWidget, public Ui::ScoreBrowser
      {
      Q_OBJECT

      QListWidget* createScoreList();
      QList<ScoreListWidget*> scoreLists;
      bool _stripNumbers  { false };

      ScoreItem* genScoreItem(const QFileInfo& fi);

   private slots:
      void scoreChanged(QListWidgetItem*, QListWidgetItem*);
      void setScoreActivated(QListWidgetItem*);

   signals:
      void leave();
      void scoreSelected(QString);
      void scoreActivated(QString);

   public:
      ScoreBrowser(QWidget* parent = 0);
      void setScores(QFileInfoList);
      void setStripNumbers(bool val) { _stripNumbers = val; }
      void selectFirst();
      void selectLast();
      };
}

#endif

