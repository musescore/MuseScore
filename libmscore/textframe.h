//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TEXTFRAME_H__
#define __TEXTFRAME_H__

#include "box.h"

namespace Ms {

class Text;

//---------------------------------------------------------
//   @@ TBox
///    Text frame.
//---------------------------------------------------------

class TBox : public VBox {
      Text* _text;

   public:
      TBox(Score* score);
      TBox(const TBox&);
      ~TBox();
      virtual TBox* clone() const        { return new TBox(*this); }
      virtual ElementType type() const   { return ElementType::TBOX;       }
      virtual void write(XmlWriter&) const override;
      using VBox::write;
      virtual void read(XmlReader&) override;
      virtual Element* drop(EditData&) override;
      virtual void add(Element* e) override;
      virtual void remove(Element* el) override;

      virtual void layout();
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      Text* text()                        { return _text; }
      };


}     // namespace Ms
#endif

