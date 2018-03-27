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

#ifndef __REHEARSALMARK_H__
#define __REHEARSALMARK_H__

#include "systemtext.h"

namespace Ms {

//---------------------------------------------------------
//   @@ RehearsalMark
//---------------------------------------------------------

class RehearsalMark final : public TextBase  {
   public:
      RehearsalMark(Score* score);
      virtual RehearsalMark* clone() const override { return new RehearsalMark(*this); }
      virtual ElementType type() const override     { return ElementType::REHEARSAL_MARK; }
      Segment* segment() const                      { return (Segment*)parent(); }
      virtual void layout() override;
      virtual QVariant propertyDefault(Pid id) const override;
      };


}     // namespace Ms
#endif

