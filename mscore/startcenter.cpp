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


#include "musescore.h"
#include "startcenter.h"
#include "scoreBrowser.h"

namespace Ms {

//---------------------------------------------------------
//   showStartcenter
//---------------------------------------------------------

void MuseScore::showStartcenter(bool val)
      {
      QAction* a = getAction("startcenter");
      if (val && startcenter == nullptr) {
            startcenter = new Startcenter;
            startcenter->addAction(a);
            startcenter->readSettings(settings);
            connect(startcenter, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            }
      startcenter->setVisible(val);
      }

//---------------------------------------------------------
//   Startcenter
//---------------------------------------------------------

Startcenter::Startcenter()
 : QDialog(0)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setWindowModality(Qt::ApplicationModal);
      connect(recentPage,  &ScoreBrowser::scoreActivated, this, &Startcenter::loadScore);
      connect(openScore, SIGNAL(clicked()), this, SLOT(openScoreClicked()));

      //init webview
      webView->setUrl(QUrl("https://connect.musescore.com/"));
      recentPage->setBoldTitle(true);
      updateRecentScores();
      }

//---------------------------------------------------------
//   loadScore
//---------------------------------------------------------

void Startcenter::loadScore(QString s)
      {
      if (s.endsWith("Create_New_Score.mscz")) {
            newScore();
      } else {
            mscore->openScore(s);
            close();
            }
      }

//---------------------------------------------------------
//   newScore
//---------------------------------------------------------

void Startcenter::newScore()
      {
      close();
      getAction("file-new")->trigger();
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void Startcenter::closeEvent(QCloseEvent*)
      {
      emit closed(false);
      }

//---------------------------------------------------------
//   updateRecentScores
//---------------------------------------------------------

void Startcenter::updateRecentScores()
      {
      QFileInfoList fil = mscore->recentScores();
      QFileInfo newScore(":/data/Create_New_Score.mscz");
      fil.prepend(newScore);
      recentPage->setScores(fil);
      recentPage->selectLast();
      }

//---------------------------------------------------------
//   openScoreClicked
//---------------------------------------------------------

void Startcenter::openScoreClicked()
      {
      close();
      getAction("file-open")->trigger();
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void Startcenter::writeSettings(QSettings& settings)
      {
      settings.beginGroup("Startcenter");
      settings.setValue("size", size());
      settings.setValue("pos", pos());
      settings.endGroup();
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void Startcenter::readSettings(QSettings& settings)
      {
      settings.beginGroup("Startcenter");
      resize(settings.value("size", QSize(1161, 694)).toSize());
      move(settings.value("pos", QPoint(200, 100)).toPoint());
      settings.endGroup();
      }

}

