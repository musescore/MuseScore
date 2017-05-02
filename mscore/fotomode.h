//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __FOTOMODE_H__
#define __FOTOMODE_H__

#include "libmscore/lasso.h"

namespace Ms {

//---------------------------------------------------------
//   FotoLasso
//---------------------------------------------------------

class FotoLasso : public Lasso {
   public:
      FotoLasso(Score* s) : Lasso(s) {}
      virtual void startEdit(EditData&) override;
      virtual void endEdit(EditData&) override;
      virtual void updateGrips(EditData&) const override;
      virtual void drawEditMode(QPainter*, EditData&) override;
      };

}

#endif



