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

#ifndef __CHORDLINE_H__
#define __CHORDLINE_H__

#include "element.h"
#include "draw/painterpath.h"

namespace Ms {
class Chord;

// subtypes:
enum class ChordLineType : char {
    NOTYPE, FALL, DOIT,
    PLOP, SCOOP
};

//---------------------------------------------------------
//   @@ ChordLine
///    bezier line attached to top note of a chord
///    implements fall, doit, plop, bend
//---------------------------------------------------------

class ChordLine final : public Element
{
    ChordLineType _chordLineType;
    bool _straight;
    mu::PainterPath path;
    bool modified;
    qreal _lengthX;
    qreal _lengthY;
    const int _initialLength = 2;

public:
    ChordLine(Score*);
    ChordLine(const ChordLine&);

    ChordLine* clone() const override { return new ChordLine(*this); }
    ElementType type() const override { return ElementType::CHORDLINE; }

    void setChordLineType(ChordLineType);
    ChordLineType chordLineType() const { return _chordLineType; }
    Chord* chord() const { return (Chord*)(parent()); }
    bool isStraight() const { return _straight; }
    void setStraight(bool straight) { _straight =  straight; }
    void setLengthX(qreal length) { _lengthX = length; }
    void setLengthY(qreal length) { _lengthY = length; }

    void read(XmlReader&) override;
    void write(XmlWriter& xml) const override;
    void layout() override;
    void draw(mu::draw::Painter*) const override;

    void startEditDrag(EditData&) override;
    void editDrag(EditData&) override;

    QString accessibleInfo() const override;

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid) const override;
    Pid propertyId(const QStringRef& xmlName) const override;

    Element::EditBehavior normalModeEditBehavior() const override { return Element::EditBehavior::Edit; }
    int gripsCount() const override { return _straight ? 1 : path.elementCount(); }
    Grip initialEditModeGrip() const override { return Grip(gripsCount() - 1); }
    Grip defaultGrip() const override { return initialEditModeGrip(); }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;
};

extern const char* scorelineNames[];
}     // namespace Ms
#endif
