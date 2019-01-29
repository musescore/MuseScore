//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __FINGERING_H__
#define __FINGERING_H__

#include "text.h"

namespace Ms {

//---------------------------------------------------------
//   @@ Fingering
//---------------------------------------------------------

class Fingering final : public TextBase {

   public:
      Fingering(Score*, Tid tid, ElementFlags ef = ElementFlag::HAS_TAG);
      Fingering(Score* s, ElementFlags ef = ElementFlag::HAS_TAG);

      virtual Fingering* clone() const override { return new Fingering(*this); }
      virtual ElementType type() const override { return ElementType::FINGERING; }

      Note* note() const { return toNote(parent()); }
      ElementType layoutType();
      void calculatePlacement();

      virtual void draw(QPainter*) const override;
      virtual void layout() override;

      virtual QVariant propertyDefault(Pid id) const override;

      virtual QString accessibleInfo() const override;
      };


}     // namespace Ms
#endif

