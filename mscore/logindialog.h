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

#ifndef __LOGINDIALOG_H__
#define __LOGINDIALOG_H__

#include "ui_logindialog.h"

namespace Ms {

class LoginManager;

//---------------------------------------------------------
//   LoginDialog
//    Old-style login dialog in case QtWebEngine is
//    unavailable.
//---------------------------------------------------------

class LoginDialog : public QDialog, public Ui::LoginDialog
      {
      Q_OBJECT
      
      LoginManager* _loginManager;

      virtual void hideEvent(QHideEvent*);

   signals:
      void loginSuccessful();
      
   private slots:
      void buttonBoxClicked(QAbstractButton* button);
      void onLoginSuccess();
      void onLoginError(const QString& error);
      void login();

   public:
      LoginDialog(LoginManager* loginManager);
      };
}

#endif

