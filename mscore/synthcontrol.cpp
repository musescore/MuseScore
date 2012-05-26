//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: select.cpp 2054 2009-08-28 16:15:01Z wschweer $
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

#include "synthcontrol.h"
#include "musescore.h"
#include "seq.h"
#include "msynth/synti.h"
#include "preferences.h"
#include "mixer.h"
#include "aeolus/aeolus/aeolus.h"
#include "libmscore/score.h"
#include "file.h"
#include "msynth/sparm_p.h"
#include "fluid/rev.h"
#include "fluid/fluid.h"
#include "icons.h"
#include "libmscore/mscore.h"

using namespace FluidS;

//---------------------------------------------------------
//   SynthControl
//---------------------------------------------------------

SynthControl::SynthControl(QWidget* parent)
   : QWidget(parent, Qt::Dialog)
      {
      setupUi(this);
      saveReverbPreset->setIcon(*icons[fileSave_ICON]);
      saveChorusPreset->setIcon(*icons[fileSave_ICON]);

      reverbRoomSize->setId(REVERB_ROOMSIZE);
      reverbDamp->setId(REVERB_DAMP);
      reverbWidth->setId(REVERB_WIDTH);
      reverb->setId(REVERB_GAIN);

      position->setId(AEOLUS_STPOSIT);

      chorus->setId(CHORUS_GAIN);
      chorusSpeed->setId(CHORUS_SPEED);
      chorusDepth->setId(CHORUS_DEPTH);

      connect(position, SIGNAL(valueChanged(double, int)), SLOT(setAeolusValue(double, int)));

      aeolusSection[0][0] = aeolusAzimuth3;
      aeolusSection[0][1] = aeolusWidth3;
      aeolusSection[0][2] = aeolusDirect3;
      aeolusSection[0][3] = aeolusReflect3;
      aeolusSection[0][4] = aeolusReverb3;

      aeolusSection[1][0] = aeolusAzimuth2;
      aeolusSection[1][1] = aeolusWidth2;
      aeolusSection[1][2] = aeolusDirect2;
      aeolusSection[1][3] = aeolusReflect2;
      aeolusSection[1][4] = aeolusReverb2;

      aeolusSection[2][0] = aeolusAzimuth1;
      aeolusSection[2][1] = aeolusWidth1;
      aeolusSection[2][2] = aeolusDirect1;
      aeolusSection[2][3] = aeolusReflect1;
      aeolusSection[2][4] = aeolusReverb1;

      aeolusSection[3][0] = aeolusAzimuthP;
      aeolusSection[3][1] = aeolusWidthP;
      aeolusSection[3][2] = aeolusDirectP;
      aeolusSection[3][3] = aeolusReflectP;
      aeolusSection[3][4] = aeolusReverbP;

      for (int i = 0; i < 4; ++i) {
            for (int k = 0; k < 5; ++k) {
                  aeolusSection[i][k]->init(synti->parameter(SParmId(AEOLUS_ID, i+1, k).val));
                  aeolusSection[i][k]->setId(((i+1) << 8) + k);
                  connect(aeolusSection[i][k], SIGNAL(valueChanged(double, int)), SLOT(setAeolusValue(double, int)));
                  }
            }

      soundFontUp->setEnabled(false);
      soundFontDown->setEnabled(false);
      soundFontDelete->setEnabled(false);
      soundFontAdd->setEnabled(true);

      connect(gain,            SIGNAL(valueChanged(double,int)), SLOT(gainChanged(double,int)));
      connect(masterTuning,    SIGNAL(valueChanged(double)),     SLOT(masterTuningChanged(double)));

      connect(reverb,          SIGNAL(valueChanged(double,int)), SLOT(reverbValueChanged(double,int)));
      connect(reverbRoomSize,  SIGNAL(valueChanged(double,int)), SLOT(reverbValueChanged(double,int)));
      connect(reverbDamp,      SIGNAL(valueChanged(double,int)), SLOT(reverbValueChanged(double,int)));
      connect(reverbWidth,     SIGNAL(valueChanged(double,int)), SLOT(reverbValueChanged(double,int)));

      connect(chorus,          SIGNAL(valueChanged(double,int)), SLOT(chorusValueChanged(double,int)));
      connect(chorusSpeed,     SIGNAL(valueChanged(double,int)), SLOT(chorusValueChanged(double,int)));
      connect(chorusDepth,     SIGNAL(valueChanged(double,int)), SLOT(chorusValueChanged(double,int)));
      connect(chorusSpeedBox,  SIGNAL(valueChanged(double)),     SLOT(chorusSpeedChanged(double)));
      connect(chorusDepthBox,  SIGNAL(valueChanged(double)),     SLOT(chorusDepthChanged(double)));
      connect(chorusNumber,    SIGNAL(valueChanged(int)),        SLOT(chorusNumberChanged(int)));
      connect(chorusType,      SIGNAL(currentIndexChanged(int)), SLOT(chorusTypeChanged(int)));

      connect(soundFontUp,     SIGNAL(clicked()),                SLOT(sfUpClicked()));
      connect(soundFontDown,   SIGNAL(clicked()),                SLOT(sfDownClicked()));
      connect(soundFontDelete, SIGNAL(clicked()),                SLOT(sfDeleteClicked()));
      connect(soundFontAdd,    SIGNAL(clicked()),                SLOT(sfAddClicked()));
      connect(soundFonts,      SIGNAL(currentRowChanged(int)),   SLOT(currentSoundFontChanged(int)));

      updateSyntiValues();
      }

