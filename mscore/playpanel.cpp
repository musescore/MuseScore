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

#include "playpanel.h"
#include "libmscore/sig.h"
#include "libmscore/score.h"
#include "libmscore/repeatlist.h"
#include "seq.h"
#include "musescore.h"
#include "libmscore/measure.h"
#include "audio/midi/msynthesizer.h"


namespace Ms {

//---------------------------------------------------------
//   PlayPanel
//---------------------------------------------------------

PlayPanel::PlayPanel(QWidget* parent)
    : QDockWidget(qApp->translate("PlayPanelBase", "Play Panel"), parent)
      {
      cachedTickPosition = -1;
      cachedTimePosition = -1;
      cs                 = 0;
      _isSpeedSliderPressed = false;
      setupUi(this);
      setWindowFlags(Qt::Tool);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
      MuseScore::restoreGeometry(this);

      setScore(0);

      playButton->setDefaultAction(getAction("play"));
      rewindButton->setDefaultAction(getAction("rewind"));
      countInButton->setDefaultAction(getAction("countin"));
      metronomeButton->setDefaultAction(getAction("metronome"));
      loopButton->setDefaultAction(getAction("loop"));
      loopInButton->setDefaultAction(getAction("loop-in"));
      loopOutButton->setDefaultAction(getAction("loop-out"));
      enablePlay = new EnablePlayForWidget(this);

      float minDecibels = synti->minGainAsDecibels;
      float maxDecibels = synti->maxGainAsDecibels;
      volSpinBox->setRange(minDecibels, maxDecibels);

      volumeSlider->setLog(false);
      volumeSlider->setRange(minDecibels, maxDecibels);
      volumeSlider->setDclickValue1(synti->defaultGainAsDecibels);

      speedSlider->setDclickValue1(100.0);
      speedSlider->setDclickValue2(100.0);
      speedSlider->setUseActualValue(true);
      mgainSlider->setValue(seq->metronomeGain());
      mgainSlider->setDclickValue1(seq->metronomeGain() - 10.75f);
      mgainSlider->setDclickValue2(seq->metronomeGain() - 10.75f);

      volumeSlider->setDclickValue1(synti->defaultGainAsDecibels); // double click restores -40dB default
      volumeSlider->setDclickValue2(synti->defaultGainAsDecibels);

      connect(volumeSlider, SIGNAL(valueChanged(double,int)), SLOT(volumeChanged(double,int)));
      connect(mgainSlider,  SIGNAL(valueChanged(double,int)), SLOT(metronomeGainChanged(double,int)));
      connect(posSlider,    SIGNAL(sliderMoved(int)),         SLOT(setPos(int)));
      connect(speedSlider,  SIGNAL(valueChanged(double,int)), SLOT(speedChanged(double,int)));
      connect(speedSlider,  SIGNAL(sliderPressed(int)),       SLOT(speedSliderPressed(int)));
      connect(speedSlider,  SIGNAL(sliderReleased(int)),      SLOT(speedSliderReleased(int)));
      connect(speedSpinBox, SIGNAL(valueChanged(int)),        SLOT(speedChanged()));
      connect(volSpinBox,   SIGNAL(valueChanged(double)),     SLOT(volSpinBoxEdited()));
      connect(seq,          SIGNAL(heartBeat(int,int,int)),   SLOT(heartBeat(int,int,int)));

      volLabel();
      volSpinBoxEdited();     //update spinbox and, as a side effect, the slider with current gain value
      }

PlayPanel::~PlayPanel()
      {
      // if widget is visible, store geometry and pos into settings
      // if widget is not visible/closed, pos is not reliable (and anyway
      // has been stored into settings when the widget has been hidden)
      if (isVisible()) {
            MuseScore::saveGeometry(this);
            }
      }

//---------------------------------------------------------
//   speedChanged
//---------------------------------------------------------

void PlayPanel::speedChanged(double d, int)
      {
      double speed = d * .01;
      // Snap speed slider to 100% when it gets close
      if (speed < 1.01 && speed > 0.99) {
            speed = 1.00;
            }
      emit speedChanged(speed);
      setTempo(seq->curTempo() * speed);
      setSpeed(speed);
      }

//---------------------------------------------------------
//   speedChanged
//---------------------------------------------------------

void PlayPanel::speedChanged()
      {
      double v = speedSpinBox->value();
      speedSlider->setValue(v);
      speedChanged(v, 0);
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
      QDockWidget::closeEvent(ev);
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
      MuseScore::saveGeometry(this);
      QDockWidget::hideEvent(ev);
      }

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void PlayPanel::showEvent(QShowEvent* e)
      {
      if (e->spontaneous() && !isFloating()) {
            QDockWidget::showEvent(e);
            }
      else {
            enablePlay->showEvent(e);
            QDockWidget::showEvent(e);
            activateWindow();
            setFocus();
            }
      }

//---------------------------------------------------------
//   eventFilter
//---------------------------------------------------------

bool PlayPanel::eventFilter(QObject* obj, QEvent* e)
      {
      if (enablePlay->eventFilter(obj, e))
            return true;
      return QDockWidget::eventFilter(obj, e);
      }

void PlayPanel::keyPressEvent(QKeyEvent* ev) {
      if (ev->key() == Qt::Key_Escape && ev->modifiers() == Qt::NoModifier) {
            close();
            return;
            }
      QDockWidget::keyPressEvent(ev);
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
      speedSlider->setEnabled(enable);
      if (cs && seq && seq->canStart()) {
            setTempo(cs->tempomap()->tempo(0));
            setSpeed(cs->tempomap()->relTempo());
            setEndpos(cs->repeatList().ticks());
            Fraction tick = cs->pos(POS::CURRENT);
            heartBeat(tick.ticks(), tick.ticks(), 0);
            }
      else {
            setTempo(2.0);
            setSpeed(1.0);
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
//   speed
//---------------------------------------------------------

double PlayPanel::speed() const
      {
      return speedSpinBox->value() / 100.0;
      }

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void PlayPanel::setTempo(double val)
      {
      tempoLabel->setText(tr("Tempo\n%1 BPM").arg(val * 60, 6, 'f', 2, QLatin1Char(' ')));
      }

//---------------------------------------------------------
//   setSpeed
//---------------------------------------------------------

void PlayPanel::setSpeed(double speed)
      {
      const auto clampedRoundedVal = qBound(10, static_cast<int>((100.0 * speed) - std::remainder(100.0 * speed, 1.0)), 300);
      speedSlider->setValue(clampedRoundedVal);
      speedSpinBox->setValue(speedSlider->value());
      }

//---------------------------------------------------------
//   increaseSpeed
//---------------------------------------------------------

void PlayPanel::increaseSpeed()
      {
      const auto speed = speedSpinBox->value();
      const auto step = speedSpinBox->singleStep();

      // Increase to the next higher increment (relative to 100%).
      setSpeed((100 + step * (((speed + ((speed >= 100) ? 0 : 1 - step) -100) / step) + 1)) / 100.0);
      }

//---------------------------------------------------------
//   decreaseSpeed
//---------------------------------------------------------

void PlayPanel::decreaseSpeed()
      {
      const auto speed = speedSpinBox->value();
      const auto step = speedSpinBox->singleStep();

      // Decrease to the next lower increment (relative to 100%).
      setSpeed((100 + step * (((speed + ((speed >= 100) ? 0 : 1 - step) - 100) / step) - 1)) / 100.0);
      }

//---------------------------------------------------------
//   resetSpeed
//---------------------------------------------------------

void PlayPanel::resetSpeed()
      {
      setSpeed(1.0);
      }

//---------------------------------------------------------
//   setGain
//---------------------------------------------------------

void PlayPanel::setGain(float gain)  // respond to gainChanged() SIGNAL from MasterSynthesizer
      {
      Q_UNUSED(gain);
      const QSignalBlocker blockVolumeSpinBoxSignals(volSpinBox);
      volumeSlider->setValue(synti->gainAsDecibels());
      volLabel();
      }


//---------------------------------------------------------
//   volumeChanged
//---------------------------------------------------------

void PlayPanel::volumeChanged(double decibels, int)
      {
      synti->setGainAsDecibels(decibels);
      }

//---------------------------------------------------------
//   metronomeGainChanged
//---------------------------------------------------------

void PlayPanel::metronomeGainChanged(double val, int)
      {
      emit metronomeGainChanged(val);
      }

//---------------------------------------------------------
//    setPos
//---------------------------------------------------------

void PlayPanel::setPos(int utick)
      {
      if (!cs)
            return;
      if (cachedTickPosition != utick)
            emit posChange(utick);
      updatePosLabel(utick);
      updateTimeLabel(cs->utick2utime(utick));
      }

//---------------------------------------------------------
//   heartBeat
//---------------------------------------------------------

void PlayPanel::heartBeat(int /*tick*/, int utick, int samples)
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
      
      // time is displayed in three separate labels
      // this prevents jitter as width of time grows and shrinks
      // alternative would be to use a monospaced font and a
      // single label
      char hourBuffer[8];
      sprintf(hourBuffer, "%d", h);
      hourLabel->setText(QString(hourBuffer));

      char minBuffer[8];
      sprintf(minBuffer, "%02d", m);
      minuteLabel->setText(QString(minBuffer));
      
      char secondBuffer[8];
      sprintf(secondBuffer, "%02d", sec);
      secondLabel->setText(QString(secondBuffer));
          
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
            tick = cs->repeatList().utick2tick(utick);
            cs->sigmap()->tickValues(tick, &bar, &beat, &t);
            double tpo = cs->tempomap()->tempo(tick) * cs->tempomap()->relTempo();
            setTempo(tpo);
            }
     
      // position is displayed in two separate labels
      // this prevents jitter as width of time grows and shrinks
      // alternative would be to use a monospaced font and a
      // single label

      char barBuffer[12];
      sprintf(barBuffer, "%d", bar+1);// sprintf(barBuffer, "%03d", bar+1);
      measureLabel->setText(QString(barBuffer));

      char beatBuffer[12];
      sprintf(beatBuffer, "%02d", beat+1);
      beatLabel->setText(QString(beatBuffer));
      }

//---------------------------------------------------------
//   speedSliderPressed
//---------------------------------------------------------

void PlayPanel::speedSliderPressed(int)
      {
      _isSpeedSliderPressed = true;
      }
//---------------------------------------------------------
//   setVolume
//---------------------------------------------------------
      
void PlayPanel::volLabel()
      {
      volSpinBox->setValue(synti->gainAsDecibels());
      volSpinBox->setSuffix(" dB");
      }


void PlayPanel::volSpinBoxEdited()
      {
      synti->setGainAsDecibels(volSpinBox->value());
      volLabel();
      }


//---------------------------------------------------------
//   speedSliderReleased
//---------------------------------------------------------

void PlayPanel::speedSliderReleased(int)
      {
      _isSpeedSliderPressed = false;
      }

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void PlayPanel::changeEvent(QEvent *event)
      {
      QDockWidget::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
      }

//---------------------------------------------------------
//   setSpeedIncrement
//---------------------------------------------------------

void PlayPanel::setSpeedIncrement(const int speedIncrement)
      {
      speedSpinBox->setSingleStep(speedIncrement);
      }
}
