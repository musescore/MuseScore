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

#ifndef __SYSTEMDIVIDER_H__
#define __SYSTEMDIVIDER_H__

#include "symbol.h"
#include "sym.h"

namespace Ms {

//---------------------------------------------------------
//   SystemDivider
//---------------------------------------------------------

class SystemDivider : public Symbol {
      Q_OBJECT
      Q_ENUMS(Type)

   public:
      enum Type { LEFT, RIGHT };

   private:
      Type _dividerType;

   public:
      SystemDivider(Score* s = 0);
      SystemDivider(const SystemDivider&);

      virtual SystemDivider* clone() const override   { return new SystemDivider(*this); }
      virtual Element::Type type() const override     { return Element::Type::SYSTEM_DIVIDER; }

      Type dividerType() const                        { return _dividerType; }
      void setDividerType(Type v);

      virtual QRectF drag(EditData*) override;
      virtual void write(Xml&) const override;
      virtual void read(XmlReader&) override;
      virtual void layout() override;

      virtual Segment* segment() const override       { return 0; }
      System* system() const                          { return (System*)parent(); }
      };

} // namespace Ms

#endif

