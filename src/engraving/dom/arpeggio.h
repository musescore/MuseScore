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

public:

    Arpeggio* clone() const override { return new Arpeggio(*this); }

    ArpeggioType arpeggioType() const { return m_arpeggioType; }
    void setArpeggioType(ArpeggioType v) { m_arpeggioType = v; }
    const TranslatableString& arpeggioTypeName() const;

    Chord* chord() const { return (Chord*)explicitParent(); }

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    bool isEditable() const override { return true; }
    void editDrag(EditData&) override;
    bool isEditAllowed(EditData&) const override;
    bool edit(EditData&) override;

    void reset() override;

    int span() const { return m_span; }
    void setSpan(int val) { m_span = val; }

    double userLen1() const { return m_userLen1; }
    double userLen2() const { return m_userLen2; }
    void setUserLen1(double v) { m_userLen1 = v; }
    void setUserLen2(double v) { m_userLen2 = v; }

    double insetDistance(std::vector<Accidental*>& accidentals, double mag_) const;

    bool playArpeggio() const { return m_playArpeggio; }
    void setPlayArpeggio(bool p) { m_playArpeggio = p; }

    double Stretch() const { return m_stretch; }
    void setStretch(double val) { m_stretch = val; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

    // TODO: add a grip for moving the entire arpeggio
    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 2; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData& = EditData()) const override;

    struct LayoutData : public EngravingItem::LayoutData {
        // cache
        double top = 0.0;
        double bottom = 0.0;
        double magS = 0.0;

        // out
        SymIdList symbols;
        RectF symsBBox;
        double arpeggioHeight = -1.0;
    };
    DECLARE_LAYOUTDATA_METHODS(Arpeggio);

private:

    friend class Factory;

    Arpeggio(Chord* parent);

    void spatiumChanged(double /*oldValue*/, double /*newValue*/) override;
    std::vector<mu::LineF> dragAnchorLines() const override;
    std::vector<mu::LineF> gripAnchorLines(Grip) const override;
    void startEdit(EditData&) override;

    double insetTop() const;
    double insetBottom() const;
    double insetWidth() const;

    ArpeggioType m_arpeggioType = ArpeggioType::NORMAL;
    double m_userLen1 = 0.0;
    double m_userLen2 = 0.0;

    int m_span = 1;                // spanning staves
    bool m_playArpeggio = true;

    double m_stretch = 1.0;
};
} // namespace mu::engraving

#endif
