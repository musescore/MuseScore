//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __OSSIA_H__
#define __OSSIA_H__

#include "element.h"

namespace Ms {

//---------------------------------------------------------
//   @@ Ossia
///    not implemented yet
//---------------------------------------------------------

class Ossia : public Element {
      Q_OBJECT


   public:
      Ossia(Score*);
      Ossia(const Ossia&);
      virtual Ossia* clone() const override       { return new Ossia(*this); }
      virtual Element::Type type() const override { return Element::Type::OSSIA; }
      };


}     // namespace Ms
#endif

