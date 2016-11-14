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
      int _nid;

      virtual void hideEvent(QHideEvent*);
   private slots:
      void buttonBoxClicked(QAbstractButton* button);
      void uploadSuccess(const QString& url);
      void uploadError(const QString& error);
      void onGetScoreSuccess(const QString &title, const QString &description, bool priv, const QString& license, const QString& tags, const QString &url);
      void onGetScoreError(const QString& error);
      void logout();
      void display();

   private:
      void upload(int nid);
      void clear();

   public:
      UploadScoreDialog(LoginManager*);
      void setTitle(const QString& t) { title->setText(t); }
      };
}

#endif
