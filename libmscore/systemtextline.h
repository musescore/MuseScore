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

#ifndef __SYSTEMTEXTLINE_H__
#define __SYSTEMTEXTLINE_H__

#include "textlinebase.h"

namespace Ms {

class Note;

//---------------------------------------------------------
//   @@ SystemTextLineSegment
//---------------------------------------------------------

class SystemTextLineSegment final : public TextLineBaseSegment {

      Sid getPropertyStyle(Pid) const override;

   public:
      SystemTextLineSegment(Spanner* sp, Score* s);
      ElementType type() const override             { return ElementType::SYSTEM_TEXTLINE_SEGMENT; }
      SystemTextLineSegment* clone() const override { return new SystemTextLineSegment(*this); }
      SystemTextLine* systemTextLine() const        { return toSystemTextLine(spanner()); }
      void layout() override;
      };

//---------------------------------------------------------
//   @@ SystemTextLine
//---------------------------------------------------------

class SystemTextLine final : public TextLineBase {

      Sid getPropertyStyle(Pid) const override;

   public:
      SystemTextLine(Score* s);
      SystemTextLine(const SystemTextLine&);
      ~SystemTextLine() {}

      SystemTextLine* clone() const override        { return new SystemTextLine(*this); }
      ElementType type() const override             { return ElementType::SYSTEM_TEXTLINE; }
      void write(XmlWriter&) const override;
      LineSegment* createLineSegment() override;
      QVariant propertyDefault(Pid) const override;
      };


}     // namespace Ms
#endif


