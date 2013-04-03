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

#include "synthesizer/synthesizergui.h"
#include "ui_fluid_gui.h"
#include "fluid.h"

//---------------------------------------------------------
//   ListDialog
//---------------------------------------------------------

class ListDialog : public QDialog {
      Q_OBJECT
      int _idx = -1;
      QListWidget* list;

   private slots:
      void itemSelected(QListWidgetItem*);

   public:
      ListDialog();
      QString name();
      QString path();
      void add(const QString& name, const QString& path);
      };

//---------------------------------------------------------
//   FluidGui
//---------------------------------------------------------

class FluidGui : public SynthesizerGui, Ui::FluidGui {
      Q_OBJECT

      FluidS::Fluid* fluid() { return static_cast<FluidS::Fluid*>(synthesizer()); }
      void updateUpDownButtons();

   private slots:
      void soundFontUpClicked();
      void soundFontDownClicked();
      void soundFontAddClicked();
      void soundFontDeleteClicked();

   public slots:
      virtual void synthesizerChanged();

   public:
      FluidGui(Synthesizer*);
      };

#endif

