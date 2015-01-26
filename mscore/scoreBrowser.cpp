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

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize ScoreListWidget::sizeHint() const
      {
      int cols = (width()-SPACE) / (CELLW + SPACE);
      int n    = count();
      int rows = 1;
      if (cols > 0)
            rows = (n+cols-1) / cols;
      if (rows <= 0)
            rows = 1;
      return QSize(cols * CELLW, rows * (CELLH + SPACE) + SPACE);
      }

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
      scoreList->setLayout(new QVBoxLayout);
      scoreList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
      scoreList->layout()->setMargin(0);
      connect(preview, SIGNAL(doubleClicked(QString)), SIGNAL(scoreActivated(QString)));
      if (!_showPreview)
            preview->setVisible(false);
      }

//---------------------------------------------------------
//   createScoreList
//---------------------------------------------------------

ScoreListWidget* ScoreBrowser::createScoreList()
      {
      ScoreListWidget* sl = new ScoreListWidget;
      static_cast<QVBoxLayout*>(scoreList->layout())->addWidget(sl);
      sl->setWrapping(true);
      sl->setViewMode(QListView::IconMode);
      sl->setIconSize(QSize(sl->cellWidth(), sl->cellHeight()-30));
      sl->setSpacing(10);
      sl->setResizeMode(QListView::Adjust);
      sl->setFlow(QListView::LeftToRight);
      sl->setMovement(QListView::Static);
      sl->setTextElideMode(Qt::ElideRight);
      sl->setWordWrap(true);
      sl->setUniformItemSizes(true);
      sl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
      sl->setLineWidth(0);
      sl->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
      sl->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      sl->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      sl->setLayoutMode(QListView::SinglePass);
      sl->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
      if (!_showPreview)
            sl->setSelectionMode(QAbstractItemView::NoSelection);

      connect(sl, SIGNAL(itemClicked(QListWidgetItem*)),   this, SLOT(scoreChanged(QListWidgetItem*)), Qt::QueuedConnection);
      connect(sl, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(setScoreActivated(QListWidgetItem*)));
      scoreLists.append(sl);
      return sl;
      }

//---------------------------------------------------------
//   genScoreItem
//---------------------------------------------------------

ScoreItem* ScoreBrowser::genScoreItem(const QFileInfo& fi, ScoreListWidget* l)
      {
      ScoreInfo si(fi);
      QPixmap pm = mscore->extractThumbnail(fi.filePath());
      if (pm.isNull())
            pm = icons[int(Icons::file_ICON)]->pixmap(QSize(50,60));
      // add border
      QPainter p(&pm);
      p.setPen(QColor(0,0,0,128));
      p.drawRect(0, 0, pm.width() - 1, pm.height() - 1);
      si.setPixmap(pm);
      ScoreItem* item = new ScoreItem(si);
      item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);

      QString s(si.completeBaseName());
      if (!s.isEmpty() && s[0].isNumber() && _stripNumbers)
            s = s.mid(3);
      s = s.replace('_', ' ');
      item->setText(s);
      QFont f = item->font();
      f.setPointSize(f.pointSize() - 2.0);
      f.setBold(_boldTitle);
      item->setFont(f);
      item->setTextAlignment(Qt::AlignHCenter | Qt::AlignTop);
      item->setIcon(QIcon(si.pixmap()));
      item->setSizeHint(l->cellSize());
      return item;
      }

//---------------------------------------------------------
//   setScores
//---------------------------------------------------------

void ScoreBrowser::setScores(const QFileInfoList& s)
      {
      qDeleteAll(scoreLists);
      scoreLists.clear();

      QLayout* l = scoreList->layout();
      while (l->count())
            l->removeItem(l->itemAt(0));

      ScoreListWidget* sl = 0;

      QStringList filter = { "*.mscz" };

      QSet<QString> entries; //to avoid duplicates
      for (const QFileInfo& fi : s) {
            if (fi.isFile()) {
                  QString s = fi.filePath();
                  if(entries.contains(s))
                      continue;
                  if (s.endsWith(".mscz") || s.endsWith(".mscx")) {
                        if (!sl)
                              sl = createScoreList();
                        sl->addItem(genScoreItem(fi, sl));
                        entries.insert(s);
                        }
                  }
            }
      for (const QFileInfo& fi : s) {
            if (fi.isDir()) {
                  QString s(fi.fileName());
                  if (!s.isEmpty() && s[0].isNumber() && _stripNumbers)
                        s = s.mid(3);
                  s = s.replace('_', ' ');
                  QLabel* label = new QLabel(s);
                  QFont f = label->font();
                  f.setBold(true);
                  label->setFont(f);
                  static_cast<QVBoxLayout*>(l)->addWidget(label);
                  QDir dir(fi.filePath());
                  sl = createScoreList();
                  unsigned count = 0; //nbr of entries added
                  for (const QFileInfo& fi : dir.entryInfoList(filter, QDir::Files, QDir::Name)){
                        if(entries.contains(fi.filePath()))
                            continue;
                        sl->addItem(genScoreItem(fi, sl));
                        count++;
                        entries.insert(fi.filePath());
                        }
                  if(count==0){
                        delete label;
                        delete sl;
                        }
                  sl = 0;
                  }
            }
      }

//---------------------------------------------------------
//   selectFirst
//---------------------------------------------------------

void ScoreBrowser::selectFirst()
      {
      if (scoreLists.isEmpty())
            return;
      ScoreListWidget* w = scoreLists.front();
      if (w->count() == 0)
            return;
      ScoreItem* item = static_cast<ScoreItem*>(w->item(0));
      w->setCurrentItem(item);
      preview->setScore(item->info());
      }

//---------------------------------------------------------
//   selectLast
//---------------------------------------------------------

void ScoreBrowser::selectLast()
      {
      if (scoreLists.isEmpty())
            return;
      ScoreListWidget* w = scoreLists.front();
      if (w->count() == 0)
            return;
      ScoreItem* item = static_cast<ScoreItem*>(w->item(w->count()-1));
      w->setCurrentItem(item);
      preview->setScore(item->info());
      }

//---------------------------------------------------------
//   scoreChanged
//---------------------------------------------------------

void ScoreBrowser::scoreChanged(QListWidgetItem* current)
      {
      if (!current)
            return;
      ScoreItem* item = static_cast<ScoreItem*>(current);
      if (!_showPreview)
            emit scoreActivated(item->info().filePath());
      else {
            preview->setScore(item->info());
            emit scoreSelected(item->info().filePath());

            for (ScoreListWidget* sl : scoreLists) {
                  if (static_cast<QListWidget*>(sl) != item->listWidget()) {
                        sl->clearSelection();
                        }
                  }
            }
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

