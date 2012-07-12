//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspectorLasso.h"
#include "musescore.h"
#include "libmscore/lasso.h"

//---------------------------------------------------------
//   InspectorLasso
//---------------------------------------------------------

InspectorLasso::InspectorLasso(QWidget* parent)
   : InspectorBase(parent)
      {
      QWidget* w = new QWidget;
      b.setupUi(w);
      layout->addWidget(w);
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorLasso::setElement(Element* e)
      {
      Lasso* lasso = static_cast<Lasso*>(e);
      QRectF bb(lasso->rect());

      b.posX->setValue(bb.x() / MScore::DPMM);
      b.posY->setValue(bb.y() / MScore::DPMM);
      b.sizeWidth->setValue(bb.width() / MScore::DPMM);
      b.sizeHeight->setValue(bb.height() / MScore::DPMM);

      InspectorBase::setElement(e);
      }

