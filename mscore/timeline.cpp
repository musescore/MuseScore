//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "timeline.h"
#include "navigator.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/page.h"
#include "preferences.h"
#include "libmscore/mscore.h"
#include "libmscore/system.h"
#include "libmscore/measurebase.h"
#include "libmscore/measure.h"
#include "libmscore/chord.h"
#include "libmscore/staff.h"
#include "libmscore/rest.h"
#include "libmscore/part.h"
#include "libmscore/tempo.h"
#include "libmscore/keysig.h"
#include "libmscore/timesig.h"
#include "libmscore/key.h"
#include "libmscore/tempotext.h"
#include "libmscore/text.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/barline.h"
#include "libmscore/jump.h"
#include "libmscore/marker.h"
#include "texttools.h"
#include "mixer.h"
#include "tourhandler.h"

namespace Ms {

//---------------------------------------------------------
//   showTimeline
//---------------------------------------------------------

void MuseScore::showTimeline(bool visible)
      {
      QSplitter* split = static_cast<QSplitter*>(_timeline->widget());
      Timeline* time = static_cast<Timeline*>(split->widget(1));

      QAction* act = getAction("toggle-timeline");
      if (!time) {
            time = new Timeline(_timeline);
            time->setScore(0);
            time->setScoreView(cv);
            }
      connect(_timeline, SIGNAL(visibilityChanged(bool)), act, SLOT(setChecked(bool)));
      connect(_timeline, SIGNAL(closed(bool)), act, SLOT(setChecked(bool)));
      _timeline->setVisible(visible);

      getAction("toggle-timeline")->setChecked(visible);
      if (visible)
            TourHandler::startTour("timeline-tour");
      }

//---------------------------------------------------------
//   TDockWidget
//---------------------------------------------------------

TDockWidget::TDockWidget(QWidget* w)
   : QDockWidget(w)
      {
      setFocusPolicy(Qt::NoFocus);
      _grid = new QSplitter();
      setWidget(_grid);
      _grid->setHandleWidth(0);
      setAllowedAreas(Qt::DockWidgetAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea));
      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

      setWindowTitle(tr("Timeline"));
      setObjectName("Timeline");
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void TDockWidget::closeEvent(QCloseEvent* event)
      {
      emit closed(false);
      QWidget::closeEvent(event);
      }

//---------------------------------------------------------
//   TRowLabels
//---------------------------------------------------------

TRowLabels::TRowLabels(TDockWidget* dock_widget, Timeline* time, QGraphicsView* w)
  : QGraphicsView(w)
      {
      setFocusPolicy(Qt::NoFocus);
      scrollArea = dock_widget;
      parent = time;
      setScene(new QGraphicsScene);
      scene()->setBackgroundBrush(Qt::lightGray);
      setSceneRect(0, 0, 50, time->height());

      setMinimumWidth(0);

      QSplitter* split = scrollArea->grid();
      QList<int> sizes;
      //TODO: Replace 70 with hard coded value
      sizes << 70 << 10000;
      split->setSizes(sizes);

      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      setContentsMargins(0, 0, 0, 0);
      setAlignment(Qt::Alignment((Qt::AlignLeft | Qt::AlignTop)));

      connect(verticalScrollBar(), SIGNAL(valueChanged(int)), time->verticalScrollBar(), SLOT(setValue(int)));
      connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(restrict_scroll(int)));
      connect(this, SIGNAL(moved(QPointF)), time, SLOT(mouseOver(QPointF)));

      static const char* ud_arrow[] = {
                  "10 18 2 1",
                  "# c #000000",
                  ". c #d3d3d3",
                  "..........",
                  "..........",
                  "....##....",
                  "...####...",
                  "..##..##..",
                  "..#....#..",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..#....#..",
                  "..##..##..",
                  "...####...",
                  "....##....",
                  "..........",
                  ".........."
                  };

      static const char* u_arrow[] = {
                  "10 18 2 1",
                  "# c #000000",
                  ". c #d3d3d3",
                  "..........",
                  "..........",
                  "....##....",
                  "...####...",
                  "..##..##..",
                  "..#....#..",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  ".........."
                  };

      static const char* d_arrow[] = {
                  "10 18 2 1",
                  "# c #000000",
                  ". c #d3d3d3",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..........",
                  "..#....#..",
                  "..##..##..",
                  "...####...",
                  "....##....",
                  "..........",
                  ".........."
                  };

      static const char* cu_arrow[] = {
                  "9 18 2 1",
                  "# c #000000",
                  ". c #d3d3d3",
                  ".........",
                  ".........",
                  ".........",
                  ".........",
                  "....#....",
                  "...###...",
                  "..#.#.#..",
                  ".#..#..#.",
                  "....#....",
                  "....#....",
                  "....#....",
                  "....#....",
                  ".........",
                  ".........",
                  ".........",
                  ".........",
                  ".........",
                  "........."
                  };

      static const char* cd_arrow[] = {
                  "9 18 2 1",
                  "# c #000000",
                  ". c #d3d3d3",
                  ".........",
                  ".........",
                  ".........",
                  ".........",
                  ".........",
                  "....#....",
                  "....#....",
                  "....#....",
                  "....#....",
                  "....#....",
                  ".#..#..#.",
                  "..#.#.#..",
                  "...###...",
                  "....#....",
                  ".........",
                  ".........",
                  ".........",
                  "........."
                  };

      static const char* open_eye[] = {
                  "11 18 2 1",
                  "# c #000000",
                  ". c #d3d3d3",
                  "...........",
                  "...........",
                  "...........",
                  "...........",
                  "...####....",
                  ".##....##..",
                  ".#......#..",
                  "#........#.",
                  "#...##...#.",
                  "#...##...#.",
                  "#........#.",
                  ".#......#..",
                  ".##....##..",
                  "...####....",
                  "...........",
                  "...........",
                  "...........",
                  "..........."
                  };

      static const char* closed_eye[] = {
                  "11 18 2 1",
                  "# c #000000",
                  ". c #d3d3d3",
                  "...........",
                  "...........",
                  "...........",
                  "...........",
                  "...........",
                  "...........",
                  "...........",
                  "..######...",
                  "##..##..##.",
                  "##..##..##.",
                  "..######...",
                  "...........",
                  "...........",
                  "...........",
                  "...........",
                  "...........",
                  "...........",
                  "..........."
                  };


      mouseover_map[MouseOverValue::COLLAPSE_DOWN_ARROW] = new QPixmap(cd_arrow);
      mouseover_map[MouseOverValue::COLLAPSE_UP_ARROW] = new QPixmap(cu_arrow);
      mouseover_map[MouseOverValue::MOVE_DOWN_ARROW] = new QPixmap(d_arrow);
      mouseover_map[MouseOverValue::MOVE_UP_DOWN_ARROW] = new QPixmap(ud_arrow);
      mouseover_map[MouseOverValue::MOVE_UP_ARROW] = new QPixmap(u_arrow);
      mouseover_map[MouseOverValue::OPEN_EYE] = new QPixmap(open_eye);
      mouseover_map[MouseOverValue::CLOSED_EYE] = new QPixmap(closed_eye);

      std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned int> tmp(nullptr, MouseOverValue::NONE, -1);
      old_item_info = tmp;

      connect(this, SIGNAL(requestContextMenu(QContextMenuEvent*)), parent, SLOT(contextMenuEvent(QContextMenuEvent*)));
      }

//---------------------------------------------------------
//   restrict_scroll
//---------------------------------------------------------

void TRowLabels::restrict_scroll(int value)
      {
      if (value > parent->verticalScrollBar()->maximum())
            verticalScrollBar()->setValue(parent->verticalScrollBar()->maximum());
      for (std::vector<std::pair<QGraphicsItem*, int>>::iterator it = meta_labels.begin();
           it != meta_labels.end(); ++it) {
            std::pair<QGraphicsItem*, int> pair_graphic_int = *it;

            QGraphicsItem* graphics_item = pair_graphic_int.first;

            QGraphicsRectItem* graphics_rect_item = qgraphicsitem_cast<QGraphicsRectItem*>(graphics_item);
            QGraphicsLineItem* graphics_line_item = qgraphicsitem_cast<QGraphicsLineItem*>(graphics_item);
            QGraphicsPixmapItem* graphics_pixmap_item = qgraphicsitem_cast<QGraphicsPixmapItem*>(graphics_item);
            int y = pair_graphic_int.second * 20;
            int scrollbar_value = verticalScrollBar()->value();

            if (graphics_rect_item) {
                  QRectF rectf = graphics_rect_item->rect();
                  rectf.setY(qreal(scrollbar_value + y));
                  rectf.setHeight(20);
                  graphics_rect_item->setRect(rectf);
                  }
            else if (graphics_line_item) {
                  QLineF linef = graphics_line_item->line();
                  linef.setLine(linef.x1(), y + scrollbar_value + 1, linef.x2(), y + scrollbar_value + 1);
                  graphics_line_item->setLine(linef);
                  }
            else if (graphics_pixmap_item)
                  graphics_pixmap_item->setY(qreal(scrollbar_value + y + 1));
            else
                  graphics_item->setY(qreal(scrollbar_value + y));
            }
      viewport()->update();

      }

//---------------------------------------------------------
//   updateLabels
//---------------------------------------------------------

