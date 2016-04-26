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

#include "zita.h"
#include "zitagui.h"
#include "effects/effectgui.h"

namespace Ms {

//---------------------------------------------------------
//   gui
//---------------------------------------------------------

EffectGui* ZitaReverb::gui()
      {
      if (!_gui) {
            _gui = new ZitaEffectGui(this);
            _gui->setGeometry(0, 0, 640, 79);
            }
      return _gui;
      }

//---------------------------------------------------------
// ZitaEffectGui
//---------------------------------------------------------

ZitaEffectGui::ZitaEffectGui(ZitaReverb* effect, QWidget* parent)
   : EffectGui(effect, parent)
      {
      pm = {
            QPixmap(":/zita1/revsect.png"),
            QPixmap(":/zita1/eq1sect.png"),
            QPixmap(":/zita1/eq2sect.png"),
            QPixmap(":/zita1/mixsect.png"),
            QPixmap(":/zita1/redzita.png")
            };
      int x1 = pm[0].width();
      int x2 = x1 + pm[1].width();
      int x3 = x2 + pm[2].width();
      rotary = {
            Rotary { "delay", 30, 32, 0.0 },
            Rotary { "xover", 92, 17, 0.0 },
            Rotary { "rtlow", 147, 17, 0.0 },
            Rotary { "rtmid", 207, 17, 0.0 },
            Rotary { "fdamp", 267, 17, 0.0 },
            Rotary { "eq1fr", x1 + 19, 32, 0.0 },
            Rotary { "eq1gn", x1 + 68, 17, 0.0 },
            Rotary { "eq2fr", x2 + 19, 32, 0.0 },
            Rotary { "eq2gn", x2 + 68, 17, 0.0 },
            Rotary { "opmix", x3 + 23, 32, 0.0 }
            };
      r = -1;
      }

//---------------------------------------------------------
// updateValues
//---------------------------------------------------------

void ZitaEffectGui::updateValues()
      {
      for (Rotary& r : rotary) {
            r.value = effect()->value(r.id);
            }
      update();
      }

//---------------------------------------------------------
// mousePressEvent
//---------------------------------------------------------

void ZitaEffectGui::mousePressEvent(QMouseEvent* e)
      {
      for (unsigned idx = 0; idx < rotary.size(); ++idx) {
            const Rotary& ro = rotary[idx];
            QRect re(ro.x, ro.y, 23, 23);
            if (re.contains(e->pos())) {
                  r = idx;
                  break;
                  }
            }
      oval = rotary[r].value;
      mx = e->globalX();
      my = e->globalY();
      }

//---------------------------------------------------------
// mouseReleaseEvent
//---------------------------------------------------------

void ZitaEffectGui::mouseReleaseEvent(QMouseEvent*)
      {
      r = -1;
      }

//---------------------------------------------------------
// mouseMoveEvent
//---------------------------------------------------------

void ZitaEffectGui::mouseMoveEvent(QMouseEvent* e)
      {
      if (r == -1)
            return;
      int dx = e->globalX() - mx;
      int dy = e->globalY() - my;
      if (dy > 0 && dy > dx)
            dx = dy;
      else if (dy < 0 && dy < dx)
            dx = dy;
      qreal v = oval;
      v = v + dx * .01;
      if (v < 0)
            v = 0;
      else if (v > 1.0)
            v = 1.0;
      rotary[r].value = v;

      valueChanged(rotary[r].id, v);
      update();
      }

//---------------------------------------------------------
// paintEvent
//---------------------------------------------------------

void ZitaEffectGui::paintEvent(QPaintEvent*)
      {
      QPainter p(this);
      int x = 0;
      for (const QPixmap& pix : pm) {
            p.drawPixmap(x, 0, pix);
            x += pix.width();
            }
      p.setBrush(QColor(0x3f, 0x3f, 0x3f));
      for (const Rotary& r : rotary) {
            p.save();
            p.translate(r.x + 11.5, r.y + 11.5);
            p.rotate(r.value * 270.0 - 225);
            p.drawRect(QRectF(-2, -2, 11.5 + 2, 4));
            p.restore();
            }
      }

}

