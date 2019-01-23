#ifndef TIMELINEABSTRACT_H
#define TIMELINEABSTRACT_H

#include "libmscore/score.h"
#include "scoreview.h"

namespace Ms {

//---------------------------------------------------------
//   TimelineComponent
//---------------------------------------------------------

class TimelineComponent {
      static Score* _score;
      static ScoreView* _scoreView;

      // Sizing for all timeline elements
      static int _cellHeight;
      static int _cellWidth;
      static const int _minCellWidth = 3;
      static const int _maxCellWidth = 25;

   public:
      Score* score() { return _score; }
      void setScore(Score* s) { _score = s; updateScore(); }
      virtual void updateScore() {}

      ScoreView* scoreView() { return _scoreView; }
      void setScoreView(ScoreView* sv) { _scoreView = sv; updateScoreView(); }
      virtual void updateScoreView() {}

      int cellHeight() { return _cellHeight; }
      void setCellHeight(int newHeight) { _cellHeight = newHeight; }
      int cellWidth() { return _cellWidth; }
      void setCellWidth(int newWidth) { _cellWidth = newWidth; }
      int minCellWidth() { return _minCellWidth; }
      int maxCellWidth() { return _maxCellWidth; }

      QFont getFont();
      };

//---------------------------------------------------------
//   TimelineLabel
//---------------------------------------------------------

class TimelineLabel : public QGraphicsItemGroup, public TimelineComponent
      {
      QGraphicsView* _view;
      QGraphicsRectItem* _box;
      QGraphicsTextItem* _textItem;
      int _rowNumber;
      QString _text;

public:
      TimelineLabel(QGraphicsView* view, QString text, QFont font, int rowNumber, int height);
      void updateWidth(int newWidth, int handleWidth);
      void centerTextVertically();
      int getTextWidth();
      };

}

#endif // timelineabstract_H
