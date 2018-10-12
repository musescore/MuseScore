//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: mscore.cpp 4220 2011-04-22 10:31:26Z wschweer $
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "layer.h"
#include "libmscore/score.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   LayerManager
//---------------------------------------------------------

LayerManager::LayerManager(Score* s, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("LayerManager");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      score = s;

      for (int i = 0; i < 31; ++i) {
            QTableWidgetItem* item = new QTableWidgetItem(score->layerTags()[i+1]);
            tags->setItem(i, 0, item);
            item = new QTableWidgetItem(score->layerTagComments()[i+1]);
            tags->setItem(i, 1, item);
            }

      layers->setRowCount(score->layer().size());
      int row = 0;
      foreach(const Layer& l, score->layer()) {
            QTableWidgetItem* item = new QTableWidgetItem(l.name);
            layers->setItem(row, 0, item);
            QString tagString;
            for (int i = 0; i < 31; ++i) {
                  uint mask = 1 << (i+1);
                  if (mask &  l.tags) {
                        if (!tagString.isEmpty())
                              tagString += ",";
                        tagString += tags->item(i, 0)->text();
                        }
                  }
            item = new QTableWidgetItem(tagString);
            layers->setItem(row, 1, item);
            ++row;
            }
      layers->setCurrentCell(score->currentLayer(), 0);

      connect(createButton, SIGNAL(clicked()), SLOT(createClicked()));
      connect(deleteButton, SIGNAL(clicked()), SLOT(deleteClicked()));
      connect(addTagButton, SIGNAL(clicked()), SLOT(addTagClicked()));
      connect(deleteTagButton, SIGNAL(clicked()), SLOT(deleteTagClicked()));

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   showLayerManager
//---------------------------------------------------------

void MuseScore::showLayerManager()
      {
      LayerManager am(cs);
      am.exec();
      }

//---------------------------------------------------------
//   createClicked
//---------------------------------------------------------

void LayerManager::createClicked()
      {
      int row      = layers->rowCount();
      QString name = QString("layer%1").arg(row + 1);
      QTableWidgetItem* item = new QTableWidgetItem(name);
      layers->setRowCount(row+1);
      layers->setItem(row, 0, item);
      item = new QTableWidgetItem("");
      layers->setItem(row, 1, item);
      }

//---------------------------------------------------------
//   deleteClicked
//---------------------------------------------------------

void LayerManager::deleteClicked()
      {
      qDebug("TODO");
      }

//---------------------------------------------------------
//   addTagClicked
//---------------------------------------------------------

void LayerManager::addTagClicked()
      {
      int row = layers->currentRow();
      if (row == -1)
            return;
      QStringList items;
      for (int i = 0; i < 31; ++i) {
            QString s = score->layerTags()[i+1];
            if (!s.isEmpty())
                  items.append(s);
            }
      for (int i = 0; i < 31; ++i) {
            QString tag(tags->item(i, 0)->text());
            if (!tag.isEmpty())
                  items.append(tag);
            }

      if (items.isEmpty()) {
            qDebug("no tags defined");
            return;
            }
      bool ok;
      QString item = QInputDialog::getItem(this, tr("Select layer tag"), tr("layer tag"),
         items, 0, false, &ok);
      if (ok && !item.isEmpty()) {
//            uint tagBits = 0;
            for (int i = 0; i < 31; ++i) {
                  QString s = score->layerTags()[i+1];
                  if (s == item) {
//                        tagBits = 1 << (i+1);
                        break;
                        }
                  }
            QTableWidgetItem* wi = layers->item(row, 1);
            QString s = wi->text();
            if (!s.isEmpty())
                  s += ",";
            s += item;
            wi->setText(s);
            }
      }

//---------------------------------------------------------
//   deleteTagClicked
//---------------------------------------------------------

void LayerManager::deleteTagClicked()
      {
      int row = layers->currentRow();
      if (row == -1)
            return;
      QTableWidgetItem* item = layers->item(row, 1);
      QString s = item->text();
      QStringList items = s.split(",");
      bool ok;
      QString tag = QInputDialog::getItem(this, tr("Select layer tag"), tr("layer tag"),
         items, 0, false, &ok);
      if (ok && !tag.isEmpty()) {
            items.removeOne(tag);
            item->setText(items.join(","));
            }
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void LayerManager:: accept()
      {
      score->startCmd();
      for (int i = 0; i < 31; ++i) {
            QString tag(tags->item(i, 0)->text());
            QString comment(tags->item(i, 1)->text());
            score->layerTags()[i+1] = tag;
            score->layerTagComments()[i+1] = comment;
            }
      int row = layers->currentRow();
      if (row != -1)
            score->setCurrentLayer(row);

      QList<Layer>& layer = score->layer();
      layer.clear();

      int n = layers->rowCount();
      for (int i = 0; i < n; ++i) {
            Layer l;
            l.name           = layers->item(i, 0)->text();
            l.tags           = 1;
            QString ts       = layers->item(i, 1)->text();
            QStringList tgs  = ts.split(",");
            foreach (QString tag, tgs) {
                  for (int idx = 0; idx < 32; ++idx) {
                        if (tag == score->layerTags()[idx]) {
                              l.tags |= 1 << idx;
                              break;
                              }
                        }
                  }
            if (i == 0)             // hardwired default tag
                  l.tags |= 1;
            layer.append(l);
            }
      score->setLayoutAll();
      score->endCmd();
      if (enableExperimental)
      	mscore->updateLayer();
      QDialog::accept();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void LayerManager::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

}

