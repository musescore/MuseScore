//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: playpanel.cpp 4775 2011-09-12 14:25:31Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "playpanel.h"
#include "libmscore/sig.h"
#include "libmscore/score.h"
#include "libmscore/repeatlist.h"
#include "seq.h"
#include "musescore.h"
#include "libmscore/measure.h"
#include "libmscore/undo.h"

const int MIN_VOL = -60;
const int MAX_VOL = 10;

//---------------------------------------------------------
//   PlayPanel
//---------------------------------------------------------

PlayPanel::PlayPanel(QWidget* parent)
   : QWidget(parent, Qt::Dialog)
      {
      cachedTickPosition = -1;
      cachedTimePosition = -1;
      cs                 = 0;
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      setScore(0);
      playButton->setDefaultAction(getAction("play"));
      rewindButton->setDefaultAction(getAction("rewind"));
      metronomeButton->setDefaultAction(getAction("metronome"));
      loopButton->setDefaultAction(getAction("loop"));

      connect(volumeSlider,         SIGNAL(valueChanged(double,int)), SLOT(volumeChanged(double,int)));
      connect(posSlider,            SIGNAL(sliderMoved(int)),         SLOT(setPos(int)));
      connect(tempoSlider,          SIGNAL(valueChanged(double,int)), SLOT(relTempoChanged(double,int)));
      connect(loopButton,           SIGNAL(toggled(bool)),            SLOT(loopingSetup(bool)));
      connect(loopButton,           SIGNAL(toggled(bool)),            SLOT(setLoopingVisible(bool)));
      connect(tempoFrom,            SIGNAL(valueChanged(double)),     SLOT(cancelRepetition()));
      connect(tempoTo,              SIGNAL(valueChanged(double)),     SLOT(cancelRepetition()));
      connect(tempoIncrementBy,     SIGNAL(valueChanged(double)),     SLOT(cancelRepetition()));
      connect(transposeFrom,        SIGNAL(valueChanged(double)),     SLOT(cancelRepetition()));
      connect(transposeTo,          SIGNAL(valueChanged(double)),     SLOT(cancelRepetition()));
      connect(transposeIncrementBy, SIGNAL(valueChanged(double)),     SLOT(cancelRepetition()));
      connect(tempoFrom,            SIGNAL(valueChanged(double)),     SLOT(updateTempoIncrementBy()));
      connect(tempoTo,              SIGNAL(valueChanged(double)),     SLOT(updateTempoIncrementBy()));
      connect(transposeFrom,        SIGNAL(valueChanged(double)),     SLOT(updateTransposeIncrementBy()));
      connect(transposeTo,          SIGNAL(valueChanged(double)),     SLOT(updateTransposeIncrementBy()));
      }

//---------------------------------------------------------
//   relTempoChanged
//---------------------------------------------------------

