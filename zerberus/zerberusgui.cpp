//=============================================================================
//  Zerberus
//  Zample player
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "zerberusgui.h"

#include "mscore/preferences.h"
#include "mscore/extension.h"

//---------------------------------------------------------
//   SfzListDialog
//---------------------------------------------------------

SfzListDialog::SfzListDialog(QWidget* parent)
   : QDialog(parent)
      {
      setWindowTitle(tr("SFZ Files"));
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      list = new QListWidget;
      list->setSelectionMode(QAbstractItemView::ExtendedSelection);

      okButton = new QPushButton;
      cancelButton = new QPushButton;
      okButton->setText(tr("Load"));
      cancelButton->setText(tr("Cancel"));

      QVBoxLayout* layout = new QVBoxLayout;
      buttonBox = new QDialogButtonBox;
      layout->addWidget(list);
      layout->addWidget(buttonBox);
      buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
      buttonBox->addButton(cancelButton, QDialogButtonBox::RejectRole);
      setLayout(layout);
      connect(okButton, SIGNAL(clicked()), SLOT(okClicked()));
      connect(cancelButton, SIGNAL(clicked()), SLOT(cancelClicked()));
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void SfzListDialog::add(const QString& name, const QString& path)
      {
      QListWidgetItem* item = new QListWidgetItem;
      item->setText(name);
      item->setData(Qt::UserRole, path);
      list->addItem(item);
      }

//---------------------------------------------------------
//   okClicked
//---------------------------------------------------------

void SfzListDialog::okClicked()
      {
      for (auto item : list->selectedItems()) {
            _namePaths.push_back({item->text(), item->data(Qt::UserRole).toString()});
            }
      accept();
      }

//---------------------------------------------------------
//   cancelClicked
//---------------------------------------------------------

void SfzListDialog::cancelClicked()
      {
      reject();
      }

//---------------------------------------------------------
//   gui
//---------------------------------------------------------

Ms::SynthesizerGui* Zerberus::gui()
      {
      if (_gui == 0)
            _gui = new ZerberusGui(this);
      return _gui;
      }

//---------------------------------------------------------
//   Zerberusgui
//---------------------------------------------------------

ZerberusGui::ZerberusGui(Ms::Synthesizer* s)
   : SynthesizerGui(s)
      {
      setupUi(this);
      connect(soundFontUp,     SIGNAL(clicked()), SLOT(soundFontUpClicked()));
      connect(soundFontDown,   SIGNAL(clicked()), SLOT(soundFontDownClicked()));
      connect(soundFontAdd, SIGNAL(clicked()), SLOT(soundFontAddClicked()));
      connect(soundFontDelete, SIGNAL(clicked()), SLOT(soundFontDeleteClicked()));
      connect(&_futureWatcher, SIGNAL(finished()), this, SLOT(onSoundFontLoaded()));
      _progressDialog = new QProgressDialog(tr("Loading..."), tr("Cancel"), 0, 100, 0, Qt::FramelessWindowHint);
      _progressDialog->reset(); // required for Qt 5.5, see QTBUG-47042
      connect(_progressDialog, SIGNAL(canceled()), this, SLOT(cancelLoadClicked()));
      _progressTimer = new QTimer(this);
      connect(_progressTimer, SIGNAL(timeout()), this, SLOT(updateProgress()));
      connect(files, SIGNAL(itemSelectionChanged()), this, SLOT(updateButtons()));
      updateButtons();
      }

//---------------------------------------------------------
//   moveSoundfontInTheList
//---------------------------------------------------------

void ZerberusGui::moveSoundfontInTheList(int currentIdx, int targetIdx)
      {
      QStringList sfonts = zerberus()->soundFonts();
      sfonts.swap(currentIdx, targetIdx);
      zerberus()->removeSoundFonts(zerberus()->soundFonts());
      
      loadSoundFontsAsync(sfonts);
      files->setCurrentRow(targetIdx);
      emit sfChanged();
      }

//---------------------------------------------------------
//   soundFontUpClicked
//---------------------------------------------------------

void ZerberusGui::soundFontUpClicked()
      {
      int row = files->currentRow();
      if (row <= 0)
            return;

      moveSoundfontInTheList(row, row - 1);
      }

//---------------------------------------------------------
//   soundFontDownClicked
//---------------------------------------------------------

void ZerberusGui::soundFontDownClicked()
      {
      int rows = files->count();
      int row = files->currentRow();
      if (row + 1 >= rows)
            return;

      moveSoundfontInTheList(row, row + 1);
      }

//---------------------------------------------------------
//   loadSounfFontsAsync
//---------------------------------------------------------

void ZerberusGui::loadSoundFontsAsync(QStringList sfonts)
      {
      QFuture<bool> future = QtConcurrent::run(zerberus(), &Zerberus::loadSoundFonts, sfonts);
      _futureWatcher.setFuture(future);
      _progressTimer->start(1000);
      _progressDialog->exec();

      synthesizerChanged();
      }

//---------------------------------------------------------
//   collectFiles
//---------------------------------------------------------

static void collectFiles(QFileInfoList* l, const QString& path)
      {
//printf("collect files <%s>\n", qPrintable(path));

      QDir dir(path);
      foreach (const QFileInfo& s, dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
            if (path == s.absoluteFilePath())
                  return;

            if (s.isDir() && !s.isHidden())
                  collectFiles(l, s.absoluteFilePath());
            else {
                  if (s.suffix().toLower() == "sfz")
                        l->append(s);
                  }
            }
      }

//---------------------------------------------------------
//   sfzFiles
//---------------------------------------------------------

QFileInfoList Zerberus::sfzFiles()
      {
      QFileInfoList l;

      QStringList pl = Ms::preferences.getString(PREF_APP_PATHS_MYSOUNDFONTS).split(";");
      pl.prepend(QFileInfo(QString("%1%2").arg(Ms::mscoreGlobalShare).arg("sound")).absoluteFilePath());

      // append extensions directory
      QStringList extensionsDir = Ms::Extension::getDirectoriesByType(Ms::Extension::sfzsDir);
      pl.append(extensionsDir);

      foreach (const QString& s, pl) {
            QString ss(s);
            if (!s.isEmpty() && s[0] == '~')
                  ss = QDir::homePath() + s.mid(1);
            collectFiles(&l, ss);
            }
      return l;
      }

//---------------------------------------------------------
//   loadSfz
//---------------------------------------------------------

void ZerberusGui::loadSfz() {

      if (_sfzToLoad.empty())
            return;

     struct SfNamePath item = _sfzToLoad.front();
     QString sfName = item.name;
     QString sfPath = item.path;
     _sfzToLoad.pop_front();

     QStringList sl;
     for (int i = 0; i < files->count(); ++i) {
           QListWidgetItem* item1 = files->item(i);
           sl.append(item1->text());
           }

      if (sl.contains(sfName)) {
            QMessageBox::warning(this,
            tr("MuseScore"),
            tr("SoundFont %1 already loaded").arg(sfPath));
            }
      else {
            _loadedSfName = sfName;
            _loadedSfPath = sfPath;
            QFuture<bool> future = QtConcurrent::run(zerberus(), &Zerberus::addSoundFont, sfName);
            _futureWatcher.setFuture(future);
            _progressTimer->start(1000);
            _progressDialog->exec();
            }
      }

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void ZerberusGui::soundFontAddClicked()
      {
      zerberus()->setLoadWasCanceled(false);

      QFileInfoList l = Zerberus::sfzFiles();

      SfzListDialog ld(this);
      foreach (const QFileInfo& fi, l)
            ld.add(fi.fileName(), fi.absoluteFilePath());
      if (!ld.exec())
            return;

      for (auto item : ld.getNamePaths()) {
            _sfzToLoad.push_back(item);
            }
      loadSfz();
      }

//---------------------------------------------------------
//   cancelLoad
//---------------------------------------------------------

void ZerberusGui::cancelLoadClicked()
      {
      zerberus()->setLoadWasCanceled(true);
      }

//---------------------------------------------------------
//   updateProgress
//---------------------------------------------------------

void ZerberusGui::updateProgress()
      {
      _progressDialog->setValue(zerberus()->loadProgress());
      }

//---------------------------------------------------------
//   updateButtons
//---------------------------------------------------------

void ZerberusGui::updateButtons()
      {
      int row = files->currentRow();
      soundFontDelete->setEnabled(row != -1);
      soundFontUp->setEnabled(row != -1);
      soundFontDown->setEnabled(row != -1);
      }

//---------------------------------------------------------
//   onSoundFontLoaded
//---------------------------------------------------------

void ZerberusGui::onSoundFontLoaded()
      {
      bool loaded = _futureWatcher.result();
      bool wasNotCanceled = !_progressDialog->wasCanceled();
      _progressTimer->stop();
      _progressDialog->reset();
      if (loaded) {
            QListWidgetItem* item = new QListWidgetItem;
            item->setText(_loadedSfName);
            item->setData(Qt::UserRole, _loadedSfPath);
            //files->insertItem(0, item);
            files->addItem(item);
            emit valueChanged();
            emit sfChanged();
            }
      else if (wasNotCanceled) {
            QMessageBox::warning(this,
            tr("MuseScore"),
            tr("Cannot load SoundFont %1").arg(_loadedSfPath));
            }
      loadSfz();
      }

//---------------------------------------------------------
//   removeClicked
//---------------------------------------------------------

void ZerberusGui::soundFontDeleteClicked()
      {
      int row = files->currentRow();
      if (row >= 0) {
            QString path(files->item(row)->data(Qt::UserRole).toString());
            if (!zerberus()->removeSoundFont(path))
                  qDebug("ZerberusGui::removeClicked: cannot remove sf %s", qPrintable(files->item(row)->text()));
            delete files->takeItem(row);
            emit valueChanged();
            emit sfChanged();
            updateButtons();
            }
      }

//---------------------------------------------------------
//   synthesizerChanged
//---------------------------------------------------------

void ZerberusGui::synthesizerChanged()
      {
      files->clear();
      QStringList sfonts = zerberus()->soundFonts();
      for (QString path : sfonts) {
            QListWidgetItem* item = new QListWidgetItem;
            item->setText(QFileInfo(path).fileName());
            item->setData(Qt::UserRole, path);
            files->addItem(item);
            }
      }
