//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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

#ifndef __PARTEDITBASE_H__
#define __PARTEDITBASE_H__

#include "ui_parteditbase.h"
#include "libmscore/instrument.h"
#include "enableplayforwidget.h"

namespace Ms {

class Score;
class Channel;
class Part;

//---------------------------------------------------------
//   PartEdit
//   deprecated
//   TODO: This class should no longer be used and will be removed at a future date
//---------------------------------------------------------

class PartEdit : public QWidget, public Ui::PartEditBase {
      Q_OBJECT

      Channel* channel;
      Part* part;

      QList<QToolButton*> voiceButtons;


   private slots:
      void patchChanged(int, bool syncControls = true);
      void volChanged(double, bool syncControls = true);
      void panChanged(double, bool syncControls = true);
      void reverbChanged(double, bool syncControls = true);
      void chorusChanged(double, bool syncControls = true);
      void muteChanged(bool, bool syncControls = true);
      void soloToggled(bool, bool syncControls = true);
      void drumsetToggled(bool, bool syncControls = true);
      void midiChannelChanged(int);
      void sync(bool syncControls);
      void expandToggled(bool);
      void playbackVoiceChanged();

   public slots:

   signals:
      void soloChanged(bool);

   public:
      PartEdit(QWidget* parent = 0);
      void setPart(Part*, Channel*);
      };

}
#endif // __PARTEDITBASE_H__
