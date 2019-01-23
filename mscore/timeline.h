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
#include "timelineabstract.h"

namespace Ms {

class TimelineMeta;
class TimelineData;

//---------------------------------------------------------
//   Timeline
//---------------------------------------------------------

class Timeline : public QDockWidget, public TimelineComponent
      {
      Q_OBJECT

      QSplitter* _topBottom;

      int determineHeight();
      void changeWidth(int change);

      virtual void closeEvent(QCloseEvent* event);
      void configureMetaAndDataWidgets();

      void updateScore() { updateTimeline(); }
      void updateScoreView();

   signals:
      void closed(bool);

   private slots:
      void updateSliders();

   public:
      Timeline(QWidget* parent = 0);
      void updateTimeline();
      void updateSelection();

      TimelineMeta* metaWidget() { return (TimelineMeta*)_topBottom->widget(0); }
      TimelineData* dataWidget() { return (TimelineData*)_topBottom->widget(1); }

      virtual void wheelEvent(QWheelEvent *event);
      };

} // namespace Ms

#endif
