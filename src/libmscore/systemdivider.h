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

class SystemDivider final : public Symbol
{
public:
    enum Type {
        LEFT, RIGHT
    };

private:
    Type _dividerType;

public:
    SystemDivider(Score* s = 0);
    SystemDivider(const SystemDivider&);

    SystemDivider* clone() const override { return new SystemDivider(*this); }
    ElementType type() const override { return ElementType::SYSTEM_DIVIDER; }

    Type dividerType() const { return _dividerType; }
    void setDividerType(Type v);

    QRectF drag(EditData&) override;

    void write(XmlWriter&) const override;
    void read(XmlReader&) override;

    void layout() override;

    Segment* segment() const override { return nullptr; }
    System* system() const { return (System*)parent(); }
};
} // namespace Ms

#endif
