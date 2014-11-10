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
            int w = 600;
            int h = 400;
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
      webView->setUrl(QUrl("http://www.musescore.org"));
      connect(createNewScore, SIGNAL(clicked()), getAction("file-new"), SLOT(trigger()));
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void Startcenter::closeEvent(QCloseEvent*)
      {
      emit closed(false);
      }

}

