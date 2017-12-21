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

class Ossia final : public Element {
   public:
      Ossia(Score*);
      Ossia(const Ossia&);
      virtual Ossia* clone() const override     { return new Ossia(*this); }
      virtual ElementType type() const override { return ElementType::OSSIA; }
      };


}     // namespace Ms
#endif

