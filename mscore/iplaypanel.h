#ifndef IPLAYPANEL_H
#define IPLAYPANEL_H

namespace Ms {
class IPlayPanel
{
public:
    virtual ~IPlayPanel() {}

    virtual void setTempo(double) = 0;
    virtual void setRelTempo(qreal) = 0;
    virtual bool isTempoSliderPressed() const = 0;
};
}

#endif // IPLAYPANEL_H
