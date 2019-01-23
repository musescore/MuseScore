#ifndef TIMELINEDATA_H
#define TIMELINEDATA_H

#include "timeline.h"
#include "timelineabstract.h"

namespace Ms {

class Timeline;
class TimelineData;
class TimelineDataLabels;
class TimelineDataGrid;

//---------------------------------------------------------
//   TimelineDataLabel
//---------------------------------------------------------

class TimelineDataLabel : public TimelineLabel
      {
      TimelineDataLabels* _parent;

   public:
      TimelineDataLabel(TimelineDataLabels* view, QString text, int nMeta);
      };

//---------------------------------------------------------
//   TimelineDataLabels
//---------------------------------------------------------

class TimelineDataLabels : public QGraphicsView, public TimelineComponent
      {
      Q_OBJECT

      QList<TimelineDataLabel*> _labels;
      int _maxTextWidth = 0;

   public slots:
      void updateLabelWidths(int newWidth);

   public:
      TimelineDataLabels(TimelineData* parent);
      TimelineData* getParent();
      void updateLabels();
      int maximumTextWidth() { return _maxTextWidth; }
      };

//---------------------------------------------------------
//   TimelineDataCell
//---------------------------------------------------------

class TimelineDataGridCell : public QGraphicsRectItem
      {
      Measure* _measure;
      int _measureIdx; // For lasso selection, ignores measure properties
      int _staffIdx;

      bool isFilled() { return !_measure->isMeasureRest(_staffIdx) ||
                                _measure->isRepeatMeasure(_measure->score()->staff(_staffIdx)); }
   public:
      TimelineDataGridCell(Measure* measure, int staffIdx, int measureIdx);
      QPair<Measure*, int> infoPair();

      Measure* getMeasureToSelect();
      Measure* measure() { return _measure; }
      int measureIdx() { return _measureIdx; }
      int staffIdx() { return _staffIdx; }
};

//---------------------------------------------------------
//   TimelineDataGrid
//---------------------------------------------------------

class TimelineDataGrid : public QGraphicsView, public TimelineComponent
      {
      Q_OBJECT

      enum ZValues {
            CELL,
            VIEW,
            SELECTION,
            LASSO
            };

      QVector<TimelineDataGridCell*> _grid;
      int _nStaves; // Updated each time the grid is populated. Used for grid cell lookup.
      QGraphicsPathItem* _selectionPathItem = nullptr; // Used to determine if the selection is visible in the timeline
      QPainterPath _oldSelectionPath;

      QPointF _oldMousePos; // Used for mouse dragging
      QGraphicsRectItem* _lassoSelection;
      bool _draggingGrid = false;

      // Stored to facilitate updating the view without updating the whole grid
      QGraphicsPathItem* _visibleCellsPathItem = nullptr;
      QGraphicsPathItem* _nonVisibleCellsPathItem = nullptr;

      TimelineDataGridCell* getCell(int row, int column);
      void populateGrid();
      QList<TimelineDataGridCell*> getVisibleCells();

      QRectF staffMeasureBounds(Measure* measure, int staffIdx, Score* localScore);

      void keepSelectionChangeInView();
      QList<TimelineDataGridCell*> getSelectedCells();
      QPainterPath createCellPath(QList<TimelineDataGridCell*> selectedCells);

      void setMouseCursor(QMouseEvent* event);
      void selectLassoItems(QList<QGraphicsItem*> items);

   public slots:
      void updateView();

   public:
      TimelineDataGrid(TimelineData* parent);

      QRectF gridBounds();

      void updateSelection();
      void updateGrid();
      void redrawGrid();

      int scrollBarValue() { return horizontalScrollBar()->value(); }
      void setScrollBarValue(int v) { horizontalScrollBar()->setValue(v); }
      void adjustScrollBar(int originalScrollValue, QPoint globalCursorPos, int originalGridWidth);

      virtual void mousePressEvent(QMouseEvent* event);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void mouseReleaseEvent(QMouseEvent* event);
      };

//---------------------------------------------------------
//   TimelineData
//---------------------------------------------------------

class TimelineData : public QSplitter, public TimelineComponent
      {
      Q_OBJECT

   public slots:
      void metaSplitterMoved();

   public:
      TimelineData(Timeline* parent);

      void updateData() { gridView()->updateGrid();
                          labelView()->updateLabels(); }
      TimelineDataLabels* labelView() { return static_cast<TimelineDataLabels*>(widget(0)); }
      TimelineDataGrid* gridView() { return static_cast<TimelineDataGrid*>(widget(1)); }

      Timeline* getParent();
      };

}

#endif // TIMELINEDATA_H