void PlayPanel::relTempoChanged(double d, int)
      {
      emit relTempoChanged(d * .01);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void PlayPanel::closeEvent(QCloseEvent* ev)
      {
      emit closed();
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void PlayPanel::setScore(Score* s)
      {
      if (cs != 0 && cs == s)
            return;
      cs = s;
      bool enable = cs != 0;
      volumeSlider->setEnabled(enable);
      posSlider->setEnabled(enable);
      tempoSlider->setEnabled(enable);
      if (cs && seq && seq->canStart()) {
            setTempo(cs->tempomap()->tempo(0));
            setRelTempo(cs->tempomap()->relTempo());
            setEndpos(cs->repeatList()->ticks());
            int tick = cs->playPos();
            heartBeat(tick, tick);
            }
      else {
            setTempo(120.0);
            setRelTempo(1.0);
            setEndpos(0);
            heartBeat(0, 0);
            updatePosLabel(0);
            }
      update();
      }

//---------------------------------------------------------
//   setEndpos
//---------------------------------------------------------

void PlayPanel::setEndpos(int val)
      {
      posSlider->setRange(0, val);
      }

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void PlayPanel::setTempo(double val)
      {
      int tempo = lrint(val * 60.0);
      tempoLabel->setText(QString("%1 bpm").arg(tempo, 3, 10, QLatin1Char(' ')));
      }

//---------------------------------------------------------
//   setRelTempo
//---------------------------------------------------------

void PlayPanel::setRelTempo(qreal val)
      {
      val *= 100;
      relTempo->setText(QString("%1 %").arg(val, 3, 'f', 0));
      tempoSlider->setValue(val);
      }

//---------------------------------------------------------
//   setGain
//---------------------------------------------------------

void PlayPanel::setGain(float val)
      {
      volumeSlider->setValue(val);
      }

//---------------------------------------------------------
//   volumeChanged
//---------------------------------------------------------

void PlayPanel::volumeChanged(double val, int)
      {
      emit gainChange(val);
      }

//---------------------------------------------------------
//    setPos
//---------------------------------------------------------

void PlayPanel::setPos(int utick)
      {
      if(!cs)
            return;
      if (cachedTickPosition != utick)
            emit posChange(utick);
      updatePosLabel(utick);
      updateTimeLabel(cs->utick2utime(utick));
      }

//---------------------------------------------------------
//   heartBeat
//---------------------------------------------------------

void PlayPanel::heartBeat(int tick, int utick)
      {
      if (cachedTickPosition == utick)
            return;
      updatePosLabel(utick);
      posSlider->setValue(utick);
      }

//---------------------------------------------------------
//   heartBeat2
//---------------------------------------------------------

void PlayPanel::heartBeat2(int samples)
      {
      int sec = samples/MScore::sampleRate;
      if (sec == cachedTimePosition)
            return;
      updateTimeLabel(sec);
      }

//---------------------------------------------------------
//   updateTime
//---------------------------------------------------------

void PlayPanel::updateTimeLabel(int sec)
      {
      cachedTimePosition = sec;
      int m              = sec / 60;
      sec                = sec % 60;
      int h              = m / 60;
      m                  = m % 60;
      char buffer[32];
      sprintf(buffer, "%d:%02d:%02d", h, m, sec);
      timeLabel->setText(QString(buffer));
      }

//---------------------------------------------------------
//   updatePos
//---------------------------------------------------------
      
void PlayPanel::updatePosLabel(int utick)      
      {
      cachedTickPosition = utick;
      int bar = 0;
      int beat = 0;
      int t = 0;
      int tick = 0;
      if (cs) {
            tick = cs->repeatList()->utick2tick(utick);
            cs->sigmap()->tickValues(tick, &bar, &beat, &t);
            double tpo = cs->tempomap()->tempo(tick) * cs->tempomap()->relTempo();
            setTempo(tpo);
            }
      char buffer[32];
      sprintf(buffer, "%03d.%02d", bar+1, beat+1);
      posLabel->setText(QString(buffer));
      }

//---------------------------------------------------------
//   setLoopingVisible
//---------------------------------------------------------

void PlayPanel::setLoopingVisible(bool visible)
      {
      if (visible) {
           cachedSize = size();
           groupBox->show();
           adjustSize();
           }
      else {
           groupBox->hide();
           resize(cachedSize);
           adjustSize();
           }
      }

//---------------------------------------------------------
//   nextValue
//---------------------------------------------------------

qreal PlayPanel::nextValue(QDoubleSpinBox* fromBox, QDoubleSpinBox* toBox, QDoubleSpinBox* incrementByBox)
      {
      qreal from = fromBox->value();
      qreal to = toBox->value();
      qreal inc = incrementByBox->value();
      qreal next = from + currentIteration * inc;
      if (from < to)
            return next > to ? to : next;
      else if (from > to)
            return next < to ? to : next;
      else
            return to;
      }

//---------------------------------------------------------
//   getTransposeDirection
//---------------------------------------------------------

TransposeDirection PlayPanel::getTransposeDirection(bool flip)
      {
      int from = transposeFrom->value();
      int to = transposeTo->value();
      if (!flip)
            return from < to ? TRANSPOSE_UP : TRANSPOSE_DOWN;
      else
            return to < from ? TRANSPOSE_UP : TRANSPOSE_DOWN;
      }

//---------------------------------------------------------
//   nextIteration
//---------------------------------------------------------

void PlayPanel::nextIteration()
      {
      qDebug() << currentIteration << currentTransposition;
      qreal relTempo = nextValue(tempoFrom, tempoTo, tempoIncrementBy) * .01;
      cs->tempomap()->setRelTempo(relTempo);
      emit relTempoChanged(relTempo);
      int transpose = (int) nextValue(transposeFrom, transposeTo, transposeIncrementBy);
      int times = abs(transpose != currentTransposition ? (int) transposeIncrementBy->value() : 0);
      transposeSelectionBySemitone(times);
      currentTransposition = transpose;
      currentIteration++;
      }

//---------------------------------------------------------
//   loopingSetup
//---------------------------------------------------------

void PlayPanel::loopingSetup(bool start)
      {
      if (start) {
            currentIteration = 0;
            currentTransposition = 0;
            }
      else {
            if (playButton->isChecked()) {
                  undoTransposition();
                  getAction("play")->trigger();
                  }
            }
      }

//---------------------------------------------------------
//   undoTransposition
//---------------------------------------------------------

void PlayPanel::undoTransposition()
      {
      for (int i = 0; i < currentIteration; i++)
            cs->undo()->undo();
      cs->endUndoRedo();
      }

//---------------------------------------------------------
//   transposeBySemitone
//---------------------------------------------------------

void PlayPanel::transposeSelectionBySemitone(int times, bool flip)
      {
      Selection selection = cs->selection();
      if (selection.state() == SEL_NONE)
            cs->cmdSelectAll();
      cs->startCmd();
      for (int i = 0; i < times; i++) {
            cs->transpose(TRANSPOSE_BY_INTERVAL, getTransposeDirection(flip), 0, 1, false, true, false);
            }
      cs->endCmd();
      cs->setSelection(selection);
      }

//---------------------------------------------------------
//   updateIncrementBy
//---------------------------------------------------------

void PlayPanel::updateIncrementBy(QDoubleSpinBox* fromBox, QDoubleSpinBox* toBox, QDoubleSpinBox* incrementByBox)
      {
      qreal from = fromBox->value();
      qreal to = toBox->value();
      qreal incrementBy = incrementByBox->value();
      if (from < to) {
            incrementByBox->setRange(1, to - from);
            if (incrementBy < 0)
                  incrementByBox->setValue(incrementBy * -1);
      }
      else if (from > to) {
            incrementByBox->setRange(to - from, -1);
            if (incrementBy > 0)
                  incrementByBox->setValue(incrementBy * -1);
            }
      else {
            incrementByBox->setRange(0, 0);
            incrementByBox->setValue(0);
            }
      }

//---------------------------------------------------------
//   updateTempoIncrementBy
//---------------------------------------------------------

void PlayPanel::updateTempoIncrementBy()
      {
      updateIncrementBy(tempoFrom, tempoTo, tempoIncrementBy);
      }

//---------------------------------------------------------
//   updateTransposeIncrementBy
//---------------------------------------------------------

void PlayPanel::updateTransposeIncrementBy()
      {
      updateIncrementBy(transposeFrom, transposeTo, transposeIncrementBy);
      }

//---------------------------------------------------------
//   cancelRepetition
//---------------------------------------------------------

void PlayPanel::cancelRepetition()
      {
            loopingSetup(false);
            loopingSetup(true);
      }
