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

#include "fluidgui.h"
#include "mscore/preferences.h"
#include "mscore/icons.h"

using namespace Ms;

SfListDialog::SfListDialog(QWidget* parent)
   : QDialog(parent)
      {
      setWindowTitle(tr("SoundFont Files"));
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
      connect(list, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(itemSelected(QListWidgetItem*)));
      connect(okButton, SIGNAL(clicked()), SLOT(okClicked()));
      connect(cancelButton, SIGNAL(clicked()), SLOT(cancelClicked()));
      }

void SfListDialog::add(const QString& name, const QString& path)
      {
      QListWidgetItem* item = new QListWidgetItem;
      item->setText(name);
      item->setData(Qt::UserRole, path);
      list->addItem(item);
      }

//---------------------------------------------------------
//   okClicked
//---------------------------------------------------------

void SfListDialog::okClicked()
      {
      for (auto item : list->selectedItems()) {
            _namePaths.push_back({item->text(), item->data(Qt::UserRole).toString()});
            }
      accept();
      }

//---------------------------------------------------------
//   cancelClicked
//---------------------------------------------------------

void SfListDialog::cancelClicked()
      {
      reject();
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString SfListDialog::name()
      {
      if (_idx == -1)
            return QString();
      return list->item(_idx)->text();
      }

//---------------------------------------------------------
//   path
//---------------------------------------------------------

QString SfListDialog::path()
      {
      if (_idx == -1)
            return QString();
      return list->item(_idx)->data(Qt::UserRole).toString();
      }

//---------------------------------------------------------
//   gui
//---------------------------------------------------------

SynthesizerGui* FluidS::Fluid::gui()
      {
      if (_gui == 0)
            _gui = new FluidGui(this);
      return _gui;
      }

//---------------------------------------------------------
//   FluidGui
//---------------------------------------------------------

FluidGui::FluidGui(Synthesizer* s)
   : SynthesizerGui(s)
      {
      setupUi(this);
      connect(soundFontUp,     SIGNAL(clicked()), SLOT(soundFontUpClicked()));
      connect(soundFontDown,   SIGNAL(clicked()), SLOT(soundFontDownClicked()));
      connect(soundFontAdd,    SIGNAL(clicked()), SLOT(soundFontAddClicked()));
      connect(soundFontDelete, SIGNAL(clicked()), SLOT(soundFontDeleteClicked()));
      connect(soundFonts,      SIGNAL(itemSelectionChanged ()),  SLOT(updateUpDownButtons()));
      connect(&_futureWatcher, SIGNAL(finished()), this, SLOT(onSoundFontLoaded()));
      _progressDialog = new QProgressDialog(tr("Loading…"), tr("Cancel"), 0, 100, 0, Qt::FramelessWindowHint);
      _progressDialog->reset(); // required for Qt 5.5, see QTBUG-47042
      connect(_progressDialog, SIGNAL(canceled()), this, SLOT(cancelLoadClicked()));
      _progressTimer = new QTimer(this);
      connect(_progressTimer, SIGNAL(timeout()), this, SLOT(updateProgress()));
      connect(soundFonts, SIGNAL(itemSelectionChanged()), this, SLOT(updateUpDownButtons()));
      updateUpDownButtons();
      }

//---------------------------------------------------------
//   synthesizerChanged
//---------------------------------------------------------

void FluidGui::synthesizerChanged()
      {
      QStringList sfonts = fluid()->soundFonts();
      soundFonts->clear();
      soundFonts->addItems(sfonts);
      updateUpDownButtons();
      emit sfChanged();
      }

//---------------------------------------------------------
//   moveSoundfontInTheList
//---------------------------------------------------------

void FluidGui::moveSoundfontInTheList(int currentIdx, int targetIdx)
      {
      QStringList sfonts = fluid()->soundFonts();
      for (auto sfName : sfonts)
            fluid()->removeSoundFont(sfName);
      
      sfonts.swap(currentIdx, targetIdx);
      fluid()->loadSoundFonts(sfonts);
      sfonts = fluid()->soundFonts();
      soundFonts->clear();
      soundFonts->addItems(sfonts);
      soundFonts->setCurrentRow(targetIdx);
      emit sfChanged();
      }

//---------------------------------------------------------
//   soundFontUpClicked
//---------------------------------------------------------

void FluidGui::soundFontUpClicked()
      {
      int row = soundFonts->currentRow();
      if (row <= 0)
            return;
      
      moveSoundfontInTheList(row, row - 1);
      }

//---------------------------------------------------------
//   soundFontDownClicked
//---------------------------------------------------------

void FluidGui::soundFontDownClicked()
      {
      int rows = soundFonts->count();
      int row = soundFonts->currentRow();
      if (row + 1 >= rows)
            return;

      moveSoundfontInTheList(row, row + 1);
      }

//---------------------------------------------------------
//   soundFontDeleteClicked
//---------------------------------------------------------

void FluidGui::soundFontDeleteClicked()
      {
      int row = soundFonts->currentRow();
      if (row >= 0) {
            QString s(soundFonts->item(row)->text());
            fluid()->removeSoundFont(s);
            delete soundFonts->takeItem(row);
            emit sfChanged();
            emit valueChanged();
            }
      updateUpDownButtons();
      }

//---------------------------------------------------------
//   updateUpDownButtons
//---------------------------------------------------------

void FluidGui::updateUpDownButtons()
      {
      int rows = soundFonts->count();
      int row = soundFonts->currentRow();
      soundFontUp->setEnabled(row > 0);
      soundFontDown->setEnabled((row != -1) && (row < (rows-1)));
      soundFontDelete->setEnabled(row != -1);
      }

//---------------------------------------------------------
//   soundFontAddClicked
//---------------------------------------------------------

void FluidGui::soundFontAddClicked()
      {
      QFileInfoList l = FluidS::Fluid::sfFiles();

      SfListDialog ld(this);
      foreach (const QFileInfo& fi, l)
            ld.add(fi.fileName(), fi.absoluteFilePath());
      if (!ld.exec())
            return;

      for (auto item : ld.getNamePaths()) {
            _sfToLoad.push_back(item);
            }
      loadSf();

      updateUpDownButtons();
      }

//---------------------------------------------------------
//   cancelLoad
//---------------------------------------------------------

void FluidGui::cancelLoadClicked()
      {
      fluid()->setLoadWasCanceled(true);
      }

//---------------------------------------------------------
//   updateProgress
//---------------------------------------------------------

void FluidGui::updateProgress()
      {
      _progressDialog->setValue(fluid()->loadProgress());
      }


//---------------------------------------------------------
//   loadSf
//---------------------------------------------------------

void FluidGui::loadSf()
      {
      if (_sfToLoad.empty())
            return;

      struct SfNamePath item = _sfToLoad.front();
      QString sfName = item.name;
      QString sfPath = item.path;
      _sfToLoad.pop_front();

      QStringList sl;
      for (int i = 0; i < soundFonts->count(); ++i) {
            QListWidgetItem* widgetItem = soundFonts->item(i);
            sl.append(widgetItem->text());
            }

      if (sl.contains(sfPath)) {
            QMessageBox::warning(this,
                                 tr("MuseScore"),
                                 tr("SoundFont %1 already loaded").arg(sfPath));
            }
      else {

            _loadedSfName = sfName;
            _loadedSfPath = sfPath;
            QFuture<bool> future = QtConcurrent::run(fluid(), &FluidS::Fluid::addSoundFont, sfPath);
            _futureWatcher.setFuture(future);
            _progressTimer->start(1000);
            _progressDialog->exec();
            }
      }

//---------------------------------------------------------
//   onSoundFontLoaded
//---------------------------------------------------------

void FluidGui::onSoundFontLoaded()
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
            soundFonts->insertItem(0, item);
            emit valueChanged();
            emit sfChanged();
            }
      else if (wasNotCanceled) {
            QMessageBox::warning(this,
            tr("MuseScore"),
            tr("Cannot load SoundFont %1").arg(_loadedSfPath));
            }
      loadSf();
      }
