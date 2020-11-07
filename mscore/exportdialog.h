//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#ifndef __EXPORTDIALOG_H__
#define __EXPORTDIALOG_H__

#include "ui_exportdialog.h"
#include "abstractdialog.h"
#include "libmscore/excerpt.h"
#include "libmscore/mscore.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   ExportScoreItem
//---------------------------------------------------------

class ExportScoreItem : public QListWidgetItem {
      Score* _score;

   public:
      ExportScoreItem(Score*, QListWidget* parent = 0);

      Score* score() { return _score; }
      
      bool isChecked() const { return checkState() == Qt::CheckState::Checked; }
      void setChecked(bool b) { setCheckState(b ? Qt::CheckState::Checked : Qt::CheckState::Unchecked); }

      QString title() const { return _score->title(); }
      };

//---------------------------------------------------------
//   ExportDialog
//---------------------------------------------------------

class ExportDialog : public AbstractDialog, public Ui::ExportDialog {
      Q_OBJECT
      
      QButtonGroup* pdfSeparateOrSingleFiles;
      
      Score* cs = nullptr;
      
      void loadValues();
      void loadScoreAndPartsList();

      virtual void showEvent(QShowEvent*);
      virtual void hideEvent(QHideEvent*);
      
   private slots:
      void fileTypeChosen(int);
      void selectAll();
      void selectCurrent();
      void selectScore();
      void selectParts();
      void clearSelection();
      void setOkButtonEnabled();

   protected:
      virtual void retranslate();
      
   public:
      ExportDialog(Score* s, QWidget* parent);
      void setScore(Score* s) { cs = s; }
      void setType(const QString& type = "");
    
   public slots:
      void accept();
};
}

#endif
