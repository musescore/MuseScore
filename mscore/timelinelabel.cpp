#include "timelinelabel.h"

TimelineLabel::TimelineLabel(QGraphicsView* view, QString text, QFont font, int rowNumber, int height)
      : _view(view), _rowNumber(rowNumber), _text(text)
      {
      // Gen box (x and y pos?)
      _box = new QGraphicsRectItem(0, _rowNumber * height, 0, height);
      addToGroup(_box);
      _textItem = new QGraphicsTextItem(_text);
      _textItem->setFont(font);
      centerTextVertically();
      addToGroup(_textItem);
      }

void TimelineLabel::updateWidth(int newWidth)
      {
      QRectF newRect = _box->rect();
      newRect.setWidth(newWidth);
      _box->setRect(newRect);
      }

void TimelineLabel::centerTextVertically()
      {
      QPointF targetCenter = _box->boundingRect().center();
      QPointF currentCenter = _textItem->boundingRect().center();
      QPointF offset = targetCenter - currentCenter;
      _textItem->setPos(0, _textItem->boundingRect().translated(offset).y());
      }
