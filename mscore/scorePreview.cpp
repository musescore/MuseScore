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

#include "scorePreview.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   ScorePreview
//---------------------------------------------------------

ScorePreview::ScorePreview(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ScorePreview::setScore(const QString& s)
      {
      printf("setScore %s\n", qPrintable(s));
      QFileInfo fi(s);
      name->setText(fi.baseName());
      creationDate->setText(fi.created().toString());
      fileSize->setText(QString("%1 KiB").arg(fi.size() / 1024));
      creationDate->setEnabled(true);
      fileSize->setEnabled(true);
      if (fi.suffix() == "mscz") {
            QPixmap pm = extractThumbnail(s);
            icon->setPixmap(pm);
            }
      else
            icon->setPixmap(QPixmap());
      }
}




