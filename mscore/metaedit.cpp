//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: metaedit.cpp 5290 2012-02-07 16:27:27Z wschweer $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#include "metaedit.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"

namespace Ms {

//---------------------------------------------------------
//   MetaEditDialog
//---------------------------------------------------------

MetaEditDialog::MetaEditDialog(Score* s, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      score = s;
      dirty = false;

      level->setValue(score->mscVersion());
      version->setText(score->mscoreVersion());
      revision->setValue(score->mscoreRevision());
      filePath->setText(score->importedFilePath());

      int idx = 0;
      QMapIterator<QString, QString> i(s->metaTags());
      QGridLayout* grid = static_cast<QGridLayout*>(scrollWidget->layout());
      while (i.hasNext()) {
            i.next();
            QLabel* label = new QLabel;
            label->setText(i.key());
            QLineEdit* text = new QLineEdit(i.value(), 0);
            connect(text, SIGNAL(textChanged(const QString&)), SLOT(setDirty()));
            grid->addWidget(label, idx, 0);
            grid->addWidget(text, idx, 1);
            ++idx;
            }
      connect(newButton, SIGNAL(clicked()), SLOT(newClicked()));
      }

//---------------------------------------------------------
//   newClicked
//---------------------------------------------------------

void MetaEditDialog::newClicked()
      {
      QString s = QInputDialog::getText(this,
         tr("MuseScore: Input Tag Name"),
         tr("New tag name:")
         );
      QGridLayout* grid = static_cast<QGridLayout*>(scrollWidget->layout());
      if (!s.isEmpty()) {
            int idx = grid->rowCount();
            QLabel* label = new QLabel;
            label->setText(s);
            QLineEdit* text = new QLineEdit;
            grid->addWidget(label, idx, 0);
            grid->addWidget(text, idx, 1);
            }
      dirty = true;
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void MetaEditDialog::accept()
      {
      if (dirty) {
            QGridLayout* grid = static_cast<QGridLayout*>(scrollWidget->layout());
            int idx = grid->rowCount();
            QMap<QString, QString> m;
            for (int i = 0; i < idx; ++i) {
                  QLayoutItem* labelItem = grid->itemAtPosition(i, 0);
                  QLayoutItem* dataItem  = grid->itemAtPosition(i, 1);
                  if (labelItem && dataItem) {
                        QLabel* label = static_cast<QLabel*>(labelItem->widget());
                        QLineEdit* le = static_cast<QLineEdit*>(dataItem->widget());
                        m.insert(label->text(), le->text());
                        }
                  }
            score->undo(new ChangeMetaTags(score, m));
            }
      QDialog::accept();
      }
}

