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

      const std::vector<InspectorItem> iiList {
            { Pid::IRREGULAR,             0,    me.exclude,       0 },
            { Pid::MEASURE_NUMBER_MODE,   0,    me.numberMode,    0 },
            { Pid::NO_OFFSET,             0,    me.offset,        0 },
            { Pid::BREAK_MMR,             0,    me.breakMMR,      0 },
            { Pid::USER_STRETCH,          0,    me.layoutStretch, 0 },
            { Pid::REPEAT_COUNT,          0,    me.playCount,     0 },
            { Pid::USER_STRETCH,          0,    me.layoutStretch, 0 },
            { Pid::TIMESIG_ACTUAL_NUMERATOR,   0, me.actualZ,     0 },
            { Pid::TIMESIG_ACTUAL_DENOMINATOR, 0, me.actualN,     0 },
            };
      const std::vector<InspectorPanel> ppList = {
            { me.title, me.panel }
            };

      mapSignals(iiList, ppList);

      me.actualN->blockSignals(true);
      Fraction actualTs = m->ticks();
      int index = me.actualN->findText(QString::number(actualTs.denominator()));
      if (index == -1)
            index = 2;
      me.actualN->setCurrentIndex(index);
      me.actualN->blockSignals(false);

      // Disable play count if not a repeat measure
      bool enableCount = m->repeatEnd();
      me.playCount->setEnabled(enableCount);
      me.playCount->setVisible(enableCount);
      me.playCountLabel->setVisible(enableCount);

#if 0
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
#endif

      qDebug("created inspector measure at %p", this);
      }

InspectorMeasure::~InspectorMeasure()
      {
      qDebug("deleting inspector measure at %p", this);
      }

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure* InspectorMeasure::measure() const
      {
      return toMeasure(inspector->element());
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void InspectorMeasure::valueChanged(int idx)
      {
      if (iList[idx].t == Pid::TIMESIG_ACTUAL_DENOMINATOR || iList[idx].t == Pid::TIMESIG_ACTUAL_NUMERATOR) {
            Measure* m = measure();
            Score* s = m->score();
            int staffStart = s->selection().staffStart();
            int staffEnd   = s->selection().staffEnd();
            }

      InspectorBase::valueChanged(idx);

      if ()
      s->selection().setRange(m->first(), m->last(), staffStart, staffEnd);
      s->setSelectionChanged(true);
      s->selection().updateSelectedElements();
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
      Measure* m = measure();
      Score* s = m->score();
      Fraction timesig = getInputTimesig();

      // This is needed to be able to restore selection after measure resize
      int staffStart = s->selection().staffStart();
      int staffEnd   = s->selection().staffEnd();

      s->deselectAll();

      // Adjust measure length
      inspector->setInspectorEdit(true);
      m->score()->startCmd();
      m->adjustToLen(timesig);
      m->score()->endCmd();
      inspector->setInspectorEdit(false);

      mscore->timeline()->updateGrid();

      s->selection().setRange(m->first(), m->last(), staffStart, staffEnd);
      s->selection().updateSelectedElements();
      s->setSelectionChanged(true);

      inspector->update();
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

