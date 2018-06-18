//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __WORKSPACEDIALOG_H__
#define __WORKSPACEDIALOG_H__

#include "ui_workspacedialog.h"

namespace Ms {

class WorkspaceDialog : public QDialog, Ui::WorkspaceDialog
      {
      Q_OBJECT

   public:
      explicit WorkspaceDialog(QWidget *parent = 0);
      bool editMode; // Set when editing the workspace
      void display();

   private slots:
      void accepted();

   protected:
      virtual void changeEvent(QEvent* event);
      void retranslate()  { retranslateUi(this); }

      };

}
#endif // __WORKSPACEDIALOG_H__
