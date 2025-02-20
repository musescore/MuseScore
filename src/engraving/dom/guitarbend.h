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

#ifndef MU_ENGRAVING_GUITARBEND_H
#define MU_ENGRAVING_GUITARBEND_H

#include "engravingitem.h"
#include "line.h"
#include "property.h"
#include "textbase.h"
#include "types.h"

namespace mu::engraving {
enum class QuarterOffset : unsigned char {
    QUARTER_FLAT,
    NONE,
    QUARTER_SHARP
};

class GuitarBend final : public SLine
{
    OBJECT_ALLOCATOR(engraving, GuitarBend)
    DECLARE_CLASSOF(ElementType::GUITAR_BEND)

    M_PROPERTY2(DirectionV, direction, setDirection, DirectionV::AUTO)
    M_PROPERTY2(GuitarBendType, type, setType, GuitarBendType::BEND)
    M_PROPERTY2(int, bendAmountInQuarterTones, setBendAmountInQuarterTones, 4)
    M_PROPERTY2(GuitarBendShowHoldLine, showHoldLine, setShowHoldLine, GuitarBendShowHoldLine::AUTO)
    M_PROPERTY2(float, startTimeFactor, setStartTimeFactor, 0.f)
    M_PROPERTY2(float, endTimeFactor, setEndTimeFactor, 1.f)

public:
    static constexpr float GRACE_NOTE_BEND_DEFAULT_END_TIME_FACTOR = 0.25f;

    GuitarBend(EngravingItem* parent);
    GuitarBend(const GuitarBend&);
    ~GuitarBend() override;

    GuitarBend* clone() const override { return new GuitarBend(*this); }

    LineSegment* createLineSegment(System* parent) override;

    bool allowTimeAnchor() const override { return false; }

    Note* startNote() const;
    Note* startNoteOfChain() const;

    Note* endNote() const;
    void setEndNotePitch(int pitch, QuarterOffset quarterOff = QuarterOffset::NONE);

    bool isReleaseBend() const;
    bool isFullRelease() const;
    bool angledPreBend() const;

    static void fixNotesFrettingForGraceBend(Note* grace, Note* main);
    static void fixNotesFrettingForStandardBend(Note* startNote, Note* endNote);
    static Note* createEndNote(Note* startNote);

    PropertyValue getProperty(Pid id) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid id) const override;

    void computeBendAmount();
    int totBendAmountIncludingPrecedingBends() const;
    void computeBendText();
    void computeIsInvalidOrNeedsWarning();
    bool isInvalid() const { return m_isInvalid; }
    bool isBorderlineUnplayable() const { return m_isBorderlineUnplayable; }

    GuitarBend* findPrecedingBend() const;

    void updateHoldLine();
    void setHoldLine(GuitarBendHold* hold) { m_holdLine = hold; }
    GuitarBendHold* holdLine() const { return m_holdLine; }

    double lineWidth() const;

    Color uiColor() const;

    static void adaptBendsFromTabToStandardStaff(const Staff* staff);

    struct LayoutData : public SLine::LayoutData
    {
    public:
        bool up() const { return m_up; }
        void setUp(bool v) { m_up = v; }

        bool isInside() const { return m_isInside; }
        void setIsInside(bool v) { m_isInside = v; }

        const String& bendDigit() const { return m_bendDigit; }
        void setBendDigit(const String& s) { m_bendDigit = s; }

    private:
        bool m_up = true;
        bool m_isInside = false;
        String m_bendDigit;
    };
    DECLARE_LAYOUTDATA_METHODS(GuitarBend)

private:
    GuitarBendHold* m_holdLine = nullptr;
    bool m_isInvalid = false;
    bool m_isBorderlineUnplayable = false;
};

class GuitarBendText; // forward decl
class GuitarBendSegment final : public LineSegment
{
    OBJECT_ALLOCATOR(engraving, GuitarBendSegment)
    DECLARE_CLASSOF(ElementType::GUITAR_BEND_SEGMENT)

public:
    GuitarBendSegment(GuitarBend* sp, System* parent);
    GuitarBendSegment(const GuitarBendSegment&);
    ~GuitarBendSegment() override;

