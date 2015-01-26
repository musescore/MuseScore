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

      IconType _iconType { IconType::NONE };
      QByteArray _action;
      QIcon _icon;

   public:
      Icon(Score* s) : Element(s) { }
      virtual ~Icon() {}

      virtual Icon* clone() const override        { return new Icon(*this);    }
      virtual Element::Type type() const override { return Element::Type::ICON;  }
      IconType iconType() const          { return _iconType;          }
      void setIconType(IconType val)     { _iconType = val;           }
      void setAction(const QByteArray& a, const QIcon& i) { _action = a; _icon = i; }
      const QByteArray& action() const   { return _action; }
      QIcon icon() const                 { return _icon;   }
      virtual void write(Xml&) const override;
      virtual void read(XmlReader&) override;
      };


}     // namespace Ms
#endif

