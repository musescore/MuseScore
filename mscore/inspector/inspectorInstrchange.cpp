//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorInstrchange.h"
#include "musescore.h"
#include "inspector.h"
#include "libmscore/instrchange.h"
#include "libmscore/score.h"
#include "libmscore/chord.h"
#include "libmscore/segment.h"
#include "scoreview.h"

namespace Ms {

extern void populatePlacement(QComboBox*);

//---------------------------------------------------------
//   InspectorInstrumentChange
//---------------------------------------------------------

InspectorInstrumentChange::InspectorInstrumentChange(QWidget* parent)
   : InspectorTextBase(parent)
      {
      ic.setupUi(addWidget());

      const std::vector<InspectorItem> il = {
            { Pid::SUB_STYLE,        0, ic.style,        ic.resetStyle        },
            { Pid::PLACEMENT,        0, ic.placement,    ic.resetPlacement    }
            };
      const std::vector<InspectorPanel> ppList = {
            { ic.title, ic.panel }
            };
      populatePlacement(ic.placement);
      populateStyle(ic.style);
      mapSignals(il, ppList);
      connect(ic.selectInstrument, SIGNAL(clicked()), SLOT(selectInstrumentClicked()));
      connect(ic.showWarning, SIGNAL(stateChanged(int)), SLOT(showWarningChanged(int)));
      }

//---------------------------------------------------------
//   selectInstrumentClicked
//---------------------------------------------------------

void InspectorInstrumentChange::selectInstrumentClicked()
      {
      InstrumentChange* i = static_cast<InstrumentChange*>(inspector->element());
      Score* score = i->score();
      score->startCmd();
      mscore->currentScoreView()->selectInstrument(i);
      score->setLayoutAll();
      score->endCmd();
      }

//---------------------------------------------------------
//   showWarningChanged
//---------------------------------------------------------

void InspectorInstrumentChange::showWarningChanged(int state)
      {
      InstrumentChange* i = toInstrumentChange(inspector->element());
      Score* score = i->score();
      score->startCmd();
      if (state == 0) {
            i->setShowWarning(false);
            InstrumentChangeWarning* warning = score->nextICWarning(i->part(), i->segment());
            if (warning)
                  score->undoRemoveElement(warning);
            }
      else {
            i->setShowWarning(true);
            Chord* nextChord = score->nextChord(i->segment(), i->part());
            if (nextChord)
                  i->setNextChord(nextChord);
            }
      score->setLayoutAll();
      score->endCmd();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorInstrumentChange::setElement()
      {
      InspectorTextBase::setElement();
      InstrumentChange* i = toInstrumentChange(inspector->element());
      if (i->showWarning())
            ic.showWarning->setChecked(true);
      }

} // namespace Ms

