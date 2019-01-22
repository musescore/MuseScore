//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "cursor.h"
#include "elements.h"
#include "libmscore/measure.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/text.h"

namespace Ms {
namespace PluginAPI {

//---------------------------------------------------------
//   Score::newCursor
//---------------------------------------------------------

Cursor* Score::newCursor()
      {
      return new Cursor(score());
      }

//---------------------------------------------------------
//   Score::addText
//---------------------------------------------------------

void Score::addText(const QString& type, const QString& txt)
      {
      MeasureBase* measure = score()->first();
      if (!measure || !measure->isVBox()) {
            score()->insertMeasure(ElementType::VBOX, measure);
            measure = score()->first();
            }
      Tid tid = Tid::DEFAULT;
      if (type == "title")
            tid = Tid::TITLE;
      else if (type == "subtitle")
            tid = Tid::SUBTITLE;
      else if (type == "composer")
            tid = Tid::COMPOSER;
      else if (type == "lyricist")
            tid = Tid::POET;

      Ms::Text* text = new Ms::Text(score(), tid);
      text->setParent(measure);
      text->setXmlText(txt);
      score()->undoAddElement(text);
      }

//---------------------------------------------------------
//   Score::firstSegment
//---------------------------------------------------------

Segment* Score::firstSegment()
      {
      return wrap<Segment>(score()->firstSegment(Ms::SegmentType::All), Ownership::SCORE);
      }

//---------------------------------------------------------
//   Score::lastSegment
//---------------------------------------------------------

Segment* Score::lastSegment()
      {
      return wrap<Segment>(score()->lastSegment(), Ownership::SCORE);
      }

//---------------------------------------------------------
//   Score::firstMeasure
//---------------------------------------------------------

Measure* Score::firstMeasure()
      {
      return wrap<Measure>(score()->firstMeasure(), Ownership::SCORE);
      }

//---------------------------------------------------------
//   Score::lastMeasure
//---------------------------------------------------------

Measure* Score::lastMeasure()
      {
      return wrap<Measure>(score()->lastMeasure(), Ownership::SCORE);
      }
}
}
