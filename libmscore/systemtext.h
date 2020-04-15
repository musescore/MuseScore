//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SYSTEMTEXT_H__
#define __SYSTEMTEXT_H__

#include "stafftext.h"

namespace Ms {

//---------------------------------------------------------
//   SystemText
//---------------------------------------------------------

class SystemText final : public StaffTextBase  {
      void layout() override;
      QVariant propertyDefault(Pid id) const override;

   public:
      SystemText(Score* = 0, Tid = Tid::SYSTEM);

      SystemText* clone() const override    { return new SystemText(*this); }
      ElementType type() const override     { return ElementType::SYSTEM_TEXT; }
      Segment* segment() const              { return (Segment*)parent(); }
      };


}     // namespace Ms
#endif

