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

#ifndef __ICON_H__
#define __ICON_H__

#include "element.h"

namespace Ms {

//---------------------------------------------------------
//   Icon
//    dummy element, used for drag&drop
//---------------------------------------------------------

class Icon : public Element {
      Q_OBJECT

      IconType _iconType;
      const char* _action;
      QIcon _icon;

   public:
      Icon(Score* s) : Element(s), _iconType(IconType::NONE), _action(0) { }
      virtual Icon* clone() const        { return new Icon(*this);    }
      virtual Element::Type type() const { return Element::Type::ICON;  }
      IconType iconType() const          { return _iconType;          }
      void setIconType(IconType val)     { _iconType = val;           }
      void setAction(const char* s, const QIcon& i)   { _action = s; _icon = i;  }
      const char* action() const         { return _action;            }
      QIcon icon() const                 { return _icon;              }
      virtual void write(Xml&) const;
      virtual void read(XmlReader&);
      };


}     // namespace Ms
#endif

