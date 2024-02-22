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

#ifndef MU_ENGRAVING_HAIRPIN_H
#define MU_ENGRAVING_HAIRPIN_H

#include "textlinebase.h"

#include "types/types.h"

namespace mu::engraving {
class Hairpin;

enum class HairpinType : signed char {
    INVALID = -1,
    CRESC_HAIRPIN,
    DECRESC_HAIRPIN,
    CRESC_LINE,
    DECRESC_LINE
};

//---------------------------------------------------------
//   @@ HairpinSegment
//---------------------------------------------------------

class HairpinSegment final : public TextLineBaseSegment
{
    OBJECT_ALLOCATOR(engraving, HairpinSegment)
    DECLARE_CLASSOF(ElementType::HAIRPIN_SEGMENT)

public:
    HairpinSegment(Hairpin* sp, System* parent);

    int subtype() const override;

    HairpinSegment* clone() const override { return new HairpinSegment(*this); }

    Hairpin* hairpin() const { return (Hairpin*)spanner(); }

    bool drawCircledTip() const { return m_drawCircledTip; }
    void setDrawCircledTip(bool arg) { m_drawCircledTip = arg; }
    double circledTipRadius() const { return m_circledTipRadius; }
    void setCircledTipRadius(double r) { m_circledTipRadius = r; }
    mu::PointF circledTip() const { return m_circledTip; }
    void setCircledTip(const mu::PointF& p) { m_circledTip = p; }

    EngravingItem* propertyDelegate(Pid) override;

    int gripsCount() const override;
    std::vector<mu::PointF> gripsPositions(const EditData& = EditData()) const override;

    std::unique_ptr<ElementGroup> getDragGroup(std::function<bool(const EngravingItem*)> isDragged) override;

private:

    void startEditDrag(EditData&) override;
    void editDrag(EditData&) override;

    Sid getPropertyStyle(Pid) const override;

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    bool m_drawCircledTip = false;
    mu::PointF m_circledTip;
    double m_circledTipRadius = 0.0;
};

//---------------------------------------------------------
//   @@ Hairpin
//   @P dynRange     enum (Dynamic.STAFF, Dynamic.PART, Dynamic.SYSTEM)
//   @P hairpinType  enum (Hairpin.CRESCENDO, Hairpin.DECRESCENDO)
//   @P veloChange   int
//---------------------------------------------------------

class Hairpin final : public TextLineBase
{
    OBJECT_ALLOCATOR(engraving, Hairpin)
    DECLARE_CLASSOF(ElementType::HAIRPIN)

public:
    Hairpin(Segment* parent);

    Hairpin* clone() const override { return new Hairpin(*this); }

    int subtype() const override;

    DynamicType dynamicTypeFrom() const;
    DynamicType dynamicTypeTo() const;

    HairpinType hairpinType() const { return m_hairpinType; }
    void setHairpinType(HairpinType val);

    Segment* segment() const { return (Segment*)explicitParent(); }
    LineSegment* createLineSegment(System* parent) override;

    bool hairpinCircledTip() const { return m_hairpinCircledTip; }
    void setHairpinCircledTip(bool val) { m_hairpinCircledTip = val; }

    int veloChange() const { return m_veloChange; }
    void setVeloChange(int v) { m_veloChange = v; }

    DynamicRange dynRange() const { return m_dynRange; }
    void setDynRange(DynamicRange t) { m_dynRange = t; }

    Spatium hairpinHeight() const { return m_hairpinHeight; }
    void setHairpinHeight(Spatium val) { m_hairpinHeight = val; }

    Spatium hairpinContHeight() const { return m_hairpinContHeight; }
    void setHairpinContHeight(Spatium val) { m_hairpinContHeight = val; }

    bool singleNoteDynamics() const { return m_singleNoteDynamics; }
    void setSingleNoteDynamics(bool val) { m_singleNoteDynamics = val; }

    ChangeMethod veloChangeMethod() const { return m_veloChangeMethod; }
    void setVeloChangeMethod(ChangeMethod val) { m_veloChangeMethod = val; }

    bool playHairpin() const { return m_playHairpin; }
    void setPlayHairpin(bool val) { m_playHairpin = val; }

    bool isCrescendo() const
    {
        return m_hairpinType == HairpinType::CRESC_HAIRPIN || m_hairpinType == HairpinType::CRESC_LINE;
    }

    bool isDecrescendo() const
    {
        return m_hairpinType == HairpinType::DECRESC_HAIRPIN || m_hairpinType == HairpinType::DECRESC_LINE;
    }

    PropertyValue getProperty(Pid id) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    String accessibleInfo() const override;
    bool isLineType() const
    {
        return m_hairpinType == HairpinType::CRESC_LINE || m_hairpinType == HairpinType::DECRESC_LINE;
    }

private:

    Sid getPropertyStyle(Pid) const override;

    HairpinType m_hairpinType = HairpinType::INVALID;
    int m_veloChange = 0;
    bool m_hairpinCircledTip = false;
    DynamicRange m_dynRange = DynamicRange::STAFF;
    bool m_singleNoteDynamics = false;
    ChangeMethod m_veloChangeMethod = ChangeMethod::NORMAL;
    bool m_playHairpin = false;

    Spatium m_hairpinHeight;
    Spatium m_hairpinContHeight;
};
} // namespace mu::engraving

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::engraving::HairpinType)
#endif

#endif
