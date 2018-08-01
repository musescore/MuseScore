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
      virtual void layout() override;

   public:
      SystemText(Score*);
      SystemText(Score*, Tid, ElementFlags = ElementFlag::NOTHING);

      virtual SystemText* clone() const override    { return new SystemText(*this); }
      virtual ElementType type() const override     { return ElementType::SYSTEM_TEXT; }
      Segment* segment() const                      { return (Segment*)parent(); }
      virtual QVariant propertyDefault(Pid id) const override;
      };


}     // namespace Ms
#endif

