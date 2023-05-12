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

#ifndef __ARPEGGIO_H__
#define __ARPEGGIO_H__

#include "engravingitem.h"

namespace mu::engraving {
class Factory;
class Chord;

//---------------------------------------------------------
//   @@ Arpeggio
//---------------------------------------------------------

class Arpeggio final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Arpeggio)
    DECLARE_CLASSOF(ElementType::ARPEGGIO)

    ArpeggioType _arpeggioType;
    double _userLen1;
    double _userLen2;
    double _height;
    int _span;                // spanning staves
    SymIdList m_symbols;
    bool _playArpeggio;

    double _stretch;

    friend class Factory;
    Arpeggio(Chord* parent);

    void spatiumChanged(double /*oldValue*/, double /*newValue*/) override;
    std::vector<mu::LineF> dragAnchorLines() const override;
    std::vector<mu::LineF> gripAnchorLines(Grip) const override;
    void startEdit(EditData&) override;

    double calcTop() const;
    double calcBottom() const;

private:

    double insetTop() const;
    double insetBottom() const;
    double insetWidth() const;

public:

    Arpeggio* clone() const override { return new Arpeggio(*this); }

    ArpeggioType arpeggioType() const { return _arpeggioType; }
    void setArpeggioType(ArpeggioType v) { _arpeggioType = v; }
    const TranslatableString& arpeggioTypeName() const;

    Chord* chord() const { return (Chord*)explicitParent(); }

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    void draw(mu::draw::Painter* painter) const override;
    bool isEditable() const override { return true; }
    void editDrag(EditData&) override;
    bool isEditAllowed(EditData&) const override;
    bool edit(EditData&) override;

    void reset() override;

    int span() const { return _span; }
    void setSpan(int val) { _span = val; }
    void setHeight(double) override;
    void computeHeight(bool includeCrossStaffHeight = false);

    double userLen1() const { return _userLen1; }
    double userLen2() const { return _userLen2; }
    void setUserLen1(double v) { _userLen1 = v; }
    void setUserLen2(double v) { _userLen2 = v; }

    double insetDistance(std::vector<Accidental*>& accidentals, double mag_) const;

    bool playArpeggio() const { return _playArpeggio; }
    void setPlayArpeggio(bool p) { _playArpeggio = p; }

    double Stretch() const { return _stretch; }
    void setStretch(double val) { _stretch = val; }

    void symbolLine(SymId start, SymId fill);
    const SymIdList& symbols() { return m_symbols; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

    // TODO: add a grip for moving the entire arpeggio
    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 2; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData& = EditData()) const override;
};
} // namespace mu::engraving

#endif
