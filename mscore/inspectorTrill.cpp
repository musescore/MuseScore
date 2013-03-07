//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorTrill.h"
#include "musescore.h"
#include "libmscore/trill.h"
#include "libmscore/score.h"

//---------------------------------------------------------
//   InspectorTrill
//---------------------------------------------------------

InspectorTrill::InspectorTrill(QWidget* parent)
   : InspectorBase(parent)
      {
      iElement = new InspectorElementElement(this);
      _layout->addWidget(iElement);
      QWidget* w = new QWidget;
      iTrill.setupUi(w);
      _layout->addWidget(w);
      connect(iTrill.subtype, SIGNAL(currentIndexChanged(int)), SLOT(apply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorTrill::setElement(Element* e)
      {
      TrillSegment* trillSegment = static_cast<TrillSegment*>(e);
      iElement->setElement(trillSegment);
      Trill* trill = trillSegment->trill();

      iTrill.subtype->blockSignals(true);
      iTrill.subtype->setCurrentIndex(int(trill->trillType()));
      iTrill.subtype->blockSignals(false);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorTrill::apply()
      {
      TrillSegment* trillSegment = static_cast<TrillSegment*>(inspector->element());

      Trill* trill = trillSegment->trill();
      Trill::TrillType vt = trill->trillType();
      Trill::TrillType nt = Trill::TrillType(iTrill.subtype->currentIndex());
      if (vt != nt) {
            Score* score = trill->score();
            score->startCmd();
            score->undoChangeProperty(trill, P_TRILL_TYPE, nt);
            score->endCmd();
            mscore->endCmd();
            }
      }


