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

namespace Ms {

//---------------------------------------------------------
//   showStartcenter
//---------------------------------------------------------

void MuseScore::showStartcenter(bool val)
      {
      QAction* a = getAction("startcenter");
      if (val && startcenter == nullptr) {
            startcenter = new Startcenter;
            int w = 1000;
            int h = 600;
            int x = pos().x() + width() / 2 - w/2;
            int y = pos().y() + height() / 2 - h/2;
            startcenter->setGeometry(x, y, w, h);
            startcenter->addAction(a);
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
      setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Popup);
      setWindowModality(Qt::ApplicationModal);
      connect(createNewScore, SIGNAL(clicked()), getAction("file-new"), SLOT(trigger()));
      connect(recentScores,   SIGNAL(toggled(bool)),  SLOT(recentScoresToggled(bool)));
      connect(templates,      SIGNAL(toggled(bool)),  SLOT(templatesToggled(bool)));
      connect(demos,          SIGNAL(toggled(bool)),  SLOT(demosToggled(bool)));
      connect(connectWeb,     SIGNAL(toggled(bool)),  SLOT(connectWebToggled(bool)));

      recentScoresToggled(true);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void Startcenter::closeEvent(QCloseEvent*)
      {
      emit closed(false);
      }

//---------------------------------------------------------
//   recentScoresToggled
//---------------------------------------------------------

void Startcenter::recentScoresToggled(bool val)
      {
      if (!val)
            return;
      printf("recent scores\n");
      QFileInfoList fil;
      if (!recentPageInitialized) {
            for (const QString& s : ::Ms::recentScores) {
                  if (s.isEmpty())
                        break;
                  QString data(s);
                  QFileInfo fi(s);
                  if (fi.exists())
                        fil.append(fi);
                  }
            recentPage->setScores(fil);
            recentPageInitialized = true;
            }
      stack->setCurrentWidget(recentPage);
      }

//---------------------------------------------------------
//   templatesToggled
//---------------------------------------------------------

void Startcenter::templatesToggled(bool val)
      {
      if (!val)
            return;
      stack->setCurrentWidget(templatesPage);
      }

//---------------------------------------------------------
//   demosToggled
//---------------------------------------------------------

void Startcenter::demosToggled(bool val)
      {
      if (!val)
            return;
      stack->setCurrentWidget(demosPage);
      }

//---------------------------------------------------------
//   connectWebToggled
//---------------------------------------------------------

void Startcenter::connectWebToggled(bool val)
      {
      if (!val)
            return;
      if (!webPageInitialized) {
            webView->setUrl(QUrl("http://www.musescore.org"));
            webPageInitialized = true;
            }
      stack->setCurrentWidget(webPage);
      }
}

