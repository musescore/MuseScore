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
#include "musescore.h"
#include "scoreInfo.h"

namespace Ms {

//---------------------------------------------------------
//   ScorePreview
//---------------------------------------------------------

ScorePreview::ScorePreview(QWidget* parent)
   : QWidget(parent)
      {
      messageNothingToShow = tr("Nothing selected");
      setupUi(this);
      icon->setText(messageNothingToShow);
      }

//---------------------------------------------------------
//   displayInfo
//---------------------------------------------------------

void ScorePreview::displayInfo(bool show)
      {
      info->setVisible(show);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ScorePreview::setScore(const QString& s)
      {
      ScoreInfo fi(s);
      fi.setPixmap(mscore->extractThumbnail(s));
      setScore(fi);
      }

void ScorePreview::setScore(const ScoreInfo& si)
      {
      scoreInfo = si;
      name->setText(si.completeBaseName());
      creationDate->setText(si.created().toString());
      fileSize->setText(QString("%1 KiB").arg(si.size() / 1024));
      name->setEnabled(true);
      creationDate->setEnabled(true);
      fileSize->setEnabled(true);
      icon->setPixmap(si.pixmap());
      }

//---------------------------------------------------------
//   unsetScore
//---------------------------------------------------------

void ScorePreview::unsetScore()
      {
      scoreInfo = ScoreInfo();
      name->setText("");
      creationDate->setText("");
      fileSize->setText("");
      name->setEnabled(false);
      creationDate->setEnabled(true);
      fileSize->setEnabled(true);
      icon->setText(messageNothingToShow);
      }
}




