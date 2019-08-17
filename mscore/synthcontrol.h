//=============================================================================
//  MusE Score
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

#ifndef __SYNTHCONTROL_H__
#define __SYNTHCONTROL_H__

#include "ui_synthcontrol.h"
#include "enableplayforwidget.h"

namespace Ms {

class Score;

//---------------------------------------------------------
//   SynthControl
//---------------------------------------------------------

class SynthControl : public QWidget, Ui::SynthControl {
      Q_OBJECT

      Score* _score;
      EnablePlayForWidget* enablePlay;
      bool _dirty    { false };

      virtual void closeEvent(QCloseEvent*);
      virtual void showEvent(QShowEvent*);
      virtual bool eventFilter(QObject*, QEvent*);
      virtual void keyPressEvent(QKeyEvent*) override;
      void updateGui();
      void readSettings();
      void updateExpressivePatches();
      void updateMixer();
      void setAllUserBankController(bool val);

   private slots:
      void gainChanged(double, int);
      void masterTuningChanged(double);
      void changeMasterTuning();
      void effectAChanged(int);
      void effectBChanged(int);
      void loadButtonClicked();
      void saveButtonClicked();
      void storeButtonClicked();
      void recallButtonClicked();
      void dynamicsMethodChanged(int);
      void ccToUseChanged(int);
      void switchExprButtonClicked();
      void switchNonExprButtonClicked();
      void resetExprButtonClicked();
      void setDirty();

   signals:
      void gainChanged(float);
      void soundFontChanged();
      void closed(bool);

   protected:
      virtual void changeEvent(QEvent *event);
      void retranslate()  { retranslateUi(this); }

   public slots:
      void setGain(float);

   public:
      SynthControl(QWidget* parent);
      void setMeter(float, float, float, float);
      void stop();
      void setScore(Score* s);
      void writeSettings();
      };
}

#endif

