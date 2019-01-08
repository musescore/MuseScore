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

#ifndef __TIMELINE_H__
#define __TIMELINE_H__

#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "scoreview.h"
#include "timelinemeta.h"
#include "timelinedata.h"

namespace Ms {

class TimelineMeta;
class TimelineData;

//---------------------------------------------------------
//   Timeline
//---------------------------------------------------------

class Timeline : public QDockWidget {
      Q_OBJECT

      QSplitter* _topBottom;
      Score* _score;
      ScoreView* _scoreView;

      // Sizing for all timeline elements
      int _cellHeight = 10;
      int _cellWidth = 10;
      const int _minCellWidth = 3;
      const int _maxCellWidth = 25;

      int determineHeight();
      void changeWidth(int change);

      virtual void closeEvent(QCloseEvent* event);
      void configureMetaAndDataWidgets();

   signals:
      void closed(bool);

   private slots:
      void updateSliders();

   public:
      Timeline(QWidget* parent = 0);
      void updateTimeline();
      void updateSelection();

      void setScore(Score* score);
      Score* score() { return _score; }
      void setScoreView(ScoreView* scoreView);
      ScoreView* scoreView() { return _scoreView; }

      TimelineMeta* metaWidget() { return (TimelineMeta*)_topBottom->widget(0); }
      TimelineData* dataWidget() { return (TimelineData*)_topBottom->widget(1); }

      int cellHeight() { return _cellHeight; }
      int cellWidth() { return _cellWidth; }
      int minCellWidth() { return _minCellWidth; }
      int maxCellWidth() { return _maxCellWidth; }

      QFont getFont();

      virtual void wheelEvent(QWheelEvent *event);
      };

} // namespace Ms

#endif
