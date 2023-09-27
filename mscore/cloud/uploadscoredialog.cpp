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

#include <QMessageBox>

#include "musescore.h"
#include "downloadUtils.h"
#include "icons.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"
#include "loginmanager.h"
#include "uploadscoredialog.h"

namespace Ms {

//---------------------------------------------------------
//   showUploadScore
//---------------------------------------------------------

void MuseScore::showUploadScoreDialog()
      {
      if (MuseScore::unstable()) {
            QMessageBox::warning(this, tr("Save online"), tr("Saving scores online is disabled in this unstable prerelease version of MuseScore."));
            return;
      }
      if (!currentScore())
            return;
      if (!currentScore()->sanityCheck(QString())) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Upload Error"));
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
      setObjectName("UploadScoreDialog"); // changed object name to reset the saved geometry of the previous dialog which contained a lot more content
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      scoreIconLabel->setPixmap(icons[int(Icons::mscz_ICON)]->pixmap(scoreIconLabel->size()));

      buttonBox->addButton(tr("Continue"), QDialogButtonBox::AcceptRole);
      buttonBox->addButton(QDialogButtonBox::Cancel);

      connect(buttonBox,   SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
      _loginManager = loginManager;
      connect(_loginManager, SIGNAL(uploadSuccess(QString, QString, QString)), this, SLOT(uploadSuccess(QString, QString, QString)));
      connect(_loginManager, SIGNAL(uploadError(QString)), this, SLOT(uploadError(QString)));
      connect(_loginManager, SIGNAL(getScoreSuccess(QString, QString, bool, QString, QString, QString)), this, SLOT(onGetScoreSuccess(QString, QString, bool, QString, QString, QString)));
      connect(_loginManager, SIGNAL(getScoreError(QString)), this, SLOT(onGetScoreError(QString)));
      connect(_loginManager, SIGNAL(tryLoginSuccess()), this, SLOT(display()));
      connect(_loginManager, &LoginManager::mediaUploadSuccess, this, QOverload<>::of(&UploadScoreDialog::updateScoreData));
      connect(btnSignout, SIGNAL(pressed()), this, SLOT(logout()));
      }

//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void UploadScoreDialog::buttonBoxClicked(QAbstractButton* button)
      {
      QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(button);
      if (role == QDialogButtonBox::AcceptRole)
            upload(updateExistingCb->isChecked() ? _nid : -1);
      else
           setVisible(false);
      }

//---------------------------------------------------------
//   upload
//---------------------------------------------------------

void UploadScoreDialog::upload(int nid)
     {
     Score* score = mscore->currentScore()->masterScore();
     const QString scoreTitle = title->text().trimmed().isEmpty() ? score->title() : title->text();
     //revert changes partially made in c8278789267ab6d1c6fcf1cd2b39a2495862255c
     /*QString path = QDir::tempPath() + "/" + mscore->currentScore()->masterScore()->fileInfo()->fileName();
     if (QFile::exists(path))
           path = QDir::tempPath() + QString("/%1-").arg(qrand() % 100000) + mscore->currentScore()->masterScore()->fileInfo()->fileName();
     if (mscore->saveAs(score, true, path, "mscz")) {*/
     QString path = QDir::tempPath() + QString("/temp_%1.mscz").arg(qrand() % 100000);
     if(mscore->saveAs(score, true, path, "mscz")) {
           _nid = nid;
           _loginManager->upload(path, nid, scoreTitle);
           }
     // TODO: Find out where, when and how to delete this temp score.
     //       Won't be here, as the upload happens asynchronously.
     //       Maybe in LoginManager::upload()
     }

//---------------------------------------------------------
//   uploadSuccess
//---------------------------------------------------------

void UploadScoreDialog::uploadSuccess(const QString& url, const QString& nid, const QString& vid)
      {
      setVisible(false);
      _url = url;
      const int oldScoreID = _nid;

      bool ok;
      _nid = nid.toInt(&ok);
      if (!ok)
            _nid = 0;

      _newScore = !(oldScoreID && oldScoreID == _nid);

      MasterScore* score = mscore->currentScore()->masterScore();
      QMap<QString, QString>  metatags = score->metaTags();
      if (metatags.value("source") != url) {
            metatags.insert("source", url);
            score->startCmd();
            score->undo(new ChangeMetaTags(score, metatags));
            score->endCmd();

            //!Note Automatically save score file with received web-link if file already exists
            if (!score->created()) {
                mscore->saveFile();
            }
      }
      if (uploadAudio->isChecked())
            _loginManager->getMediaUrl(nid, vid, "mp3");
      else
            updateScoreData(nid, _newScore);
      }

//---------------------------------------------------------
//   updateScoreData
//---------------------------------------------------------

void UploadScoreDialog::updateScoreData(const QString& nid, bool newScore)
      {
      _loginManager->updateScoreData(nid, newScore);
      }

void UploadScoreDialog::updateScoreData()
      {
      updateScoreData(QString::number(_nid), _newScore);
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
      }

//---------------------------------------------------------
//   display
//---------------------------------------------------------

void UploadScoreDialog::display()
      {
      DownloadUtils* avatarDownload = new DownloadUtils(this);
      const QSize avatarSize = userAvatarLabel->maximumSize();
      const QString avatarUrl = _loginManager->avatar().toString().replace(QRegExp("\\@[0-9]+x[0-9]+"), QString("@%1x%2").arg(avatarSize.width()).arg(avatarSize.height()));
      avatarDownload->setTarget(avatarUrl);
      connect(avatarDownload, &DownloadUtils::done, this, [this, avatarSize, avatarDownload]() {
            QPixmap pm;
            pm.loadFromData(avatarDownload->returnData());
            if (pm.size() != avatarSize)
                  pm = pm.scaled(avatarSize);

            userAvatarLabel->setPixmap(pm);

            avatarDownload->deleteLater();
            });
      avatarDownload->download();

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

void UploadScoreDialog::onGetScoreSuccess(const QString& t, const QString& /*desc*/, bool /*priv*/, const QString& /*lic*/, const QString& /*tag*/, const QString& url)
      {
      // fill with score info
      title->setText(t);
      updateExistingCb->setChecked(true);
      updateExistingCb->setVisible(true);
      linkToScore->setVisible(true);
      linkToScore->setText("[<a href=\"" + url + "\">" + tr("Link") + "</a>]");
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
      updateExistingCb->setChecked(false);
      updateExistingCb->setVisible(false);
      linkToScore->setVisible(false);
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

void UploadScoreDialog::showEvent(QShowEvent* event)
      {
      if (!event->spontaneous())
            adjustSize();

      QWidget::showEvent(event);
      }
}

