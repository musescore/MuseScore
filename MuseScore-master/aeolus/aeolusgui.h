//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __AEOLUSGUI_H__
#define __AEOLUSGUI_H__

#include "synthesizer/synthesizergui.h"
#include "ui_aeolus_gui.h"
#include "aeolus.h"

//---------------------------------------------------------
//   AeolusGui
//---------------------------------------------------------

class AeolusGui : public SynthesizerGui, Ui::AeolusGui {
      Q_OBJECT

      Aeolus* aeolus() { return static_cast<Aeolus*>(synthesizer()); }

   private slots:
      void valueHasChanged(double, int);

   public slots:
      virtual void synthesizerChanged();

   public:
      AeolusGui(Synthesizer*);
      };

#endif

