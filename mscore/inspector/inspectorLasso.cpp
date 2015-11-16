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

namespace Ms {

//---------------------------------------------------------
//   InspectorLasso
//---------------------------------------------------------

InspectorLasso::InspectorLasso(QWidget* parent)
   : InspectorBase(parent)
      {
      b.setupUi(addWidget());

      iList = {
            { P_ID::LASSO_POS,    0, false, b.posX,       0 },
            { P_ID::LASSO_POS,    1, false, b.posY,       0 },
            { P_ID::LASSO_SIZE,   0, false, b.sizeWidth,  0 },
            { P_ID::LASSO_SIZE,   1, false, b.sizeHeight, 0 },
            };

      mapSignals();
      }

#if 0
//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorLasso::setElement(Element* e)
      {
      Lasso* lasso = static_cast<Lasso*>(e);
      QRectF bb(lasso->rect());

      b.posX->setValue(bb.x() / DPMM);
      b.posY->setValue(bb.y() / DPMM);
      b.sizeWidth->setValue(bb.width() / DPMM);
      b.sizeHeight->setValue(bb.height() / DPMM);

      InspectorBase::setElement();
      }
#endif

}

