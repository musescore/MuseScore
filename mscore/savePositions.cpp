//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/repeatlist.h"
#include "libmscore/system.h"
#include "libmscore/xml.h"
#include "mscore/globals.h"
#include "mscore/preferences.h"
#include "mscore/musescore.h"

namespace Ms {

static QHash<void*, int> segs;

//---------------------------------------------------------
//   saveMeasureEvents
//---------------------------------------------------------

static void saveMeasureEvents(XmlWriter& xml, Measure* m, int offset)
      {
      for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            int tick = s->tick().ticks() + offset;
            int id = segs[(void*)s];
            int time = lrint(m->score()->repeatList().utick2utime(tick) * 1000);
            xml.tagE(QString("event elid=\"%1\" position=\"%2\"")
               .arg(id)
               .arg(time)
               );
            }
      }

//---------------------------------------------------------
//   savePositions
//---------------------------------------------------------

bool MuseScore::savePositions(Score* score, QIODevice* device, bool segments, bool legacyExport)
      {
      QString element;
      if (legacyExport)
            element = QString("element id=\"%1\" x=\"%2\" y=\"%3\" sx=\"%4\" sy=\"%5\" page=\"%6\"");
      else
            element = QString("element id=\"%1\" x=\"%2\" y=\"%3\" w=\"%4\" h=\"%5\" page=\"%6\"");

      qreal ndpi = (qreal) preferences.getDouble(PREF_EXPORT_PNG_RESOLUTION) / DPI;
      if (legacyExport)
            ndpi *= 12;

      QRectF pagebox = score->pages().front()->abbox();
      int pagewidth  = qCeil(pagebox.width() * ndpi);
      int pageheight = qCeil(pagebox.height() * ndpi);

      segs.clear();
      XmlWriter xml(score, device);
      xml.header();
      xml.stag(QString("score posformat=\"%1\" kind=\"%2\" width=\"%3\" height=\"%4\"")
               .arg(legacyExport ? 1 : 2)
               .arg(segments ? "segment" : "measure")
               .arg(pagewidth).arg(pageheight));
      xml.stag("elements");
      int id = 0;

      if (segments) {
            for (Segment* s = score->firstMeasureMM()->first(SegmentType::ChordRest);
               s; s = s->next1MM(SegmentType::ChordRest)) {
                  QRectF b = s->sposBBox();

                  // offset into the page, round down
                  int x    = qFloor(b.x() * ndpi);
                  int y    = qFloor(b.y() * ndpi);

                  // width/height, round up
                  int w    = qCeil(b.width() * ndpi);
                  int h    = qCeil(b.height() * ndpi);

                  Page* p  = s->measure()->system()->page();
                  int page = score->pageIdx(p);

                  xml.tagE(element
                     .arg(id)
                     .arg(x)
                     .arg(y)
                     .arg(w)
                     .arg(h)
                     .arg(page));

                  segs[(void*)s] = id++;
                  }
            xml.etag();
            }
      else {
            for (Measure* m = score->firstMeasureMM(); m; m = m->nextMeasureMM()) {
                  int x    = qFloor(m->pagePos().x() * ndpi);
                  int y    = qFloor(m->system()->pagePos().y() * ndpi);
                  int w    = qCeil(m->bbox().width() * ndpi);
                  int h    = qCeil(m->system()->height() * ndpi);

                  Page* p  = m->system()->page();
                  int page = score->pageIdx(p);

                  xml.tagE(element
                     .arg(id)
                     .arg(x)
                     .arg(y)
                     .arg(w)
                     .arg(h)
                     .arg(page));

                  segs[(void*)m] = id++;
                  }
            xml.etag();
            }

      xml.stag("events");
      score->masterScore()->setExpandRepeats(true);
      for (const RepeatSegment* rs : score->repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len();
            int tickOffset = rs->utick - rs->tick;
            for (Measure* m = score->tick2measureMM(Fraction::fromTicks(startTick)); m; m = m->nextMeasureMM()) {
                        if (segments)
                              saveMeasureEvents(xml, m, tickOffset);
                        else {
                              int tick = m->tick().ticks() + tickOffset;
                              int i = segs[(void*)m];
                              int time = lrint(m->score()->repeatList().utick2utime(tick) * 1000);
                              xml.tagE(QString("event elid=\"%1\" position=\"%2\"")
                                 .arg(i)
                                 .arg(time)
                                 );
                              }
                  if (m->endTick().ticks() >= endTick)
                        break;
                  }
            }
      xml.etag();

      xml.etag(); // score
      return true;
      }

bool MuseScore::savePositions(Score* score, const QString& name, bool segments, bool legacyExport)
      {
      QFile fp(name);
      if (!fp.open(QIODevice::WriteOnly)) {
            qDebug("Open <%s> failed", qPrintable(name));
            return false;
            }
      return savePositions(score, &fp, segments, legacyExport);
      }
}
