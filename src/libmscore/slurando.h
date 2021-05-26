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

#ifndef __SLURANDO_H__
#define __SLURANDO_H__

#include "tie.h"
#include "property.h"

namespace Ms {
//---------------------------------------------------------
// SlurandoSegment
//---------------------------------------------------------

class SlurandoSegment final : public TieSegment
{
public:
    SlurandoSegment(Score* s)
        : TieSegment(s) {}
    SlurandoSegment(const SlurandoSegment& ss)
        : TieSegment(ss) {}

    SlurandoSegment* clone() const override { return new SlurandoSegment(*this); }
    ElementType type() const override { return ElementType::SLURANDO_SEGMENT; }
    int subtype() const override { return static_cast<int>(spanner()->type()); }
    void draw(mu::draw::Painter*) const override;
};

//---------------------------------------------------------
// Slurando
//---------------------------------------------------------

class Slurando final : public SlurTie
{
    M_PROPERTY(SlurandoType, slurType, setSlurType)
    M_PROPERTY(SlurandoTextPlacement, textPlace, setTextPlace)
    M_PROPERTY(QString, text, setText)
    M_PROPERTY(QString, fontFace, setFontFace)
    M_PROPERTY(qreal, fontSize, setFontSize)
    M_PROPERTY(FontStyle, fontStyle, setFontStyle)
    QRectF _labelBox;
public:
    Slurando(Score* = 0);

    Slurando* clone() const override { return new Slurando(*this); }
    ElementType type() const override { return ElementType::SLURANDO; }

    void setStartNote(Note* note);
    void setEndNote(Note* note) { setEndElement((Element*)note); }
    Note* startNote() const;
    Note* endNote() const;

    bool noteHasTieOrSlurando(const Note*) const;
    void calculateDirection();
    void slurPos(SlurPos*) override;

    QFont getFont() const;
    bool showLabel() const;
    const QString& getLabel() const;
    QRectF labelBox() const { return _labelBox; }

    void write(XmlWriter& xml) const override;
    void read(XmlReader& e) override;

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid) const override;

    void layout() override;
    void layoutLabel();
    qreal layoutWidthTab(const Note*, Chord*);
    qreal layoutWidth(const Note*, Chord*);

    SlurandoSegment* layoutFor(System*);
    SlurandoSegment* layoutBack(System*);

    SlurandoSegment* frontSegment() { return toSlurandoSegment(Spanner::frontSegment()); }
    const SlurandoSegment* frontSegment() const { return toSlurandoSegment(Spanner::frontSegment()); }
    SlurandoSegment* backSegment() { return toSlurandoSegment(Spanner::backSegment()); }
    const SlurandoSegment* backSegment() const { return toSlurandoSegment(Spanner::backSegment()); }
    SlurandoSegment* segmentAt(int n) { return toSlurandoSegment(Spanner::segmentAt(n)); }
    const SlurandoSegment* segmentAt(int n) const { return toSlurandoSegment(Spanner::segmentAt(n)); }

    SlurTieSegment* newSlurTieSegment() override { return new SlurandoSegment(score()); }
};
}     // namespace Ms
#endif
