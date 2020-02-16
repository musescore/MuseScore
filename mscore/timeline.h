//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include "libmscore/select.h"
#include "scoreview.h"
#include <vector>

namespace Ms {

class Score;
class ScoreView;
class Page;
class Timeline;
class ViewRect;

//---------------------------------------------------------
//   TDockWidget
//    modified QScrollArea for Timeline
//---------------------------------------------------------

class TDockWidget : public QDockWidget {
      Q_OBJECT

      QSplitter* _grid;

      virtual void closeEvent(QCloseEvent* event);

   signals:
      void closed(bool);

   public:
      TDockWidget(QWidget* w = 0);
      QSplitter* grid() { return _grid; }
      };

//---------------------------------------------------------
//   TRowLabels
//---------------------------------------------------------

class TRowLabels : public QGraphicsView {
      Q_OBJECT

   public:
      enum class MouseOverValue {
            NONE,
            MOVE_UP_ARROW,
            MOVE_DOWN_ARROW,
            MOVE_UP_DOWN_ARROW,
            COLLAPSE_UP_ARROW,
            COLLAPSE_DOWN_ARROW,
            OPEN_EYE,
            CLOSED_EYE
            };

   private:
      TDockWidget* _scrollArea { nullptr };
      Timeline* parent { nullptr };

      QPoint _oldLoc;

      bool _dragging = false;

      std::vector<std::pair<QGraphicsItem*, int>> _metaLabels;
      std::map<MouseOverValue, QPixmap*> _mouseoverMap;
      std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned> _oldItemInfo;

      virtual void resizeEvent(QResizeEvent*);
      virtual void mousePressEvent(QMouseEvent* event);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void mouseReleaseEvent(QMouseEvent* event);
      virtual void contextMenuEvent(QContextMenuEvent*) override;
      virtual void leaveEvent(QEvent*);

   private slots:
      void restrictScroll(int value);

   public slots:
      void mouseOver(QPointF scenePt);

   signals:
      void moved(QPointF p);
      void swapMeta(unsigned r, bool up);
      void requestContextMenu(QContextMenuEvent*);

   public:
      TRowLabels(TDockWidget* dockWidget, Timeline* time, QGraphicsView* w = 0);
      void updateLabels(std::vector<std::pair<QString, bool>> labels, int height);
      QString cursorIsOn();
      };

//---------------------------------------------------------
//   TimelineTheme
//---------------------------------------------------------

struct TimelineTheme {
      QColor backgroundColor, labelsColor1, labelsColor2, labelsColor3, gridColor1, gridColor2;
      QColor measureMetaColor, selectionColor, nonVisiblePenColor, nonVisibleBrushColor, colorBoxColor;
      QColor metaValuePenColor, metaValueBrushColor;
      };

//---------------------------------------------------------
//   Timeline
//---------------------------------------------------------

class Timeline : public QGraphicsView {
      Q_OBJECT

   public:
      enum class ItemType {
            TYPE_UNKNOWN = 0,
            TYPE_MEASURE,
            TYPE_META,
            };
      Q_ENUM(ItemType);

   private:
      static constexpr int keyItemType = 15;

      int _gridWidth = 20;
      int _gridHeight = 20;
      int _maxZoom = 50;
      int _minZoom = 5;
      int _spacing = 5;

      TimelineTheme _lightTheme, _darkTheme;

      std::tuple<int, qreal, Element*, Element*, bool> _repeatInfo;
      std::tuple<QGraphicsItem*, int, QColor> _oldHoverInfo;

      std::map<BarLineType, QPixmap*> _barlines;
      bool _isBarline { false };

      TDockWidget* _scrollArea { nullptr };
      TRowLabels* _rowNames { nullptr };

      Score* _score { nullptr };
      ScoreView* _cv { nullptr };

      int gridRows = 0;
      int gridCols = 0;

      QGraphicsPathItem* nonVisiblePathItem = nullptr;
      QGraphicsPathItem* visiblePathItem = nullptr;
      QGraphicsPathItem* selectionItem = nullptr;

      QGraphicsRectItem* _selectionBox { nullptr };
      std::vector<std::pair<QGraphicsItem*, int>> _metaRows;

      QPainterPath _selectionPath;
      QRectF _oldSelectionRect;
      bool _mousePressed { false };
      QPoint _oldLoc;

      bool _collapsedMeta { false };

      std::vector<std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool>> _metas;
      void tempoMeta(Segment* seg, int* stagger, int pos);
      void timeMeta(Segment* seg, int* stagger, int pos);
      void measureMeta(Segment*, int*, int pos);
      void rehearsalMeta(Segment* seg, int* stagger, int pos);
      void keyMeta(Segment* seg, int* stagger, int pos);
      void barlineMeta(Segment* seg, int* stagger, int pos);
      void jumpMarkerMeta(Segment* seg, int* stagger, int pos);

      bool addMetaValue(int x, int pos, QString metaText, int row, ElementType elementType, Element* element, Segment* seg, Measure* measure, QString tooltip = "");
      void setMetaData(QGraphicsItem* gi, int staff, ElementType et, Measure* m, bool full_measure, Element* e, QGraphicsItem* pairItem = nullptr, Segment* seg = nullptr);
      unsigned getMetaRow(QString targetText);

      int _globalMeasureNumber { 0 };
      int _globalZValue        { 0 };

      // True if meta value was last clicked
      bool _metaValue = false;
      ViewState state = ViewState::NORMAL;

      virtual void mousePressEvent(QMouseEvent* event);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void mouseReleaseEvent(QMouseEvent*);
      virtual void wheelEvent(QWheelEvent *event);
      virtual void leaveEvent(QEvent*);
      void showEvent(QShowEvent*) override;

      unsigned correctMetaRow(unsigned row);
      int correctStave(int stave);

      QList<Part*> getParts();

      QRectF getMeasureRect(int measureIndex, int row, int numMetas) { return QRectF(measureIndex * _gridWidth, _gridHeight * (row + numMetas) + 3, _gridWidth, _gridHeight); }
      void clearScene();

      void updateGrid(int startMeasure = -1, int endMeasure = -1);

   private slots:
      void handleScroll(int value);
      void updateView();
      void objectDestroyed(QObject*);

   public slots:
      void changeSelection(SelState);
      void mouseOver(QPointF pos);
      void swapMeta(unsigned row, bool switchUp);
      virtual void contextMenuEvent(QContextMenuEvent* event) override;
      void requestInstrumentDialog();
      void toggleMetaRow();
      void updateTimelineTheme();

   signals:
      void moved(QPointF);

   public:
      Timeline(TDockWidget* dockWidget, QWidget* parent = nullptr);
      int correctPart(int stave);

      void drawSelection();
      void drawGrid(int globalRows, int globalCols, int startMeasure = 0, int endMeasure = -1);

      void setScore(Score* s);
      void setScoreView(ScoreView* sv);

      int nstaves() const;

      int getWidth() const;
      int getHeight() const;
      const TimelineTheme& activeTheme() const;

      void updateGridFull() { updateGrid(0, -1); }
      void updateGridView() { updateGrid(-1, -1); }
      void updateGridFromCmdState();

      QColor colorBox(QGraphicsRectItem* item);

      std::vector<std::pair<QString, bool>> getLabels();

      unsigned nmetas() const;

      bool collapsed() { return _collapsedMeta; }
      void setCollapsed(bool st) { _collapsedMeta = st; }

      Staff* numToStaff(int staff);
      void toggleShow(int staff);
      QString cursorIsOn();
      };


} // namespace Ms

Q_DECLARE_METATYPE(Ms::TRowLabels::MouseOverValue);

#endif
