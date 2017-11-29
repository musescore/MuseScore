//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "pathlistdialog.h"
#include "preferences.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   PathListDialog
//---------------------------------------------------------

PathListDialog::PathListDialog(QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("PathListDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      connect(add, SIGNAL(clicked()), SLOT(addClicked()));
      connect(remove, SIGNAL(clicked()), SLOT(removeClicked()));

      MuseScore::restoreGeometry(this);
      }


//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void PathListDialog::addClicked()
      {
      QString newPath = QFileDialog::getExistingDirectory(
         this,
         tr("Choose a directory"),
         QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).arg(QCoreApplication::applicationName()),
         QFileDialog::ShowDirsOnly | (preferences.getBool(PREF_UI_APP_USENATIVEDIALOGS) ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog)
         );
      if (!newPath.isEmpty()) {
            newPath = QDir(newPath).absolutePath();
            if(files->findItems(newPath, Qt::MatchExactly).size() == 0)
                  files->addItem(newPath);
            }
      }

//---------------------------------------------------------
//   removeClicked
//---------------------------------------------------------

void PathListDialog::removeClicked()
      {
      int n = files->currentRow();
      if (n == -1)
            return;
      files->takeItem(n);
      }

//---------------------------------------------------------
//   path
//---------------------------------------------------------
QString PathListDialog::path()
      {
      int n = files->count();
      QStringList sl;
      for (int i = 0; i < n; ++i) {
            QListWidgetItem* item = files->item(i);
            sl.append(item->text());
            }
      return sl.join(";");
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------
void PathListDialog::setPath(QString path)
      {
      QStringList pl = path.split(";");
      files->addItems(pl);
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void PathListDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

}

