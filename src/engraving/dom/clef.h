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

#ifndef MU_ENGRAVING_CLEF_H
#define MU_ENGRAVING_CLEF_H

/**
 \file
 Definition of classes Clef
*/

#include "engravingitem.h"

namespace mu::engraving {
class Factory;
class Segment;

static const int NO_CLEF = -1000;

//---------------------------------------------------------
//   ClefTypeList
//---------------------------------------------------------

struct ClefTypeList {
    ClefType concertClef = ClefType::G;
    ClefType transposingClef = ClefType::G;

    ClefTypeList() {}
    ClefTypeList(ClefType a, ClefType b)
        : concertClef(a), transposingClef(b) {}
    ClefTypeList(ClefType a)
        : concertClef(a), transposingClef(a) {}
    bool operator==(const ClefTypeList& t) const;
    bool operator!=(const ClefTypeList& t) const;
};

//---------------------------------------------------------
//   ClefInfo
///   Info about a clef.
//---------------------------------------------------------

struct ClefInfo
{
    static const ClefInfo clefTable[];

    ClefType m_type = ClefType::INVALID;
    int m_line = 0;                 ///< Line positioning on the staff
    int m_pitchOffset = 0;          ///< Pitch offset for line 0.
    signed char m_lines[14];
    SymId m_symId = SymId::noSym;
    StaffGroup m_staffGroup = StaffGroup::STANDARD;

    static int line(ClefType t) { return clefTable[int(t)].m_line; }
    static int pitchOffset(ClefType t) { return clefTable[int(t)].m_pitchOffset; }
    static SymId symId(ClefType t) { return clefTable[int(t)].m_symId; }
    static const signed char* lines(ClefType t) { return clefTable[int(t)].m_lines; }
    static StaffGroup staffGroup(ClefType t) { return clefTable[int(t)].m_staffGroup; }
};

//---------------------------------------------------------
//   @@ Clef
///    Graphic representation of a clef.
//
//   @P showCourtesy  bool    show/hide courtesy clef when applicable
//   @P isSmall       bool    small, mid-staff clef (read only, set by layout)
//---------------------------------------------------------

class Clef final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Clef)
    DECLARE_CLASSOF(ElementType::CLEF)

public:

    Clef* clone() const override { return new Clef(*this); }
    double mag() const override;

    Segment* segment() const { return (Segment*)explicitParent(); }
    Measure* measure() const { return (Measure*)explicitParent()->explicitParent(); }

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    bool isEditable() const override { return false; }

    bool isSmall() const { return m_isSmall; }
    void setSmall(bool val);

    bool showCourtesy() const { return m_showCourtesy; }
    void setShowCourtesy(bool v) { m_showCourtesy = v; }
    void undoSetShowCourtesy(bool v);
    Clef* otherClef();

    ClefType clefType() const;
    void setClefType(ClefType i);

    int subtype() const override { return int(clefType()); }
    TranslatableString subtypeUserName() const override;

    void setForInstrumentChange(bool forInstrumentChange) { m_forInstrumentChange = forInstrumentChange; }
    bool forInstrumentChange() const { return m_forInstrumentChange; }

    ClefTypeList clefTypeList() const { return m_clefTypes; }
    ClefType concertClef() const { return m_clefTypes.concertClef; }
    ClefType transposingClef() const { return m_clefTypes.transposingClef; }
    void setConcertClef(ClefType val);
    void setTransposingClef(ClefType val);
    void setClefType(const ClefTypeList& ctl) { m_clefTypes = ctl; }
    void spatiumChanged(double oldValue, double newValue) override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;
    String accessibleInfo() const override;
    void clear();

    void changeClefToBarlinePos(ClefToBarlinePosition newPos);

    void undoChangeProperty(Pid id, const PropertyValue& v) { EngravingObject::undoChangeProperty(id, v); }
    void undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps) override;

    ClefToBarlinePosition clefToBarlinePosition() const { return m_clefToBarlinePosition; }
    void setClefToBarlinePosition(ClefToBarlinePosition val) { m_clefToBarlinePosition = val; }
    bool isHeader() const { return m_isHeader; }
    void setIsHeader(bool val) { m_isHeader = val; }

    bool isMidMeasureClef() const;

    bool canBeExcludedFromOtherParts() const override { return !isHeader(); }
    void manageExclusionFromParts(bool exclude) override;

    struct LayoutData : public EngravingItem::LayoutData {
        SymId symId = SymId::noSym;
    };
    DECLARE_LAYOUTDATA_METHODS(Clef)

private:

    friend class Factory;
    Clef(Segment* parent);

    bool m_showCourtesy = true;
    bool m_isSmall = false;
    bool m_forInstrumentChange = false;
    bool m_isHeader = false;
    ClefToBarlinePosition m_clefToBarlinePosition = ClefToBarlinePosition::AUTO;
    ClefTypeList m_clefTypes = ClefType::INVALID;
};
} // namespace mu::engraving
#endif
