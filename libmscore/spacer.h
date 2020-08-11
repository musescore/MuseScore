//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SPACER_H__
#define __SPACER_H__

#include "element.h"

namespace Ms {
//---------------------------------------------------------
//   SpacerType
//---------------------------------------------------------

enum class SpacerType : char {
    UP, DOWN, FIXED
};

//-------------------------------------------------------------------
//   @@ Spacer
///    Vertical spacer element to adjust the distance of staves.
//-------------------------------------------------------------------

class Spacer final : public Element
{
    SpacerType _spacerType;
    qreal _gap;

    QPainterPath path;

    void layout0();

public:
    Spacer(Score*);
    Spacer(const Spacer&);

    Spacer* clone() const override { return new Spacer(*this); }
    ElementType type() const override { return ElementType::SPACER; }
    Measure* measure() const { return toMeasure(parent()); }

    SpacerType spacerType() const { return _spacerType; }
    void setSpacerType(SpacerType t) { _spacerType = t; }

    void write(XmlWriter&) const override;
    void read(XmlReader&) override;

    void draw(QPainter*) const override;

    void scanElements(void* data, void (* func)(void*, Element*), bool all=true) override;

    bool isEditable() const override { return true; }
    void startEditDrag(EditData&) override;
    void editDrag(EditData&) override;
    void spatiumChanged(qreal, qreal) override;

    void setGap(qreal sp);
    qreal gap() const { return _gap; }

    EditBehavior normalModeEditBehavior() const override { return EditBehavior::Edit; }
    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<QPointF> gripsPositions(const EditData&) const override;

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid id) const override;
};
}     // namespace Ms
#endif
