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

#ifndef MU_ENGRAVING_HAIRPIN_H
#define MU_ENGRAVING_HAIRPIN_H

#include "../types/types.h"

#include "textlinebase.h"

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

    HairpinSegment* clone() const override { return new HairpinSegment(*this); }

    Hairpin* hairpin() const { return (Hairpin*)spanner(); }

    bool drawCircledTip() const { return m_drawCircledTip; }
    void setDrawCircledTip(bool arg) { m_drawCircledTip = arg; }
    double circledTipRadius() const { return m_circledTipRadius; }
    void setCircledTipRadius(double r) { m_circledTipRadius = r; }
    PointF circledTip() const { return m_circledTip; }
    void setCircledTip(const PointF& p) { m_circledTip = p; }

    EngravingItem* propertyDelegate(Pid) override;

    int subtype() const override;
    TranslatableString subtypeUserName() const override;

    int gripsCount() const override;
    std::vector<PointF> gripsPositions(const EditData& = EditData()) const override;

    std::unique_ptr<ElementGroup> getDragGroup(std::function<bool(const EngravingItem*)> isDragged) override;

    bool hasVoiceAssignmentProperties() const override { return spanner()->hasVoiceAssignmentProperties(); }

    EngravingItem* findElementToSnapBefore(bool ignoreInvisible = true) const;
    EngravingItem* findElementToSnapAfter(bool ignoreInvisible = true) const;

    void endEditDrag(EditData& ed) override;

private:
    TextBase* findStartDynamicOrExpression(bool ignoreInvisible = true) const;
    TextBase* findEndDynamicOrExpression(bool ignoreInvisible = true) const;

    void startEditDrag(EditData&) override;
    void editDrag(EditData&) override;

    Sid getPropertyStyle(Pid) const override;

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    void setPropertyFlags(Pid id, PropertyFlags f) override;

    bool m_drawCircledTip = false;
    PointF m_circledTip;
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

    DynamicType dynamicTypeFrom() const;
    DynamicType dynamicTypeTo() const;

    const Dynamic* dynamicSnappedBefore() const;
    const Dynamic* dynamicSnappedAfter() const;

    HairpinType hairpinType() const { return m_hairpinType; }
    void setHairpinType(HairpinType val);

    Segment* segment() const { return (Segment*)explicitParent(); }
    LineSegment* createLineSegment(System* parent) override;

    bool hairpinCircledTip() const { return m_hairpinCircledTip; }
    void setHairpinCircledTip(bool val) { m_hairpinCircledTip = val; }

    int veloChange() const { return m_veloChange; }
    void setVeloChange(int v) { m_veloChange = v; }

    DynamicRange dynRange() const { return m_dynRange; }
    void setDynRange(DynamicRange t);

    Spatium hairpinHeight() const { return m_hairpinHeight; }
    void setHairpinHeight(Spatium val) { m_hairpinHeight = val; }

    Spatium hairpinContHeight() const { return m_hairpinContHeight; }
    void setHairpinContHeight(Spatium val) { m_hairpinContHeight = val; }

    bool singleNoteDynamics() const { return m_singleNoteDynamics; }
    void setSingleNoteDynamics(bool val) { m_singleNoteDynamics = val; }

    ChangeMethod veloChangeMethod() const { return m_veloChangeMethod; }
    void setVeloChangeMethod(ChangeMethod val) { m_veloChangeMethod = val; }

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

    PointF linePos(Grip grip, System** system) const override;

    bool hasVoiceAssignmentProperties() const override { return true; }

    void reset() override;

    void setVoiceAssignment(VoiceAssignment v) { m_voiceAssignment = v; }
    VoiceAssignment voiceAssignment() const { return m_voiceAssignment; }
    void setDirection(DirectionV v) { m_direction = v; }
    DirectionV direction() const { return m_direction; }
    void setCenterBetweenStaves(AutoOnOff v) { m_centerBetweenStaves = v; }
    AutoOnOff centerBetweenStaves() const { return m_centerBetweenStaves; }

    bool snapToItemBefore() const { return m_snapToItemBefore; }
    void setSnapToItemBefore(bool v) { m_snapToItemBefore = v; }
    bool snapToItemAfter() const { return m_snapToItemAfter; }
    void setSnapToItemAfter(bool v) { m_snapToItemAfter = v; }

    int subtype() const override { return int(m_hairpinType); }
    TranslatableString subtypeUserName() const override;

private:

    Sid getPropertyStyle(Pid) const override;

    HairpinType m_hairpinType = HairpinType::INVALID;
    int m_veloChange = 0;
    bool m_hairpinCircledTip = false;
    DynamicRange m_dynRange = DynamicRange::PART;
    bool m_singleNoteDynamics = false;
    ChangeMethod m_veloChangeMethod = ChangeMethod::NORMAL;

    Spatium m_hairpinHeight;
    Spatium m_hairpinContHeight;

    VoiceAssignment m_voiceAssignment = VoiceAssignment::ALL_VOICE_IN_INSTRUMENT;
    DirectionV m_direction = DirectionV::AUTO;
    AutoOnOff m_centerBetweenStaves = AutoOnOff::AUTO;

    bool m_snapToItemBefore = true;
    bool m_snapToItemAfter = true;
};
} // namespace mu::engraving

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::engraving::HairpinType)
#endif

#endif
