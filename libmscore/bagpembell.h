//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __BAGPEMBELL_H__
#define __BAGPEMBELL_H__

#include "element.h"

namespace Ms {

//---------------------------------------------------------
//   BagpipeEmbellishment
//    dummy element, used for drag&drop
//---------------------------------------------------------

class BagpipeEmbellishment : public Element {
      Q_OBJECT

      int _embelType;

   public:
      BagpipeEmbellishment(Score* s) : Element(s), _embelType(0) { }
      virtual BagpipeEmbellishment* clone() const { return new BagpipeEmbellishment(*this); }
      virtual ElementType type() const            { return BAGPIPE_EMBELLISHMENT;           }
      int embelType() const                       { return _embelType;                      }
      void setEmbelType(int val)                  { _embelType = val;                       }
      virtual void write(Xml&) const;
      virtual void read(XmlReader&);
      };


}     // namespace Ms
#endif

