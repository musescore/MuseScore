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
      changeLoopingPanelVisibility(false);

      setScore(0);
      playButton->setDefaultAction(getAction("play"));
      rewindButton->setDefaultAction(getAction("rewind"));
      metronomeButton->setDefaultAction(getAction("metronome"));
      loopButton->setDefaultAction(getAction("loop"));

      connect(volumeSlider, SIGNAL(valueChanged(double,int)), SLOT(volumeChanged(double,int)));
      connect(posSlider,    SIGNAL(sliderMoved(int)),         SLOT(setPos(int)));
      connect(tempoSlider,  SIGNAL(valueChanged(double,int)), SLOT(relTempoChanged(double,int)));
      connect(loopButton,   SIGNAL(toggled(bool)),            SLOT(changeLoopingPanelVisibility(bool)));
      connect(fromMeasure,  SIGNAL(currentIndexChanged(QString)), SLOT(updateFromMeasure()));
      connect(toMeasure,    SIGNAL(currentIndexChanged(QString)), SLOT(updateToMeasure()));
      connect(fromSegment,  SIGNAL(currentIndexChanged(QString)), SLOT(updateFromSegment()));
      connect(toSegment,    SIGNAL(currentIndexChanged(QString)), SLOT(updateToSegment()));
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

void PlayPanel::setCurrentIndexWithBlockSignals(QComboBox* comboBox, int currentIndex)
{
      bool oldState = comboBox->blockSignals(true);
      comboBox->setCurrentIndex(currentIndex);
      comboBox->blockSignals(oldState);
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
            // measure changes
            connect(cs, SIGNAL(measuresUpdated()), SLOT(updateLoopingInterface()));
            initialization = true;
            updateFromMeasure();
            initialization = false;
//            setCurrentIndexWithBlockSignals(fromMeasure, 0);
//            setCurrentIndexWithBlockSignals(fromSegment, 0);
//            setCurrentIndexWithBlockSignals(toMeasure, toMeasure->count() - 1);
//            setCurrentIndexWithBlockSignals(toSegment, toSegment->count() - 1);
//            fromMeasure->setCurrentIndex(0);
//            fromSegment->setCurrentIndex(0);
//            toMeasure->setCurrentIndex(toMeasure->count() - 1);
//            toSegment->setCurrentIndex(toSegment->count() - 1);
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
            }
      char buffer[32];
      sprintf(buffer, "%03d.%02d", bar+1, beat+1);
      posLabel->setText(QString(buffer));
      }

void debugComboBox(QComboBox* comboBox)
{
      QString out = "";
      for (int i = 0; i < comboBox->count(); i++)
           out += comboBox->itemText(i) + ", ";
      qDebug() << out;
}

void PlayPanel::updateLoopingInterface()
{
      updateFromMeasure(true);
      updateToMeasure(true);
      updateFromSegment(true);
      updateToSegment(true);
}

//---------------------------------------------------------
//   updateComboBox
//---------------------------------------------------------

void PlayPanel::updateComboBox(QComboBox* comboBox)
      {
      int scoreCount;
      bool isTo = false;
      int from = -1;
      if (comboBox == fromMeasure || comboBox == toMeasure) {
            scoreCount = cs->measures()->size();
            }
      else if (comboBox == fromSegment || comboBox == toSegment) {
            int measureNumber;
            if (comboBox == fromSegment)
                  measureNumber = getFromMeasure();
            else
                  measureNumber = getToMeasure();
            scoreCount = getSegmentCount(measureNumber);
            }
      else {
            throw(QString("Wrong comboBox"));
            }
      if (comboBox == toMeasure || comboBox == toSegment) {
            isTo = true;
            if (comboBox == toMeasure)
                  from = getFromMeasure();
            else
                  from = getFromSegment();
            }
      int cachedCount = comboBox->count();
      int max = cachedCount > scoreCount ? cachedCount : scoreCount;
      for (int number = 0; number < max; number++) {
            QString numberString = QString::number(number + 1);
            int index = comboBox->findText(numberString);
            if (number < scoreCount) {
                  bool missing = index == -1;
                  if (missing && ((!isTo) || (isTo && number >= from))) {
                        int newIndex = comboBox->findText(QString::number(number)) + 1;
                        comboBox->insertItem(newIndex, numberString);
                        }
                  if (!missing && isTo && number < from)
                        comboBox->removeItem(index);
                  }
            else {
                  comboBox->removeItem(index);
                  }
//            debugComboBox(comboBox);
            }
      if (initialization)
            setCurrentIndexWithBlockSignals(comboBox, !isTo ? 0 : comboBox->count() - 1);
      }

//---------------------------------------------------------
//   updateFromMeasure
//---------------------------------------------------------

void PlayPanel::updateFromMeasure(bool skipUpdates)
      {
      qDebug() <<"updateFromMeasure";
      updateComboBox(fromMeasure);
      if (!skipUpdates) {
            updateToMeasure(true);
            updateFromSegment();
            }
      }

//---------------------------------------------------------
//   updateToMeasure
//---------------------------------------------------------

void PlayPanel::updateToMeasure(bool skipUpdates)
      {
      qDebug() <<"updateToMeasure";
      updateComboBox(toMeasure);
      if (!skipUpdates)
            updateToSegment();
      }

//---------------------------------------------------------
//   updateFromSegment
//---------------------------------------------------------

void PlayPanel::updateFromSegment(bool skipUpdates)
      {
      qDebug() <<"updateFromSegment";
      updateComboBox(fromSegment);
      if (!skipUpdates)
            updateToSegment();
      }

//---------------------------------------------------------
//   updateToSegment
//---------------------------------------------------------

void PlayPanel::updateToSegment(bool skipUpdates)
      {
      qDebug() <<"updateToSegment";
      updateComboBox(toSegment);
      }

//---------------------------------------------------------
//   getSegmentCount
//---------------------------------------------------------

int PlayPanel::getSegmentCount(int measureNumber) {
      MeasureBase* mb = cs->measure(measureNumber);
      Measure* m = static_cast<Measure*>(mb);
      int segmentCount = 0;
      static const Segment::SegmentType st = Segment::SegChordRestGrace;
      for (Segment* s = m->first(st); s; s = s->next(st), segmentCount++);
      return segmentCount;
}

//---------------------------------------------------------
//   getFromMeasure
//---------------------------------------------------------

int PlayPanel::getFromMeasure()
      {
         return fromMeasure->currentText().toInt() - 1;
      }

//---------------------------------------------------------
//   getToMeasure
//---------------------------------------------------------

int PlayPanel::getToMeasure()
      {
         return toMeasure->currentText().toInt() - 1;
      }

//---------------------------------------------------------
//   getFromSegment
//---------------------------------------------------------

int PlayPanel::getFromSegment()
      {
         return fromSegment->currentText().toInt() - 1;
      }

//---------------------------------------------------------
//   getToSegment
//---------------------------------------------------------

int PlayPanel::getToSegment()
      {
         return toSegment->currentText().toInt() - 1;
      }

//---------------------------------------------------------
//   changeLoopingPanelVisibility
//---------------------------------------------------------

void PlayPanel::changeLoopingPanelVisibility(bool toggled)
      {
      if (toggled) {
            rangeLabel->show();
            fromLabel->show();
            fromMeasure->show();
            fromSegment->show();
            toLabel->show();
            toMeasure->show();
            toSegment->show();
      } else {
            rangeLabel->hide();
            fromLabel->hide();
            fromMeasure->hide();
            fromSegment->hide();
            toLabel->hide();
            toMeasure->hide();
            toSegment->hide();
            }
      }
