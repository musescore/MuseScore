//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __INSTRDIALOG_H__
#define __INSTRDIALOG_H__

#include "ui_instrdialog.h"

namespace Ms {

class Score;

//---------------------------------------------------------
//   InstrumentsDialog
//---------------------------------------------------------

class InstrumentsDialog : public QDialog, public Ui::InstrumentsDialog {
      Q_OBJECT

      void readSettings();

   private slots:
      virtual void accept();
      void on_saveButton_clicked();
      void on_loadButton_clicked();

   public:
      InstrumentsDialog(QWidget* parent = 0);
      void writeSettings();
      void genPartList(Score*);
      QTreeWidget* partiturList();
      void buildInstrumentsList();
      };

} // namespace Ms


#endif

