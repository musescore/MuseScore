//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA
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

#ifndef __MUSESCOREDIALOGS_H__
#define __MUSESCOREDIALOGS_H__

#include "ui_measuresdialog.h"
#include "ui_insertmeasuresdialog.h"
#include "ui_aboutbox.h"
#include "ui_aboutmusicxmlbox.h"

namespace Ms {

//---------------------------------------------------------
//   AboutBoxDialog
//---------------------------------------------------------

class AboutBoxDialog : public QDialog, Ui::AboutBox {
      Q_OBJECT

   public:
      AboutBoxDialog();

   private slots:
      void copyRevisionToClipboard();
      };

//---------------------------------------------------------
//   AboutMusicXMLBoxDialog
//---------------------------------------------------------

class AboutMusicXMLBoxDialog : public QDialog, Ui::AboutMusicXMLBox {
      Q_OBJECT

   public:
      AboutMusicXMLBoxDialog();
      };

//---------------------------------------------------------
//   InsertMeasuresDialog
//   Added by DK, 05.08.07
//---------------------------------------------------------

class InsertMeasuresDialog : public QDialog, public Ui::InsertMeasuresDialogBase {
      Q_OBJECT

      void hideEvent(QHideEvent*) override;
      void accept() override;

   private slots:
      void buttonBoxClicked(QAbstractButton*);

   public:
      InsertMeasuresDialog(QWidget* parent = 0);
      };

//---------------------------------------------------------
//   MeasuresDialog
//---------------------------------------------------------

class MeasuresDialog : public QDialog, public Ui::MeasuresDialogBase {
      Q_OBJECT

      void accept() override;

   private slots:
      void buttonBoxClicked(QAbstractButton*);

   public:
      MeasuresDialog(QWidget* parent = 0);
      };

} // namespace Ms

#endif
