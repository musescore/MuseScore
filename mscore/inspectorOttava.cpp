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

#include "inspectorOttava.h"
#include "musescore.h"
#include "libmscore/ottava.h"
#include "libmscore/score.h"

//---------------------------------------------------------
//   InspectorOttava
//---------------------------------------------------------

InspectorOttava::InspectorOttava(QWidget* parent)
   : InspectorBase(parent)
      {
      iElement = new InspectorElementElement(this);
      layout->addWidget(iElement);
      QWidget* w = new QWidget;
      iOttava.setupUi(w);
      layout->addWidget(w);
      connect(iOttava.subtype, SIGNAL(currentIndexChanged(int)), SLOT(apply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorOttava::setElement(Element* e)
      {
      OttavaSegment* ottavaSegment = static_cast<OttavaSegment*>(e);
      iElement->setElement(ottavaSegment);
      Ottava* ottava = ottavaSegment->ottava();

      iOttava.subtype->blockSignals(true);
      iOttava.subtype->setCurrentIndex(int(ottava->ottavaType()));
      iOttava.subtype->blockSignals(false);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorOttava::apply()
      {
      OttavaSegment* ottavaSegment = static_cast<OttavaSegment*>(inspector->element());

      Ottava* ottava = ottavaSegment->ottava();
      Ottava::OttavaType vt = ottava->ottavaType();
      Ottava::OttavaType nt = Ottava::OttavaType(iOttava.subtype->currentIndex());
      if (vt != nt) {
            Score* score = ottava->score();
            score->startCmd();
            score->undoChangeProperty(ottava, P_OTTAVA_TYPE, nt);
            score->endCmd();
            mscore->endCmd();
            }
      }


