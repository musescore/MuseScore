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

#ifndef __CLEF_H__
#define __CLEF_H__

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
    ClefType _concertClef = ClefType::G;
    ClefType _transposingClef = ClefType::G;

    ClefTypeList() {}
    ClefTypeList(ClefType a, ClefType b)
        : _concertClef(a), _transposingClef(b) {}
    ClefTypeList(ClefType a)
        : _concertClef(a), _transposingClef(a) {}
    bool operator==(const ClefTypeList& t) const;
    bool operator!=(const ClefTypeList& t) const;
};

//---------------------------------------------------------
//   ClefInfo
///   Info about a clef.
//---------------------------------------------------------

class ClefInfo
{
public:
    static const ClefInfo clefTable[];

    ClefType type;
    int _line;                 ///< Line positioning on the staff
    int _pitchOffset;          ///< Pitch offset for line 0.
    signed char _lines[14];
    SymId _symId;
    StaffGroup _staffGroup;

public:
    static int line(ClefType t) { return clefTable[int(t)]._line; }
    static int pitchOffset(ClefType t) { return clefTable[int(t)]._pitchOffset; }
    static SymId symId(ClefType t) { return clefTable[int(t)]._symId; }
    static const signed char* lines(ClefType t) { return clefTable[int(t)]._lines; }
    static StaffGroup staffGroup(ClefType t) { return clefTable[int(t)]._staffGroup; }
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

    void setForInstrumentChange(bool forInstrumentChange) { m_forInstrumentChange = forInstrumentChange; }
    bool forInstrumentChange() const { return m_forInstrumentChange; }

    ClefTypeList clefTypeList() const { return m_clefTypes; }
    ClefType concertClef() const { return m_clefTypes._concertClef; }
    ClefType transposingClef() const { return m_clefTypes._transposingClef; }
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

    struct LayoutData : public EngravingItem::LayoutData {
        SymId symId = SymId::noSym;
    };

    DECLARE_LAYOUTDATA_METHODS(Clef);

    //! --- Old Interface ---
    SymId symId() const { return layoutData()->symId; }
    void setSymId(SymId s) { mutLayoutData()->symId = s; }
    //! ---------------------

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