void TRowLabels::updateLabels(std::vector<std::pair<QString, bool>> labels, int height)
      {

      scene()->clear();
      meta_labels.clear();
      if (labels.empty())
            return;

      unsigned int num_metas = parent->nmetas();
      int max_width = -1;
      int measure_width = 0;
      for (unsigned int row = 0; row < labels.size(); row++) {
            //Draw instrument name rectangle
            int ypos = (row < num_metas)? row * height + verticalScrollBar()->value() : row * height + 3;
            QGraphicsRectItem* graphics_rect_item = new QGraphicsRectItem(0, ypos, width(), height);
            QGraphicsTextItem* graphics_text_item = new QGraphicsTextItem(labels[row].first);

            if (row == num_metas - 1)
                  measure_width = graphics_text_item->boundingRect().width();
            max_width = max(max_width, int(graphics_text_item->boundingRect().width()));

            QFontMetrics f(QApplication::font());
            QString part_name = f.elidedText(labels[row].first, Qt::ElideRight, width());
            graphics_text_item->setPlainText(part_name);
            graphics_text_item->setX(0);
            graphics_text_item->setY(ypos);
            if (labels[row].second)
                  graphics_text_item->setDefaultTextColor(QColor(Qt::black));
            else
                  graphics_text_item->setDefaultTextColor(QColor(150, 150, 150));
            graphics_rect_item->setPen(QPen(QColor(150, 150, 150)));
            graphics_rect_item->setBrush(QBrush(QColor(211, 211, 211)));
            graphics_text_item->setZValue(-1);
            graphics_rect_item->setZValue(-1);

            graphics_rect_item->setData(0, QVariant::fromValue<bool>(false));
            graphics_text_item->setData(0, QVariant::fromValue<bool>(false));

            MouseOverValue mouse_over_arrow = MouseOverValue::NONE;
            if (num_metas - 1 == row && (num_metas > 2 || parent->collapsed())) {
                  //Measures meta
                  if (parent->collapsed())
                        mouse_over_arrow = MouseOverValue::COLLAPSE_DOWN_ARROW;
                  else
                        mouse_over_arrow = MouseOverValue::COLLAPSE_UP_ARROW;
                  }
            else if (row < num_metas - 1) {
                  if (row != 0 && row + 1 <= num_metas - 2)
                        mouse_over_arrow = MouseOverValue::MOVE_UP_DOWN_ARROW;
                  else if (row == 0 && row + 1 < num_metas - 1)
                        mouse_over_arrow = MouseOverValue::MOVE_DOWN_ARROW;
                  else if (row == num_metas - 2 && row != 0)
                        mouse_over_arrow = MouseOverValue::MOVE_UP_ARROW;
                  }
            else if (num_metas <= row) {
                  if (parent->numToStaff(row - num_metas) &&
                      parent->numToStaff(row - num_metas)->show())
                        mouse_over_arrow = MouseOverValue::OPEN_EYE;
                  else
                        mouse_over_arrow = MouseOverValue::CLOSED_EYE;
                  }
            graphics_text_item->setData(1, QVariant::fromValue<MouseOverValue>(mouse_over_arrow));
            graphics_rect_item->setData(1, QVariant::fromValue<MouseOverValue>(mouse_over_arrow));

            graphics_text_item->setData(2, QVariant::fromValue<unsigned int>(row));
            graphics_rect_item->setData(2, QVariant::fromValue<unsigned int>(row));

            scene()->addItem(graphics_rect_item);
            scene()->addItem(graphics_text_item);
            if (row < num_metas) {
                  std::pair<QGraphicsItem*, int> p1(graphics_rect_item, row);
                  std::pair<QGraphicsItem*, int> p2(graphics_text_item, row);
                  meta_labels.push_back(p1);
                  meta_labels.push_back(p2);
                  graphics_rect_item->setZValue(1);
                  graphics_text_item->setZValue(2);
                  }
            }
      QGraphicsLineItem* graphics_line_item = new QGraphicsLineItem(0,
                                                    height * num_metas + verticalScrollBar()->value() + 1,
                                                    max(max_width + 20, 70),
                                                    height * num_metas + verticalScrollBar()->value() + 1);
      graphics_line_item->setPen(QPen(QColor(150, 150, 150), 4));
      graphics_line_item->setZValue(0);
      graphics_line_item->setData(0, QVariant::fromValue<bool>(false));
      scene()->addItem(graphics_line_item);

      std::pair<QGraphicsItem*, int> graphics_line_item_pair(graphics_line_item, num_metas);
      meta_labels.push_back(graphics_line_item_pair);

      setSceneRect(0, 0, max_width, parent->getHeight() + parent->horizontalScrollBar()->height());

      std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned int> tmp(nullptr, MouseOverValue::NONE, -1);
      old_item_info = tmp;

      setMinimumWidth(measure_width + 9);
      setMaximumWidth(max(max_width + 20, 70));
      mouseOver(mapToScene(mapFromGlobal(QCursor::pos())));
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void TRowLabels::resizeEvent(QResizeEvent*)
      {
      std::vector<std::pair<QString, bool>> labels = parent->getLabels();
      updateLabels(labels, 20);
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void TRowLabels::mousePressEvent(QMouseEvent* event)
      {
      if (event->button() == Qt::RightButton)
            return;

      QPointF scene_pt = mapToScene(event->pos());
      unsigned int num_metas = parent->nmetas();

      //Check if mouse position in scene is on the last meta
      QPointF measure_meta_tl = QPointF(0, (num_metas - 1) * 20 + verticalScrollBar()->value());
      QPointF measure_meta_br = QPointF(width(), num_metas * 20 + verticalScrollBar()->value());
      if (QRectF(measure_meta_tl, measure_meta_br).contains(scene_pt) && (num_metas > 2 || parent->collapsed())) {
            if (std::get<0>(old_item_info)) {
                  std::pair<QGraphicsItem*, int> p(std::get<0>(old_item_info), std::get<2>(old_item_info));
                  std::vector<std::pair<QGraphicsItem*, int>>::iterator it = std::find(meta_labels.begin(), meta_labels.end(), p);
                  if (it != meta_labels.end())
                        meta_labels.erase(it);
                  scene()->removeItem(std::get<0>(old_item_info));
                  }
            std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned int> tmp(nullptr, MouseOverValue::NONE, -1);
            old_item_info = tmp;
            mouseOver(mapToScene(mapFromGlobal(QCursor::pos())));

            parent->setCollapsed(!parent->collapsed());
            parent->updateGrid();
            }
      else {
            //Check if pixmap was selected
            if (QGraphicsItem* graphics_item = scene()->itemAt(scene_pt, transform())) {
                  QGraphicsPixmapItem* graphics_pixmap_item = qgraphicsitem_cast<QGraphicsPixmapItem*>(graphics_item);
                  if (graphics_pixmap_item) {
                        unsigned int row = graphics_pixmap_item->data(2).value<unsigned int>();
                        if (row == num_metas - 1)
                              return;
                        else if (row < num_metas - 1) {
                              //Find mid point between up and down arrow
                              qreal mid_point = graphics_pixmap_item->boundingRect().height() / 2 + graphics_pixmap_item->scenePos().y();
                              if (scene_pt.y() > mid_point)
                                    emit swapMeta(row, false);
                              else
                                    emit swapMeta(row, true);
                              }
                        else if (row >= num_metas)
                              parent->toggleShow(row - num_metas);
                        }
                  else {
                        dragging = true;
                        this->setCursor(Qt::SizeAllCursor);
                        old_loc = QPoint(int(scene_pt.x()), int(scene_pt.y()));
                        }

                  }
            else {
                  dragging = true;
                  this->setCursor(Qt::SizeAllCursor);
                  old_loc = QPoint(int(scene_pt.x()), int(scene_pt.y()));
                  }
            }
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void TRowLabels::mouseMoveEvent(QMouseEvent* event)
      {
      QPointF scene_pt = mapToScene(event->pos());
      if (dragging) {
            this->setCursor(Qt::SizeAllCursor);
            int y_offset = int(old_loc.y()) - int(scene_pt.y());
            verticalScrollBar()->setValue(verticalScrollBar()->value() + y_offset);
            }
      else
            mouseOver(scene_pt);
      emit moved(QPointF(-1, -1));
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void TRowLabels::mouseReleaseEvent(QMouseEvent* event)
      {
      if (QGraphicsItem* graphics_item = scene()->itemAt(mapToScene(event->pos()), transform())) {
                  QGraphicsPixmapItem* graphics_pixmap_item = qgraphicsitem_cast<QGraphicsPixmapItem*>(graphics_item);
                  if (graphics_pixmap_item)
                        this->setCursor(Qt::PointingHandCursor);
                  else
                        this->setCursor(Qt::ArrowCursor);
            }
      else
            this->setCursor(Qt::ArrowCursor);
      dragging = false;
      }

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void TRowLabels::leaveEvent(QEvent*)
      {
      if (!rect().contains(mapFromGlobal(QCursor::pos())))
            mouseOver(mapToScene(mapFromGlobal(QCursor::pos())));
      }
//---------------------------------------------------------
//   contextMenuEvent
//---------------------------------------------------------

void TRowLabels::contextMenuEvent(QContextMenuEvent* event)
      {
      emit requestContextMenu(event);
      }

//---------------------------------------------------------
//   mouseOver
//---------------------------------------------------------

void TRowLabels::mouseOver(QPointF scene_pt)
      {
      //Handle drawing of arrows
      if (QGraphicsItem* graphics_item = scene()->itemAt(scene_pt, transform())) {
            QGraphicsPixmapItem* graphics_pixmap_item = qgraphicsitem_cast<QGraphicsPixmapItem*>(graphics_item);
            if (graphics_pixmap_item) {
                  this->setCursor(Qt::PointingHandCursor);
                  return;
                  }

            MouseOverValue mouse_over_arrow = graphics_item->data(1).value<MouseOverValue>();
            if (mouse_over_arrow != MouseOverValue::NONE) {
                  QPixmap* pixmap_arrow = mouseover_map[mouse_over_arrow];
                  QGraphicsPixmapItem* graphics_pixmap_item_arrow = new QGraphicsPixmapItem(*pixmap_arrow);
                  unsigned int row = graphics_item->data(2).value<unsigned int>();

                  QString tooltip;
                  switch (mouse_over_arrow) {
                        case MouseOverValue::COLLAPSE_DOWN_ARROW:
                              tooltip = tr("Expand meta rows");
                              break;
                        case MouseOverValue::COLLAPSE_UP_ARROW:
                              tooltip = tr("Collapse meta rows");
                              break;
                        case MouseOverValue::MOVE_DOWN_ARROW:
                              tooltip = tr("Move meta row down one");
                              break;
                        case MouseOverValue::MOVE_UP_ARROW:
                              tooltip = tr("Move meta row up one");
                              break;
                        case MouseOverValue::MOVE_UP_DOWN_ARROW:
                              tooltip = tr("Move meta row up/down one");
                              break;
                        case MouseOverValue::OPEN_EYE:
                              tooltip = tr("Hide instrument in score");
                              break;
                        case MouseOverValue::CLOSED_EYE:
                              tooltip = tr("Show instrument in score");
                              break;
                        default:
                              tooltip = "";
                              break;
                        }

                  graphics_pixmap_item_arrow->setToolTip(tooltip);
                  if (mouse_over_arrow == MouseOverValue::OPEN_EYE || mouse_over_arrow == MouseOverValue::CLOSED_EYE)
                        graphics_pixmap_item_arrow->setData(0, QVariant::fromValue<bool>(false));
                  else
                        graphics_pixmap_item_arrow->setData(0, QVariant::fromValue<bool>(true));
                  graphics_pixmap_item_arrow->setData(1, QVariant::fromValue<MouseOverValue>(mouse_over_arrow));
                  graphics_pixmap_item_arrow->setData(2, QVariant::fromValue<unsigned int>(row));

                  //Draw arrow at correct location
                  if (row < parent->nmetas()) {
                        graphics_pixmap_item_arrow->setPos(width() - 12, verticalScrollBar()->value() + 1 + row * 20);
                        graphics_pixmap_item_arrow->setZValue(3);
                        }
                  else {
                        graphics_pixmap_item_arrow->setPos(width() - 13, row * 20 + 5);
                        graphics_pixmap_item_arrow->setZValue(-1);
                        }


                  if (std::get<2>(old_item_info) == row && std::get<1>(old_item_info) == mouse_over_arrow) {
                        //DO NOTHING
                        }
                  else {
                        if (std::get<0>(old_item_info)) {
                              std::pair<QGraphicsItem*, int> p(std::get<0>(old_item_info), std::get<2>(old_item_info));
                              std::vector<std::pair<QGraphicsItem*, int>>::iterator it = std::find(meta_labels.begin(), meta_labels.end(), p);
                              if (it != meta_labels.end())
                                    meta_labels.erase(it);
                              scene()->removeItem(std::get<0>(old_item_info));
                              }
                        std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned int> tmp(graphics_pixmap_item_arrow, mouse_over_arrow, row);
                        old_item_info = tmp;
                        if (mouse_over_arrow != MouseOverValue::OPEN_EYE && mouse_over_arrow != MouseOverValue::CLOSED_EYE) {
                              std::pair<QGraphicsItem*, int> p(graphics_pixmap_item_arrow, row);
                              meta_labels.push_back(p);
                              }
                        scene()->addItem(graphics_pixmap_item_arrow);
                        }

                  }
            else {
                  if (std::get<0>(old_item_info))
                        scene()->removeItem(std::get<0>(old_item_info));
                  std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned int> tmp(nullptr, MouseOverValue::NONE, -1);
                  old_item_info = tmp;
                  }
            }
      else {
            if (std::get<0>(old_item_info)) {
                  std::pair<QGraphicsItem*, int> p(std::get<0>(old_item_info), std::get<2>(old_item_info));
                  std::vector<std::pair<QGraphicsItem*, int>>::iterator it = std::find(meta_labels.begin(), meta_labels.end(), p);
                  if (it != meta_labels.end())
                        meta_labels.erase(it);
                  scene()->removeItem(std::get<0>(old_item_info));
                  }
            std::tuple<QGraphicsPixmapItem*, MouseOverValue, unsigned int> tmp(nullptr, MouseOverValue::NONE, -1);
            old_item_info = tmp;
            }
      if (QGraphicsItem* graphics_item = scene()->itemAt(scene_pt, transform())) {
            QGraphicsPixmapItem* graphics_pixmap_item = qgraphicsitem_cast<QGraphicsPixmapItem*>(graphics_item);
            if (graphics_pixmap_item)
                  this->setCursor(Qt::PointingHandCursor);
            else
                  this->setCursor(Qt::ArrowCursor);
            }
      else
            this->setCursor(Qt::ArrowCursor);
      }

//---------------------------------------------------------
//   cursorIsOn
//---------------------------------------------------------

QString TRowLabels::cursorIsOn()
      {
      QPointF scene_pos = mapToScene(mapFromGlobal(QCursor::pos()));
      QGraphicsItem* graphics_item = scene()->itemAt(scene_pos, transform());
      if (graphics_item) {
            auto it = meta_labels.begin();
            for (;it != meta_labels.end(); ++it) {
                  if ((*it).first == graphics_item)
                        break;
                  }
            if (it != meta_labels.end())
                  return "meta";
            else
                  return "instrument";
            }
      else
            return "";
      }

//---------------------------------------------------------
//   Timeline
//---------------------------------------------------------

Timeline::Timeline(TDockWidget* dock_widget, QWidget* parent)
  : QGraphicsView(parent)
      {
      setFocusPolicy(Qt::NoFocus);
      setAlignment(Qt::Alignment((Qt::AlignLeft | Qt::AlignTop)));
      setAttribute(Qt::WA_NoBackground);

      scrollArea = dock_widget;
      QSplitter* split = static_cast<QSplitter*>(scrollArea->widget());

      row_names = new TRowLabels(dock_widget, this);
      split->addWidget(row_names);
      split->addWidget(this);
      split->setChildrenCollapsible(false);
      split->setStretchFactor(0, 0);
      split->setStretchFactor(1, 0);

      setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

      setScene(new QGraphicsScene);
      setSceneRect(0, 0, 100, 100);
      scene()->setBackgroundBrush(Qt::lightGray);

      connect(verticalScrollBar(),SIGNAL(valueChanged(int)),row_names->verticalScrollBar(),SLOT(setValue(int)));
      connect(verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(handle_scroll(int)));
      connect(row_names, SIGNAL(swapMeta(uint,bool)), this, SLOT(swapMeta(uint,bool)));
      connect(this, SIGNAL(moved(QPointF)), row_names, SLOT(mouseOver(QPointF)));

      std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> t1(tr("Tempo"), &Ms::Timeline::tempo_meta, true);
      std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> t2(tr("Time Signature"), &Ms::Timeline::time_meta, true);
      std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> t3(tr("Rehearsal Mark"), &Ms::Timeline::rehearsal_meta, true);
      std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> t4(tr("Key Signature"), &Ms::Timeline::key_meta, true);
      std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> t5(tr("Barlines"), &Ms::Timeline::barline_meta, true);
      std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> t6(tr("Jumps and Markers"), &Ms::Timeline::jump_marker_meta, true);
      std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> t7(tr("Measures"), &Ms::Timeline::measure_meta, true);
      metas.push_back(t1);
      metas.push_back(t2);
      metas.push_back(t3);
      metas.push_back(t4);
      metas.push_back(t5);
      metas.push_back(t6);
      metas.push_back(t7);

      std::tuple<QGraphicsItem*, int, QColor> ohi(nullptr, -1, QColor());
      old_hover_info = ohi;
      std::tuple<int, qreal, Element*, Element*, bool> ri(0, 0, nullptr, nullptr, false);
      repeat_info = ri;

      static const char* left_repeat[] = {
                  "7 14 2 1",
                  "# c #000000",
                  ". c None",
                  "##.#...",
                  "##.#...",
                  "##.#...",
                  "##.#...",
                  "##.#.##",
                  "##.#.##",
                  "##.#...",
                  "##.#...",
                  "##.#.##",
                  "##.#.##",
                  "##.#...",
                  "##.#...",
                  "##.#...",
                  "##.#..."
                  };

      static const char* right_repeat[] = {
                  "7 14 2 1",
                  "# c #000000",
                  ". c None",
                  "...#.##",
                  "...#.##",
                  "...#.##",
                  "...#.##",
                  "##.#.##",
                  "##.#.##",
                  "...#.##",
                  "...#.##",
                  "##.#.##",
                  "##.#.##",
                  "...#.##",
                  "...#.##",
                  "...#.##",
                  "...#.##"
                  };

      static const char* final_barline[] = {
                  "7 14 2 1",
                  "# c #000000",
                  ". c None",
                  "...#.##",
                  "...#.##",
                  "...#.##",
                  "...#.##",
                  "...#.##",
                  "...#.##",
                  "...#.##",
                  "...#.##",
                  "...#.##",
                  "...#.##",
                  "...#.##",
                  "...#.##",
                  "...#.##",
                  "...#.##"
                  };

      static const char* double_barline[] = {
                  "7 14 2 1",
                  "# c #000000",
                  ". c None",
                  "..#.#..",
                  "..#.#..",
                  "..#.#..",
                  "..#.#..",
                  "..#.#..",
                  "..#.#..",
                  "..#.#..",
                  "..#.#..",
                  "..#.#..",
                  "..#.#..",
                  "..#.#..",
                  "..#.#..",
                  "..#.#..",
                  "..#.#.."
                  };

      QPixmap* left_repeat_pixmap = new QPixmap(left_repeat);
      QPixmap* right_repeat_pixmap = new QPixmap(right_repeat);
      QPixmap* final_barline_pixmap = new QPixmap(final_barline);
      QPixmap* double_barline_pixmap = new QPixmap(double_barline);

      barlines["Start repeat"] = left_repeat_pixmap;
      barlines["End repeat"] = right_repeat_pixmap;
      barlines["Final barline"] = final_barline_pixmap;
      barlines["Double barline"] = double_barline_pixmap;
      }

//---------------------------------------------------------
//   drawGrid
//---------------------------------------------------------

void Timeline::drawGrid(int global_rows, int global_cols)
      {
      scene()->clear();
      meta_rows.clear();

      if (global_rows == 0 || global_cols == 0) return;
      int stagger = 0;
      unsigned int num_metas = nmetas();
      setMinimumHeight(grid_height * (num_metas + 1) + 5 + horizontalScrollBar()->height());
      setMinimumWidth(grid_width * 3);
      global_z_value = 1;

      //Draw grid
      Measure* curr_measure = _score->firstMeasure();
      QList<Part*> part_list = _score->parts();
      for (int col = 0; col < global_cols; col++) {
            for (int row = 0; row < global_rows; row++) {
                  QGraphicsRectItem* graphics_rect_item = new QGraphicsRectItem(col * grid_width,
                                                                                grid_height * (row + num_metas) + 3,
                                                                                grid_width,
                                                                                grid_height);

                  setMetaData(graphics_rect_item, row, ElementType::INVALID, curr_measure, false, 0);

                  QString translate_measure = tr("Measure");
                  QChar initial_letter = translate_measure[0];
                  QTextDocument doc;
                  QString part_name = "";
                  if (part_list.size() > row) {
                        doc.setHtml(part_list.at(row)->longName());
                        part_name = doc.toPlainText();
                        }
                  if (part_name.isEmpty() && part_list.size() > row)
                        part_name = part_list.at(row)->instrumentName();

                  graphics_rect_item->setToolTip(initial_letter + QString(" ") + QString::number(curr_measure->no() + 1) + QString(", ") + part_name);
                  graphics_rect_item->setPen(QPen(QColor(Qt::lightGray)));
                  graphics_rect_item->setBrush(QBrush(colorBox(graphics_rect_item)));
                  graphics_rect_item->setZValue(-3);
                  scene()->addItem(graphics_rect_item);
                  }

            curr_measure = curr_measure->nextMeasure();
            }
      setSceneRect(0, 0, getWidth(), getHeight());

      //Draw meta rows and separator
      QGraphicsLineItem* graphics_line_item_separator = new QGraphicsLineItem(0,
                                                                              grid_height * num_metas + verticalScrollBar()->value() + 1,
                                                                              getWidth() - 1,
                                                                              grid_height * num_metas + verticalScrollBar()->value() + 1);
      graphics_line_item_separator->setPen(QPen(QColor(150, 150, 150), 4));
      graphics_line_item_separator->setZValue(-2);
      scene()->addItem(graphics_line_item_separator);
      std::pair<QGraphicsItem*, int> pair_graphics_int_separator(graphics_line_item_separator, num_metas);
      meta_rows.push_back(pair_graphics_int_separator);

      for (unsigned int row = 0; row < num_metas; row++) {
            QGraphicsRectItem* meta_row = new QGraphicsRectItem(0,
                                                                grid_height * row + verticalScrollBar()->value(),
                                                                getWidth(),
                                                                grid_height);
            meta_row->setBrush(QBrush(QColor(211,211,211)));
            meta_row->setPen(QPen(QColor(150, 150, 150)));
            meta_row->setData(0, QVariant::fromValue<int>(-1));

            scene()->addItem(meta_row);

            std::pair<QGraphicsItem*, int> pair_graphics_int_meta(meta_row, row);
            meta_rows.push_back(pair_graphics_int_meta);
            }

      int x_pos = 0;

      //Create stagger array if collapsed_meta is false
#if (!defined (_MSCVER) && !defined (_MSC_VER))
      int stagger_arr[num_metas];
      for (unsigned int row = 0; row < num_metas; row++)
         stagger_arr[row] = 0;
#else
      // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
      //    heap allocation is slow, an optimization might be used.
      std::vector<int> stagger_arr(num_metas, 0);  // Default initialized, loop not required
#endif

      bool no_key = true;
      std::get<4>(repeat_info) = false;

      for (Measure* cm = _score->firstMeasure(); cm; cm = cm->nextMeasure()) {
            for (Segment* curr_seg = cm->first(); curr_seg; curr_seg = curr_seg->next()) {
                  //Toggle no_key if initial key signature is found
                  if (curr_seg->isKeySigType() && cm == _score->firstMeasure()) {
                        if (no_key && curr_seg->tick() == 0)
                              no_key = false;
                        }

                  //If no initial key signature is found, add key signature
                  if (cm == _score->firstMeasure() && no_key &&
                      (curr_seg->isTimeSigType() || curr_seg->isChordRestType())) {

                        if (getMetaRow(tr("Key Signature")) != num_metas) {
                              if (collapsed_meta)
                                    key_meta(0, &stagger, x_pos);
                              else
                                    key_meta(0, &stagger_arr[getMetaRow(tr("Key Signature"))], x_pos);
                              }
                        no_key = false;
                        }
                  int row = 0;
                  for (auto it = metas.begin(); it != metas.end(); ++it) {
                        std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> meta = *it;
                        if (!std::get<2>(meta))
                              continue;
                        void (Timeline::*func)(Segment*, int*, int) = std::get<1>(meta);
                        if (collapsed_meta)
                              (this->*func)(curr_seg, &stagger, x_pos);
                        else
                              (this->*func)(curr_seg, &stagger_arr[row], x_pos);
                        row++;
                        }
                  }
            //Handle all jumps here
            if (getMetaRow(tr("Jumps and Markers")) != num_metas) {
                  ElementList measure_elements_list = cm->el();
                  for (Element* element : measure_elements_list) {
                        std::get<3>(repeat_info) = element;
                        if (element->isMarker())
                              jump_marker_meta(0, &stagger, x_pos);
                        }
                  for (Element* element : measure_elements_list) {
                        if (element->isJump()) {
                              std::get<2>(repeat_info) = element;
                              if (collapsed_meta)
                                    jump_marker_meta(0, &stagger, x_pos);
                              else
                                    jump_marker_meta(0, &std::get<0>(repeat_info), x_pos);
                              }
                        }
                  }
            stagger = 0;
            std::get<0>(repeat_info) = 0;

            for (unsigned int row = 0; row < num_metas; row++) {
                  stagger_arr[row] = 0;
                  }
            x_pos += grid_width;
            std::get<4>(repeat_info) = false;
            }
      drawSelection();
      }

//---------------------------------------------------------
//   tempo_meta
//---------------------------------------------------------

void Timeline::tempo_meta(Segment* seg, int* stagger, int pos)
      {
      //Find position of tempo_meta in metas
      int row = getMetaRow(tr("Tempo"));

      //Add all tempo texts in this segment
      const std::vector<Element*> annotations = seg->annotations();
      for (Element* element : annotations) {
            if (element->isTempoText()) {
                  TempoText* text = toTempoText(element);
                  qreal x = pos + (*stagger) * spacing;
                  if (addMetaValue(x, pos, text->plainText(), row, ElementType::TEMPO_TEXT, element, 0, seg->measure())) {
                        (*stagger)++;
                        global_z_value++;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   time_meta
//---------------------------------------------------------

void Timeline::time_meta(Segment* seg, int* stagger, int pos)
      {
      if (!seg->isTimeSigType())
            return;

      int x = pos + (*stagger) * spacing;

      //Find position of time_meta in metas
      int row = getMetaRow(tr("Time Signature"));

      TimeSig* original_time_sig = toTimeSig(seg->element(0));
      if (!original_time_sig)
            return;

      //Check if same across all staves
      const int nrows = _score->staves().size();
      bool same = true;
      for (int track = 0; track < nrows; track++) {
            const TimeSig* curr_time_sig = toTimeSig(seg->element(track * VOICES));
            if (!curr_time_sig) {
                  same = false;
                  break;
                  }
            if (*curr_time_sig == *original_time_sig) {
                  continue;
                  }
            same = false;
            break;
            }
      if (!same)
            return;
      QString text = QString::number(original_time_sig->numerator()) + QString("/") + QString::number(original_time_sig->denominator());

      if (addMetaValue(x, pos, text, row, ElementType::TIMESIG, 0, seg, seg->measure())) {
            (*stagger)++;
            global_z_value++;
            }
      }

//---------------------------------------------------------
//   rehearsal_meta
//---------------------------------------------------------

void Timeline::rehearsal_meta(Segment* seg, int* stagger, int pos)
      {
      int row = getMetaRow(tr("Rehearsal Mark"));

      for (Element* element : seg->annotations()) {
            int x = pos + (*stagger) * spacing;
            if (element->isRehearsalMark()) {

                  RehearsalMark* rehersal_mark = toRehearsalMark(element);
                  if (!rehersal_mark)
                        continue;

                  if (addMetaValue(x, pos, rehersal_mark->plainText(), row, ElementType::REHEARSAL_MARK, element, 0, seg->measure())) {
                        (*stagger)++;
                        global_z_value++;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   key_meta
//---------------------------------------------------------

void Timeline::key_meta(Segment* seg, int* stagger, int pos)
      {
      //If seg is null, handle initial key signature
      if (seg && !seg->isKeySigType())
            return;

      int row = getMetaRow(tr("Key Signature"));
      std::map<Key, int> key_frequencies;
      QList<Staff*> staves = _score->staves();

      int track = 0;
      for (Staff* stave : staves) {
            if (!stave->show()) {
                  track += VOICES;
                  continue;
                  }

            //Ignore unpitched staves
            if ((seg && !stave->isPitchedStaff(seg->tick())) || (!seg && !stave->isPitchedStaff(0))) {
                  track += VOICES;
                  continue;
                  }

            //Add corrected key signature to map
            //Atonal -> Key::INVALID
            //Custom -> Key::NUM_OF
            const KeySig* curr_key_sig = nullptr;
            if (seg)
                  curr_key_sig = toKeySig(seg->element(track));

            Key global_key;
            if (seg)
                  global_key = stave->key(seg->tick());
            else
                  global_key = stave->key(0);
            if (curr_key_sig) {
                  if (curr_key_sig->generated())
                        return;
                  global_key = curr_key_sig->key();
                  }

            if (curr_key_sig && curr_key_sig->isAtonal())
                  global_key = Key::INVALID;
            else if (curr_key_sig && curr_key_sig->isCustom())
                  global_key = Key::NUM_OF;
            else {
                  const Interval curr_interval = stave->part()->instrument()->transpose();
                  global_key = transposeKey(global_key, curr_interval);
                  }

            std::map<Key, int>::iterator it = key_frequencies.find(global_key);
            if (it != key_frequencies.end())
                  key_frequencies[global_key]++;
            else
                  key_frequencies[global_key] = 1;

            track += VOICES;
            }

      //Change key into QString
      Key new_key = Key::C;
      int max_key_freq = 0;
      for (std::map<Key, int>::iterator iter = key_frequencies.begin(); iter != key_frequencies.end(); ++iter) {
            if (iter->second > max_key_freq) {
                  new_key = iter->first;
                  max_key_freq = iter->second;
                  }
            }
      QString key_text;
      QString tooltip;
      if (new_key == Key::INVALID) {
            key_text = "X";
            tooltip = keyNames[15];
            }
      else if (new_key == Key::NUM_OF) {
            key_text = "?";
            tooltip = tr("Custom Key");
            }
      else if (int(new_key) == 0) {
            key_text = "\u266E";
            tooltip = keyNames[14];
            }
      else if (int(new_key) < 0) {
            key_text = QString::number(abs(int(new_key))) + "\u266D";
            tooltip = keyNames[(7 + int(new_key)) * 2 + 1];
            }
      else {
            key_text = QString::number(abs(int(new_key))) + "\u266F";
            tooltip = keyNames[(int(new_key) - 1) * 2];
            }

      int x = pos + (*stagger) * spacing;
      Measure* measure = (seg)? seg->measure() : 0;
      if (addMetaValue(x, pos, key_text, row, ElementType::KEYSIG, 0, seg, measure, tooltip)) {
            (*stagger)++;
            global_z_value++;
            }
      }

//---------------------------------------------------------
//   repeat_meta
//---------------------------------------------------------

void Timeline::barline_meta(Segment* seg, int* stagger, int pos)
      {
      if (!seg->isBeginBarLineType() && !seg->isEndBarLineType() && !seg->isBarLine() && !seg->isStartRepeatBarLineType())
            return;

      //Find position of repeat_meta in metas
      int row = getMetaRow(tr("Barlines"));

      QString repeat_text = "";
      BarLine* barline = toBarLine(seg->element(0));

      if (barline) {
            switch (barline->barLineType()) {
                  case BarLineType::START_REPEAT:
                        repeat_text = QString("Start repeat");
                        break;
                  case BarLineType::END_REPEAT:
                        repeat_text = QString("End repeat");
                        break;
                  case BarLineType::DOUBLE:
                        repeat_text = QString("Double barline");
                        break;
                  case BarLineType::END:
                        repeat_text = QString("Final barline");
                        break;
                  default:
                        break;
                  }
            is_barline = true;
            }
      else
            return;

      Measure* measure = seg->measure();
      ElementType element_type = ElementType::BAR_LINE;
      Element* element = nullptr;

      if (repeat_text == "") {
            is_barline = false;
            return;
            }

      int x = pos + (*stagger) * spacing;
      if (addMetaValue(x, pos, repeat_text, row, element_type, element, seg, measure)) {
            (*stagger)++;
            global_z_value++;
            }
      is_barline = false;
      }

//---------------------------------------------------------
//   jump_marker_meta
//---------------------------------------------------------

void Timeline::jump_marker_meta(Segment* seg, int* stagger, int pos)
      {
      if (seg)
            return;

      //Find position of repeat_meta in metas
      int row = getMetaRow(tr("Jumps and Markers"));

      QString text = "";
      Element* element = nullptr;
      if (std::get<2>(repeat_info))
            element = std::get<2>(repeat_info);
      else if (std::get<3>(repeat_info))
            element = std::get<3>(repeat_info);

      Measure* measure;
      ElementType element_type;
      if (std::get<2>(repeat_info)) {
            Jump* jump = toJump(std::get<2>(repeat_info));
            text = jump->plainText();
            measure = jump->measure();
            element_type = ElementType::JUMP;
            }
      else {
            Marker* marker = toMarker(std::get<3>(repeat_info));
            QList<TextFragment> tf_list = marker->fragmentList();
            for (TextFragment tf: tf_list)
                  text.push_back(tf.text);
            measure = marker->measure();
            if (marker->markerType() == Marker::Type::FINE || marker->markerType() == Marker::Type::TOCODA) {
                  element_type = ElementType::MARKER;
                  std::get<2>(repeat_info) = std::get<3>(repeat_info);
                  std::get<3>(repeat_info) = nullptr;
                  }
            else
                  element_type = ElementType::MARKER;
            }

      if (text == "") {
            std::get<2>(repeat_info) = nullptr;
            std::get<3>(repeat_info) = nullptr;
            return;
            }

      int x = pos + (*stagger) * spacing;
      if (addMetaValue(x, pos, text, row, element_type, element, seg, measure)) {
            (*stagger)++;
            global_z_value++;
            }
      std::get<2>(repeat_info) = nullptr;
      std::get<3>(repeat_info) = nullptr;
      }

//---------------------------------------------------------
//   measure_meta
//---------------------------------------------------------

void Timeline::measure_meta(Segment* , int* , int pos)
      {
      //Increment decided by zoom level
      int increment_value = 1;
      int halfway = (max_zoom + min_zoom) / 2;
      if (grid_width <= max_zoom && grid_width > halfway)
            increment_value = 1;
      else if (grid_width <= halfway && grid_width > min_zoom)
            increment_value = 5;
      else
            increment_value = 10;

      int curr_measure_number = pos / grid_width;
      if (curr_measure_number == global_measure_number)
            return;

      global_measure_number = curr_measure_number;
      //Check if 1 or 5*n
      if (curr_measure_number + 1 != 1 && (curr_measure_number + 1) % increment_value != 0)
            return;

      //Find position of measure_meta in metas
      int row = getMetaRow(tr("Measures"));

      //Adjust number
      Measure* curr_measure;
      for (curr_measure = _score->firstMeasure(); curr_measure_number != 0; curr_measure_number--, curr_measure = curr_measure->nextMeasure()) {

            }

      //Add measure number
      QString measure_number = (curr_measure->irregular())? "( )" : QString::number(curr_measure->no() + 1);
      QGraphicsTextItem* graphics_text_item = new QGraphicsTextItem(measure_number);
      graphics_text_item->setDefaultTextColor(QColor(0, 0, 0));
      graphics_text_item->setX(pos);
      graphics_text_item->setY(grid_height * row + verticalScrollBar()->value());

      QFont f = graphics_text_item->font();
      f.setPointSizeF(7.0);
      graphics_text_item->setFont(f);

      //Center text
      qreal remaining_width =  grid_width - graphics_text_item->boundingRect().width();
      qreal remaining_height =  grid_height - graphics_text_item->boundingRect().height();
      graphics_text_item->setX(graphics_text_item->x() + remaining_width / 2);
      graphics_text_item->setY(graphics_text_item->y() + remaining_height / 2);

      int end_of_text = graphics_text_item->x() + graphics_text_item->boundingRect().width();
      int end_of_grid = getWidth();
      if (end_of_text <= end_of_grid) {
            scene()->addItem(graphics_text_item);

            std::pair<QGraphicsItem*, int> pair_measure_text(graphics_text_item, row);
            meta_rows.push_back(pair_measure_text);
            }
      }

//---------------------------------------------------------
//   getMetaRow
//---------------------------------------------------------

unsigned int Timeline::getMetaRow(QString target_text)
      {
      if (collapsed_meta) {
            if (target_text == tr("Measures"))
                  return 1;
            else
                  return 0;
            }
      int row = 0;
      for (auto it = metas.begin(); it != metas.end(); ++it) {
            std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> meta = *it;
            QString meta_text = std::get<0>(meta);
            bool visible = std::get<2>(meta);
            if (meta_text == target_text && visible)
                  break;
            else if (!visible)
                  continue;
            row++;
            }
      return row;
      }

//---------------------------------------------------------
//   addMetaValue
//---------------------------------------------------------

bool Timeline::addMetaValue(int x, int pos, QString meta_text, int row, ElementType element_type, Element* element, Segment* seg, Measure* measure, QString tooltip)
      {
      QGraphicsTextItem* graphics_text_item = new QGraphicsTextItem(meta_text);
      qreal text_width = graphics_text_item->boundingRect().width();

      QGraphicsPixmapItem* graphics_pixmap_item = nullptr;
      if (is_barline)
            graphics_pixmap_item = new QGraphicsPixmapItem(*barlines[meta_text]);

      if (graphics_pixmap_item) {
            text_width = 10;
            if (text_width > grid_width) {
                  text_width = grid_width;
                  if (meta_text == QString("End repeat") && std::get<4>(repeat_info))
                        text_width /= 2;
                  }
            }

      if (text_width + x > getWidth())
            text_width = getWidth() - x;

      //Adjust x for end repeats
      if ((meta_text == QString("End repeat") || meta_text == QString("Final barline") || meta_text == QString("Double barline") || std::get<2>(repeat_info)) && !collapsed_meta) {
            if (std::get<0>(repeat_info) > 0)
                  x = pos + grid_width - std::get<1>(repeat_info) + std::get<0>(repeat_info) * spacing;
            else {
                  x = pos + grid_width - text_width;
                  std::get<1>(repeat_info) = text_width;
                  }
            //Check if extending past left side
            if (x < 0) {
                  text_width = text_width + x;
                  x = 0;
                  }
            }

      //Return if past width
      if (x >= getWidth())
            return false;

      QGraphicsItem* item_to_add;
      if (graphics_pixmap_item) {
            //Exact values required for repeat pixmap to work visually
            if (text_width != 10)
                  graphics_pixmap_item = new QGraphicsPixmapItem();
            if (meta_text == QString("Start repeat"))
                  std::get<4>(repeat_info) = true;
            graphics_pixmap_item->setX(x + 2);
            graphics_pixmap_item->setY(grid_height * row + verticalScrollBar()->value() + 3);
            item_to_add = graphics_pixmap_item;
            }
      else if (meta_text == "\uE047" || meta_text == "\uE048") {
            graphics_text_item->setX(x);
            graphics_text_item->setY(grid_height * row + verticalScrollBar()->value() - 2);
            item_to_add = graphics_text_item;
            }
      else {
            graphics_text_item->setX(x);
            graphics_text_item->setY(grid_height * row + verticalScrollBar()->value());
            item_to_add = graphics_text_item;
            }

      QFontMetrics f(QApplication::font());
      QString part_name = f.elidedText(graphics_text_item->toPlainText(),
                                       Qt::ElideRight,
                                       text_width);

      //Set tool tip if elided
      if (tooltip != "")
            graphics_text_item->setToolTip(tooltip);
      else if (part_name != meta_text)
            graphics_text_item->setToolTip(graphics_text_item->toPlainText());
      graphics_text_item->setPlainText(part_name);

      //Make text fit within rectangle
      while (graphics_text_item->boundingRect().width() > text_width &&
             graphics_text_item->toPlainText() != "") {
            QString text = graphics_text_item->toPlainText();
            text.chop(1);
            graphics_text_item->setPlainText(text);
            }

      QGraphicsRectItem* graphics_rect_item = new QGraphicsRectItem(x,
                                                                    grid_height * row + verticalScrollBar()->value(),
                                                                    text_width,
                                                                    grid_height);
      if (tooltip != "")
            graphics_rect_item->setToolTip(tooltip);
      else if (part_name != meta_text)
            graphics_rect_item->setToolTip(meta_text);
      else if (graphics_pixmap_item)
            graphics_rect_item->setToolTip(tr(meta_text.toLatin1().constData()));

      setMetaData(graphics_rect_item, -1, element_type, measure, true, element, item_to_add, seg);
      setMetaData(item_to_add, -1, element_type, measure, true, element, graphics_rect_item, seg);

      graphics_rect_item->setZValue(global_z_value);
      item_to_add->setZValue(global_z_value);

      graphics_rect_item->setPen(QPen(Qt::black));
      graphics_rect_item->setBrush(QBrush(Qt::gray));

      scene()->addItem(graphics_rect_item);
      scene()->addItem(item_to_add);

      std::pair<QGraphicsItem*, int> pair_time_rect(graphics_rect_item, row);
      std::pair<QGraphicsItem*, int> pair_time_text(item_to_add, row);
      meta_rows.push_back(pair_time_rect);
      meta_rows.push_back(pair_time_text);

      if (meta_text == QString("End repeat"))
            std::get<0>(repeat_info)++;

      return true;
      }

//---------------------------------------------------------
//   setMetaData
//---------------------------------------------------------

void Timeline::setMetaData(QGraphicsItem* gi, int staff, ElementType et, Measure* m, bool full_measure, Element* e, QGraphicsItem* pair_item, Segment* seg)
      {
      //full_measure true for meta values
      //pr is null for grid items, set for meta values
      //seg is set if key meta
      gi->setData(0, QVariant::fromValue<int>(staff));
      gi->setData(1, QVariant::fromValue<ElementType>(et));
      gi->setData(2, QVariant::fromValue<void*>(m));
      gi->setData(3, QVariant::fromValue<bool>(full_measure));
      gi->setData(4, QVariant::fromValue<void*>(e));
      gi->setData(5, QVariant::fromValue<void*>(pair_item));
      gi->setData(6, QVariant::fromValue<void*>(seg));
      }

//---------------------------------------------------------
//   getWidth
//---------------------------------------------------------

int Timeline::getWidth()
      {
      if (_score)
            return int(_score->nmeasures() * grid_width);
      else
            return 0;
      }

//---------------------------------------------------------
//   getHeight
//---------------------------------------------------------

int Timeline::getHeight()
      {
      if (_score)
            return int((nstaves() + nmetas()) * grid_height + 3);
      else
            return 0;
      }

//---------------------------------------------------------
//   correctStave
//---------------------------------------------------------

int Timeline::correctStave(int stave)
      {
      //Find correct stave (skipping hidden staves)
      QList<Staff*> list = _score->staves();
      int count = 0;
      while (stave >= count) {
            if (count >= list.size()) {
                  count = list.size() - 1;
                  return count;
                  }
            if (!list.at(count)->show())
                  stave++;
            count++;
            }
      return stave;
      }


//---------------------------------------------------------
//   correctPart
//---------------------------------------------------------

int Timeline::correctPart(int stave)
      {
      //Find correct stave (skipping hidden staves)
      QList<Staff*> list = _score->staves();
      int count = correctStave(stave);
      return _score->parts().indexOf(list.at(count)->part());
      }


//---------------------------------------------------------
//   changeSelection
//---------------------------------------------------------

void Timeline::changeSelection(SelState)
      {
      scene()->blockSignals(true);
      scene()->clearSelection();

      QRectF selection_rect = selection_path.boundingRect();
      if (selection_rect == QRectF())
            return;

      int nmeta = nmetas();

      //Get borders of the current viewport
      int left_border = horizontalScrollBar()->value();
      int right_border = horizontalScrollBar()->value() + viewport()->width();
      int top_border = verticalScrollBar()->value() + nmeta * grid_height;
      int bottom_border = verticalScrollBar()->value() + viewport()->height();

      bool selection_extends_up = false, selection_extends_left = false;
      bool selection_extends_right = false, selection_extends_down = false;

      //Figure out which directions the selection extends
      if (selection_rect.top() < top_border)
            selection_extends_up = true;
      if (selection_rect.left() < left_border - 1)
            selection_extends_left = true;
      if (selection_rect.right() > right_border)
            selection_extends_right = true;
      if (selection_rect.bottom() > bottom_border)
            selection_extends_down = true;

      if (selection_extends_down
          && old_selection_rect.bottom() != selection_rect.bottom()
          && !meta_value) {
            int new_scrollbar_value = int(verticalScrollBar()->value() + selection_rect.bottom() - bottom_border);
            verticalScrollBar()->setValue(new_scrollbar_value);
            }
      else if (selection_extends_up
               && !selection_extends_down
               && old_selection_rect.bottom() != selection_rect.bottom()
               && !meta_value
               && old_selection_rect.contains(selection_rect)) {
            int new_scrollbar_value = int(verticalScrollBar()->value() + selection_rect.bottom() - bottom_border);
            verticalScrollBar()->setValue(new_scrollbar_value);
            }

      if (selection_extends_right
          && old_selection_rect.right() != selection_rect.right()) {
            int new_scrollbar_value = int(horizontalScrollBar()->value() + selection_rect.right() - right_border);
            horizontalScrollBar()->setValue(new_scrollbar_value);
            }
      if (selection_extends_up
          && old_selection_rect.top() != selection_rect.top()
          && !meta_value) {
            int new_scrollbar_value = int(selection_rect.top()) - nmeta * grid_height;
            verticalScrollBar()->setValue(new_scrollbar_value);
            }
      if (selection_extends_left
          && old_selection_rect.left() != selection_rect.left()) {
            int new_scrollbar_value = int(selection_rect.left());
            horizontalScrollBar()->setValue(new_scrollbar_value);
            }

      if (selection_extends_left
          && !selection_extends_right
          && old_selection_rect.right() != selection_rect.right()
          && old_selection_rect.contains(selection_rect)) {
            int new_scrollbar_value = int(horizontalScrollBar()->value() + selection_rect.right() - right_border);
            horizontalScrollBar()->setValue(new_scrollbar_value);
            }
      if (selection_extends_right
          && !selection_extends_left
          && old_selection_rect.left() != selection_rect.left()
          && old_selection_rect.contains(selection_rect)) {
            int new_scrollbar_value = int(selection_rect.left());
            horizontalScrollBar()->setValue(new_scrollbar_value);
            }

      if (selection_extends_down
          && !selection_extends_up
          && old_selection_rect.top() != selection_rect.top()
          && !meta_value
          && old_selection_rect.contains(selection_rect)) {
            int new_scrollbar_value = int(selection_rect.top()) - nmeta * grid_height;
            verticalScrollBar()->setValue(new_scrollbar_value);
            }

      old_selection_rect = selection_rect;

      meta_value = false;
      scene()->blockSignals(false);
      }

//---------------------------------------------------------
//   drawSelection
//---------------------------------------------------------

void Timeline::drawSelection()
      {
      selection_path = QPainterPath();
      selection_path.setFillRule(Qt::WindingFill);

      std::set<std::tuple<Measure*, int, ElementType>> meta_labels_set;

      const Selection selection = _score->selection();
      QList<Element*> el = selection.elements();
      for (Element* element : el) {
            if (element->tick() == -1)
                  continue;
            else {
                  switch (element->type()) {
                        case ElementType::INSTRUMENT_NAME:
                        case ElementType::VBOX:
                        case ElementType::HBOX:
                        case ElementType::TEXT:
                        case ElementType::TIE_SEGMENT:
                        case ElementType::SLUR_SEGMENT:
                        case ElementType::TIE:
                        case ElementType::SLUR:
                              continue;
                              break;
                        default: break;
                        }
                  }

            int staffIdx;
            int tick = element->tick();
            Measure* measure = _score->tick2measure(tick);
            staffIdx = element->staffIdx();
            if (numToStaff(staffIdx) && !numToStaff(staffIdx)->show())
                  continue;

            if ((element->isTempoText() ||
                 element->isKeySig() ||
                 element->isTimeSig() ||
                 element->isRehearsalMark() ||
                 element->isJump() ||
                 element->isMarker()) &&
                !element->generated())
                  staffIdx = -1;

            if (element->isBarLine()) {
                  staffIdx = -1;
                  BarLine* barline = toBarLine(element);
                  if (barline &&
                      (barline->barLineType() == BarLineType::END_REPEAT || barline->barLineType() == BarLineType::DOUBLE || barline->barLineType() == BarLineType::END) &&
                      measure != _score->lastMeasure()) {
                        if (measure->prevMeasure())
                              measure = measure->prevMeasure();
                        }
                  }

            //element->type() for meta rows, invalid for everything else
            ElementType element_type = (staffIdx == -1)? element->type() : ElementType::INVALID;

            //If has a multi measure rest, find the count and add each measure to it
            // ws: If style flag Sid::createMultiMeasureRests is not set, then
            // measure->mmRest() is not valid

//            if (measure->mmRest() ) {
            if (measure->mmRest() && measure->score()->styleB(Sid::createMultiMeasureRests)) {
                  int mmrest_count = measure->mmRest()->mmRestCount();
                  Measure* tmp_measure = measure;
                  for (int mmrest_measure = 0; mmrest_measure < mmrest_count; mmrest_measure++) {
                        std::tuple<Measure*, int, ElementType> tmp(tmp_measure, staffIdx, element_type);
                        meta_labels_set.insert(tmp);
                        tmp_measure = tmp_measure->nextMeasure();
                        }
                  }
            else {
                  std::tuple<Measure*, int, ElementType> tmp(measure, staffIdx, element_type);
                  meta_labels_set.insert(tmp);
                  }
            }

      QList<QGraphicsItem*> graphics_item_list = scene()->items();
      for (QGraphicsItem* graphics_item : graphics_item_list) {

            int stave = graphics_item->data(0).value<int>();
            ElementType element_type = graphics_item->data(1).value<ElementType>();
            Measure* measure = static_cast<Measure*>(graphics_item->data(2).value<void*>());

            std::tuple<Measure*, int, ElementType> target_tuple(measure, stave, element_type);
            std::set<std::tuple<Measure*, int, ElementType>>::iterator it;
            it = meta_labels_set.find(target_tuple);

            if (stave == -1 && it != meta_labels_set.end()) {
                  //Make sure the element is correct
                  QList<Element*> element_list = _score->selection().elements();
                  Element* target_element = static_cast<Element*>(graphics_item->data(4).value<void*>());
                  Segment* seg = static_cast<Segment*>(graphics_item->data(6).value<void*>());

                  if (target_element) {
                        for (Element* element : element_list) {
                              if (element == target_element) {
                                    QGraphicsRectItem* graphics_rect_item = qgraphicsitem_cast<QGraphicsRectItem*>(graphics_item);
                                    if (graphics_rect_item)
                                          graphics_rect_item->setBrush(QBrush(QColor(173,216,230)));
                                    }
                              }
                        }
                  else if (seg) {
                        for (Element* element : element_list) {
                              QGraphicsRectItem* graphics_rect_item = qgraphicsitem_cast<QGraphicsRectItem*>(graphics_item);
                              if (graphics_rect_item) {
                                    for (int track = 0; track < _score->nstaves() * VOICES; track++) {
                                          if (element == seg->element(track))
                                                graphics_rect_item->setBrush(QBrush(QColor(173,216,230)));
                                          }
                                    }
                              }
                        }
                  else {
                        QGraphicsRectItem* graphics_rect_item = qgraphicsitem_cast<QGraphicsRectItem*>(graphics_item);
                        if (graphics_rect_item)
                              graphics_rect_item->setBrush(QBrush(QColor(173,216,230)));
                        }
                  }
            //Change color from gray to only blue
            else if (it != meta_labels_set.end()) {
                  QGraphicsRectItem* graphics_rect_item = qgraphicsitem_cast<QGraphicsRectItem*>(graphics_item);
                  graphics_rect_item->setBrush(QBrush(QColor(graphics_rect_item->brush().color().red(),
                                                             graphics_rect_item->brush().color().green(),
                                                             255)));
                  selection_path.addRect(graphics_rect_item->rect());
                  }
            }

      QGraphicsPathItem* graphics_path_item = new QGraphicsPathItem(selection_path.simplified());
      if (selection.isRange())
            graphics_path_item->setPen(QPen(QColor(0, 0, 255), 3));
      else
            graphics_path_item->setPen(QPen(QColor(0, 0, 0), 1));

      graphics_path_item->setBrush(Qt::NoBrush);
      graphics_path_item->setZValue(-1);
      scene()->addItem(graphics_path_item);

      if (std::get<0>(old_hover_info)) {
            std::get<0>(old_hover_info) = nullptr;
            std::get<1>(old_hover_info) = -1;
            }
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void Timeline::mousePressEvent(QMouseEvent* event)
      {
      if (!_score)
            return;

      if (event->button() == Qt::RightButton)
          return;

      //Set as clicked
      mouse_pressed = true;
      scene()->clearSelection();

      QPointF scene_pt = mapToScene(event->pos());
      //Set as old location
      old_loc = QPoint(int(scene_pt.x()), int(scene_pt.y()));
      QList<QGraphicsItem*> graphics_item_list = scene()->items(scene_pt);
      //Find highest z value for rect
      int max_z_value = -4;
      QGraphicsItem* curr_graphics_item = nullptr;
      for (QGraphicsItem* graphics_item: graphics_item_list) {
            QGraphicsRectItem* graphics_rect_item = qgraphicsitem_cast<QGraphicsRectItem*>(graphics_item);
            if (graphics_rect_item && graphics_item->zValue() > max_z_value) {
                  curr_graphics_item = graphics_item;
                  max_z_value = graphics_item->zValue();
                  }
            }
      if (curr_graphics_item) {
            int stave = curr_graphics_item->data(0).value<int>();
            Measure* curr_measure = static_cast<Measure*>(curr_graphics_item->data(2).value<void*>());
            if (numToStaff(stave) && !numToStaff(stave)->show())
                  return;

            if (!curr_measure) {
                  int nmeta = nmetas();
                  int bottom_of_meta = nmeta * grid_height + verticalScrollBar()->value();

                  //Handle measure box clicks
                  if (scene_pt.y() > (nmeta - 1) * grid_height + verticalScrollBar()->value() &&
                      scene_pt.y() < bottom_of_meta) {

                        QRectF tmp(scene_pt.x(), 0, 3, nmeta * grid_height + nstaves() * grid_height);
                        QList<QGraphicsItem*> gl = scene()->items(tmp);
                        Measure* measure = nullptr;

                        for (QGraphicsItem* graphics_item : gl) {
                              measure = static_cast<Measure*>(graphics_item->data(2).value<void*>());
                              //-3 z value is the grid square values
                              if (graphics_item->zValue() == -3 && measure)
                                    break;
                              }
                        if (measure)
                              _cv->adjustCanvasPosition(measure, false);
                        }
                  if (scene_pt.y() < bottom_of_meta)
                        return;

                  QList<QGraphicsItem*> gl = items(event->pos());
                  for (QGraphicsItem* graphics_item : gl) {
                        curr_measure = static_cast<Measure*>(graphics_item->data(2).value<void*>());
                        stave = graphics_item->data(0).value<int>();
                        if (curr_measure)
                              break;
                        }
                  if (!curr_measure) {
                        _score->select(0, SelectType::SINGLE, 0);
                        return;
                        }
                  }

            bool meta_value_clicked = curr_graphics_item->data(3).value<bool>();

            scene()->clearSelection();
            if (meta_value_clicked) {

                  meta_value = true;
                  old_selection_rect = QRect();

                  _cv->adjustCanvasPosition(curr_measure, false, 0);
                  verticalScrollBar()->setValue(0);

                  Segment* seg = static_cast<Segment*>(curr_graphics_item->data(6).value<void*>());

                  if (seg) {
                        _score->deselectAll();
                        for (int track = 0; track < _score->nstaves() * VOICES; track++) {
                              Element* element = seg->element(track);
                              if (element)
                                    _score->select(seg->element(track), SelectType::ADD);
                              }
                        }
                  else {
                        //Also select the elements that they correspond to
                        ElementType element_type = curr_graphics_item->data(1).value<ElementType>();
                        SegmentType segment_type = SegmentType::Invalid;
                        if (element_type == ElementType::KEYSIG)
                              segment_type = SegmentType::KeySig;
                        else if (element_type == ElementType::TIMESIG)
                              segment_type = SegmentType::TimeSig;

                        if (segment_type != SegmentType::Invalid) {
                              Segment* curr_seg = curr_measure->first();
                              for (; curr_seg && curr_seg->segmentType() != segment_type; curr_seg = curr_seg->next()) {
                                    }
                              if (curr_seg) {
                                    _score->deselectAll();
                                    for (int j = 0; j < _score->nstaves(); j++) {
                                          Element* element = curr_seg->firstElement(j);
                                          if (element)
                                                _score->select(element, SelectType::ADD);
                                          }
                                    }
                              }
                        else {
                              _score->deselectAll();
                              _score->select(curr_measure, SelectType::ADD, 0);
                              //Select just the element for tempo_text
                              Element* element = static_cast<Element*>(curr_graphics_item->data(4).value<void*>());
                              _score->deselectAll();
                              _score->select(element);
                              }
                        }
                  }
            else {
                  //Handle cell clicks
                  if (event->modifiers() == Qt::ShiftModifier) {
                        if (curr_measure->mmRest())
                              curr_measure = curr_measure->mmRest();
                        else if (curr_measure->mmRestCount() == -1)
                              curr_measure = curr_measure->prevMeasureMM();
                        _score->select(curr_measure, SelectType::RANGE, stave);
                        _score->setUpdateAll();
                        }
                  else if (event->modifiers() == Qt::ControlModifier) {

                        if (_score->selection().isNone()) {
                              if (curr_measure->mmRest())
                                    curr_measure = curr_measure->mmRest();
                              else if (curr_measure->mmRestCount() == -1)
                                    curr_measure = curr_measure->prevMeasureMM();

                              _score->select(curr_measure, SelectType::RANGE, 0);
                              _score->select(curr_measure, SelectType::RANGE, _score->nstaves()-1);
                              _score->setUpdateAll();
                              }
                        else
                              _score->deselectAll();
                        }
                  else {
                        if (curr_measure->mmRest())
                              curr_measure = curr_measure->mmRest();
                        else if (curr_measure->mmRestCount() == -1)
                              curr_measure = curr_measure->prevMeasureMM();
                        _score->select(curr_measure, SelectType::SINGLE, stave);
                        }
                  _cv->adjustCanvasPosition(curr_measure, false, stave);

                  }
            mscore->endCmd();
            _score->update();
            }
      else {
            _score->deselectAll();
            mscore->endCmd();
            _score->update();
            }
      _score->setUpdateAll();
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Timeline::mouseMoveEvent(QMouseEvent* event)
      {
      QPointF new_loc = mapToScene(event->pos());
      if (!mouse_pressed) {

            if (cursorIsOn() == "meta") {
                  this->setCursor(Qt::ArrowCursor);
                  mouseOver(new_loc);
                  }
            else if (cursorIsOn() == "invalid")
                  this->setCursor(Qt::ForbiddenCursor);
            else
                  this->setCursor(Qt::ArrowCursor);

            emit moved(QPointF(-1, -1));
            return;
            }

      if (state == ViewState::NORMAL) {
            if (event->modifiers() == Qt::ShiftModifier) {
                  //Slight wiggle room for selection (Same as score)
                  if (abs(new_loc.x() - old_loc.x()) > 2 ||
                      abs(new_loc.y() - old_loc.y()) > 2) {
                        _score->deselectAll();
                        updateGrid();
                        state = ViewState::LASSO;
                        selection_box = new QGraphicsRectItem();
                        selection_box->setRect(old_loc.x(), old_loc.y(), 0, 0);
                        selection_box->setPen(QPen(QColor(0, 0, 255), 2));
                        selection_box->setBrush(QBrush(QColor(0, 0, 255, 50)));
                        scene()->addItem(selection_box);
                        }
                  }
            else {
                  state = ViewState::DRAG;
                  this->setCursor(Qt::SizeAllCursor);
                  }
            }

      if (state == ViewState::LASSO) {
            QRect tmp = QRect((old_loc.x() < new_loc.x())? old_loc.x() : new_loc.x(),
                              (old_loc.y() < new_loc.y())? old_loc.y() : new_loc.y(),
                              abs(new_loc.x() - old_loc.x()),
                              abs(new_loc.y() - old_loc.y()));
            selection_box->setRect(tmp);
            }
      else if (state == ViewState::DRAG) {
            int x_offset = int(old_loc.x()) - int(new_loc.x());
            int y_offset = int(old_loc.y()) - int(new_loc.y());
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() + x_offset);
            verticalScrollBar()->setValue(verticalScrollBar()->value() + y_offset);
            }
      emit moved(QPointF(-1, -1));
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void Timeline::mouseReleaseEvent(QMouseEvent*)
      {
      mouse_pressed = false;
      if (state == ViewState::LASSO) {

            scene()->removeItem(selection_box);
            _score->deselectAll();

            int width, height;
            QPoint loc = mapFromScene(selection_box->rect().topLeft());
            width = int(selection_box->rect().width());
            height = int(selection_box->rect().height());

            QList<QGraphicsItem*> graphics_item_list = items(QRect(loc.x(), loc.y(), width, height));
            //Find top left and bottom right to create selection
            QGraphicsItem* tl_graphics_item = nullptr;
            QGraphicsItem* br_graphics_item = nullptr;
            for (QGraphicsItem* graphics_item : graphics_item_list) {
                  Measure* curr_measure = static_cast<Measure*>(graphics_item->data(2).value<void*>());
                  if (!curr_measure) continue;
                  int stave = graphics_item->data(0).value<int>();
                  if (stave == -1) continue;

                  if (!tl_graphics_item && !br_graphics_item) {
                        tl_graphics_item = graphics_item;
                        br_graphics_item = graphics_item;
                        continue;
                        }

                  if (graphics_item->boundingRect().top() < tl_graphics_item->boundingRect().top())
                        tl_graphics_item = graphics_item;
                  if (graphics_item->boundingRect().left() < tl_graphics_item->boundingRect().left())
                        tl_graphics_item = graphics_item;

                  if (graphics_item->boundingRect().bottom() > br_graphics_item->boundingRect().bottom())
                        br_graphics_item = graphics_item;
                  if (graphics_item->boundingRect().right() > br_graphics_item->boundingRect().right())
                        br_graphics_item = graphics_item;

                  }

            //Select single tl_graphics_item and then range br_graphics_item
            if (tl_graphics_item && br_graphics_item) {
                  Measure* tl_measure = static_cast<Measure*>(tl_graphics_item->data(2).value<void*>());
                  int tl_stave = tl_graphics_item->data(0).value<int>();
                  Measure* br_measure = static_cast<Measure*>(br_graphics_item->data(2).value<void*>());
                  int br_stave = br_graphics_item->data(0).value<int>();
                  if (tl_measure && br_measure) {
                        //Focus selection of mmRests here
                        if (tl_measure->mmRest())
                              tl_measure = tl_measure->mmRest();
                        else if (tl_measure->mmRestCount() == -1)
                              tl_measure = tl_measure->prevMeasureMM();
                        if (br_measure->mmRest())
                              br_measure = br_measure->mmRest();
                        else if (br_measure->mmRestCount() == -1)
                              br_measure = br_measure->prevMeasureMM();

                        _score->select(tl_measure, SelectType::SINGLE, tl_stave);
                        _score->select(br_measure, SelectType::RANGE, br_stave);
                        }
                  _cv->adjustCanvasPosition(tl_measure, false, tl_stave);
                  }

            _score->update();
            mscore->endCmd();
            }
      else if (state == ViewState::DRAG) {
            this->setCursor(Qt::ArrowCursor);
            mscore->endCmd();
            }
      state = ViewState::NORMAL;
      }

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void Timeline::leaveEvent(QEvent*)
      {
      if (!rect().contains(mapFromGlobal(QCursor::pos()))) {
            QPointF p = mapToScene(mapFromGlobal(QCursor::pos()));
            mouseOver(p);
            }
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void Timeline::wheelEvent(QWheelEvent* event)
      {
      if (event->modifiers().testFlag(Qt::ControlModifier)) {
            qreal original_cursor_pos = mapToScene(mapFromGlobal(QCursor::pos())).x();
            int original_scroll_value = horizontalScrollBar()->value();
            qreal ratio = original_cursor_pos / qreal(getWidth());

            if (event->angleDelta().y() > 0 && grid_width < max_zoom) {
                  grid_width++;
                  updateGrid();
                  }
            else if (event->angleDelta().y() < 0 && grid_width > min_zoom) {
                  grid_width--;
                  updateGrid();
                  }

            //Attempt to keep mouse in original spot
            qreal new_pos = qreal(getWidth()) * ratio;
            int offset = new_pos - original_cursor_pos;
            horizontalScrollBar()->setValue(original_scroll_value + offset);
            }
      else if (event->modifiers().testFlag(Qt::ShiftModifier)) {
            qreal num_of_steps = qreal(event->angleDelta().y()) / 2;
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - int(num_of_steps));
            }
      else
            QGraphicsView::wheelEvent(event);
      }

//---------------------------------------------------------
//   updateGrid
//---------------------------------------------------------

void Timeline::updateGrid()
      {
      if (_score && _score->firstMeasure()) {
            drawGrid(nstaves(), _score->nmeasures());
            updateView();
            drawSelection();
            mouseOver(mapToScene(mapFromGlobal(QCursor::pos())));
            row_names->updateLabels(getLabels(), grid_height);
            }
      viewport()->update();
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Timeline::setScore(Score* s)
      {
      _score = s;
      scene()->clear();

      if (_score) {
            connect(_score, &QObject::destroyed, this, &Timeline::objectDestroyed, Qt::UniqueConnection);
            drawGrid(nstaves(), _score->nmeasures());
            changeSelection(SelState::NONE);
            row_names->updateLabels(getLabels(), grid_height);
            }
      else {
            //Clear timeline if no score is present
            QSplitter* sp = scrollArea->grid();
            if (sp && sp->count() > 0) {
                  TRowLabels* t_row_labels = static_cast<TRowLabels*>(sp->widget(0));
                  std::vector<std::pair<QString, bool>> no_labels;
                  t_row_labels->updateLabels(no_labels, 0);
                  }
            meta_rows.clear();
            setSceneRect(0, 0, 0, 0);
            }

      viewport()->update();
      }

//---------------------------------------------------------
//   setScoreView
//---------------------------------------------------------

void Timeline::setScoreView(ScoreView* v)
      {
      _cv = v;
      if (_cv) {
            connect(_cv, &ScoreView::sizeChanged, this, &Timeline::updateView, Qt::UniqueConnection);
            connect(_cv, &ScoreView::viewRectChanged, this, &Timeline::updateView, Qt::UniqueConnection);
            connect(_cv, &QObject::destroyed, this, &Timeline::objectDestroyed, Qt::UniqueConnection);
            updateView();
            }
      }

//---------------------------------------------------------
//   objectDestroyed
//---------------------------------------------------------

void Timeline::objectDestroyed(QObject* obj)
      {
      if (_cv == obj)
            setScoreView(nullptr);
      else if (_score == obj)
            setScore(nullptr);
      }

//---------------------------------------------------------
//   updateView
//---------------------------------------------------------

void Timeline::updateView()
      {
      if (_cv && _score) {
            QRectF canvas = QRectF(_cv->matrix().inverted().mapRect(_cv->geometry()));

            std::set<std::pair<Measure*, int>> visible_items_set;

            //Find visible measures of score
            for (Measure* curr_measure = _score->firstMeasure(); curr_measure; curr_measure = curr_measure->nextMeasure()) {
                  System* system = curr_measure->system();

                  if (curr_measure->mmRest() && _score->styleB(Sid::createMultiMeasureRests)) {
                        //Handle mmRests
                        Measure* mmrest_measure = curr_measure->mmRest();
                        system = mmrest_measure->system();
                        if (!system)
                              continue;

                        //Add all measures within mmRest to visible_items_set if mmRest_visible
                        for (; curr_measure != mmrest_measure->mmRestLast(); curr_measure = curr_measure->nextMeasure()) {
                              for (int staff = 0; staff < _score->staves().length(); staff++) {
                                    if (!_score->staff(staff)->show())
                                          continue;
                                    QRectF stave_rect = QRectF(system->canvasBoundingRect().left(),
                                                               system->staffCanvasYpage(staff),
                                                               system->width(),
                                                               system->staff(staff)->bbox().height());
                                    QRectF show_rect = mmrest_measure->canvasBoundingRect().intersected(stave_rect);

                                    if (canvas.intersects(show_rect)) {
                                          std::pair<Measure*, int> p(curr_measure, staff);
                                          visible_items_set.insert(p);
                                          }
                                    }
                              }

                        //Handle last measure in mmRest
                        for (int staff = 0; staff < _score->staves().length(); staff++) {
                              if (!_score->staff(staff)->show())
                                    continue;
                              QRectF stave_rect = QRectF(system->canvasBoundingRect().left(),
                                                         system->staffCanvasYpage(staff),
                                                         system->width(),
                                                         system->staff(staff)->bbox().height());
                              QRectF show_rect = mmrest_measure->canvasBoundingRect().intersected(stave_rect);

                              if (canvas.intersects(show_rect)) {
                                    std::pair<Measure*, int> p(curr_measure, staff);
                                    visible_items_set.insert(p);
                                    }
                              }
                        continue;
                        }

                  if (!system)
                        continue;

                  for (int staff = 0; staff < _score->staves().length(); staff++) {
                        if (!_score->staff(staff)->show())
                              continue;
                        QRectF stave_rect = QRectF(system->canvasBoundingRect().left(),
                                                   system->staffCanvasYpage(staff),
                                                   system->width(),
                                                   system->staff(staff)->bbox().height());
                        QRectF show_rect = curr_measure->canvasBoundingRect().intersected(stave_rect);

                        if (canvas.intersects(show_rect)) {
                              std::pair<Measure*, int> p(curr_measure, staff);
                              visible_items_set.insert(p);
                              }
                        }
                  }

            //Find respective visible elements in timeline
            QPainterPath visible_painter_path = QPainterPath();
            visible_painter_path.setFillRule(Qt::WindingFill);
            for (QGraphicsItem* graphics_item : scene()->items()) {
                  int stave = graphics_item->data(0).value<int>();
                  Measure* measure = static_cast<Measure*>(graphics_item->data(2).value<void*>());
                  if (numToStaff(stave) && !numToStaff(stave)->show())
                        continue;

                  std::pair<Measure*, int> tmp(measure, stave);
                  std::set<std::pair<Measure*, int>>::iterator it;
                  it = visible_items_set.find(tmp);
                  if (it != visible_items_set.end())
                        visible_painter_path.addRect(graphics_item->boundingRect());
                  }

            QPainterPath non_visible_painter_path = QPainterPath();
            non_visible_painter_path.setFillRule(Qt::WindingFill);

            QRectF timeline_rect = QRectF(0, 0, getWidth(), getHeight());
            non_visible_painter_path.addRect(timeline_rect);

            non_visible_painter_path = non_visible_painter_path.subtracted(visible_painter_path);

            QGraphicsPathItem* non_visible_path_item = new QGraphicsPathItem(non_visible_painter_path.simplified());

            QPen non_visible_pen = QPen(QColor(100, 150, 250));
            QBrush non_visible_brush = QBrush(QColor(192, 192, 192, 180));
            non_visible_path_item->setPen(QPen(non_visible_brush.color()));
            non_visible_path_item->setBrush(non_visible_brush);
            non_visible_path_item->setZValue(-3);

            QGraphicsPathItem* visible = new QGraphicsPathItem(visible_painter_path.simplified());
            visible->setPen(non_visible_pen);
            visible->setBrush(Qt::NoBrush);
            visible->setZValue(-2);

            //Find old path, remove it
            for (QGraphicsItem* graphics_item : scene()->items()) {
                  if (graphics_item->type() == QGraphicsPathItem().type()) {
                        QGraphicsPathItem* old_path_item = static_cast<QGraphicsPathItem*>(graphics_item);
                        QBrush old_brush = old_path_item->brush();
                        QPen old_pen = old_path_item->pen();
                        if (old_brush == non_visible_brush || old_pen == non_visible_pen)
                              scene()->removeItem(old_path_item);
                        }
                  }

            scene()->addItem(non_visible_path_item);
            scene()->addItem(visible);
            }
      }

//---------------------------------------------------------
//   nstaves
//---------------------------------------------------------

int Timeline::nstaves()
      {
      return _score->staves().size();
      }

//---------------------------------------------------------
//   colorBox
//---------------------------------------------------------

QColor Timeline::colorBox(QGraphicsRectItem* item)
      {
      Measure* measure = static_cast<Measure*>(item->data(2).value<void*>());
      int stave = item->data(0).value<int>();
      for (Segment* seg = measure->first(); seg; seg = seg->next()) {
            if (!seg->isChordRestType())
                  continue;
            for (int track = stave * VOICES; track < stave * VOICES + VOICES; track++) {
                  ChordRest* chord_rest = seg->cr(track);
                  if (chord_rest) {
                        if (chord_rest->type() == ElementType::CHORD)
                              return QColor(Qt::gray);
                        }
                  }
            }
      return QColor(224,224,224);
      }

//---------------------------------------------------------
//   getLabels
//---------------------------------------------------------

std::vector<std::pair<QString, bool>> Timeline::getLabels()
      {
      if (!_score) {
            std::vector<std::pair<QString, bool>> no_labels;
            return no_labels;
            }
      QList<Part*> part_list = _score->parts();
      //transfer them into a vector of qstrings and then add the meta row names
      std::vector<std::pair<QString, bool>> row_labels;
      if (collapsed_meta) {
            std::pair<QString, bool> first("", true);
            std::pair<QString, bool> second(tr("Measures"), true);
            row_labels.push_back(first);
            row_labels.push_back(second);
            }
      else {
            for (auto it = metas.begin(); it != metas.end(); ++it) {
                  std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> meta = *it;
                  if (!std::get<2>(meta))
                        continue;
                  std::pair<QString, bool> meta_label(std::get<0>(meta), true);
                  row_labels.push_back(meta_label);
                  }
            }

      for (int stave = 0; stave < part_list.size(); stave++) {
            QTextDocument doc;
            QString part_name = "";
            doc.setHtml(part_list.at(stave)->longName());
            part_name = doc.toPlainText();

            if (part_name.isEmpty())
                  part_name = part_list.at(stave)->instrumentName();
            std::pair<QString, bool> instrument_label(part_name, part_list.at(stave)->show());
            row_labels.push_back(instrument_label);
            }
      return row_labels;
      }

//---------------------------------------------------------
//   handle_scroll
//---------------------------------------------------------

void Timeline::handle_scroll(int value)
      {
      if (!_score)
            return;
      for (std::vector<std::pair<QGraphicsItem*, int>>::iterator it = meta_rows.begin();
           it != meta_rows.end(); ++it) {
            std::pair<QGraphicsItem*, int> pair_graphics_int = *it;

            QGraphicsItem* graphics_item = pair_graphics_int.first;
            QGraphicsRectItem* graphics_rect_item = qgraphicsitem_cast<QGraphicsRectItem*>(graphics_item);
            QGraphicsLineItem* graphics_line_item = qgraphicsitem_cast<QGraphicsLineItem*>(graphics_item);
            QGraphicsPixmapItem* graphics_pixmap_item = qgraphicsitem_cast<QGraphicsPixmapItem*>(graphics_item);

            int row_y = pair_graphics_int.second * grid_height;
            int scrollbar_value = value;

            if (graphics_rect_item) {
                  QRectF rectf = graphics_rect_item->rect();
                  rectf.setY(qreal(scrollbar_value + row_y));
                  rectf.setHeight(grid_height);
                  graphics_rect_item->setRect(rectf);
                  }
            else if (graphics_line_item) {
                  QLineF linef = graphics_line_item->line();
                  linef.setLine(linef.x1(), row_y + scrollbar_value + 1, linef.x2(), row_y + scrollbar_value + 1);
                  graphics_line_item->setLine(linef);
                  }
            else if (graphics_pixmap_item)
                  graphics_pixmap_item->setY(qreal(scrollbar_value + row_y + 3));
            else
                  graphics_item->setY(qreal(scrollbar_value + row_y));
            }
      viewport()->update();
      }

//---------------------------------------------------------
//   mouseOver
//---------------------------------------------------------

void Timeline::mouseOver(QPointF pos)
      {
      //Choose item with the largest original Z value...
      QList<QGraphicsItem*> graphics_list = scene()->items(pos);
      QGraphicsItem* hovered_graphics_item = 0;
      int max_z_value = -1;
      for (QGraphicsItem* curr_graphics_item: graphics_list) {
            if (qgraphicsitem_cast<QGraphicsTextItem*>(curr_graphics_item))
                  continue;
            if (curr_graphics_item->zValue() >= max_z_value && curr_graphics_item->zValue() < global_z_value) {
                  hovered_graphics_item = curr_graphics_item;
                  max_z_value = hovered_graphics_item->zValue();
                  }
            else if (curr_graphics_item->zValue() > global_z_value && std::get<1>(old_hover_info) >= max_z_value){
                  hovered_graphics_item = curr_graphics_item;
                  max_z_value = std::get<1>(old_hover_info);
                  }
            }

      if (!hovered_graphics_item) {
            if (std::get<0>(old_hover_info)) {
                  std::get<0>(old_hover_info)->setZValue(std::get<1>(old_hover_info));
                  static_cast<QGraphicsItem*>(std::get<0>(old_hover_info)->data(5).value<void*>())->setZValue(std::get<1>(old_hover_info));
                  QGraphicsRectItem* graphics_rect_item1 = qgraphicsitem_cast<QGraphicsRectItem*>(std::get<0>(old_hover_info));
                  QGraphicsRectItem* graphics_rect_item2 = qgraphicsitem_cast<QGraphicsRectItem*>(static_cast<QGraphicsItem*>(std::get<0>(old_hover_info)->data(5).value<void*>()));
                  if (graphics_rect_item1)
                        graphics_rect_item1->setBrush(QBrush(std::get<2>(old_hover_info)));
                  if (graphics_rect_item2)
                        graphics_rect_item2->setBrush(QBrush(std::get<2>(old_hover_info)));
                  std::get<0>(old_hover_info) = nullptr;
                  std::get<1>(old_hover_info) = -1;
                  }
            return;
            }
      QGraphicsItem* pair_item = static_cast<QGraphicsItem*>(hovered_graphics_item->data(5).value<void*>());
      if (!pair_item) {
            if (std::get<0>(old_hover_info)) {
                  std::get<0>(old_hover_info)->setZValue(std::get<1>(old_hover_info));
                  static_cast<QGraphicsItem*>(std::get<0>(old_hover_info)->data(5).value<void*>())->setZValue(std::get<1>(old_hover_info));
                  QGraphicsRectItem* graphics_rect_item1 = qgraphicsitem_cast<QGraphicsRectItem*>(std::get<0>(old_hover_info));
                  QGraphicsRectItem* graphics_rect_item2 = qgraphicsitem_cast<QGraphicsRectItem*>(static_cast<QGraphicsItem*>(std::get<0>(old_hover_info)->data(5).value<void*>()));
                  if (graphics_rect_item1)
                        graphics_rect_item1->setBrush(QBrush(std::get<2>(old_hover_info)));
                  if (graphics_rect_item2)
                        graphics_rect_item2->setBrush(QBrush(std::get<2>(old_hover_info)));
                  std::get<0>(old_hover_info) = nullptr;
                  std::get<1>(old_hover_info) = -1;
                  }
            return;
            }

      if (std::get<0>(old_hover_info) == hovered_graphics_item)
            return;

      if (std::get<0>(old_hover_info)) {
            std::get<0>(old_hover_info)->setZValue(std::get<1>(old_hover_info));
            static_cast<QGraphicsItem*>(std::get<0>(old_hover_info)->data(5).value<void*>())->setZValue(std::get<1>(old_hover_info));
            QGraphicsRectItem* graphics_rect_item1 = qgraphicsitem_cast<QGraphicsRectItem*>(std::get<0>(old_hover_info));
            QGraphicsRectItem* graphics_rect_item2 = qgraphicsitem_cast<QGraphicsRectItem*>(static_cast<QGraphicsItem*>(std::get<0>(old_hover_info)->data(5).value<void*>()));
            if (graphics_rect_item1)
                  graphics_rect_item1->setBrush(QBrush(std::get<2>(old_hover_info)));
            if (graphics_rect_item2)
                  graphics_rect_item2->setBrush(QBrush(std::get<2>(old_hover_info)));

            std::get<0>(old_hover_info) = nullptr;
            std::get<1>(old_hover_info) = -1;
            }

      std::get<1>(old_hover_info) = hovered_graphics_item->zValue();
      std::get<0>(old_hover_info) = hovered_graphics_item;

      //Give items the top z value
      hovered_graphics_item->setZValue(global_z_value + 1);
      pair_item->setZValue(global_z_value + 1);

      QGraphicsRectItem* graphics_rect_item1 = qgraphicsitem_cast<QGraphicsRectItem*>(hovered_graphics_item);
      QGraphicsRectItem* graphics_rect_item2 = qgraphicsitem_cast<QGraphicsRectItem*>(pair_item);
      if (graphics_rect_item1) {
            std::get<2>(old_hover_info) = graphics_rect_item1->brush().color();
            if (std::get<2>(old_hover_info) != QColor(173,216,230))
                  graphics_rect_item1->setBrush(QBrush(Qt::lightGray));
            }
      if (graphics_rect_item2) {
            std::get<2>(old_hover_info) = graphics_rect_item2->brush().color();
            if (std::get<2>(old_hover_info) != QColor(173,216,230))
                  graphics_rect_item2->setBrush(QBrush(Qt::lightGray));
            }
      }

//---------------------------------------------------------
//   swapMeta
//---------------------------------------------------------

void Timeline::swapMeta(unsigned int row, bool switch_up)
      {
      //Attempt to switch row up or down, skipping non visible rows
      if (switch_up && row != 0) {
            //traverse backwards until visible one is found
            auto swap = metas.begin() + correctMetaRow(row) - 1;
            while (!std::get<2>(*swap))
                  swap--;
            iter_swap(metas.begin() + correctMetaRow(row), swap);
            }
      else if (!switch_up && row != nmetas() - 2) {
            //traverse forwards until visible one is found
            auto swap = metas.begin() + correctMetaRow(row) + 1;
            while (!std::get<2>(*swap))
                  swap++;
            iter_swap(metas.begin() + correctMetaRow(row), swap);
            }

      updateGrid();
      }

//---------------------------------------------------------
//   numToStaff
//---------------------------------------------------------

Staff* Timeline::numToStaff(int staff)
      {
      if (!_score)
            return 0;
      QList<Staff*> staves = _score->staves();
      if (staves.size() > staff && staff >= 0)
            return staves.at(staff);
      else
            return 0;
      }

//---------------------------------------------------------
//   toggleShow
//---------------------------------------------------------

void Timeline::toggleShow(int staff)
      {
      if (!_score)
            return;
      QList<Part*> parts = _score->parts();
      if (parts.size() > staff && staff >= 0) {
            parts.at(staff)->setShow(!parts.at(staff)->show());
            parts.at(staff)->undoChangeProperty(Pid::VISIBLE, parts.at(staff)->show());
            _score->masterScore()->setLayoutAll();
            _score->masterScore()->update();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   showContextMenu
//---------------------------------------------------------

void Timeline::contextMenuEvent(QContextMenuEvent*)
      {
      QMenu* context_menu = new QMenu(tr("Context menu"), this);
      if (row_names->cursorIsOn() == "instrument") {
            QAction* edit_instruments = new QAction(tr("Edit Instruments"), this);
            connect(edit_instruments, SIGNAL(triggered()), this, SLOT(requestInstrumentDialog()));
            context_menu->addAction(edit_instruments);
            context_menu->exec(QCursor::pos());
            }
      else if (row_names->cursorIsOn() == "meta" || cursorIsOn() == "meta") {
            for (auto it = metas.begin(); it != metas.end(); ++it) {
                  std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> meta = *it;
                  QString row_name = std::get<0>(meta);
                  if (row_name != tr("Measures")) {
                        QAction* action = new QAction(row_name, this);
                        action->setCheckable(true);
                        action->setChecked(std::get<2>(meta));
                        connect(action, SIGNAL(triggered()), this, SLOT(toggleMetaRow()));
                        context_menu->addAction(action);
                        }
                  }
            context_menu->addSeparator();
            QAction* hide_all = new QAction(tr("Hide all"), this);
            connect(hide_all, SIGNAL(triggered()), this, SLOT(toggleMetaRow()));
            context_menu->addAction(hide_all);
            QAction* show_all = new QAction(tr("Show all"), this);
            connect(show_all, SIGNAL(triggered()), this, SLOT(toggleMetaRow()));
            context_menu->addAction(show_all);
            context_menu->exec(QCursor::pos());
            }
      }

//---------------------------------------------------------
//   toggleMetaRow
//---------------------------------------------------------

void Timeline::toggleMetaRow()
      {
      QAction* action = qobject_cast<QAction*>(QObject::sender());
      if (action) {
            QString target_text = action->text();

            if (target_text == tr("Hide all")) {
                  for (auto it = metas.begin(); it != metas.end(); ++it) {
                        QString meta_text = std::get<0>(*it);
                        if (meta_text != tr("Measures"))
                              std::get<2>(*it) = false;
                        }
                  updateGrid();
                  return;
                  }
            else if (target_text == tr("Show all")) {
                  for (auto it = metas.begin(); it != metas.end(); ++it)
                        std::get<2>(*it) = true;
                  updateGrid();
                  return;
                  }

            bool checked = action->isChecked();
            //Find target text in metas and toggle visibility to the checked status of action
            for (auto it = metas.begin(); it != metas.end(); ++it) {
                  QString meta_text = std::get<0>(*it);
                  if (meta_text == target_text) {
                        std::get<2>(*it) = checked;
                        updateGrid();
                        break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   nmetas
//---------------------------------------------------------

unsigned int Timeline::nmetas()
      {
      unsigned int total = 0;
      if (collapsed_meta)
            return 2;
      for (auto it = metas.begin(); it != metas.end(); ++it) {
            std::tuple<QString, void (Timeline::*)(Segment*, int*, int), bool> meta = *it;
            if (std::get<2>(meta))
                  total++;
            }
      return total;
      }

//---------------------------------------------------------
//   correctMetaRow
//---------------------------------------------------------

unsigned int Timeline::correctMetaRow(unsigned int row)
      {
      unsigned int count = 0;
      auto it = metas.begin();
      while (row >= count) {
            if (!std::get<2>(*it))
                  row++;
            count++;
            ++it;
            }
      return row;
      }

//---------------------------------------------------------
//   correctMetaRow
//---------------------------------------------------------

QString Timeline::cursorIsOn()
      {
      QPointF scene_pos = mapToScene(mapFromGlobal(QCursor::pos()));
      QGraphicsItem* graphics_item = scene()->itemAt(scene_pos, transform());
      if (graphics_item) {
            auto it = meta_rows.begin();
            for (;it != meta_rows.end(); ++it) {
                  if ((*it).first == graphics_item)
                        break;
                  }
            if (it != meta_rows.end())
                  return "meta";
            else {
                  QList<QGraphicsItem*> graphics_item_list = scene()->items(scene_pos);
                  for (QGraphicsItem* curr_graphics_item : graphics_item_list) {
                        Measure* curr_measure = static_cast<Measure*>(curr_graphics_item->data(2).value<void*>());
                        int stave = curr_graphics_item->data(0).value<int>();
                        if (curr_measure && !numToStaff(stave)->show())
                              return "invalid";
                        }
                  return "instrument";
                  }
            }
      else
            return "";
      }

//---------------------------------------------------------
//   requestInstrumentDialog
//---------------------------------------------------------

void Timeline::requestInstrumentDialog()
      {
      QAction* act = getAction("instruments");
      mscore->cmd(act);
      if (mscore->getMixer())
            mscore->getMixer()->updateAll(_score->masterScore());
      }

}
