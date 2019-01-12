#ifndef TIMELINELABEL_H
#define TIMELINELABEL_H

class TimelineLabel : public QGraphicsItemGroup {

      QGraphicsView* _view;
      QGraphicsRectItem* _box;
      QGraphicsTextItem* _textItem;
      int _rowNumber;
      QString _text;

public:
      TimelineLabel(QGraphicsView* view, QString text, QFont font, int rowNumber, int height);
      void updateWidth(int newWidth);
      void centerTextVertically();
      };

#endif // TIMELINELABEL_H
