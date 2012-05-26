//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 20011 Werner Schweer and others
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

#include "mediadialog.h"
#include "icons.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/audio.h"
#include "scoreview.h"
#include "omr/omr.h"

//---------------------------------------------------------
//   MediaDialog
//---------------------------------------------------------

MediaDialog::MediaDialog(QWidget* parent)
   : QDialog()
      {
      setupUi(this);
      setWindowTitle(tr("MuseScore: Additional Media"));
      scanFileButton->setIcon(*icons[fileOpen_ICON]);
      audioFileButton->setIcon(*icons[fileOpen_ICON]);

      connect(addScan,         SIGNAL(clicked()), SLOT(addScanPressed()));
      connect(removeScan,      SIGNAL(clicked()), SLOT(removeScanPressed()));
      connect(addAudio,        SIGNAL(clicked()), SLOT(addAudioPressed()));
      connect(removeAudio,     SIGNAL(clicked()), SLOT(removeAudioPressed()));
      connect(scanFileButton,  SIGNAL(clicked()), SLOT(scanFileButtonPressed()));
      connect(audioFileButton, SIGNAL(clicked()), SLOT(audioFileButtonPressed()));
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void MediaDialog::setScore(Score* s)
      {
      score = s;
      Omr* omr = score->omr();
      if (omr) {
            scanFile->setText(omr->path());
            addScan->setEnabled(false);
            removeScan->setEnabled(true);
            scanFileButton->setEnabled(false);
            }
      else {
            scanFile->setText(QString());
            addScan->setEnabled(true);
            removeScan->setEnabled(false);
            scanFileButton->setEnabled(true);
            }
      Audio* audio = score->audio();
      if (audio) {
            audioFile->setText(audio->path());
            addAudio->setEnabled(false);
            removeAudio->setEnabled(true);
            audioFileButton->setEnabled(false);
            }
      else {
            audioFile->setText(QString());
            addAudio->setEnabled(true);
            removeAudio->setEnabled(false);
            audioFileButton->setEnabled(true);
            }
      }

//---------------------------------------------------------
//   addScanPressed
//---------------------------------------------------------

void MediaDialog::addScanPressed()
      {
      QString path = scanFile->text();
      if (score->omr() || path.isEmpty())
            return;
      Omr* omr = new Omr(path, score);
      if (!omr->readPdf()) {
            qDebug("read omr failed\n");
            delete omr;
            return;
            }
      score->setOmr(omr);
      score->setDirty(true);
      mscore->currentScoreView()->showOmr(true);
      }

//---------------------------------------------------------
//   removeScanPressed
//---------------------------------------------------------

void MediaDialog::removeScanPressed()
      {
      mscore->currentScoreView()->showOmr(false);
      score->removeOmr();
      score->setDirty(true);
      scanFile->setText(QString());
      addScan->setEnabled(true);
      removeScan->setEnabled(false);
      scanFileButton->setEnabled(true);
      }

//---------------------------------------------------------
//   addAudioPressed
//---------------------------------------------------------

void MediaDialog::addAudioPressed()
      {
      QString path = audioFile->text();
      if (score->audio() || path.isEmpty())
            return;
      QFile f(path);
      if (!f.open(QIODevice::ReadOnly))
            return;
      QByteArray ba = f.readAll();
      f.close();
      Audio* audio = new Audio;
      audio->setPath(path);
      audio->setData(ba);
      score->setAudio(audio);
      score->setDirty(true);
      }

//---------------------------------------------------------
//   removeAudioPressed
//---------------------------------------------------------

void MediaDialog::removeAudioPressed()
      {
      score->removeAudio();
      score->setDirty(true);
      audioFile->setText(QString());
      addAudio->setEnabled(true);
      removeAudio->setEnabled(false);
      audioFileButton->setEnabled(true);
      }

//---------------------------------------------------------
//   scanFileButtonPressed
//---------------------------------------------------------

void MediaDialog::scanFileButtonPressed()
      {
      QString s = mscore->getScanFile(QString());
      if (!s.isNull())
            scanFile->setText(s);
      }

//---------------------------------------------------------
//   audioFileButtonPressed
//---------------------------------------------------------

void MediaDialog::audioFileButtonPressed()
      {
      QString s = mscore->getAudioFile(QString());
      if (!s.isNull())
            audioFile->setText(s);
      }