    GuitarBend* guitarBend() const { return toGuitarBend(spanner()); }

    GuitarBendSegment* clone() const override { return new GuitarBendSegment(*this); }

    int gripsCount() const override { return 4; }
    std::vector<PointF> gripsPositions(const EditData& = EditData()) const override;

    PropertyValue getProperty(Pid id) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid id) const override;
    EngravingItem* propertyDelegate(Pid id) override;

    PointF vertexPointOff() const { return m_vertexPointOff; }
    void setVertexPointOff(PointF p) { m_vertexPointOff = p; }

    void reset() override;

    double lineWidth() const;

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all) override;

    GuitarBendText* bendText() const { return m_text; }
    void setBendText(GuitarBendText* t) { m_text = t; }

    bool isUserModified() const override;

    Color uiColor() const { return guitarBend()->uiColor(); }

    struct LayoutData : public LineSegment::LayoutData
    {
    public:
        const PointF& vertexPoint() const { return m_vertexPoint; }
        void setVertexPoint(PointF p) { m_vertexPoint = p; }

        const PainterPath& path() const { return m_path; }
        void setPath(const PainterPath& path) { m_path = path; }

        const PolygonF& arrow() const { return m_arrow; }
        PolygonF& arrow() { return m_arrow; }
        void setArrow(const PolygonF& a) { m_arrow = a; }

    private:
        PointF m_vertexPoint = PointF(); // Coordinates relative to pos()
        PainterPath m_path = PainterPath();
        PolygonF m_arrow = PolygonF();
    };
    DECLARE_LAYOUTDATA_METHODS(GuitarBendSegment)

private:
    void startEditDrag(EditData& ed) override;
    void editDrag(EditData& ed) override;

    PointF m_vertexPointOff = PointF();
    GuitarBendText* m_text = nullptr;
};

class GuitarBendHold final : public SLine
{
    OBJECT_ALLOCATOR(engraving, GuitarBendHold)
    DECLARE_CLASSOF(ElementType::GUITAR_BEND_HOLD)

public:
    GuitarBendHold(GuitarBend* parent);
    GuitarBendHold(const GuitarBendHold&);

    GuitarBendHold* clone() const override { return new GuitarBendHold(*this); }

    LineSegment* createLineSegment(System* parent) override;

    bool allowTimeAnchor() const override { return false; }

    Note* startNote() const;
    Note* endNote() const;

    GuitarBend* guitarBend() const { return toGuitarBend(explicitParent()); }

    double lineWidth() const;
};

class GuitarBendHoldSegment final : public LineSegment
{
    OBJECT_ALLOCATOR(engraving, GuitarBendHoldSegment)
    DECLARE_CLASSOF(ElementType::GUITAR_BEND_HOLD_SEGMENT)

public:
    GuitarBendHoldSegment(GuitarBendHold* sp, System* parent);

    GuitarBendHold* guitarBendHold() const { return toGuitarBendHold(spanner()); }

    GuitarBendHoldSegment* clone() const override { return new GuitarBendHoldSegment(*this); }

    GuitarBendSegment* startBendSeg() const { return m_startBendSeg; }
    GuitarBendSegment* endBendSeg() const { return m_endBendSeg; }
    void setStartBendSeg(GuitarBendSegment* s) { m_startBendSeg = s; }
    void setEndBendSeg(GuitarBendSegment* s) { m_endBendSeg = s; }

    double dashLength() const { return m_dashLength; }

    double lineWidth() const { return guitarBendHold()->lineWidth(); }

private:
    void editDrag(EditData& ed) override;

    double m_dashLength = 3.0;

    GuitarBendSegment* m_startBendSeg = nullptr;
    GuitarBendSegment* m_endBendSeg = nullptr;
};

class GuitarBendText final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, GuitarBendText)
    DECLARE_CLASSOF(ElementType::GUITAR_BEND_TEXT)

public:
    GuitarBendText(GuitarBendSegment* parent);
    GuitarBendText* clone() const override { return new GuitarBendText(*this); }

    bool isEditable() const override { return false; }
};
} // namespace mu::engraving

#endif // MU_ENGRAVING_GUITARBEND_H
