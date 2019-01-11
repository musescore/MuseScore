#include "timelinelabel.h"

TimelineLabel::TimelineLabel(QGraphicsView* view, int rowNumber, int height)
      : _view(view), _rowNumber(rowNumber)
      {
      // Gen box (x and y pos?)
      _box = new QGraphicsRectItem(0, rowNumber * height, 0, height);
      addToGroup(_box);
      }

void TimelineLabel::updateWidth(int newWidth)
      {
      QRectF newRect = _box->rect();
      newRect.setWidth(newWidth);
      _box->setRect(newRect);
      }
