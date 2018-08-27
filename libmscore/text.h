//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TEXT_H__
#define __TEXT_H__

#include "textbase.h"

namespace Ms {

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

class Text final : public TextBase {

   public:
      Text(Score* s = 0, Tid tid = Tid::DEFAULT);

      virtual ElementType type() const override    { return ElementType::TEXT; }
      virtual Text* clone() const override         { return new Text(*this); }
      virtual void read(XmlReader&) override;
      virtual QVariant propertyDefault(Pid id) const override;
      };

}     // namespace Ms

#endif

