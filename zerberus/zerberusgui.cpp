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

//---------------------------------------------------------
//   SfzListDialog
//---------------------------------------------------------

SfzListDialog::SfzListDialog()
   : QDialog(0)
      {
      setWindowTitle(tr("SFZ Files"));
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      list = new QListWidget;
      QVBoxLayout* layout = new QVBoxLayout;
      layout->addWidget(list);
      setLayout(layout);
      connect(list, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(itemSelected(QListWidgetItem*)));
      }

void SfzListDialog::add(const QString& name, const QString& path)
      {
      QListWidgetItem* item = new QListWidgetItem;
      item->setText(name);
      item->setData(Qt::UserRole, path);
      list->addItem(item);
      }

//---------------------------------------------------------
//   itemSelected
//---------------------------------------------------------

void SfzListDialog::itemSelected(QListWidgetItem* item)
      {
      _idx = list->row(item);
      accept();
      }

QString SfzListDialog::name()
      {
      if (_idx == -1)
            return QString();
      return list->item(_idx)->text();
      }

QString SfzListDialog::path()
      {
      if (_idx == -1)
            return QString();
      return list->item(_idx)->data(Qt::UserRole).toString();
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
      connect(add, SIGNAL(clicked()), SLOT(addClicked()));
      connect(remove, SIGNAL(clicked()), SLOT(removeClicked()));
      connect(&_futureWatcher, SIGNAL(finished()), this, SLOT(onSoundFontLoaded()));
      _progressDialog = new QProgressDialog("Loading...", "", 0, 100, 0, Qt::FramelessWindowHint);
      _progressDialog->setCancelButton(0);
      _progressTimer = new QTimer(this);
      connect(_progressTimer, SIGNAL(timeout()), this, SLOT(updateProgress()));
      connect(files, SIGNAL(itemSelectionChanged()), this, SLOT(updateButtons()));
      updateButtons();
      }

//---------------------------------------------------------
//   collectFiles
//---------------------------------------------------------

static void collectFiles(QFileInfoList* l, const QString& path)
      {
      // printf("collect files <%s>\n", qPrintable(path));

      QDir dir(path);
      foreach (const QFileInfo& s, dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
            if (path == s.absoluteFilePath())
                  return;

            if (s.isDir())
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

      QString path = Ms::preferences.sfzPath;
      QStringList pl = path.split(";");
      foreach (const QString& s, pl) {
            QString ss(s);
            if (!s.isEmpty() && s[0] == '~')
                  ss = QDir::homePath() + s.mid(1);
            collectFiles(&l, ss);
            }
      return l;
      }

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void ZerberusGui::addClicked()
      {
      QFileInfoList l = Zerberus::sfzFiles();

      SfzListDialog ld;
      foreach (const QFileInfo& fi, l)
            ld.add(fi.fileName(), fi.absoluteFilePath());
      if (!ld.exec())
            return;

      QString sfName = ld.name();
      QString sfPath = ld.path();

      int n = files->count();
      QStringList sl;
      for (int i = 0; i < n; ++i) {
            QListWidgetItem* item = files->item(i);
            sl.append(item->text());
            }

      if (sl.contains(sfPath)) {
            QMessageBox::warning(this,
            tr("MuseScore"),
            tr("Soundfont %1 already loaded").arg(sfPath));
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

void ZerberusGui::updateProgress()
      {
      _progressDialog->setValue(zerberus()->loadProgress());
      }

void ZerberusGui::updateButtons()
      {
      int row = files->currentRow();
      remove->setEnabled(row != -1);
      }

void ZerberusGui::onSoundFontLoaded()
      {
      bool loaded = _futureWatcher.result();
      _progressTimer->stop();
      _progressDialog->reset();
      if (!loaded) {
            QMessageBox::warning(this,
            tr("MuseScore"),
            tr("cannot load soundfont %1").arg(_loadedSfPath));
            }
      else {
            files->insertItem(0, _loadedSfName);
            }
      emit valueChanged();
      }

//---------------------------------------------------------
//   removeClicked
//---------------------------------------------------------

void ZerberusGui::removeClicked()
      {
      int row = files->currentRow();
      if (row >= 0) {
            QString s(files->item(row)->text());
            zerberus()->removeSoundFont(s);
            delete files->takeItem(row);
            emit valueChanged();
            updateButtons();
            }
      }

//---------------------------------------------------------
//   synthesizerChanged
//---------------------------------------------------------

void ZerberusGui::synthesizerChanged()
      {
      QStringList sfonts = zerberus()->soundFonts();
      files->clear();
      files->addItems(sfonts);
      }


