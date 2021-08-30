/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
    mu::LineF line;   // p1 is attached to notehead
    qreal _lineWidth;
    qreal _userLen;
    qreal _len       { 0.0 };       // always positive

public:
    Stem(Chord* parent = 0);
    Stem& operator=(const Stem&) = delete;

    Stem* clone() const override { return new Stem(*this); }
    void draw(mu::draw::Painter*) const override;
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

    mu::PointF hookPos() const;
    qreal stemLen() const;
    mu::PointF p2() const { return line.p2(); }

    EditBehavior normalModeEditBehavior() const override { return EditBehavior::Edit; }
    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;
};
}     // namespace Ms
#endif
