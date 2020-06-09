#ifndef IPLAYPANEL_H
#define IPLAYPANEL_H

namespace Ms {

class IPlayPanel {

   public:
      virtual ~IPlayPanel() {}

      virtual double speed() const = 0;
      virtual void setTempo(double) = 0;
      virtual void setSpeed(double) = 0;
      virtual void increaseSpeed() = 0;
      virtual void decreaseSpeed() = 0;
      virtual void resetSpeed() = 0;
      virtual bool isSpeedSliderPressed() const = 0;
};

}

#endif // IPLAYPANEL_H
