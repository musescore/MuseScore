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

#ifndef __STEM_H__
#define __STEM_H__

#include "element.h"

namespace Ms {
class Chord;

//---------------------------------------------------------
//   @@ Stem
///    Graphic representation of a note stem.
//---------------------------------------------------------

class Stem final : public Element
{
    QLineF line;                    // p1 is attached to notehead
    qreal _lineWidth;
    qreal _userLen;
    qreal _len       { 0.0 };       // always positive

public:
    Stem(Score* = 0);
    Stem& operator=(const Stem&) = delete;

    Stem* clone() const override { return new Stem(*this); }
    ElementType type() const override { return ElementType::STEM; }
    void draw(QPainter*) const override;
    bool isEditable() const override { return true; }
    void layout() override;
    void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;
    Element* elementBase() const override;

    void startEdit(EditData&) override;
    void editDrag(EditData&) override;
    void write(XmlWriter& xml) const override;
    void read(XmlReader& e) override;
    bool readProperties(XmlReader&) override;
    void reset() override;
    bool acceptDrop(EditData&) const override;
    Element* drop(EditData&) override;

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid id) const override;

    int vStaffIdx() const override;

    Chord* chord() const { return toChord(parent()); }
    bool up() const;

    qreal userLen() const { return _userLen; }
    void setUserLen(qreal l) { _userLen = l; }

    qreal lineWidth() const { return _lineWidth; }
    qreal lineWidthMag() const { return _lineWidth * mag(); }
    void setLineWidth(qreal w) { _lineWidth = w; }

    void setLen(qreal l);
    qreal len() const { return _len; }

    QPointF hookPos() const;
    qreal stemLen() const;
    QPointF p2() const { return line.p2(); }

    EditBehavior normalModeEditBehavior() const override { return EditBehavior::Edit; }
    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<QPointF> gripsPositions(const EditData&) const override;
};
}     // namespace Ms
#endif
