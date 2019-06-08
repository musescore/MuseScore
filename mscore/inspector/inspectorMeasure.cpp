//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorMeasure.h"
#include "inspectorGroupElement.h"
#include "inspector.h"
#include "musescore.h"
#include "timeline.h"
#include "libmscore/score.h"
#include "libmscore/element.h"
#include "libmscore/measure.h"
#include "libmscore/staff.h"
#include "measureproperties.h"
#include "libmscore/range.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorMeasure
//---------------------------------------------------------

InspectorMeasure::InspectorMeasure(QWidget* parent)
   : InspectorGroupElement(parent)
      {
      me.setupUi(addWidget());

      Measure* m = measure();
      Fraction nominalTs = m->timesig();
      me.nominalZ->setText(QString::number(nominalTs.numerator()));
      me.nominalN->setText(QString::number(nominalTs.denominator()));

      Fraction actualTs = m->ticks();
      me.actualZ->setValue(actualTs.numerator());
      int index = me.actualN->findText(QString::number(actualTs.denominator()));
      if (index == -1)
            index = 2;
      me.actualN->setCurrentIndex(index);

      // iiList is a dummy, we can't use the built-in system for this
      const std::vector<InspectorItem> iiList;
      const std::vector<InspectorPanel> ppList = {
            { me.title, me.panel }
            };

      mapSignals(iiList, ppList);

      // Disable play count if not a repeat measure
      bool enableCount = m->repeatEnd();
      me.playCount->setEnabled(enableCount);
      me.playCount->setVisible(enableCount);
      me.playCountLabel->setVisible(enableCount);

      // Init properties
      me.exclude->setChecked(m->isIrregular());
      me.numberMode->setCurrentIndex(int(m->measureNumberMode()));
      me.offset->setValue(m->noOffset());
      me.breakMMR->setChecked(m->breakMultiMeasureRest());
      me.layoutStretch->setValue(m->userStretch());
      me.playCount->setValue(m->playbackCount());

      // Connect slots
      connect(me.actualZ, SIGNAL(valueChanged(int)),        SLOT(timesigValueChanged(int)));
      connect(me.actualN, SIGNAL(currentIndexChanged(int)), SLOT(timesigValueChanged(int)));

      connect(me.exclude,     SIGNAL(toggled(bool)),              SLOT(excludeToggled(bool)));
      connect(me.numberMode,  SIGNAL(currentIndexChanged(int)),   SLOT(numberModeChanged(int)));
      connect(me.offset,      SIGNAL(valueChanged(int)),          SLOT(offsetChanged(int)));
      connect(me.breakMMR,    SIGNAL(toggled(bool)),              SLOT(breakMMRToggled(bool)));
      connect(me.layoutStretch, SIGNAL(valueChanged(qreal)),      SLOT(layoutStretchChanged(qreal)));
      connect(me.playCount,   SIGNAL(valueChanged(int)),          SLOT(playCountChanged(int)));
      }

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure* InspectorMeasure::measure() const
      {
      Element* e = inspector->element();
      return e->score()->tick2measure(e->tick());
      }

//---------------------------------------------------------
//   getInputTimesig
//---------------------------------------------------------

Fraction InspectorMeasure::getInputTimesig() const
      {
      return Fraction(me.actualZ->value(), 1 << me.actualN->currentIndex());
      }

//---------------------------------------------------------
//   timesigValueChanged
//---------------------------------------------------------

void InspectorMeasure::timesigValueChanged(int)
      {
      qDebug("timesig changed");
      if (inspector) qDebug("inspector exists");
      Measure* m = measure();
      Score* s = m->score();
      Fraction timesig = getInputTimesig();

      int staffStart = s->selection().staffStart();
      int staffEnd   = s->selection().staffEnd();

      s->deselectAll();
      s->setSelectionChanged(true);

      m->score()->startCmd();

      qDebug() << "adjust to" << timesig;
      m->adjustToLen(timesig);

      m->score()->endCmd();

      mscore->timeline()->updateGrid();

#if 0
      if (inspector) qDebug("inspector exists");

      qDebug("update");
      s->update();
      mscore->timeline()->updateGrid();

      // Update selection
      qDebug("update selection: %d to %d", m->first()->tick(), m->last()->tick());
      s->selection().setRange(m->first(), m->last(), staffStart, staffEnd);
      s->selection().updateSelectedElements();
      s->setSelectionChanged(true);
      
      qDebug("done");
      if (inspector) {
            inspector->update();
            qDebug("update inspector");
            }
#endif
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void InspectorMeasure::setProperty(Pid p, QVariant v)
      {
      Measure* m = measure();

      m->score()->startCmd();
      m->undoChangeProperty(p, v);
      m->score()->endCmd();
      }

//---------------------------------------------------------
//   excludeToggled
//---------------------------------------------------------

void InspectorMeasure::excludeToggled(bool val)
      {
      setProperty(Pid::IRREGULAR, val);
      }

//---------------------------------------------------------
//   numberModeChanged
//---------------------------------------------------------

void InspectorMeasure::numberModeChanged(int val)
      {
      setProperty(Pid::MEASURE_NUMBER_MODE, val);
      }

//---------------------------------------------------------
//   offsetChanged
//---------------------------------------------------------

void InspectorMeasure::offsetChanged(int val)
      {
      setProperty(Pid::NO_OFFSET, val);
      }

//---------------------------------------------------------
//   breakMMRToggled
//---------------------------------------------------------

void InspectorMeasure::breakMMRToggled(bool val)
      {
      setProperty(Pid::BREAK_MMR, val);
      }

//---------------------------------------------------------
//   layoutStretchChanged
//---------------------------------------------------------

void InspectorMeasure::layoutStretchChanged(qreal val)
      {
      setProperty(Pid::USER_STRETCH, val);
      }

//---------------------------------------------------------
//   playCountChanged
//---------------------------------------------------------

void InspectorMeasure::playCountChanged(int val)
      {
      setProperty(Pid::REPEAT_COUNT, val);
      }

}

