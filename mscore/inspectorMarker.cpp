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

#include "inspectorMarker.h"
#include "musescore.h"
#include "libmscore/marker.h"
#include "libmscore/score.h"

//---------------------------------------------------------
//   inspectorMarker
//---------------------------------------------------------

InspectorMarker::InspectorMarker(QWidget* parent)
   : InspectorBase(parent)
      {
      iElement = new InspectorElementElement(this);
      layout->addWidget(iElement);
      QWidget* w = new QWidget;
      iMarker.setupUi(w);
      layout->addWidget(w);
      connect(iMarker.subtype, SIGNAL(currentIndexChanged(int)), SLOT(apply()));
      connect(iMarker.jumpLabel, SIGNAL(textChanged(const QString&)), SLOT(apply()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorMarker::setElement(Element* e)
      {
      Marker* marker = static_cast<Marker*>(e);
      iElement->setElement(marker);

      iMarker.subtype->blockSignals(true);
      iMarker.jumpLabel->blockSignals(true);

      iMarker.subtype->setCurrentIndex(int(marker->markerType()));
      iMarker.jumpLabel->setText(marker->label());

      iMarker.subtype->blockSignals(false);
      iMarker.jumpLabel->blockSignals(false);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorMarker::apply()
      {
      Marker* marker = static_cast<Marker*>(inspector->element());

      if (iMarker.subtype->currentIndex() != int(marker->markerType())
         || iMarker.jumpLabel->text() != marker->label()) {
            Score* score = marker->score();
            score->startCmd();
            score->undoChangeProperty(marker, P_MARKER_TYPE, iMarker.subtype->currentIndex());
            score->undoChangeProperty(marker, P_LABEL, iMarker.jumpLabel->text());
            score->endCmd();
            mscore->endCmd();
            }
      }



