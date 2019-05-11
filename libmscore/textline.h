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

#ifndef __TEXTLINE_H__
#define __TEXTLINE_H__

#include "textlinebase.h"

namespace Ms {

class Note;

//---------------------------------------------------------
//   @@ TextLineSegment
//---------------------------------------------------------

class TextLineSegment final : public TextLineBaseSegment {

      virtual Sid getPropertyStyle(Pid) const override;

   public:
      TextLineSegment(Spanner* sp, Score* s);
      virtual ElementType type() const override       { return ElementType::TEXTLINE_SEGMENT; }
      virtual TextLineSegment* clone() const override { return new TextLineSegment(*this); }
      TextLine* textLine() const                      { return toTextLine(spanner()); }
      virtual void layout() override;
      };

//---------------------------------------------------------
//   @@ TextLine
//---------------------------------------------------------

class TextLine final : public TextLineBase {

      virtual Sid getPropertyStyle(Pid) const override;

   public:
      TextLine(Score* s);
      TextLine(const TextLine&);
      ~TextLine() {}

      virtual TextLine* clone() const           { return new TextLine(*this); }
      virtual ElementType type() const          { return ElementType::TEXTLINE; }
      virtual void write(XmlWriter&) const override;
      virtual LineSegment* createLineSegment() override;
      virtual QVariant propertyDefault(Pid) const override;
      };


}     // namespace Ms
#endif

