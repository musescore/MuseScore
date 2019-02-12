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
      TDockWidget* scrollArea;
      Timeline* parent;

      QPoint old_loc;

      bool dragging = false;

      std::vector<std::pair<QGraphicsItem*, int>> meta_labels;
      std::map<MouseOverValue, QPixmap*> mouseover_map;
      std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned int> old_item_info;

      virtual void resizeEvent(QResizeEvent*);
      virtual void mousePressEvent(QMouseEvent* event);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void mouseReleaseEvent(QMouseEvent* event);
      virtual void contextMenuEvent(QContextMenuEvent*) override;
      virtual void leaveEvent(QEvent*);

   private slots:
      void restrict_scroll(int value);

   public slots:
      void mouseOver(QPointF scene_pt);

   signals:
      void moved(QPointF p);
      void swapMeta(unsigned int r, bool up);
      void requestContextMenu(QContextMenuEvent*);

   public:
      TRowLabels(TDockWidget* dock_widget, Timeline* time, QGraphicsView* w = 0);
      void updateLabels(std::vector<std::pair<QString, bool>> labels, int height);
      QString cursorIsOn();
      };

//---------------------------------------------------------
//   Timeline
//---------------------------------------------------------

class Timeline : public QGraphicsView {
      Q_OBJECT

      int grid_width = 20;
      int grid_height = 20;
      int max_zoom = 50;
      int min_zoom = 5;
      int spacing = 5;

      std::tuple<int, qreal, Element*, Element*, bool> repeat_info;
      std::tuple<QGraphicsItem*, int, QColor> old_hover_info;

      std::map<QString, QPixmap*> barlines;
      bool is_barline = false;

      TDockWidget* scrollArea;
      TRowLabels* row_names;

      Score* _score;
      ScoreView* _cv = nullptr;

      QGraphicsRectItem* selection_box;
      std::vector<std::pair<QGraphicsItem*, int>> meta_rows;

      QPainterPath selection_path;
      QRectF old_selection_rect;
      bool mouse_pressed = false;
      QPoint old_loc;

      bool collapsed_meta = false;

      std::vector<std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool>> metas;
      void tempo_meta(Segment* seg, int* stagger, int pos);
      void time_meta(Segment* seg, int* stagger, int pos);
      void measure_meta(Segment*, int*, int pos);
      void rehearsal_meta(Segment* seg, int* stagger, int pos);
      void key_meta(Segment* seg, int* stagger, int pos);
      void barline_meta(Segment* seg, int* stagger, int pos);
      void jump_marker_meta(Segment* seg, int* stagger, int pos);

      bool addMetaValue(int x, int pos, QString meta_text, int row, ElementType element_type, Element* element, Segment* seg, Measure* measure, QString tooltip = "");
      void setMetaData(QGraphicsItem* gi, int staff, ElementType et, Measure* m, bool full_measure, Element* e, QGraphicsItem* pair_item = nullptr, Segment* seg = nullptr);
      unsigned int getMetaRow(QString target_text);

      int global_measure_number { 0 };
      int global_z_value        { 0 };

      //True if meta value was last clicked
      bool meta_value = false;
      ViewState state = ViewState::NORMAL;

      virtual void mousePressEvent(QMouseEvent* event);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void mouseReleaseEvent(QMouseEvent*);
      virtual void wheelEvent(QWheelEvent *event);
      virtual void leaveEvent(QEvent*);

      unsigned int correctMetaRow(unsigned int row);
      int correctStave(int stave);

      QList<Part*> getParts();

   private slots:
      void handle_scroll(int value);
      void updateView();
      void objectDestroyed(QObject*);

   public slots:
      void changeSelection(SelState);
      void mouseOver(QPointF pos);
      void swapMeta(unsigned int row, bool switch_up);
      virtual void contextMenuEvent(QContextMenuEvent* event) override;
      void requestInstrumentDialog();
      void toggleMetaRow();

   signals:
      void moved(QPointF);

   public:
      Timeline(TDockWidget* dock_widget, QWidget* parent = 0);
      int correctPart(int stave);

      void drawSelection();
      void drawGrid(int global_rows, int global_cols);

      void setScore(Score* s);
      void setScoreView(ScoreView* sv);

      int nstaves();

      int getWidth();
      int getHeight();

      void updateGrid();

      QColor colorBox(QGraphicsRectItem* item);

      std::vector<std::pair<QString, bool>> getLabels();

      unsigned int nmetas();

      bool collapsed() { return collapsed_meta; }
      void setCollapsed(bool st) { collapsed_meta = st; }

      Staff* numToStaff(int staff);
      void toggleShow(int staff);
      QString cursorIsOn();
      };


} // namespace Ms

Q_DECLARE_METATYPE(Ms::TRowLabels::MouseOverValue);

#endif
