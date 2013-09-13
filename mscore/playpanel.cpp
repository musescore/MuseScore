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

namespace Ms {

const int MIN_VOL = -60;
const int MAX_VOL = 10;

static const int DEFAULT_POS_X  = 300;
static const int DEFAULT_POS_Y  = 100;

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

      QSettings settings;
      restoreGeometry(settings.value("playPanel/geometry").toByteArray());
      move(settings.value("playPanel/pos", QPoint(DEFAULT_POS_X, DEFAULT_POS_Y)).toPoint());

      setScore(0);
      playButton->setDefaultAction(getAction("play"));
      rewindButton->setDefaultAction(getAction("rewind"));
      metronomeButton->setDefaultAction(getAction("metronome"));
      loopButton->setDefaultAction(getAction("loop"));
      loopInButton->setDefaultAction(getAction("loop-in"));
      loopOutButton->setDefaultAction(getAction("loop-out"));

      connect(volumeSlider, SIGNAL(valueChanged(double,int)), SLOT(volumeChanged(double,int)));
      connect(posSlider,    SIGNAL(sliderMoved(int)),         SLOT(setPos(int)));
      connect(tempoSlider,  SIGNAL(valueChanged(double,int)), SLOT(relTempoChanged(double,int)));
      connect(relTempoBox,     SIGNAL(editingFinished()),  SLOT(relTempoChanged()));
      connect(seq,          SIGNAL(heartBeat(int,int,int)),   SLOT(heartBeat(int,int,int)));
      }

PlayPanel::~PlayPanel()
{
      QSettings settings;
      // if widget is visible, store geometry and pos into settings
      // if widget is not visible/closed, pos is not reliable (and anyway
      // has been stored into settings when the widget has been hidden)
      if (isVisible()) {
            settings.setValue("playPanel/pos", pos());
            settings.setValue("playPanel/geometry", saveGeometry());
            }
}

//---------------------------------------------------------
//   relTempoChanged
//---------------------------------------------------------

void PlayPanel::relTempoChanged(double d, int)
      {
      double relTempo = d * .01;
      emit relTempoChanged(relTempo);

      setTempo(seq->curTempo() * relTempo);
      setRelTempo(relTempo);
      }

//---------------------------------------------------------
//   relTempoChanged
//---------------------------------------------------------

void PlayPanel::relTempoChanged()
      {
      double v = relTempoBox->value();
      tempoSlider->setValue(v);
      emit relTempoChanged(v * .01);
      }

//---------------------------------------------------------
//   closeEvent
//
//    Called when the PlayPanel is closed with its own button
//    but not when it is hidden with the main menu command
//---------------------------------------------------------

void PlayPanel::closeEvent(QCloseEvent* ev)
      {
      emit closed(false);
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   hideEvent
//
//    Called both when the PlayPanel is closed with its own button and
//    when it is hidden via the main menu command
//
//    Stores widget geometry and position into settings.
//---------------------------------------------------------

void PlayPanel::hideEvent(QHideEvent* ev)
      {
      QSettings settings;
      settings.setValue("playPanel/pos", pos());
      settings.setValue("playPanel/geometry", saveGeometry());
      QWidget::hideEvent(ev);
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
            heartBeat(tick, tick, 0);
            }
      else {
            setTempo(120.0);
            setRelTempo(1.0);
            setEndpos(0);
            heartBeat(0, 0, 0);
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
      //relTempo->setText(QString("%1 %").arg(val, 3, 'f', 0));
      relTempoBox->setValue(val);
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

void PlayPanel::heartBeat(int tick, int utick, int samples)
      {
      if (cachedTickPosition != utick) {
            updatePosLabel(utick);
            posSlider->setValue(utick);
            }
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
}

