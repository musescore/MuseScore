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
#pragma once

#include "engravingitem.h"
#include "line.h"
#include "property.h"
#include "textbase.h"

namespace mu::engraving {
enum class GuitarBendType : unsigned char {
    BEND,
    PRE_BEND,
    GRACE_NOTE_BEND,
    SLIGHT_BEND,

    DIVE,
    PRE_DIVE,
    DIP,
    SCOOP,
};

enum class GuitarBendShowHoldLine : unsigned char {
    AUTO,
    SHOW,
    HIDE,
};

enum class QuarterOffset : unsigned char {
    QUARTER_FLAT,
    NONE,
    QUARTER_SHARP
};

enum class ActionIconType : signed char;

class GuitarBend final : public SLine
{
    OBJECT_ALLOCATOR(engraving, GuitarBend)
    DECLARE_CLASSOF(ElementType::GUITAR_BEND)

    M_PROPERTY2(DirectionV, direction, setDirection, DirectionV::AUTO)
    M_PROPERTY2(int, bendAmountInQuarterTones, setBendAmountInQuarterTones, 4)
    M_PROPERTY2(GuitarBendShowHoldLine, showHoldLine, setShowHoldLine, GuitarBendShowHoldLine::AUTO)
    M_PROPERTY2(float, startTimeFactor, setStartTimeFactor, 0.f)
    M_PROPERTY2(float, endTimeFactor, setEndTimeFactor, 1.f)

public:
    static constexpr float GRACE_NOTE_BEND_DEFAULT_END_TIME_FACTOR = 0.25f;
    static constexpr float DIP_DEFAULT_START_TIME_FACTOR = 0.25;
    static constexpr float DIP_DEFAULT_END_TIME_FACTOR = 0.5;

    GuitarBend(EngravingItem* parent);
    GuitarBend(const GuitarBend&);
    ~GuitarBend() override;

    GuitarBend* clone() const override { return new GuitarBend(*this); }

    LineSegment* createLineSegment(System* parent) override;

    bool allowTimeAnchor() const override { return false; }

    Note* startNote() const;
    Note* startNoteOfChain() const;

    Note* endNote() const;
    void changeBendAmount(int bendAmount);
    void setEndNotePitch(int pitch, QuarterOffset quarterOff = QuarterOffset::NONE);

    bool isReleaseBend() const;
    bool isFullRelease() const;
    bool angledPreBend() const;
    bool isDive() const;
    bool isFullReleaseDive() const;

    static void fixNotesFrettingForGraceBend(Note* grace, Note* main);
    static void fixNotesFrettingForStandardBend(Note* startNote, Note* endNote);
    static Note* createEndNote(Note* startNote, GuitarBendType bendType = GuitarBendType::BEND);

    PropertyValue getProperty(Pid id) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid id) const override;

    static constexpr int SLACK_BEND_AMOUNT = -33;
    void computeBendAmount();
    int totBendAmountIncludingPrecedingBends() const;
    void computeBendText();
    void computeIsInvalidOrNeedsWarning();
    bool isInvalid() const { return m_isInvalid; }
    bool isBorderlineUnplayable() const { return m_isBorderlineUnplayable; }

    GuitarBend* findPrecedingBend() const;
    GuitarBend* findFollowingPreDive() const;
    WhammyBar* findOverlappingWhammyBar(Fraction startTick, Fraction endTick) const;

    void updateHoldLine();
    void setHoldLine(GuitarBendHold* hold) { m_holdLine = hold; }
    GuitarBendHold* holdLine() const { return m_holdLine; }

    double lineWidth() const;

    Color curColor(const rendering::PaintOptions& opt) const override;

    DirectionV diveTabPos() const { return m_diveTabPos; }
    void setDiveTabPos(DirectionV v) { m_diveTabPos = v; }

    static void adaptBendsFromTabToStandardStaff(const Staff* staff);

    static GuitarBendType bendTypeFromActionIcon(ActionIconType actionIconType);
    GuitarBendType bendType() const { return m_bendType; }
    void setBendType(GuitarBendType t);

    VibratoType dipVibratoType() const { return m_dipVibratoType; }
    void setDipVibratoType(VibratoType v) { m_dipVibratoType = v; }

    bool isSlack() const { return m_isSlack; }
    void setIsSlack(bool v) { m_isSlack = v; }

    struct LayoutData : public SLine::LayoutData
    {
    public:
        bool up() const { return m_up; }
        void setUp(bool v) { m_up = v; }

        bool isInside() const { return m_isInside; }
        void setIsInside(bool v) { m_isInside = v; }

        const String& bendDigit() const { return m_bendDigit; }
        void setBendDigit(const String& s) { m_bendDigit = s; }

        const std::set<int>& diveLevels() const { return m_diveLevels; }
        void resetDiveLevels() { m_diveLevels.clear(); }
        void setDiveLevels(const std::set<int>& v) { m_diveLevels = v; }

        bool aboveStaff() const { return m_aboveStaff; }
        void setAboveStaff(bool v) { m_aboveStaff = v; }

    private:
        bool m_up = true;
        bool m_isInside = false;
        String m_bendDigit;
        std::set<int> m_diveLevels;
        bool m_aboveStaff = false;
    };
    DECLARE_LAYOUTDATA_METHODS(GuitarBend)

private:
    GuitarBendType m_bendType = GuitarBendType::BEND;
    GuitarBendHold* m_holdLine = nullptr;
    bool m_isInvalid = false;
    bool m_isBorderlineUnplayable = false;
    DirectionV m_diveTabPos = DirectionV::AUTO;
    VibratoType m_dipVibratoType = VibratoType::NONE;
    bool m_isSlack = false;
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
    EngravingObject* propertyDelegate(Pid id) const override;

    PointF vertexPointOff() const { return m_vertexPointOff; }
    void setVertexPointOff(PointF p) { m_vertexPointOff = p; }

    void reset() override;

    double lineWidth() const;

    void scanElements(std::function<void(EngravingItem*)> func) override;

    GuitarBendText* bendText() const { return m_text; }
    void setBendText(GuitarBendText* t) { m_text = t; }

    bool isUserModified() const override;

    std::vector<LineF> gripAnchorLines(Grip) const override { return {}; }

    Color curColor(const rendering::PaintOptions& opt) const override
    {
        return guitarBend()->curColor(opt);
    }

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
    void startDragGrip(EditData& ed) override;
    void dragGrip(EditData& ed) override;

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

    PropertyValue propertyDefault(Pid id) const override;

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

    std::vector<LineF> gripAnchorLines(Grip) const override { return {}; }

    struct LayoutData : LineSegment::LayoutData {
    public:
        const SymIdList& symIds() const { return m_symIds; }
        void setSymIds(const SymIdList& v) { m_symIds = v; }

    private:
        SymIdList m_symIds;
    };
    DECLARE_LAYOUTDATA_METHODS(GuitarBendHoldSegment)

private:
    void dragGrip(EditData& ed) override;

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

    bool positionRelativeToNoteheadRest() const override { return false; }
};
}
