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

class MMRest final : public Rest
{
public:
    MMRest(Score* s = 0);
    MMRest(const MMRest&, bool link = false);
    ~MMRest()
    {
    }

    ElementType type() const override { return ElementType::MMREST; }

    MMRest* clone() const override { return new MMRest(*this, false); }
    Element* linkedClone() override { return new MMRest(*this, true); }

    void draw(QPainter*) const override;
    void layout() override;
    void setWidth(qreal width) { m_width = width; }
    qreal width() const { return m_width; }
    QRectF numberRect() const;

    void write(XmlWriter&) const override;

    QVariant propertyDefault(Pid) const override;
    bool setProperty(Pid, const QVariant&) override;
    QVariant getProperty(Pid) const override;

    Shape shape() const override;

private:
    Sid getPropertyStyle(Pid) const override;
    qreal m_width;                  // width of multimeasure rest
    int m_number;                   // number of measures represented
    qreal m_numberPos;              // vertical position of number relative to staff
    bool m_numberVisible;           // show or hide number
    std::vector<SymId> m_restSyms;  // stores symbols when using old-style rests
    qreal m_symsWidth;              // width of symbols with spacing when using old-style
};
}     // namespace Ms
#endif
