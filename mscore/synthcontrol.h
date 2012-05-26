//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: synthcontrol.h 2047 2009-08-26 18:33:38Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#ifndef __SYNTHCONTROL_H__
#define __SYNTHCONTROL_H__

#include "ui_synthcontrol.h"

class MasterSynth;
namespace Awl {
      class AbstractSlider;
      };

class Score;

//---------------------------------------------------------
//   SynthControl
//---------------------------------------------------------

class SynthControl : public QWidget, Ui::SynthControl {
      Q_OBJECT

      Awl::AbstractSlider* aeolusSection[4][5];

      virtual void closeEvent(QCloseEvent*);
      void updateSyntiValues();
      void updateUpDownButtons();

   private slots:
      void sfUpClicked();
      void sfDownClicked();
      void sfDeleteClicked();
      void sfAddClicked();

      void gainChanged(double, int);
      void masterTuningChanged(double);
      void reverbValueChanged(double val, int idx);
      void chorusValueChanged(double val, int idx);
      void chorusSpeedChanged(double val);
      void chorusDepthChanged(double val);
      void setAeolusValue(double, int);
      void currentSoundFontChanged(int row);
      void chorusNumberChanged(int val);
      void chorusTypeChanged(int val);

   signals:
      void closed(bool);
      void gainChanged(float);
      void soundFontChanged();

   public slots:
      void setGain(float);

   public:
      SynthControl(QWidget* parent);
      void updatePreferences();
      void setMeter(float, float, float, float);
      void stop();
      void setScore(Score*);
      };

#endif

