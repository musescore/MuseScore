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

#ifndef __UPLOADSCOREDIALOG_H__
#define __UPLOADSCOREDIALOG_H__

#include "ui_uploadscoredialog.h"
#include "loginmanager.h"

namespace Ms {

//---------------------------------------------------------
//   LoginDialog
//---------------------------------------------------------

class UploadScoreDialog : public QDialog, public Ui::UploadScoreDialog
      {
      Q_OBJECT

      LoginManager* _loginManager;

   private slots:
      void buttonBoxClicked(QAbstractButton* button);
      void uploadSuccess(const QString& url);
      void uploadError(const QString& error);
      void logout();
      void display();

   private:
      void upload();

   public:
      UploadScoreDialog(LoginManager*);
      void setTitle(const QString& t) { title->setText(t); }
      };
}

#endif
