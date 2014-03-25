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

#ifndef __SECTIONMARK_H__
#define __SECTIONMARK_H__

#include "text.h"

namespace Ms {

//---------------------------------------------------------
//   @@ SectionMark
//---------------------------------------------------------

class SectionMark : public Text  {
      Q_OBJECT

   public:
      SectionMark(Score* score);
      virtual SectionMark* clone() const { return new SectionMark(*this); }
      virtual ElementType type() const { return SECTION_MARK; }
      Segment* segment() const { return (Segment*)parent(); }
      };


}     // namespace Ms
#endif

