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

#include "templateBrowser.h"
#include "musescore.h"
#include "icons.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   TemplateItem
//---------------------------------------------------------

class TemplateItem : public QTreeWidgetItem
      {
      ScoreInfo _info;

   public:
      TemplateItem(const ScoreInfo& i, QTreeWidgetItem* parent = 0);
      const ScoreInfo& info() const { return _info; }
      };

TemplateItem::TemplateItem(const ScoreInfo &i, QTreeWidgetItem *parent) : QTreeWidgetItem(parent, 0), _info(i)
      {
      setFlags(Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren));
      }

//---------------------------------------------------------
//   TemplateCategory
//---------------------------------------------------------

class TemplateCategory : public QTreeWidgetItem
      {

   public:
      TemplateCategory(QString name, QTreeWidget* parent = 0);
      };

TemplateCategory::TemplateCategory(QString name, QTreeWidget *parent)
   : QTreeWidgetItem(parent, 0)
      {
      const int nameCol = 0;
      setText(nameCol, name);
      // provide feedback to blind users that they have selected a category rather than a template
      setData(nameCol, Qt::AccessibleTextRole, QVariant(QObject::tr("%1 category").arg(name))); // spoken by screen readers
      QFont nameFont = QFont();
      nameFont.setBold(true);
      setFont(nameCol, nameFont);
      setFlags(Qt::ItemIsEnabled);
      }

//---------------------------------------------------------
//   TemplateBrowser
//---------------------------------------------------------

TemplateBrowser::TemplateBrowser(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      if (_showPreview) {
            preview->displayInfo(false);
            connect(preview, SIGNAL(doubleClicked(QString)), SIGNAL(scoreActivated(QString)));
            }
      else
            preview->setVisible(false);
      connect(templateTree, &QTreeWidget::itemSelectionChanged, this, &TemplateBrowser::scoreClicked);
      templateSearch->setFilterableView(templateTree);
      }

//---------------------------------------------------------
//   genTemplateItem
//---------------------------------------------------------

TemplateItem* TemplateBrowser::genTemplateItem(QTreeWidgetItem* p, const QFileInfo& fi)
      {
      ScoreInfo si(fi);
      QPixmap pm(QSize(181,256));
      if (!QPixmapCache::find(fi.filePath(), &pm)) {
            //load and scale pixmap
            QPixmap pixmap = mscore->extractThumbnail(fi.filePath());
            if (pixmap.isNull())
                  pixmap = icons[int(Icons::file_ICON)]->pixmap(QSize(50,60));
            // draw pixmap and add border
            pm.fill(Qt::transparent);
            QPainter painter( &pm );
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::TextAntialiasing);
            painter.drawPixmap(0, 0, pixmap);
            painter.setPen(QPen(QColor(0, 0, 0, 128), 1));
            painter.setBrush(Qt::white);
            if (fi.completeBaseName() == "00-Blank" || fi.completeBaseName() == "Create_New_Score") {
                  qreal round = 8.0 * qApp->devicePixelRatio();
                  painter.drawRoundedRect(QRectF(0, 0, pm.width() - 1 , pm.height() - 1), round, round);
                  }
            else
                  painter.drawRect(0, 0, pm.width()  - 1, pm.height()  - 1);
            if (fi.completeBaseName() != "00-Blank")
                  painter.drawPixmap(1, 1, pixmap);
            painter.end();
            QPixmapCache::insert(fi.filePath(), pm);
            }

      si.setPixmap(pm);
      TemplateItem* item = new TemplateItem(si, p);

      if (fi.completeBaseName() == "00-Blank") {
            item->setText(0, tr("Choose Instruments"));
            }
      else if (fi.completeBaseName() == "Create_New_Score") {
            item->setText(0, tr("Create New Score…"));
            }
      else {
            QString s(si.completeBaseName());
            if (!s.isEmpty() && s[0].isNumber() && _stripNumbers)
                  s = s.mid(3);
            s = s.replace('_', ' ');
            item->setText(0, s);
            }
      return item;
      }

//---------------------------------------------------------
//   setScores
//---------------------------------------------------------

void TemplateBrowser::setScores(QFileInfoList& s)
      {
      QStringList filter = { "*.mscz", "*.mscx" };
      templateTree->clear(); // empty the tree

      if (_showCustomCategory)
            std::sort(s.begin(), s.end(), [](QFileInfo a, QFileInfo b)->bool { return a.fileName() < b.fileName(); });

      TemplateCategory* customCategory = new TemplateCategory(tr("Custom Templates"));

      QSet<QString> entries; //to avoid duplicates
      for (const QFileInfo& fil : s) {
            if (fil.isDir()) {
                  QString st(fil.fileName());
                  if (!st.isEmpty() && st[0].isNumber() && _stripNumbers)
                        st = st.mid(3);
                  st = st.replace('_', ' ');
                  TemplateCategory* category = new TemplateCategory(st, templateTree);
                  QDir dir(fil.filePath());
                  unsigned childCount = 0; //nbr of entries added
                  for (const QFileInfo& fi : dir.entryInfoList(filter, QDir::Files, QDir::Name)) {
                        if (entries.contains(fi.filePath()))
                              continue;
                        genTemplateItem(category, fi);
                        childCount++;
                        entries.insert(fi.filePath());
                        }
                  if (childCount == 0) {
                        // delete any unnecessary categories
                        delete category;
                        }
                  }
            else if (fil.isFile()) {
                  QString st = fil.filePath();
                  if (entries.contains(st))
                        continue;
                  if (st.endsWith(".mscz") || st.endsWith(".mscx")) {
                        genTemplateItem(customCategory, fil);
                        entries.insert(st);
                        }
                  }
            }
      if (customCategory->childCount() > 0)
            templateTree->insertTopLevelItem(1, customCategory);
      templateTree->toInitialState();
      templateTree->selectFirst();
      }

//---------------------------------------------------------
//   filter
//      filter which scores are visible based on searchString
//---------------------------------------------------------
void TemplateBrowser::filter(const QString &searchString)
      {
      templateTree->filter(searchString);
      }

//---------------------------------------------------------
//   scoreClicked
//---------------------------------------------------------

void TemplateBrowser::scoreClicked()
      {
      QList<QTreeWidgetItem*> selectedItems = templateTree->selectedItems();
      if (!selectedItems.isEmpty()) {
            QTreeWidgetItem* selectedItem = selectedItems.first();
            if (selectedItem) {
                  TemplateItem* selectedScore = static_cast<TemplateItem*>(selectedItem);
                  if (_showPreview)
                        preview->setScore(selectedScore->info());
                  emit scoreSelected(selectedScore->info().filePath());
                  return;
                  }
            }
      if (_showPreview)
            preview->unsetScore();
      emit scoreSelected(""); // no score selected
      }

}
