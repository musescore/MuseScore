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
#include "libmscore/instrtemplate.h"
#include "libmscore/measure.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/text.h"

#include "musescore.h"
#include "../qmlpluginengine.h"

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
///   \brief Adds a header text to the score.
///   \param type One of the following values:
///   - "title"
///   - "subtitle"
///   - "composer"
///   - "lyricist"
///   - Any other value corresponds to default text style.
///   \param txt Text to be added.
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
//   defaultInstrTemplate
//---------------------------------------------------------

static const InstrumentTemplate* defaultInstrTemplate()
      {
      static InstrumentTemplate defaultInstrument;
      if (defaultInstrument.channel.empty()) {
            Channel a;
            a.setChorus(0);
            a.setReverb(0);
            a.setName(Channel::DEFAULT_NAME);
            a.setBank(0);
            a.setVolume(90);
            a.setPan(0);
            defaultInstrument.channel.append(a);
            }
      return &defaultInstrument;
      }

//---------------------------------------------------------
//   instrTemplateFromName
//---------------------------------------------------------

const InstrumentTemplate* Score::instrTemplateFromName(const QString& name)
      {
      const InstrumentTemplate* t = searchTemplate(name);
      if (!t) {
            qWarning("<%s> not found", qPrintable(name));
            t = defaultInstrTemplate();
            }
      return t;
      }

//---------------------------------------------------------
//   Score::appendPart
//---------------------------------------------------------

void Score::appendPart(const QString& instrumentId)
      {
      const InstrumentTemplate* t = searchTemplate(instrumentId);

      if (!t) {
            qWarning("appendPart: <%s> not found", qPrintable(instrumentId));
            t = defaultInstrTemplate();
            }

      score()->appendPart(t);
      }

//---------------------------------------------------------
//   Score::appendPartByMusicXmlId
//---------------------------------------------------------

void Score::appendPartByMusicXmlId(const QString& instrumentMusicXmlId)
      {
      const InstrumentTemplate* t = searchTemplateForMusicXmlId(instrumentMusicXmlId);

      if (!t) {
            qWarning("appendPart: <%s> not found", qPrintable(instrumentMusicXmlId));
            t = defaultInstrTemplate();
            }

      score()->appendPart(t);
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
//   Score::firstMeasureMM
//---------------------------------------------------------

Measure* Score::firstMeasureMM()
      {
      return wrap<Measure>(score()->firstMeasureMM(), Ownership::SCORE);
      }

//---------------------------------------------------------
//   Score::lastMeasure
//---------------------------------------------------------

Measure* Score::lastMeasure()
      {
      return wrap<Measure>(score()->lastMeasure(), Ownership::SCORE);
      }
//---------------------------------------------------------
//   Score::firstMeasureMM
//---------------------------------------------------------

Measure* Score::lastMeasureMM()
      {
      return wrap<Measure>(score()->lastMeasureMM(), Ownership::SCORE);
      }

//---------------------------------------------------------
//   Score::staves
//---------------------------------------------------------

QQmlListProperty<Staff> Score::staves()
      {
      return wrapContainerProperty<Staff>(this, score()->staves());
      }

//---------------------------------------------------------
//   Score::startCmd
//---------------------------------------------------------

void Score::startCmd()
      {
      // TODO: should better use qmlEngine(this) (need to set context for wrappers then)
      const QmlPluginEngine* engine = mscore->getPluginEngine();
      if (engine->inScoreChangeActionHandler()) {
            // Plugin-originated changes made while handling onScoreStateChanged
            // should be grouped together with the action which caused this change
            // (if it was caused by actual score change).
            if (!score()->undoStack()->active())
                  score()->undoStack()->reopen();
            }
      else {
            score()->startCmd();
            }
      }
}
}
