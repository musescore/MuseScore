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
#include "synthesizer/msynthesizer.h"
#include "synthesizer/synthesizer.h"
#include "synthesizer/synthesizergui.h"
#include "aeolus/aeolus/aeolus.h"
#include "preferences.h"
#include "mixer.h"
#include "file.h"
#include "icons.h"
#include "libmscore/score.h"
#include "libmscore/mscore.h"
#include "libmscore/undo.h"
#include "effects/effectgui.h"

extern MasterSynthesizer* synti;
extern bool useFactorySettings;

static std::vector<const char*> effectNames = { "None", "Freeverb", "Zita1" };

//---------------------------------------------------------
//   SynthControl
//---------------------------------------------------------

SynthControl::SynthControl(QWidget* parent)
   : QWidget(parent, Qt::Dialog)
      {
      setupUi(this);
      _score = 0;

      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      Synthesizer* zerberus = synti->synthesizer("Zerberus");
      Synthesizer* fluid    = synti->synthesizer("Fluid");

      tabWidget->insertTab(0, zerberus->gui(), tr(zerberus->name()));
      tabWidget->insertTab(0, fluid->gui(),    tr(fluid->name()));

      // effectA        combo box
      // effectStackA   widget stack

      effectA->clear();
      for (Effect* e : synti->effectList(0)) {
            effectA->addItem(tr(e->name()));
            effectStackA->addWidget(e->gui());
            }

      effectB->clear();
      for (Effect* e : synti->effectList(1)) {
            effectB->addItem(tr(e->name()));
            effectStackB->addWidget(e->gui());
            }

      if (!useFactorySettings) {
            QSettings settings;
            settings.beginGroup("SynthControl");
            resize(settings.value("size", QSize(600, 268)).toSize());
            move(settings.value("pos", QPoint(10, 10)).toPoint());
            settings.endGroup();
            }

      tabWidget->setCurrentIndex(0);

      connect(effectA,      SIGNAL(currentIndexChanged(int)), SLOT(effectAChanged(int)));
      connect(effectB,      SIGNAL(currentIndexChanged(int)), SLOT(effectBChanged(int)));
      connect(gain,         SIGNAL(valueChanged(double,int)), SLOT(gainChanged(double,int)));
      connect(masterTuning, SIGNAL(valueChanged(double)),     SLOT(masterTuningChanged(double)));
      connect(loadButton,   SIGNAL(clicked()),                SLOT(loadButtonClicked()));
      connect(saveButton,   SIGNAL(clicked()),                SLOT(saveButtonClicked()));
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
            connect(synti, SIGNAL(gainChanged(float)), synthControl, SLOT(setGain(float)));
            connect(synthControl, SIGNAL(gainChanged(float)), synti, SLOT(setGain(float)));

            if (mixer) {
                  connect(synthControl, SIGNAL(soundFontChanged()), mixer,
                     SLOT(patchListChanged()));
                  }
            }
      synthControl->setVisible(a->isChecked());
      }

//---------------------------------------------------------
//   updatePreferences
//---------------------------------------------------------

void SynthControl::updatePreferences()
      {
      if ((preferences.tuning != masterTuning->value())
         || (preferences.masterGain != gain->value())
         ) {
            preferences.dirty  = true;
            }
      preferences.tuning     = masterTuning->value();
      preferences.masterGain = gain->value();
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void SynthControl::writeSettings() const
      {
      QSettings settings;
      settings.beginGroup("SynthControl");
      settings.setValue("size", size());
      settings.setValue("pos", pos());
      settings.endGroup();
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
//   effectAChanged
//---------------------------------------------------------

void SynthControl::effectAChanged(int idx)
      {
      synti->setEffect(0, idx);
      effectStackA->setCurrentIndex(idx);
      }

//---------------------------------------------------------
//   effectBChanged
//---------------------------------------------------------

void SynthControl::effectBChanged(int idx)
      {
      synti->setEffect(1, idx);
      effectStackB->setCurrentIndex(idx);
      }

//---------------------------------------------------------
//   loadButtonClicked
//    load synthesizer settings from score
//---------------------------------------------------------

void SynthControl::loadButtonClicked()
      {
      synti->setState(_score->synthesizerState());
      updateSyntiValues();
      }

//---------------------------------------------------------
//   saveButtonClicked
//    save synthesizer settings to score
//---------------------------------------------------------

void SynthControl::saveButtonClicked()
      {
      if (_score && synti) {
            _score->startCmd();
            _score->undo()->push(new ChangeSynthesizerState(_score, synti->state()));
            _score->endCmd();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   updateSyntiValues
//---------------------------------------------------------

void SynthControl::updateSyntiValues()
      {
      masterTuning->setValue(synti->masterTuning());
      setGain(synti->gain());

      int idx = synti->indexOfEffect(0);
      effectA->setCurrentIndex(idx);
      effectStackA->setCurrentIndex(idx);
      synti->effect(0)->gui()->updateValues();
      synti->effect(1)->gui()->updateValues();

      idx = synti->indexOfEffect(1);
      effectB->setCurrentIndex(idx);
      effectStackB->setCurrentIndex(idx);
      }

