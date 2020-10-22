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

#ifndef __FLUIDGUI_H__
#define __FLUIDGUI_H__

#include "audio/midi/synthesizergui.h"
#include "ui_fluid_gui.h"
#include "fluid.h"

//---------------------------------------------------------
//   ListDialog
//---------------------------------------------------------

struct SfNamePath {
      QString name;
      QString path;
      };

class SfListDialog : public QDialog {
      Q_OBJECT
      int _idx = -1;
      std::vector<struct SfNamePath> _namePaths;
      QListWidget* list;
      QDialogButtonBox* buttonBox;
      QPushButton* okButton;
      QPushButton* cancelButton;

   private slots:
      void okClicked();
      void cancelClicked();

   public:
      SfListDialog(QWidget* parent = 0);
      QString name();
      QString path();
      std::vector<struct SfNamePath> getNamePaths() { return _namePaths; }
      void add(const QString& name, const QString& path);
      };

//---------------------------------------------------------
//   FluidGui
//---------------------------------------------------------

class FluidGui : public Ms::SynthesizerGui, Ui::FluidGui {
      Q_OBJECT

      QFutureWatcher<bool> _futureWatcher;
      QString _loadedSfPath;
      QString _loadedSfName;
      QProgressDialog* _progressDialog;
      QTimer * _progressTimer;
      std::list<struct SfNamePath> _sfToLoad;
      void loadSf();
      void moveSoundfontInTheList(int currentIdx, int targetIdx);

   private slots:
      void soundFontTopClicked();
      void soundFontBottomClicked();
      void soundFontUpClicked();
      void soundFontDownClicked();
      void soundFontAddClicked();
      void soundFontDeleteClicked();
      void onSoundFontLoaded();
      void updateProgress();
      void cancelLoadClicked();
      void updateUpDownButtons();

   public slots:
      virtual void synthesizerChanged();

   public:
      FluidGui(Ms::Synthesizer*);
      FluidS::Fluid* fluid() { return static_cast<FluidS::Fluid*>(synthesizer()); }
      };

#endif

