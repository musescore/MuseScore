/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_ENGRAVING_ARPEGGIO_H
#define MU_ENGRAVING_ARPEGGIO_H

#include "engravingitem.h"

namespace mu::engraving {
class Factory;
class Chord;

//---------------------------------------------------------
//   @@ Arpeggio
//---------------------------------------------------------

enum class AnchorRebaseDirection : char {
    UP,
    DOWN
};

class Arpeggio final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Arpeggio)
    DECLARE_CLASSOF(ElementType::ARPEGGIO)

public:

    ~Arpeggio() override;
    Arpeggio* clone() const override { return new Arpeggio(*this); }

    ArpeggioType arpeggioType() const { return m_arpeggioType; }
    void setArpeggioType(ArpeggioType v) { m_arpeggioType = v; }
    const TranslatableString& arpeggioTypeName() const;

    int subtype() const override { return int(m_arpeggioType); }
    TranslatableString subtypeUserName() const override;

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
    track_idx_t endTrack() const { return track() + m_span - 1; }

    bool crossStaff() const;
    staff_idx_t vStaffIdx() const override;
    void findAndAttachToChords();
    void detachFromChords(track_idx_t strack, track_idx_t etrack);
    void rebaseStartAnchor(AnchorRebaseDirection direction);
    void rebaseEndAnchor(AnchorRebaseDirection direction);

    double userLen1() const { return m_userLen1; }
    double userLen2() const { return m_userLen2; }
    void setUserLen1(double v) { m_userLen1 = v; }
    void setUserLen2(double v) { m_userLen2 = v; }

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
    std::vector<PointF> gripsPositions(const EditData& = EditData()) const override;

    struct LayoutData : public EngravingItem::LayoutData {
        // cache
        double top = 0.0;
        double bottom = 0.0;
        double magS = 0.0;
        double maxChordPad = 0.0;
        double minChordX = 0.0;

        // out
        SymIdList symbols;
        RectF symsBBox;
        double arpeggioHeight = -1.0;
    };
    DECLARE_LAYOUTDATA_METHODS(Arpeggio)

private:

    friend class Factory;

    Arpeggio(Chord* parent);

    void spatiumChanged(double /*oldValue*/, double /*newValue*/) override;
    std::vector<LineF> dragAnchorLines() const override;
    std::vector<LineF> gripAnchorLines(Grip) const override;
    void startEdit(EditData&) override;
    void startEditDrag(EditData&) override;

    ArpeggioType m_arpeggioType = ArpeggioType::NORMAL;
    double m_userLen1 = 0.0;
    double m_userLen2 = 0.0;

    int m_span = 1;                // how many voices the arpeggio spans
    bool m_playArpeggio = true;

    double m_stretch = 1.0;
};
} // namespace mu::engraving

#endif
