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
#include "musescore.h"
#include "icons.h"
#include "libmscore/score.h"

namespace Ms {

static const int CELLW = 110;
static const int CELLH = 130;
static const int SPACE = 10;

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize ScoreListWidget::sizeHint() const
      {
      int cols = (width()-SPACE) / (CELLW + SPACE);
      int n    = count();
      int rows = (n+cols-1) / cols;
      if (rows <= 0)
            rows = 1;
//      printf("ScoreListWidget count %d width %d  %d %d\n", count(), width(), cols, rows);
      return QSize(cols * CELLW, rows * (CELLH + SPACE) + SPACE);
      }

//---------------------------------------------------------
//   ScoreItem
//---------------------------------------------------------

class ScoreItem : public QListWidgetItem
      {
      ScoreInfo _info;

   public:
      ScoreItem(const ScoreInfo& i) : QListWidgetItem(), _info(i) {
            setSizeHint(QSize(CELLW, CELLH));
            }
      const ScoreInfo& info() const { return _info; }
      };

//---------------------------------------------------------
//   ScoreBrowser
//---------------------------------------------------------

ScoreBrowser::ScoreBrowser(QWidget* parent)
  : QWidget(parent)
      {
      setupUi(this);
      scoreList->setLayout(new QVBoxLayout);
      scoreList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      }

//---------------------------------------------------------
//   createScoreList
//---------------------------------------------------------

QListWidget* ScoreBrowser::createScoreList()
      {
      ScoreListWidget* sl = new ScoreListWidget;
      static_cast<QVBoxLayout*>(scoreList->layout())->addWidget(sl);
      sl->setWrapping(true);
      sl->setViewMode(QListView::IconMode);
      sl->setIconSize(QSize(CELLW, CELLH-20));
      sl->setSpacing(10);
      sl->setResizeMode(QListView::Adjust);
      sl->setFlow(QListView::LeftToRight);
      sl->setMovement(QListView::Static);
      sl->setTextElideMode(Qt::ElideRight);
      sl->setUniformItemSizes(true);
      sl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
      sl->setLineWidth(0);
      sl->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
      sl->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      sl->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      sl->setLayoutMode(QListView::SinglePass);
      sl->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

      connect(sl, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),SLOT(scoreChanged(QListWidgetItem*,QListWidgetItem*)));
      connect(sl, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(setScoreActivated(QListWidgetItem*)));
      scoreLists.append(sl);
      return sl;
      }

//---------------------------------------------------------
//   genScoreItem
//---------------------------------------------------------

ScoreItem* ScoreBrowser::genScoreItem(const QFileInfo& fi)
      {
      ScoreInfo si(fi);
      QPixmap pm = mscore->extractThumbnail(fi.filePath());
      if (pm.isNull())
            pm = icons[int(Icons::file_ICON)]->pixmap(QSize(50,60));
      si.setPixmap(pm);
      ScoreItem* item = new ScoreItem(si);
      item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);

      QString s(si.completeBaseName());
      if (!s.isEmpty() && s[0].isNumber() && _stripNumbers)
            s = s.mid(3);
      s = s.replace('_', ' ');
      item->setText(s);
      item->setIcon(QIcon(si.pixmap()));
      return item;
      }

//---------------------------------------------------------
//   setScores
//---------------------------------------------------------

void ScoreBrowser::setScores(QFileInfoList s)
      {
      qDeleteAll(scoreLists);
      scoreLists.clear();

      QLayout* l = scoreList->layout();
      while (l->count())
            l->removeItem(l->itemAt(0));

      QListWidget* sl = 0;

      QStringList filter = { "*.mscz" };
      for (const QFileInfo& fi : s) {
            if (fi.isFile()) {
                  QString s = fi.filePath();
                  if (s.endsWith(".mscz") || s.endsWith(".mscx")) {
                        if (!sl)
                              sl = createScoreList();
                        sl->addItem(genScoreItem(fi));
                        }
                  }
            }
      for (const QFileInfo& fi : s) {
            if (fi.isDir()) {
                  QString s(fi.fileName());
                  if (!s.isEmpty() && s[0].isNumber() && _stripNumbers)
                        s = s.mid(3);
                  s = s.replace('_', ' ');
                  QLabel* l = new QLabel(s);
                  static_cast<QVBoxLayout*>(scoreList->layout())->addWidget(l);
                  QDir dir(fi.filePath());
                  sl = createScoreList();
                  for (const QFileInfo& fi : dir.entryInfoList(filter, QDir::Files, QDir::Name))
                        sl->addItem(genScoreItem(fi));
                  sl = 0;
                  }
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
      preview->setScore(item->info());
      emit scoreSelected(item->info().filePath());
      }

//---------------------------------------------------------
//   setScoreActivated
//---------------------------------------------------------

void ScoreBrowser::setScoreActivated(QListWidgetItem* val)
      {
      ScoreItem* item = static_cast<ScoreItem*>(val);
      emit scoreActivated(item->info().filePath());
      }

}

