//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: voiceselector.cpp 5056 2011-11-30 20:03:02Z wschweer $
//
//  Copyright (C) 2009-2010 Werner Schweer et al.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "musescore.h"
#include "preferences.h"
#include "voiceselector.h"
#include "libmscore/mscore.h"

namespace Ms {

//---------------------------------------------------------
//   VoiceButton
//---------------------------------------------------------

VoiceButton::VoiceButton(int v, QWidget* parent)
   : QToolButton(parent)
      {
      voice = v;
      setToolButtonStyle(Qt::ToolButtonIconOnly);
      setFixedWidth(preferences.iconWidth+6);
      setFixedHeight(preferences.iconHeight+6);
      setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      setCheckable(true);
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void VoiceButton::paintEvent(QPaintEvent* e)
      {
      QPainter p(this);
      QColor c(MScore::selectColor[voice]);
      QColor bg(palette().color(QPalette::Normal, QPalette::Window));
      p.fillRect(QRect(0, 2, width(), height()), isChecked() ? c.light(170) : bg);
      p.setPen(QPen(preferences.globalStyle == 0 ? Qt::white : Qt::black));
      if (isChecked())
            p.setPen(QPen(Qt::black));
      QFont f = font();
      f.setPointSizeF(16.0);
      p.setFont(f);
      p.drawText(QRect(0, 1, width(), height()), Qt::AlignCenter, QString("%1").arg(voice+1));
      }

//---------------------------------------------------------
//   VoiceSelector
//---------------------------------------------------------

VoiceSelector::VoiceSelector(QWidget* parent)
   : QFrame(parent)
      {
      setLineWidth(0);
      setFrameStyle(QFrame::Box | QFrame::Plain);
      QHBoxLayout* vwl = new QHBoxLayout;
      vwl->setSpacing(0);
      vwl->setContentsMargins(0, 0, 0, 0);

      static const char* sl2[4] = { "voice-1", "voice-2", "voice-3", "voice-4" };
      static const int v[4] = { 0, 1, 2, 3 };

      QActionGroup* vag = new QActionGroup(this);
      vag->setExclusive(true);
      for (int i = 0; i < 4; ++i) {
            QAction* a = getAction(sl2[i]);
            a->setCheckable(true);
            vag->addAction(a);
            VoiceButton* tb = new VoiceButton(v[i]);
            tb->setDefaultAction(a);
            vwl->addWidget(tb);
            }
      setLayout(vwl);
      connect(vag, SIGNAL(triggered(QAction*)), this, SIGNAL(triggered(QAction*)));
      }
}

