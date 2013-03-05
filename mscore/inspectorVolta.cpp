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

#include "inspectorVolta.h"
#include "musescore.h"
#include "libmscore/volta.h"
#include "libmscore/score.h"

//---------------------------------------------------------
//   InspectorVolta
//---------------------------------------------------------

InspectorVolta::InspectorVolta(QWidget* parent)
   : InspectorBase(parent)
      {
      iElement = new InspectorElementElement(this);
      layout->addWidget(iElement);
      QWidget* w = new QWidget;
      iVolta.setupUi(w);
      layout->addWidget(w);
      connect(iVolta.subtype, SIGNAL(currentIndexChanged(int)), SLOT(apply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorVolta::setElement(Element* e)
      {
      VoltaSegment* voltaSegment = static_cast<VoltaSegment*>(e);
      iElement->setElement(voltaSegment);
      Volta* volta = voltaSegment->volta();

      iVolta.subtype->blockSignals(true);
      iVolta.subtype->setCurrentIndex(int(volta->voltaType()));
      iVolta.subtype->blockSignals(false);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorVolta::apply()
      {
      VoltaSegment* voltaSegment = static_cast<VoltaSegment*>(inspector->element());

      Volta* volta = voltaSegment->volta();
      VoltaType vt = volta->voltaType();
      VoltaType nt = VoltaType(iVolta.subtype->currentIndex());
      if (vt != nt) {
            Score* score = volta->score();
            score->startCmd();
            score->undoChangeProperty(volta, P_VOLTA_TYPE, int(nt));
            score->endCmd();
            mscore->endCmd();
            }
      }


