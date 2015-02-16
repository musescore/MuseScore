#include "playpanel.h"
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/mscore.h"
#include "libmscore/fraction.h"

namespace Ms {

PlayPositionSlider::PlayPositionSlider(QWidget *p) : QSlider(p)
      {
      setAccessibleDescription(tr("Use Left and Right arrow keys to move beat by beat. Use Ctrl + Left and Right arrow keys to move measure by measure."));
      }

void PlayPositionSlider::keyPressEvent(QKeyEvent* e)
      {
      if (playPanel->score()) {
            Measure* m = playPanel->score()->tick2measure(value());
            float ratio = 4.0f / m->timesig().denominator();
            int increment = static_cast<int>(round(MScore::division*ratio));
            int offset = (value() - m->tick()) % increment; // align to the beat

            if (e->key() == Qt::Key_Right) {
                  if (e->modifiers() == Qt::NoModifier) {
                        setValue(value() - offset + increment);
                        return;
                        }
                  else if (e->modifiers() == Qt::ControlModifier && m->nextMeasureMM()) {
                        setValue(m->nextMeasureMM()->tick());
                        return;
                        }
                  }

            if (e->key() == Qt::Key_Left) {
                  if (e->modifiers() == Qt::NoModifier) {
                        setValue(value() - offset - increment);
                        return;
                        }
                  else if (e->modifiers() == Qt::ControlModifier && m->prevMeasureMM()) {
                        setValue(m->prevMeasureMM()->tick());
                        return;
                        }
                  }
            }
      QSlider::keyPressEvent(e);
      }

QLabel* PlayPositionSlider::posLabel() const
      {
      return playPanel->position();
      }

QLabel* PlayPositionSlider::timeLabel() const
      {
      return playPanel->time();
      }

AccessiblePlayPosSlider::AccessiblePlayPosSlider(PlayPositionSlider* s) : QAccessibleWidget(s)
      {
      posSlider = s;
      connect(posSlider, SIGNAL(updateAccessibleValue()), this, SLOT(updateValue()));
      }

QString AccessiblePlayPosSlider::text(QAccessible::Text t) const
      {
      if (t == QAccessible::Value) {
            QStringList measurebeat = posSlider->posLabel()->text().split(".");
            QStringList time = posSlider->timeLabel()->text().split(":");
            QString measure = measurebeat.at(0);

            while (measure.at(0) == '0') {
                  measure = measure.mid(1);
                  }

            QString beat = measurebeat.at(1);
            if (beat.at(0) == '0')
                  beat = beat.mid(1);

            QString minute = time.at(1);
            if (minute.at(0) == '0' )
                  minute = minute.mid(1);

            QString second = time.at(2);
            if (second.at(0) == '0')
                  second = second.mid(1);

            QString measureBeatString = tr("Measure %1 Beat %2").arg(measure).arg(beat);
            QString timeString = tr("Minute %1 Second %2").arg(minute).arg(second);
            QString hour = time.at(0);

            if (hour != "0") {
                  timeString = tr("Hour %1 ").arg(hour) + timeString;
                  }

            return QString("%1 %2").arg(measureBeatString).arg(timeString);
            }

      return QAccessibleWidget::text(t);
      }

void AccessiblePlayPosSlider::updateValue()
      {
      QAccessibleValueChangeEvent ev(posSlider, text(QAccessible::Value));
      QAccessible::updateAccessibility(&ev);
      }

QAccessibleInterface* AccessiblePlayPosSlider::PlayPosSliderFactory(const QString &classname, QObject *object)
      {
      QAccessibleInterface *iface = 0;
      if (classname == QLatin1String("Ms::PlayPositionSlider") && object && object->isWidgetType()) {
           iface = static_cast<QAccessibleInterface*>(new AccessiblePlayPosSlider(static_cast<PlayPositionSlider*>(object)));
           }

          return iface;
      }

}
