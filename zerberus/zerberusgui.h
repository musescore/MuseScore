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

#ifndef __ZERBERUSGUI_H__
#define __ZERBERUSGUI_H__

#include "synthesizer/synthesizergui.h"
#include "ui_zerberus_gui.h"
#include "zerberus.h"
#include <QDialogButtonBox>

class QProgressDialog;

struct SfzNamePath {
      QString name;
      QString path;
      };

//---------------------------------------------------------
//   SfzListDialog
//---------------------------------------------------------

class SfzListDialog : public QDialog {
      Q_OBJECT
      int _idx = -1;
      std::vector<struct SfzNamePath> _namePaths;
      QListWidget* list;
      QDialogButtonBox* buttonBox;
      QPushButton* okButton;
      QPushButton* cancelButton;

   private slots:
      void okClicked();
      void cancelClicked();

   public:
      SfzListDialog(QWidget* parent = 0);
      std::vector<struct SfzNamePath> getNamePaths() { return _namePaths; }
      void add(const QString& name, const QString& path);
      };

//---------------------------------------------------------
//   ZerberusGui
//---------------------------------------------------------

class ZerberusGui : public Ms::SynthesizerGui, Ui::ZerberusGui {
      Q_OBJECT

      QFutureWatcher<bool> _futureWatcher;
      QString _loadedSfPath;
      QString _loadedSfName;
      QProgressDialog* _progressDialog;
      QTimer * _progressTimer;
      std::vector<struct SfzNamePath> _sfzToLoad;
      void loadSfz();

   private slots:
      void addClicked();
      void cancelLoadClicked();
      void removeClicked();
      void onSoundFontLoaded();
      void updateProgress();
      void updateButtons();

   public slots:
      virtual void synthesizerChanged();

   public:
      ZerberusGui(Ms::Synthesizer*);
      Zerberus* zerberus() { return (Zerberus*)synthesizer(); }
      };

#endif

