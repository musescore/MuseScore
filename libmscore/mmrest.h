//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __MMREST_H__
#define __MMREST_H__

#include "rest.h"

namespace Ms {
//---------------------------------------------------------
//    @@ MMRest
///     This class implements a multimeasure rest.
//---------------------------------------------------------

class MMRest: public Rest {
    qreal _mmWidth;             // width of multimeasure rest
    qreal _mmRestNumberPos;     // vertical position of number of multimeasure rest
    Sid getPropertyStyle(Pid) const override;

public:
    MMRest(Score * s = 0);
    MMRest(const MMRest&, bool link = false);
    ~MMRest() {
    }

    ElementType type() const override { return ElementType::MMREST; }

    MMRest* clone() const override      { return new MMRest(* this, false); }
    Element* linkedClone() override     { return new MMRest(* this, true); }

    void draw(QPainter*) const override;
    void layout() override;
    void layout(qreal width);
    qreal mmWidth() const { return _mmWidth; }

    void write(XmlWriter&) const override;

    QVariant propertyDefault(Pid) const override;
    void resetProperty(Pid);
    bool setProperty(Pid, const QVariant&) override;
    QVariant getProperty(Pid) const override;

    Shape shape() const override;
};
}     // namespace Ms
#endif
