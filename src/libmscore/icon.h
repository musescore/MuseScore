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
#include "mscore.h"

namespace Ms {
//---------------------------------------------------------
//   Icon
//    dummy element, used for drag&drop
//---------------------------------------------------------

class Icon final : public Element
{
public:
    Icon(Score* score);
    ~Icon() override = default;

    Icon* clone() const override;
    ElementType type() const override;
    IconType iconType() const;
    const std::string& actionCode() const;

    void setIconType(IconType val);
    void setAction(const std::string& actionCode, char16_t icon);
    void setExtent(int extent);

    void write(XmlWriter&) const override;
    void read(XmlReader&) override;
    void draw(mu::draw::Painter*) const override;
    void layout() override;

    QVariant getProperty(Pid) const override;
    bool setProperty(Pid, const QVariant&) override;

private:
    QRectF boundingBox() const;

    IconType _iconType { IconType::NONE };
    std::string _actionCode;
    char16_t _icon = 0;
    int _extent { 40 };
};
}     // namespace Ms
#endif
