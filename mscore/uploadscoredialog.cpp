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
#include "libmscore/undo.h"
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
      connect(_loginManager, SIGNAL(getScoreSuccess(QString, QString, bool, QString, QString)), this, SLOT(onGetScoreSuccess(QString, QString, bool, QString, QString)));
      connect(_loginManager, SIGNAL(getScoreError(QString)), this, SLOT(onGetScoreError(QString)));
      connect(_loginManager, SIGNAL(tryLoginSuccess()), this, SLOT(display()));
      connect(btnSignout, SIGNAL(pressed()), this, SLOT(logout()));
      }
      
//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void UploadScoreDialog::buttonBoxClicked(QAbstractButton* button)
      {
      QDialogButtonBox::StandardButton sb = buttonBox->standardButton(button);
      if (sb == QDialogButtonBox::Save)
            upload(updateExistingCb->isChecked() ? _nid : -1);
      else
           setVisible(false);
      }

//---------------------------------------------------------
//   upload
//---------------------------------------------------------

void UploadScoreDialog::upload(int nid)
     {
     if (title->text().trimmed().isEmpty()) {
           QMessageBox::critical(this, tr("Missing title"), tr("Please provide a title"));
           return;
           }
     Score* score = mscore->currentScore()->rootScore();
     QString path = QDir::tempPath() + "/temp.mscz";
     if(mscore->saveAs(score, true, path, "mscz")) {
           QString licenseString = license->currentData().toString();
           QString privateString = rbPrivate->isChecked() ? "1" : "0";
            _loginManager->upload(path, nid, title->text(), description->toPlainText(), privateString, licenseString, tags->text());
           }
     }

//---------------------------------------------------------
//   uploadSuccess
//---------------------------------------------------------

void UploadScoreDialog::uploadSuccess(const QString& url)
      {
      setVisible(false);
      Score* score = mscore->currentScore()->rootScore();
      QMap<QString, QString>  metatags = score->metaTags();
      metatags.insert("source", url);
      score->startCmd();
      score->undo(new ChangeMetaTags(score, metatags));
      score->endCmd();
      mscore->endCmd();
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
      QString source = mscore->currentScore()->rootScore()->metaTag("source");
      if (!source.isEmpty()) {
            QStringList sl = source.split("/");
            if (sl.length() > 0) {
                  QString nidString = sl.last();
                  bool ok;
			int nid = nidString.toInt(&ok);
                  if (ok) {
                         _nid = nid;
                         _loginManager->getScore(nid);
                         return;
                         }
                  }
            }
      clear();
      setVisible(true);
      }

//---------------------------------------------------------
//   onGetScoreSuccess
//---------------------------------------------------------

void UploadScoreDialog::onGetScoreSuccess(const QString &t, const QString &desc, bool priv, const QString& lic, const QString& tag)
      {
      // file with score info
      title->setText(t);
      description->setPlainText(desc);
      rbPrivate->setChecked(priv);
      rbPublic->setChecked(!priv);
      int lIndex = license->findData(lic);
      if (lIndex < 0) lIndex = 0;
      license->setCurrentIndex(lIndex);
      tags->setText(tag);
      updateExistingCb->setChecked(true);
      updateExistingCb->setVisible(true);
      setVisible(true);
      }

//---------------------------------------------------------
//   onGetScoreError
//---------------------------------------------------------

void UploadScoreDialog::onGetScoreError(const QString& /*error*/)
      {
      clear();
      setVisible(true);
      }

//---------------------------------------------------------
//   onGetScoreError
//---------------------------------------------------------

void UploadScoreDialog::clear()
      {
      description->clear();
      rbPrivate->setChecked(false);
      rbPublic->setChecked(true);
      license->setCurrentIndex(0);
      tags->clear();
      updateExistingCb->setChecked(false);
      updateExistingCb->setVisible(false);
      _nid = -1;
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

