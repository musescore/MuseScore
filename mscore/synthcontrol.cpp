//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "synthcontrol.h"
#include "musescore.h"
#include "seq.h"
#include "synthesizer/msynthesizer.h"
#include "synthesizer/synthesizer.h"
#include "synthesizer/synthesizergui.h"
#include "mixer.h"
#include "file.h"
#include "icons.h"
#include "libmscore/score.h"
#include "libmscore/mscore.h"
#include "libmscore/xml.h"
#include "libmscore/undo.h"
#include "effects/effectgui.h"

namespace Ms {

extern MasterSynthesizer* synti;
extern bool useFactorySettings;

//---------------------------------------------------------
//   SynthControl
//---------------------------------------------------------

SynthControl::SynthControl(QWidget* parent)
   : QWidget(parent, Qt::Dialog)
      {
      setObjectName("SynthControl");
      setupUi(this);
      _score = 0;

      setWindowFlags(Qt::Tool);
//      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      int idx = 0;
      for (Synthesizer* s : synti->synthesizer()) {
            if (strcmp(s->name(), "Aeolus") == 0)    // no gui for aeolus
                  continue;
            tabWidget->insertTab(idx++, s->gui(), tr(s->name()));
            s->gui()->synthesizerChanged();
            connect(s->gui(), SIGNAL(valueChanged()), SLOT(setDirty()));
            }

      // effectA        combo box
      // effectStackA   widget stack

      effectA->clear();
      for (Effect* e : synti->effectList(0)) {
            effectA->addItem(tr(e->name()));
            EffectGui* gui = e->gui();
            if (gui) {
                  effectStackA->addWidget(gui);
                  connect(gui, SIGNAL(valueChanged()), SLOT(setDirty()));
                  }
            }

      effectB->clear();
      for (Effect* e : synti->effectList(1)) {
            effectB->addItem(tr(e->name()));
            EffectGui* gui = e->gui();
            if (gui) {
                  effectStackB->addWidget(gui);
                  connect(gui, SIGNAL(valueChanged()), SLOT(setDirty()));
                  }
            }
      readSettings();

      updateGui();

      storeButton->setEnabled(false);
      recallButton->setEnabled(false);
      changeTuningButton->setEnabled(false);

      enablePlay = new EnablePlayForWidget(this);
      connect(effectA,      SIGNAL(currentIndexChanged(int)), SLOT(effectAChanged(int)));
      connect(effectB,      SIGNAL(currentIndexChanged(int)), SLOT(effectBChanged(int)));
      connect(gain,         SIGNAL(valueChanged(double,int)), SLOT(gainChanged(double,int)));
      connect(masterTuning, SIGNAL(valueChanged(double)),     SLOT(masterTuningChanged(double)));
      connect(changeTuningButton, SIGNAL(clicked()),          SLOT(changeMasterTuning()));
      connect(loadButton,   SIGNAL(clicked()),                SLOT(loadButtonClicked()));
      connect(saveButton,   SIGNAL(clicked()),                SLOT(saveButtonClicked()));
      connect(storeButton,  SIGNAL(clicked()),                SLOT(storeButtonClicked()));
      connect(recallButton, SIGNAL(clicked()),                SLOT(recallButtonClicked()));
      connect(gain,         SIGNAL(valueChanged(double,int)), SLOT(setDirty()));
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
//   showEvent
//---------------------------------------------------------

void SynthControl::showEvent(QShowEvent* e)
      {
      enablePlay->showEvent(e);
      QWidget::showEvent(e);
      activateWindow();
      setFocus();
      }

//---------------------------------------------------------
//   eventFilter
//---------------------------------------------------------

bool SynthControl::eventFilter(QObject* obj, QEvent* e)
      {
      if(enablePlay->eventFilter(obj, e))
            return true;
      return QWidget::eventFilter(obj, e);
      }

void SynthControl::keyPressEvent(QKeyEvent* ev) {
      if (ev->key() == Qt::Key_Escape && ev->modifiers() == Qt::NoModifier) {
            close();
            return;
            }
      QWidget::keyPressEvent(ev);
      }

//---------------------------------------------------------
//   showSynthControl
//---------------------------------------------------------

void MuseScore::showSynthControl(bool val)
      {
      QAction* a = getAction("synth-control");
      if (synthControl == 0) {
            synthControl = new SynthControl(this);
            mscore->stackUnder(synthControl);
            synthControl->setScore(cs);
            connect(synti,        SIGNAL(gainChanged(float)), synthControl, SLOT(setGain(float)));
            connect(synthControl, SIGNAL(gainChanged(float)), synti, SLOT(setGain(float)));
            connect(synthControl, SIGNAL(closed(bool)), a,     SLOT(setChecked(bool)));
            if (mixer)
                  connect(synthControl, SIGNAL(soundFontChanged()), mixer, SLOT(patchListChanged()));
            }
      synthControl->setVisible(val);
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

void SynthControl::masterTuningChanged(double /*val*/)
      {
      changeTuningButton->setEnabled(true);
      }

//---------------------------------------------------------
//   changeMasterTuning
//---------------------------------------------------------

void SynthControl::changeMasterTuning()
      {
      synti->setMasterTuning(masterTuning->value());
      changeTuningButton->setEnabled(false);
      setDirty();
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
      if (!_score)
            return;
      synti->setState(_score->synthesizerState());
      updateGui();
      loadButton->setEnabled(false);
      saveButton->setEnabled(false);
      storeButton->setEnabled(true);
      recallButton->setEnabled(true);
      changeTuningButton->setEnabled(false);
      }

//---------------------------------------------------------
//   saveButtonClicked
//    save synthesizer settings to score
//---------------------------------------------------------

void SynthControl::saveButtonClicked()
      {
      if (!_score)
            return;
      _score->startCmd();
      _score->undo(new ChangeSynthesizerState(_score, synti->state()));
      _score->endCmd();

      loadButton->setEnabled(false);
      saveButton->setEnabled(false);
      storeButton->setEnabled(true);
      recallButton->setEnabled(true);
      }

//---------------------------------------------------------
//   recallButtonClicked
//    load stored synthesizer settings
//---------------------------------------------------------

void SynthControl::recallButtonClicked()
      {
      if (!_score) {
            qDebug("no score");
            return;
            }

      SynthesizerState state;
      QString s(dataPath + "/synthesizer.xml");
      QFile f(s);
      if (!f.open(QIODevice::ReadOnly)) {
            qDebug("cannot read synthesizer settings <%s>", qPrintable(s));
            return;
            }
      XmlReader e(&f);
      while (e.readNextStartElement()) {
            if (e.name() == "Synthesizer")
                  state.read(e);
            else
                  e.unknown();
            }
      synti->setState(state);
      updateGui();

      storeButton->setEnabled(false);
      recallButton->setEnabled(false);

      loadButton->setEnabled(true);
      saveButton->setEnabled(true);
      changeTuningButton->setEnabled(false);
      }

//---------------------------------------------------------
//   storeButtonClicked
//    save synthesizer settings
//---------------------------------------------------------

void SynthControl::storeButtonClicked()
      {
      if (!_score) {
            qDebug("no score");
            return;
            }
      QString s(dataPath + "/synthesizer.xml");
      QFile f(s);
      if (!f.open(QIODevice::WriteOnly)) {
            qDebug("cannot write synthesizer settings <%s>", qPrintable(s));
            return;
            }
      XmlWriter xml(0, &f);
      xml.header();
      synti->state().write(xml);

      storeButton->setEnabled(false);
      recallButton->setEnabled(false);
      }

//---------------------------------------------------------
//   Gui
//---------------------------------------------------------

void SynthControl::updateGui()
      {
      masterTuning->setValue(synti->masterTuning());
      setGain(synti->gain());

      int idx = synti->indexOfEffect(0);
      effectA->setCurrentIndex(idx);
      effectStackA->setCurrentIndex(idx);
      if (synti->effect(0) && synti->effect(0)->gui())
            synti->effect(0)->gui()->updateValues();
      if (synti->effect(1) && synti->effect(1)->gui())
            synti->effect(1)->gui()->updateValues();

      idx = synti->indexOfEffect(1);
      effectB->setCurrentIndex(idx);
      effectStackB->setCurrentIndex(idx);
      for (Synthesizer* s : synti->synthesizer()) {
            if (strcmp(s->name(), "Aeolus") == 0)    // no gui for aeolus
                  continue;
            s->gui()->synthesizerChanged();
            }
      }

//---------------------------------------------------------
//   setDirty
//---------------------------------------------------------

void SynthControl::setDirty()
      {
      loadButton->setEnabled(true);
      saveButton->setEnabled(true);
      storeButton->setEnabled(true);
      recallButton->setEnabled(true);
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void SynthControl::writeSettings()
      {
      QSettings settings;
      settings.beginGroup(objectName());
      settings.setValue("tab", tabWidget->currentIndex());
      settings.endGroup();

      MuseScore::saveGeometry(this);
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void SynthControl::readSettings()
      {
      if (!useFactorySettings) {
            QSettings settings;
            settings.beginGroup(objectName());
            tabWidget->setCurrentIndex(settings.value("tab", 0).toInt());
            settings.endGroup();
            }
      else {
            tabWidget->setCurrentIndex(0);
            }

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void SynthControl::changeEvent(QEvent *event)
      {
      QWidget::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
      }

}

