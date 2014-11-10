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
      printf("show %d\n", val);

      QAction* a = getAction("startcenter");
      if (val && startcenter == nullptr) {
            QQuickView* view = new Startcenter("qrc:/startcenter.qml");
            startcenter = QWidget::createWindowContainer(view, this,
               Qt::WindowStaysOnTopHint | Qt::Popup);
            startcenter->setWindowModality(Qt::ApplicationModal);
            int w = 600;
            int h = 400;
            int x = pos().x() + width() / 2 - w/2;
            int y = pos().y() + height() / 2 - h/2;
            startcenter->setGeometry(x, y, w, h);

            startcenter->addAction(a);
            connect(view, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            }
      startcenter->setVisible(val);
      }

//---------------------------------------------------------
//   Startcenter
//---------------------------------------------------------

Startcenter::Startcenter(QString s)
 : QQuickView(QUrl(s))
      {
      connect(this, SIGNAL(closing(QQuickCloseEvent*)), SLOT(closeEvent()));
      connect(this, SIGNAL(sceneGraphAboutToStop()), SLOT(closeEvent()));
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void Startcenter::closeEvent()
      {
      printf("close event\n");
      emit closed(false);
      }


}

