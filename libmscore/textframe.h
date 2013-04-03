//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TEXTFRAME_H__
#define __TEXTFRAME_H__

#include "box.h"

//---------------------------------------------------------
//   @@ TBox
///   Text frame.
//---------------------------------------------------------

class TBox : public VBox {
      Q_OBJECT

   public:
      TBox(Score* score);
      ~TBox() {}
      virtual TBox* clone() const      { return new TBox(*this); }
      virtual ElementType type() const { return TBOX;       }

      virtual void layout();
      virtual void add(Element*);
      Text* getText();
      };

#endif

