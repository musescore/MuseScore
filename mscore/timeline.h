//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: timeline.h 4785 2011-09-14 10:06:35Z wschweer $
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

      virtual void closeEvent(QCloseEvent *event);

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

      TDockWidget* scrollArea;
      Timeline* parent;

      QPoint old_loc;

      bool dragging = false;

      std::vector<std::pair<QGraphicsItem*, int>> meta_labels;

      virtual void resizeEvent(QResizeEvent *);
      virtual void mousePressEvent(QMouseEvent*event);
      virtual void mouseMoveEvent(QMouseEvent *event);
      virtual void mouseReleaseEvent(QMouseEvent *);

   private slots:
      void restrict_scroll(int value);

   public:
      TRowLabels(TDockWidget* sa, Timeline* t, QGraphicsView* w = 0);
      void updateLabels(std::vector<QString> labels, int height);
      };

//---------------------------------------------------------
//   Timeline
//---------------------------------------------------------

class Timeline : public QGraphicsView {
      Q_OBJECT

      TDockWidget* scrollArea;
      TRowLabels* row_names;

      const unsigned int metas = 5;

      Score* _score;
      ScoreView* _cv = nullptr;

      QGraphicsRectItem* selection_box;
      std::vector<std::pair<QGraphicsItem*, int>> meta_rows;

      QPainterPath path;
      QRectF old_selection;
      bool mouse_pressed = false;
      QPoint old_loc;

      QGraphicsPathItem* v = nullptr;

      //True if meta value was last clicked
      bool meta_value = false;

      ViewState state = ViewState::NORMAL;

      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent *event);
      virtual void mouseReleaseEvent(QMouseEvent *);

      int correctStave(int stave);

   private slots:
      void handle_scroll(int value);
      void updateView(double, double);

   public slots:
      void changeSelection(SelState);

   public:
      Timeline(TDockWidget* sa, QWidget* parent = 0);
      int correctPart(int stave);

      void drawSelection();
      void drawGrid(int rows, int cols);

      void setScore(Score* s);
      void setScoreView(ScoreView* sv);

      int nstaves();

      int getWidth();
      int getHeight();

      void updateGrid();

      QColor colorBox(QGraphicsRectItem* item);

      std::vector<QString> getLabels();

      unsigned int nmetas() { return metas; }
      };


} // namespace Ms
#endif
