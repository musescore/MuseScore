//=============================================================================
//  MuseSynth
//  Music Software Synthesizer
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __ZITAGUI_H__
#define __ZITAGUI_H__

#include "effects/effectgui.h"

namespace Ms {

class ZitaReverb;

//---------------------------------------------------------
//   Rotary
//---------------------------------------------------------

struct Rotary {
      const char* id;
      int x, y;
      qreal value;
      };

//---------------------------------------------------------
//   gui
//---------------------------------------------------------

class ZitaEffectGui : public EffectGui {
      Q_OBJECT

      std::vector<QPixmap> pm;
      std::vector<Rotary> rotary;

      int mx, my;
      int r;
      qreal oval;

      void paintEvent(QPaintEvent*);
      void mousePressEvent(QMouseEvent*);
      void mouseReleaseEvent(QMouseEvent*);
      void mouseMoveEvent(QMouseEvent*);

      virtual void updateValues();

   public:
      ZitaEffectGui(ZitaReverb*, QWidget* parent = 0);
      };
}

#endif


