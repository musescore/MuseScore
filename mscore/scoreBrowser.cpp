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

#include "scoreBrowser.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   ScoreItem
//---------------------------------------------------------

class ScoreItem : public QListWidgetItem
      {
      ScoreInfo _info;

   public:
      ScoreItem(const ScoreInfo& i) : QListWidgetItem(), _info(i) {}
      const ScoreInfo& info() const { return _info; }
      };

//---------------------------------------------------------
//   ScoreBrowser
//---------------------------------------------------------

ScoreBrowser::ScoreBrowser(QWidget* parent)
  : QWidget(parent)
      {
      setupUi(this);
      scoreList->setWrapping(true);

      scoreList->setViewMode(QListView::IconMode);
      scoreList->setGridSize(QSize(110, 130));
      scoreList->setIconSize(QSize(80, 100));
      scoreList->setSpacing(10);

      connect(scoreList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
         SLOT(scoreChanged(QListWidgetItem*,QListWidgetItem*)));
      connect(scoreList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
         SLOT(scoreClicked(QListWidgetItem*)));
      connect(scoreList, SIGNAL(itemActivated(QListWidgetItem*)),
         SLOT(scoreActivated(QListWidgetItem*)));
      }

//---------------------------------------------------------
//   setScores
//---------------------------------------------------------

void ScoreBrowser::setScores(QFileInfoList& s)
      {
      for (const QFileInfo& fi : s) {
            QPixmap pm = extractThumbnail(fi.filePath());
            printf("<%s>\n", qPrintable(fi.filePath()));
            ScoreInfo si(fi);
            if (fi.suffix() == "mscz")
                  si.setPixmap(extractThumbnail(fi.filePath()));
            ScoreItem* item = new ScoreItem(si);
            item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
            item->setText(si.completeBaseName());
            item->setIcon(QIcon(si.pixmap()));
            scoreList->addItem(item);
            }
      }

//---------------------------------------------------------
//   scoreChanged
//---------------------------------------------------------

void ScoreBrowser::scoreChanged(QListWidgetItem* current, QListWidgetItem*)
      {
      if (!current)
            return;
      ScoreItem* item = static_cast<ScoreItem*>(current);
      preview->setScore(item->info().filePath());
      }

void ScoreBrowser::scoreClicked(QListWidgetItem*)
      {
      printf("double click\n");
      }

void ScoreBrowser::scoreActivated(QListWidgetItem*)
      {
      printf("activated\n");
      }

}

