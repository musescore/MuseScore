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

#include "inspectorHairpin.h"
#include "musescore.h"
#include "libmscore/hairpin.h"
#include "libmscore/score.h"

//---------------------------------------------------------
//   InspectorHairpin
//---------------------------------------------------------

InspectorHairpin::InspectorHairpin(QWidget* parent)
   : InspectorBase(parent)
      {
      iElement = new InspectorElementElement(this);
      layout->addWidget(iElement);
      QWidget* w = new QWidget;
      iHairpin.setupUi(w);
      layout->addWidget(w);
      connect(iHairpin.subtype, SIGNAL(currentIndexChanged(int)), SLOT(apply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorHairpin::setElement(Element* e)
      {
      HairpinSegment* hairpinSegment = static_cast<HairpinSegment*>(e);
      iElement->setElement(hairpinSegment);
      Hairpin* hairpin = hairpinSegment->hairpin();

      iHairpin.subtype->blockSignals(true);
      iHairpin.subtype->setCurrentIndex(int(hairpin->subtype()));
      iHairpin.subtype->blockSignals(false);
      iHairpin.dynRange->setCurrentIndex(int(hairpin->dynRange()));
      iHairpin.veloChange->setValue(hairpin->veloChange());
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorHairpin::apply()
      {
      HairpinSegment* hairpinSegment = static_cast<HairpinSegment*>(inspector->element());

      Hairpin* hairpin = hairpinSegment->hairpin();
      Hairpin::HairpinType vt = hairpin->subtype();
      Hairpin::HairpinType nt = Hairpin::HairpinType(iHairpin.subtype->currentIndex());
      if (vt != nt) {
            Score* score = hairpin->score();
            score->startCmd();
            score->undoChangeProperty(hairpin, P_HAIRPIN_TYPE, nt);
            score->endCmd();
            mscore->endCmd();
            }
      }

