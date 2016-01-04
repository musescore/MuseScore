#if 0
//---------------------------------------------------------
//   layoutLinear
//---------------------------------------------------------

void Score::layoutLinear()
      {
      curMeasure     = first();
      System* system = getNextSystem(lc, false);
      system->setInstrumentNames(true);
      qreal xo = 0;

      Measure* fm = firstMeasure();
      for (MeasureBase* m = first(); m != fm ; m = m->next()) {
            if (m->type() == Element::Type::HBOX)
                  xo += point(static_cast<Box*>(m)->boxWidth());
            }

      system->layoutSystem(xo);
      system->setPos(0.0, spatium() * 10.0);
      Page* page = getEmptyPage();
      page->appendSystem(system);

      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            Element::Type t = mb->type();
            if (t == Element::Type::VBOX || t == Element::Type::TBOX || t == Element::Type::FBOX) {
                  continue;
                  }
            if (styleB(StyleIdx::createMultiMeasureRests) && mb->type() == Element::Type::MEASURE) {
                  Measure* m = static_cast<Measure*>(mb);
                  if (m->hasMMRest())
                        mb = m->mmRest();
                  }
            mb->setSystem(system);
            system->measures().append(mb);
            }
      if (system->measures().isEmpty())
            return;
      addSystemHeader(firstMeasureMM(), true);
      // also add a system header after a section break
      for (Measure* m = firstMeasureMM(); m; m = m->nextMeasureMM()) {
            if (m->sectionBreak() && m->nextMeasureMM())
                  addSystemHeader(m->nextMeasureMM(), true);
            }
      removeGeneratedElements(firstMeasureMM(), lastMeasureMM());

      QPointF pos(0.0, 0.0);
      bool isFirstMeasure = true;
      foreach (MeasureBase* mb, system->measures()) {
            qreal w = 0.0;
            if (mb->type() == Element::Type::MEASURE) {
                  if (isFirstMeasure)
                        pos.rx() += system->leftMargin();
                  Measure* m = static_cast<Measure*>(mb);
                  Measure* nm = m->nextMeasure();
                  if (m->repeatFlags() & Repeat::END) {
                        if (nm && (nm->repeatFlags() & Repeat::START))
                              m->setEndBarLineType(BarLineType::END_START_REPEAT, m->endBarLineGenerated());
                        else
                              m->setEndBarLineType(BarLineType::END_REPEAT, m->endBarLineGenerated());
                        }
                  else if (nm && (nm->repeatFlags() & Repeat::START))
                        m->setEndBarLineType(BarLineType::START_REPEAT, m->endBarLineGenerated());
                  m->createEndBarLines();
                  if (isFirstMeasure) {
                        // width with header
                        qreal w2 = computeMinWidth(m->first());
                        // width *completely* excluding header
                        // minWidth1() includes the initial key / time signatures since they are considered non-generated
                        Segment* s = m->first();
                        while (s && s->segmentType() != Segment::Type::ChordRest)
                              s = s->next();
                        qreal w1 = s ? computeMinWidth(s) : m->minWidth1();
                        w = (w2 - w1) + w1 * styleD(StyleIdx::linearStretch);
                        }
                  else {
                        w = m->minWidth1() * styleD(StyleIdx::linearStretch);
                        }
                  qreal minMeasureWidth = point(styleS(StyleIdx::minMeasureWidth));
                  if (w < minMeasureWidth)
                        w = minMeasureWidth;
                  m->layoutWidth(w);
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
      page->setHeight(system->height() + 20 * spatium());

      while (_pages.size() > 1)
            _pages.takeLast();
      }
#endif


