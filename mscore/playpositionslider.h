#ifndef __PLAYPOSITIONSLIDER__
#define __PLAYPOSITIONSLIDER__
#include <QAccessibleWidget>

namespace Ms {
class PlayPanel;

class PlayPositionSlider : public QSlider {
      Q_OBJECT
      PlayPanel* playPanel;
      virtual void keyPressEvent(QKeyEvent*) override;
public:
      PlayPositionSlider(QWidget* p = 0);
      void setPlayPanel(PlayPanel* p)  { playPanel = p;     }
      QLabel* posLabel() const;
      QLabel* timeLabel() const;
signals:
      void updateAccessibleValue();
      };

class AccessiblePlayPosSlider : public QObject, QAccessibleWidget  {
      Q_OBJECT

      PlayPositionSlider* posSlider;
      virtual QString text(QAccessible::Text t) const override;
public:
      AccessiblePlayPosSlider(PlayPositionSlider*);
      static QAccessibleInterface* PlayPosSliderFactory(const QString &classname, QObject *object);

public slots:
      void updateValue();
      };

}

#endif
