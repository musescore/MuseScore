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

      const std::vector<InspectorItem> iiList;  // dummy
      const std::vector<InspectorPanel> ppList = {
            { me.title, me.panel }
            };

      mapSignals(iiList, ppList);

      connect(me.actualZ, SIGNAL(valueChanged(int)),        SLOT(timesigValueChanged(int)));
      connect(me.actualN, SIGNAL(currentIndexChanged(int)), SLOT(timesigValueChanged(int)));
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
      Measure* m = measure();
      Score* s = m->score();
      Fraction timesig = getInputTimesig();

      int staffStart = s->selection().staffStart();
      int staffEnd   = s->selection().staffEnd();

      m->score()->startCmd();

      m->adjustToLen(timesig);
      s->setLayout(m->tick());

      m->score()->endCmd();

      // Update selection
      s->selection().setRange(m->first(), m->last(), staffStart, staffEnd);
      s->setSelectionChanged(true);

      s->update();
      mscore->timeline()->updateGrid();
      }
}

