#ifndef TIMELINELABEL_H
#define TIMELINELABEL_H

class TimelineLabel : public QGraphicsItemGroup {

      QGraphicsView* _view;
      QGraphicsRectItem* _box;
//      QGraphicsTextItem* _text;
      int _rowNumber;

public:
      TimelineLabel(QGraphicsView* view, int rowNumber, int height);
      void updateWidth(int newWidth);
      };

#endif // TIMELINELABEL_H
