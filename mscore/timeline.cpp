//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "timeline.h"
#include "musescore.h"
#include "tourhandler.h"
#include "preferences.h"

namespace Ms {

//---------------------------------------------------------
//   showTimeline
//---------------------------------------------------------

void MuseScore::showTimeline(bool visible)
      {
      QAction* act = getAction("toggle-timeline");
      if (!_timeline) {
            _timeline = new Timeline(this);
            _timeline->setScore(0);
            _timeline->setScoreView(cv);
            }
      connect(_timeline, SIGNAL(visibilityChanged(bool)), act, SLOT(setChecked(bool)));
      connect(_timeline, SIGNAL(closed(bool)), act, SLOT(setChecked(bool)));
      _timeline->setVisible(visible);

      getAction("toggle-timeline")->setChecked(visible);
      if (visible)
            TourHandler::startTour("timeline-tour");
      }

//---------------------------------------------------------
//   Timeline
//---------------------------------------------------------

Timeline::Timeline(QWidget* parent)
  : QDockWidget(parent)
      {
      configureMetaAndDataWidgets();

      setWindowTitle(tr("Timeline"));
      setObjectName("Timeline");
      }

//---------------------------------------------------------
//   configureMetaAndDataWidgets
//---------------------------------------------------------

void Timeline::configureMetaAndDataWidgets()
      {
      _topBottom = new QSplitter(Qt::Vertical, this);
      TimelineMeta* meta = new TimelineMeta(this);
      TimelineData* data = new TimelineData(this);
      _topBottom->addWidget(meta);
      _topBottom->addWidget(data);

      _topBottom->setCollapsible(1, false);

      connect(metaWidget(), SIGNAL(splitterMoved(int, int)), dataWidget(), SLOT(metaSplitterMoved()));
      connect(dataWidget(), SIGNAL(splitterMoved(int, int)), metaWidget(), SLOT(dataSplitterMoved()));

      connect(metaWidget()->rowsView()->horizontalScrollBar(), SIGNAL(valueChanged(int)),
              dataWidget()->gridView()->horizontalScrollBar(), SLOT(setValue(int)));
      connect(dataWidget()->gridView()->horizontalScrollBar(), SIGNAL(valueChanged(int)),
              metaWidget()->rowsView()->horizontalScrollBar(), SLOT(setValue(int)));

      connect(this, SIGNAL(visibilityChanged(bool)), SLOT(updateSliders()));
      setWidget(_topBottom);
      }

//---------------------------------------------------------
//   updateSliders
//---------------------------------------------------------

void Timeline::updateSliders()
      {
      metaWidget()->setSizes(dataWidget()->sizes());
      }

//---------------------------------------------------------
//   updateTimeline
//---------------------------------------------------------

void Timeline::updateTimeline()
      {
      setCellHeight(determineHeight());
      metaWidget()->updateMeta();
      dataWidget()->updateData();

      int maxLabelWidth = qMax(metaWidget()->labelView()->maximumTextWidth(),
                               dataWidget()->labelView()->maximumTextWidth());
      metaWidget()->labelView()->setMaximumWidth(maxLabelWidth);
      dataWidget()->labelView()->setMaximumWidth(maxLabelWidth);
      }

//---------------------------------------------------------
//   updateSelection
//---------------------------------------------------------

void Timeline::updateSelection()
      {
      metaWidget()->rowsView()->updateSelection();
      dataWidget()->gridView()->updateSelection();
      }

//---------------------------------------------------------
//   updateScoreView
//---------------------------------------------------------

void Timeline::updateScoreView()
      {
      connect(scoreView(), SIGNAL(viewRectChanged()), dataWidget()->gridView(), SLOT(updateView()));
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void Timeline::closeEvent(QCloseEvent* event)
      {
      emit closed(false);
      QWidget::closeEvent(event);
      }

//---------------------------------------------------------
//   determineHeight
//   Use the current font size to determine the cell height
//---------------------------------------------------------

int Timeline::determineHeight()
      {
      QFontMetrics fontMetric = QFontMetrics(getFont());

      int fontHeight = fontMetric.height();

      // the margin above and below the text
      return fontHeight + 2;
      }

//---------------------------------------------------------
//   changeWidth
//---------------------------------------------------------

void Timeline::changeWidth(int newWidth)
      {
      setCellWidth(cellWidth() + newWidth);
      if (cellWidth() < minCellWidth())
            setCellWidth(minCellWidth());
      if (cellWidth() > maxCellWidth())
            setCellWidth(maxCellWidth());
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void Timeline::wheelEvent(QWheelEvent* event)
      {
      if (event->modifiers().testFlag(Qt::ControlModifier)) {
            // Store original position values to adjust the scrollbar later
            int originalScrollBarValue = dataWidget()->gridView()->scrollBarValue();
            QPoint globalCursorPos = event->globalPos();
            int originalGridWidth = dataWidget()->gridView()->gridBounds().width();

            if (event->angleDelta().y() > 0)
                  changeWidth(1);
            else
                  changeWidth(-1);

            metaWidget()->rowsView()->redrawRows();
            dataWidget()->gridView()->redrawGrid();

            // Attempt to keep mouse in original spot
            dataWidget()->gridView()->adjustScrollBar(originalScrollBarValue, globalCursorPos, originalGridWidth);
            }
      else if (event->modifiers().testFlag(Qt::ShiftModifier)) {
            qreal num_of_steps = qreal(event->angleDelta().y()) / 2;
            dataWidget()->gridView()->setScrollBarValue(dataWidget()->gridView()->scrollBarValue() - int(num_of_steps));
            }
      else
            QDockWidget::wheelEvent(event);
      }

}
