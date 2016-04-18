//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "accidental.h"
#include "barline.h"
#include "beam.h"
#include "box.h"
#include "chord.h"
#include "clef.h"
#include "element.h"
#include "fingering.h"
#include "glissando.h"
#include "harmony.h"
#include "key.h"
#include "keysig.h"
#include "layoutbreak.h"
#include "layout.h"
#include "lyrics.h"
#include "marker.h"
#include "measure.h"
#include "mscore.h"
#include "notedot.h"
#include "note.h"
#include "ottava.h"
#include "page.h"
#include "part.h"
#include "repeat.h"
#include "score.h"
#include "segment.h"
#include "sig.h"
#include "slur.h"
#include "staff.h"
#include "stem.h"
#include "style.h"
#include "sym.h"
#include "system.h"
#include "text.h"
#include "tie.h"
#include "timesig.h"
#include "tremolo.h"
#include "tuplet.h"
#include "undo.h"
#include "utils.h"
#include "volta.h"
#include "breath.h"
#include "tempotext.h"
#include "systemdivider.h"

namespace Ms {

//---------------------------------------------------------
//   layoutLinear
//---------------------------------------------------------

void Score::layoutLinear(LayoutContext& lc)
      {
      System* system = getNextSystem(lc);
      system->setInstrumentNames(true);

      qreal xo;
      if (lc.curMeasure->isHBox())
            xo = point(toHBox(lc.curMeasure)->boxWidth());
      else
            xo = 0;
      system->layoutSystem(xo);

      system->setPos(0.0, spatium() * 10.0);
      Page* page = getEmptyPage(lc);
      page->appendSystem(system);

      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            Element::Type t = mb->type();
            if (t == Element::Type::VBOX || t == Element::Type::TBOX || t == Element::Type::FBOX)
                  continue;
            if (styleB(StyleIdx::createMultiMeasureRests) && mb->type() == Element::Type::MEASURE) {
                  Measure* m = static_cast<Measure*>(mb);
                  if (m->hasMMRest())
                        mb = m->mmRest();
                  }
            mb->setSystem(system);
            system->measures().push_back(mb);
            }
      if (system->measures().empty())
            return;
      addSystemHeader(firstMeasureMM(), true);
      // also add a system header after a section break
      for (Measure* m = firstMeasureMM(); m; m = m->nextMeasureMM()) {
            if (m->sectionBreak() && m->nextMeasureMM())
                  addSystemHeader(m->nextMeasureMM(), true);
            }

      QPointF pos(0.0, 0.0);
      bool isFirstMeasure   = true;
      qreal minMeasureWidth = point(styleS(StyleIdx::minMeasureWidth));

      foreach (MeasureBase* mb, system->measures()) {
            qreal w = 0.0;
            if (mb->type() == Element::Type::MEASURE) {
                  Measure* m  = static_cast<Measure*>(mb);
                  Measure* nm = m->nextMeasure();
                  bool lastMeasureInSystem = nm == 0;
                  m->createEndBarLines(lastMeasureInSystem);
                  if (isFirstMeasure) {
                        pos.rx() += system->leftMargin();
                        // width with header
                        qreal w2 = computeMinWidth(m->first(), isFirstMeasure);
                        // width *completely* excluding header
                        // minWidth1() includes the initial key / time signatures since they are considered non-generated
                        Segment* s = m->first();
                        while (s && s->segmentType() != Segment::Type::ChordRest)
                              s = s->next();
                        qreal w1 = s ? computeMinWidth(s, false) : m->minWidth1();
                        w = (w2 - w1) + w1 * styleD(StyleIdx::linearStretch);
                        }
                  else {
                        m->removeSystemHeader();
                        w = computeMinWidth(m->first(), false) * styleD(StyleIdx::linearStretch);
                        }
                  if (w < minMeasureWidth)
                        w = minMeasureWidth;
                  m->stretchMeasure(w);
                  isFirstMeasure = false;
                  }
            else {
                  mb->layout();
                  w = mb->width();
                  }

            mb->setPos(pos);
            pos.rx() += w;
            }
      system->setWidth(pos.x());
      page->setWidth(pos.x());
      system->layout2();

      for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure())
                  continue;
            Measure* m = toMeasure(mb);
            m->layout2();
            }

      page->setHeight(system->height() + 20 * spatium());
      page->rebuildBspTree();
      }

} // namespace Ms


