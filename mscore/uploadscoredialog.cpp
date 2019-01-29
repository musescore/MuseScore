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
#include "network/loginmanager.h"
#include "uploadscoredialog.h"

namespace Ms {

//---------------------------------------------------------
//   showUploadScore
//---------------------------------------------------------

void MuseScore::showUploadScoreDialog()
      {
      if (!currentScore())
            return;
      if (!currentScore()->sanityCheck(QString())) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(QObject::tr("Upload Error"));
            msgBox.setText(tr("This score cannot be saved online. Please fix the corrupted measures and try again."));
            msgBox.setDetailedText(MScore::lastError);
            msgBox.setTextFormat(Qt::RichText);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
            return;
            }
      if (uploadScoreDialog == nullptr) {
            uploadScoreDialog = new UploadScoreDialog(_loginManager);
            }

      uploadScoreDialog->setTitle(currentScore()->title());
      _loginManager->tryLogin();
      }

//---------------------------------------------------------
//   UploadScoreDialog
//---------------------------------------------------------

UploadScoreDialog::UploadScoreDialog(LoginManager* loginManager)
 : QDialog(0)
      {
      setObjectName("UploadScoreDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      license->addItem(tr("All Rights reserved"), "all-rights-reserved");
      license->addItem(tr("Creative Commons Attribution"), "cc-by");
      license->addItem(tr("Creative Commons Attribution No Derivative Works"), "cc-by-nd");
      license->addItem(tr("Creative Commons Attribution Share Alike"), "cc-by-sa");
      license->addItem(tr("Creative Commons Attribution Noncommercial"), "cc-by-nc");
      license->addItem(tr("Creative Commons Attribution Noncommercial Non Derivate Works"), "cc-by-nc-nd");
      license->addItem(tr("Creative Commons Attribution Noncommercial Share Alike"), "cc-by-nc-sa");
      license->addItem(tr("Creative Commons Copyright Waiver"), "cc-zero");

      licenseHelp->setText(tr("%1What does this mean?%2")
                           .arg("<a href=\"http://redirect.musescore.com/help/license\">")
                           .arg("</a>"));
      QFont font = licenseHelp->font();
      font.setPointSize(8);
      font.setItalic(true);
      licenseHelp->setFont(font);

      privateHelp->setText(tr("Respect the %1community guidelines%2. Only make your scores accessible to anyone with permission from the right holders.")
                           .arg("<a href=\"https://musescore.com/community-guidelines\">")
                           .arg("</a>"));
      privateHelp->setFont(font);

      tagsHelp->setText(tr("Use a comma to separate the tags"));
      tagsHelp->setFont(font);

      uploadAudioHelp->setFont(font);
      QString urlHelp = QString("https://musescore.org/redirect/handbook?chapter=upload-score-audio&locale=%1&utm_source=desktop&utm_medium=save-online&utm_content=%2&utm_term=upload-score-audio&utm_campaign=MuseScore%3")
         .arg(mscore->getLocaleISOCode())
         .arg(mscore->revision().trimmed())
         .arg(QString(VERSION));
      uploadAudioHelp->setText(tr("Render the score with the current synth settings. %1More info%2.")
                          .arg("<a href=\"" + urlHelp + "\">")
                          .arg("</a>"));
      lblChanges->setVisible(false);
      changes->setVisible(false);

      connect(updateExistingCb, SIGNAL(toggled(bool)), lblChanges, SLOT(setVisible(bool)));
      connect(updateExistingCb, SIGNAL(toggled(bool)), changes, SLOT(setVisible(bool)));

      connect(buttonBox,   SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
      chkSignoutOnExit->setVisible(false);  // currently unused, so hide it
      _loginManager = loginManager;
      connect(_loginManager, SIGNAL(uploadSuccess(QString, QString, QString)), this, SLOT(uploadSuccess(QString, QString, QString)));
      connect(_loginManager, SIGNAL(uploadError(QString)), this, SLOT(uploadError(QString)));
      connect(_loginManager, SIGNAL(getScoreSuccess(QString, QString, bool, QString, QString, QString)), this, SLOT(onGetScoreSuccess(QString, QString, bool, QString, QString, QString)));
      connect(_loginManager, SIGNAL(getScoreError(QString)), this, SLOT(onGetScoreError(QString)));
      connect(_loginManager, SIGNAL(tryLoginSuccess()), this, SLOT(display()));
      connect(_loginManager, SIGNAL(displaySuccess()), this, SLOT(displaySuccess()));
      connect(btnSignout, SIGNAL(pressed()), this, SLOT(logout()));

      MuseScore::restoreGeometry(this);
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
     Score* score = mscore->currentScore()->masterScore();
     QString path = QDir::tempPath() + QString("/temp_%1.mscz").arg(qrand() % 100000);
     if(mscore->saveAs(score, true, path, "mscz")) {
           QString licenseString = license->currentData().toString();
           QString privateString = cbPrivate->isChecked() ? "1" : "0";
            _loginManager->upload(path, nid, title->text(), description->toPlainText(), privateString, licenseString, tags->text(), changes->toPlainText());
           }
     }

//---------------------------------------------------------
//   uploadSuccess
//---------------------------------------------------------

void UploadScoreDialog::uploadSuccess(const QString& url, const QString& nid, const QString& vid)
      {
      setVisible(false);
      _url = url;
      Score* score = mscore->currentScore()->masterScore();
      QMap<QString, QString>  metatags = score->metaTags();
      if (metatags.value("source") != url) {
            metatags.insert("source", url);
            score->startCmd();
            score->undo(new ChangeMetaTags(score, metatags));
            score->endCmd();
      }
      if (uploadAudio->isChecked())
            _loginManager->getMediaUrl(nid, vid, "mp3");
      else
            displaySuccess();
      }

//---------------------------------------------------------
//   uploadSuccess
//---------------------------------------------------------

void UploadScoreDialog::displaySuccess()
      {
      QMessageBox::information(this,
               tr("Success"),
               tr("Finished! %1Go to my score%2.")
                               .arg("<a href=\"" + _url + "\">")
                               .arg("</a>"),
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
//   showOrHideUploadAudio
//---------------------------------------------------------

void UploadScoreDialog::showOrHideUploadAudio()
      {
      uploadAudio->setEnabled(mscore->canSaveMp3());
      bool v = !mscore->synthesizerState().isDefaultSynthSoundfont();
      uploadAudio->setVisible(v);
      uploadAudioHelp->setVisible(v);
      }

//---------------------------------------------------------
//   display
//---------------------------------------------------------

void UploadScoreDialog::display()
      {
      lblUsername->setText(_loginManager->userName());
      QString source = mscore->currentScore()->masterScore()->metaTag("source");
      if (!source.isEmpty()) {
            QStringList sl = source.split("/");
            if (sl.length() > 0) {
                  QString nidString = sl.last();
                  bool ok;
                  int nid = nidString.toInt(&ok);
                  if (ok) {
                        _nid = nid;
                        _loginManager->getScoreInfo(nid);
                        return;
                        }
                  }
            }
      showOrHideUploadAudio();
      clear();
      setVisible(true);
      }

//---------------------------------------------------------
//   onGetScoreSuccess
//---------------------------------------------------------

void UploadScoreDialog::onGetScoreSuccess(const QString &t, const QString &desc, bool priv, const QString& lic, const QString& tag, const QString& url)
      {
      // file with score info
      title->setText(t);
      description->setPlainText(desc);
      cbPrivate->setChecked(priv);
      // publicdomain used to be an option. Not anymore. Remap to CC0
      QString lice = lic;
      if (lice == "publicdomain")
            lice = "cc-zero";
      int lIndex = license->findData(lice);
      if (lIndex < 0) lIndex = 0;
      license->setCurrentIndex(lIndex);
      tags->setText(tag);
      changes->clear();
      updateExistingCb->setChecked(true);
      updateExistingCb->setVisible(true);
      linkToScore->setText(tr("[%1Link%2]")
                           .arg("<a href=\"" + url + "\">")
                           .arg("</a>"));
      showOrHideUploadAudio();
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
//   clear
//---------------------------------------------------------

void UploadScoreDialog::clear()
      {
      description->clear();
      cbPrivate->setChecked(false);
      license->setCurrentIndex(0);
      tags->clear();
      changes->clear();
      updateExistingCb->setChecked(false);
      updateExistingCb->setVisible(false);
      linkToScore->setText("");
      uploadAudio->setChecked(false);
      _nid = -1;
      _url = "";
      }

//---------------------------------------------------------
//   logout
//---------------------------------------------------------

void UploadScoreDialog::logout()
      {
      _loginManager->logout();
      setVisible(false);
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void UploadScoreDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }
}

