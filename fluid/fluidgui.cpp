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
      QVBoxLayout* layout = new QVBoxLayout;
      layout->addWidget(list);
      setLayout(layout);
      connect(list, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(itemSelected(QListWidgetItem*)));
      }

void SfListDialog::add(const QString& name, const QString& path)
      {
      QListWidgetItem* item = new QListWidgetItem;
      item->setText(name);
      item->setData(Qt::UserRole, path);
      list->addItem(item);
      }

//---------------------------------------------------------
//   itemSelected
//---------------------------------------------------------

void SfListDialog::itemSelected(QListWidgetItem* item)
      {
      _idx = list->row(item);
      accept();
      }

QString SfListDialog::name()
      {
      if (_idx == -1)
            return QString();
      return list->item(_idx)->text();
      }

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
//   soundFontUpClicked
//---------------------------------------------------------

void FluidGui::soundFontUpClicked()
      {
      int row = soundFonts->currentRow();
      if (row <= 0)
            return;
      QStringList sfonts = fluid()->soundFonts();
      sfonts.swap(row, row-1);
      fluid()->loadSoundFonts(sfonts);
      sfonts = fluid()->soundFonts();
      soundFonts->clear();
      soundFonts->addItems(sfonts);
      soundFonts->setCurrentRow(row-1);
      emit sfChanged();
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

      QStringList sfonts = fluid()->soundFonts();
      sfonts.swap(row, row+1);
      fluid()->loadSoundFonts(sfonts);
      sfonts = fluid()->soundFonts();
      soundFonts->clear();
      soundFonts->addItems(sfonts);
      soundFonts->setCurrentRow(row+1);
      emit sfChanged();
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

      QString sfName = ld.name();
      QString sfPath = ld.path();

      int n = soundFonts->count();
      QStringList sl;
      for (int i = 0; i < n; ++i) {
            QListWidgetItem* item = soundFonts->item(i);
            sl.append(item->text());
            }

      if (sl.contains(sfPath)) {
            QMessageBox::warning(this,
            tr("MuseScore"),
            tr("SoundFont %1 already loaded").arg(sfPath));
            }
      else {
            bool loaded = fluid()->addSoundFont(sfPath);
            if (!loaded) {
                  QMessageBox::warning(this,
                  tr("MuseScore"),
                  tr("Cannot load SoundFont %1").arg(sfPath));
                  }
            else {
                  soundFonts->insertItem(0, sfName);
                  emit sfChanged();
                  emit valueChanged();
                  }
            }
      updateUpDownButtons();
      }

