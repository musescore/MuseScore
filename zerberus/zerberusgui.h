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

//---------------------------------------------------------
//   SfzListDialog
//---------------------------------------------------------

class SfzListDialog : public QDialog {
      Q_OBJECT
      int _idx = -1;
      QListWidget* list;

   private slots:
      void itemSelected(QListWidgetItem*);

   public:
      SfzListDialog();
      QString name();
      QString path();
      void add(const QString& name, const QString& path);
      };

//---------------------------------------------------------
//   ZerberusGui
//---------------------------------------------------------

class ZerberusGui : public SynthesizerGui, Ui::ZerberusGui {
      Q_OBJECT

   private slots:
      void addClicked();
      void removeClicked();

   public:
      ZerberusGui(Synthesizer*);
      Zerberus* zerberus() { return (Zerberus*)synthesizer(); }
      };

#endif

