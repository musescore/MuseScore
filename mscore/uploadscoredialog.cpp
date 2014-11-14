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

#include "musescore.h"
#include "libmscore/score.h"
#include "uploadscoredialog.h"

namespace Ms {

//---------------------------------------------------------
//   showUploadScore
//---------------------------------------------------------

void MuseScore::showUploadScoreDialog()
      {
      if (uploadScoreDialog == nullptr) {
            uploadScoreDialog = new UploadScoreDialog(_loginManager);
            }
      
      if (currentScore()) {
            uploadScoreDialog->setTitle(currentScore()->title());
            _loginManager->tryLogin();
            }
      }

//---------------------------------------------------------
//   UploadScoreDialog
//---------------------------------------------------------

UploadScoreDialog::UploadScoreDialog(LoginManager* loginManager)
 : QDialog(0)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      license->addItem(tr("All Rights reserved"), "all-rights-reserved");
      license->addItem(tr("Creative Commons Attribution"), "cc-by");
	license->addItem(tr("Creative Commons Attribution Share Alike"), "cc-by-sa");
      license->addItem(tr("Creative Commons Attribution No Derivative Works"), "cc-by-nd");
      license->addItem(tr("Creative Commons Attribution Noncommercial"), "cc-by-nc");
      license->addItem(tr("Creative Commons Attribution Noncommercial Share Alike"), "cc-by-nc-sa");
	license->addItem(tr("Creative Commons Attribution Noncommercial Non Derivate Works"), "cc-by-nc-nd");
      license->addItem(tr("Public Domain"), "publicdomain");
      license->addItem(tr("Creative Commons Zero"), "cc-zero");
      connect(buttonBox,   SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
      chkSignoutOnExit->setVisible(false);
      _loginManager = loginManager;
      connect(_loginManager, SIGNAL(uploadSuccess(QString)), this, SLOT(uploadSuccess(QString)));
      connect(_loginManager, SIGNAL(uploadError(QString)), this, SLOT(uploadError(QString)));
      connect(_loginManager, SIGNAL(tryLoginSuccess()), this, SLOT(display()));
      connect(btnSignout, SIGNAL(pressed()), this, SLOT(logout()));
      }
      
//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void UploadScoreDialog::buttonBoxClicked(QAbstractButton* button)
      {
      QDialogButtonBox::StandardButton sb = buttonBox->standardButton(button);
      if (sb == QDialogButtonBox::Ok)
            upload();
      else
           setVisible(false);
      }

//---------------------------------------------------------
//   upload
//---------------------------------------------------------

void UploadScoreDialog::upload()
     {
     if (title->text().trimmed().isEmpty()) {
           QMessageBox::critical(this, tr("Missing title"), tr("Please provide a title"));
           return;
           }
     Score* score = mscore->currentScore();
     QString path = QDir::tempPath() + "/temp.mscz";
     if(mscore->saveAs(score, true, path, "mscz")) {
           QString licenseString = license->currentData().toString();
           QString privateString = rbPrivate->isChecked() ? "1" : "0";
            _loginManager->upload(path, title->text(), description->toPlainText(), privateString, licenseString, tags->text());
           }
     }

//---------------------------------------------------------
//   uploadSuccess
//---------------------------------------------------------

void UploadScoreDialog::uploadSuccess(const QString& url)
      {
      setVisible(false);
      QMessageBox::information(this,
               tr("Success"),
               tr("Finished! <a href=\"%1\">Go to my score</a>.").arg(url),
               QMessageBox::Ok, QMessageBox::NoButton);
      
      }

//---------------------------------------------------------
//   uploadError
//---------------------------------------------------------

void UploadScoreDialog::uploadError(const QString& error)
      {
      QMessageBox::information(this,
               tr("Error"),
               error,
               QMessageBox::Ok, QMessageBox::NoButton);
      }

//---------------------------------------------------------
//   display
//---------------------------------------------------------

void UploadScoreDialog::display()
      {
      lblUsername->setText(_loginManager->userName());
      // clear the content
      description->clear();
      rbPrivate->setChecked(false);
      rbPublic->setChecked(true);
      license->setCurrentIndex(0);
      tags->clear();
      setVisible(true);
      }

//---------------------------------------------------------
//   logout
//---------------------------------------------------------

void UploadScoreDialog::logout()
      {
      _loginManager->logout();
      setVisible(false);
      }
}