//---------------------------------------------------------
//   updateSyntiValues
//---------------------------------------------------------

void SynthControl::updateSyntiValues()
      {
      masterTuning->setValue(synti->masterTuning());
      setGain(synti->gain());

      roomSizeBox->setValue(synti->parameter(SParmId(FLUID_ID, REVERB_GROUP, REVERB_ROOMSIZE).val).fval());
      dampBox->setValue(synti->parameter    (SParmId(FLUID_ID, REVERB_GROUP, REVERB_DAMP).val).fval());
      widthBox->setValue(synti->parameter   (SParmId(FLUID_ID, REVERB_GROUP, REVERB_WIDTH).val).fval());
      reverb->setValue(synti->parameter     (SParmId(FLUID_ID, REVERB_GROUP, REVERB_GAIN).val).fval());

      chorus->setValue(synti->parameter     (SParmId(FLUID_ID, CHORUS_GROUP, CHORUS_GAIN).val).fval());

      float val = synti->parameter(SParmId(FLUID_ID, CHORUS_GROUP, CHORUS_SPEED).val).fval();
      chorusSpeed->setValue(val);
      chorusSpeedBox->setValue(val * 5);

      val = synti->parameter(SParmId(FLUID_ID, CHORUS_GROUP, CHORUS_DEPTH).val).fval();
      chorusDepth->setValue(val);
      chorusDepthBox->setValue(val * 10);

      chorusNumber->setValue(synti->parameter(SParmId(FLUID_ID, CHORUS_GROUP, CHORUS_BLOCKS).val).fval() * 100.0);
      chorusType->setCurrentIndex(int(synti->parameter(SParmId(FLUID_ID, CHORUS_GROUP, CHORUS_TYPE).val).fval()));

      reverbDelay->init(synti->parameter(SParmId(AEOLUS_ID, 0, AEOLUS_REVSIZE).val));
      reverbDelay->setId(AEOLUS_REVSIZE);
      connect(reverbDelay, SIGNAL(valueChanged(double, int)), SLOT(setAeolusValue(double, int)));

      reverbTime->init(synti->parameter(SParmId(AEOLUS_ID, 0, AEOLUS_REVTIME).val));
      reverbTime->setId(AEOLUS_REVTIME);
      connect(reverbTime, SIGNAL(valueChanged(double, int)), SLOT(setAeolusValue(double, int)));

      position->init(synti->parameter(SParmId(AEOLUS_ID, 0, AEOLUS_STPOSIT).val));
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void SynthControl::setScore(Score* cs)
      {
      setWindowTitle("MuseScore: Synthesizer");
      Synth* sy = synti->synth("Fluid");
      soundFonts->clear();
      if (sy)
            soundFonts->addItems(sy->soundFonts());
      updateSyntiValues();
      updateUpDownButtons();
      }

//---------------------------------------------------------
//   setGain
//---------------------------------------------------------

void SynthControl::setGain(float val)
      {
      gain->setValue(val);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void SynthControl::closeEvent(QCloseEvent* ev)
      {
      emit closed(false);
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   showSynthControl
//---------------------------------------------------------

void MuseScore::showSynthControl()
      {
      QAction* a = getAction("synth-control");

      if (synthControl == 0) {
            synthControl = new SynthControl(this);
            synthControl->setScore(cs);
            connect(synthControl, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            connect(seq, SIGNAL(gainChanged(float)), synthControl, SLOT(setGain(float)));
            connect(synthControl, SIGNAL(gainChanged(float)), seq, SLOT(setGain(float)));

            if (iledit) {
                  connect(synthControl, SIGNAL(soundFontChanged()), iledit,
                     SLOT(patchListChanged()));
                  }
            }
      synthControl->setShown(a->isChecked());
      }

//---------------------------------------------------------
//   updatePreferences
//---------------------------------------------------------

void SynthControl::updatePreferences()
      {
      if ((preferences.tuning != masterTuning->value())
         || (preferences.masterGain != gain->value())
         || (preferences.reverbRoomSize != reverbRoomSize->value())
         || (preferences.reverbDamp != reverbDamp->value())
         || (preferences.reverbWidth != reverbWidth->value())
         || (preferences.reverbGain != reverb->value())
         || (preferences.chorusGain != chorus->value())
         ) {
            preferences.dirty  = true;
            }
      preferences.tuning     = masterTuning->value();
      preferences.masterGain = gain->value();

      preferences.reverbRoomSize = reverbRoomSize->value();
      preferences.reverbDamp     = reverbDamp->value();
      preferences.reverbWidth    = reverbWidth->value();
      preferences.reverbGain     = reverb->value();
      preferences.chorusGain     = chorus->value();
      }

//---------------------------------------------------------
//   sfDeleteClicked
//---------------------------------------------------------

void SynthControl::sfDeleteClicked()
      {
      int row = soundFonts->currentRow();
      if (row >= 0) {
            QString s(soundFonts->item(row)->text());
            Synth* sy = synti->synth("Fluid");
            if (sy)
                  sy->removeSoundFont(s);
            delete soundFonts->takeItem(row);
            }
      updateUpDownButtons();
      }

//---------------------------------------------------------
//   sfAddClicked
//---------------------------------------------------------

void SynthControl::sfAddClicked()
      {
      QStringList files = mscore->getSoundFont("");
      if (!files.isEmpty()) {
            int n = soundFonts->count();
            QStringList sl;
            for (int i = 0; i < n; ++i) {
                  QListWidgetItem* item = soundFonts->item(i);
                  sl.append(item->text());
                  }
            QStringList list = files;
            QStringList::Iterator it = list.begin();
            while(it != list.end()) {
                  QString s = *it;
                  if (sl.contains(s)) {
                        QMessageBox::warning(this,
                           tr("MuseScore"),
                           QString(tr("Soundfont %1 already loaded")).arg(s));
                        }
                  else {
                        Synth* sy = synti->synth("Fluid");
                        if (sy) {
                              bool loaded = sy->addSoundFont(s);
                              if (!loaded) {
                                    QMessageBox::warning(this,
                                       tr("MuseScore"),
                                       QString(tr("cannot load soundfont %1")).arg(s));
                                    }
                              else {
                                    soundFonts->insertItem(0, s);
                                    }
                              }
                        }
                  ++it;
                  }
            }
      updateUpDownButtons();
      }

//---------------------------------------------------------
//   gainChanged
//---------------------------------------------------------

void SynthControl::gainChanged(double val, int)
      {
      emit gainChanged(val);
      }

//---------------------------------------------------------
//   masterTuningChanged
//---------------------------------------------------------

void SynthControl::masterTuningChanged(double val)
      {
      synti->setMasterTuning(val);
      }

//---------------------------------------------------------
//   setMeter
//---------------------------------------------------------

void SynthControl::setMeter(float l, float r, float left_peak, float right_peak)
      {
      gain->setMeterVal(0, l, left_peak);
      gain->setMeterVal(1, r, right_peak);
      }

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

void SynthControl::stop()
      {
      gain->setMeterVal(0, .0, .0);
      gain->setMeterVal(1, .0, .0);
      }

//---------------------------------------------------------
//   reverbValueChanged
//---------------------------------------------------------

void SynthControl::reverbValueChanged(double val, int idx)
      {
      synti->setParameter(SParmId(FLUID_ID, REVERB_GROUP, idx).val, val);
      }

//---------------------------------------------------------
//   chorusValueChanged
//---------------------------------------------------------

void SynthControl::chorusValueChanged(double val, int idx)
      {
      if (idx == CHORUS_SPEED)
            chorusSpeedBox->setValue(val * 5.0);
      else if (idx == CHORUS_DEPTH)
            chorusDepthBox->setValue(val * 10.0);
      synti->setParameter(SParmId(FLUID_ID, CHORUS_GROUP, idx).val, val);
      }

//---------------------------------------------------------
//   chorusSpeedChanged
//---------------------------------------------------------

void SynthControl::chorusSpeedChanged(double val)
      {
      val /= 5.0;
      chorusSpeed->setValue(val);
      synti->setParameter(SParmId(FLUID_ID, CHORUS_GROUP, CHORUS_SPEED).val, val);
      }

//---------------------------------------------------------
//   chorusDepthChanged
//---------------------------------------------------------

void SynthControl::chorusDepthChanged(double val)
      {
      val /= 10.0;
      chorusDepth->setValue(val);
      synti->setParameter(SParmId(FLUID_ID, CHORUS_GROUP, CHORUS_DEPTH).val, val);
      }

//---------------------------------------------------------
//   setAeolusValue
//---------------------------------------------------------

void SynthControl::setAeolusValue(double val, int idx)
      {
      synti->setParameter(SParmId(AEOLUS_ID, idx >> 8, idx & 0xff).val, val);
      }

//---------------------------------------------------------
//   currentSoundFontChanged
//---------------------------------------------------------

void SynthControl::currentSoundFontChanged(int /*row*/)
      {
      updateUpDownButtons();
      }

//---------------------------------------------------------
//   sfUpClicked
//---------------------------------------------------------

void SynthControl::sfUpClicked()
      {
      int row  = soundFonts->currentRow();
      if (row <= 0)
            return;
      Synth* sy = synti->synth("Fluid");
      if (sy) {
            QStringList sfonts = sy->soundFonts();
            sfonts.swap(row, row-1);
            sy->loadSoundFonts(sfonts);
            sfonts = sy->soundFonts();
            soundFonts->clear();
            soundFonts->addItems(sfonts);
            soundFonts->setCurrentRow(row-1);
            }
      }

//---------------------------------------------------------
//   sfDownClicked
//---------------------------------------------------------

void SynthControl::sfDownClicked()
      {
      int rows = soundFonts->count();
      int row  = soundFonts->currentRow();
      if (row + 1 >= rows)
            return;

      Synth* sy = synti->synth("Fluid");
      if (sy) {
            QStringList sfonts = sy->soundFonts();
            sfonts.swap(row, row+1);
            sy->loadSoundFonts(sfonts);
sfonts = sy->soundFonts();
            soundFonts->clear();
            soundFonts->addItems(sfonts);
            soundFonts->setCurrentRow(row+1);
            }
      }

//---------------------------------------------------------
//   updateUpDownButtons
//---------------------------------------------------------

void SynthControl::updateUpDownButtons()
      {
      int rows = soundFonts->count();
      int row = soundFonts->currentRow();
      soundFontUp->setEnabled(row > 0);
      soundFontDown->setEnabled((row != -1) && (row < (rows-1)));
      soundFontDelete->setEnabled(row != -1);
      }

//---------------------------------------------------------
//   chorusNumberChanged
//---------------------------------------------------------

void SynthControl::chorusNumberChanged(int val)
      {
      synti->setParameter(SParmId(FLUID_ID, CHORUS_GROUP, CHORUS_BLOCKS).val, double(val)/100.0);
      }

//---------------------------------------------------------
//   chorusTypeChanged
//---------------------------------------------------------

void SynthControl::chorusTypeChanged(int val)
      {
      synti->setParameter(SParmId(FLUID_ID, CHORUS_GROUP, CHORUS_TYPE).val, double(val));
      }

