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

#ifndef __BARLINE_H__
#define __BARLINE_H__

#include "engravingitem.h"

namespace mu::engraving {
class Factory;
class Segment;

static constexpr int MIN_BARLINE_FROMTO_DIST        = 2;
static constexpr int MIN_BARLINE_SPAN_FROMTO        = -2;

// bar line span for 1-line staves is special: goes from 2sp above the line to 2sp below the line;
static constexpr int BARLINE_SPAN_1LINESTAFF_FROM   = -4;
static constexpr int BARLINE_SPAN_1LINESTAFF_TO     = 4;

// data for some preset bar line span types
static constexpr int BARLINE_SPAN_TICK1_FROM        = -1;
static constexpr int BARLINE_SPAN_TICK1_TO          = -7;
static constexpr int BARLINE_SPAN_TICK2_FROM        = -2;
static constexpr int BARLINE_SPAN_TICK2_TO          = -6;
static constexpr int BARLINE_SPAN_SHORT1_FROM       = 2;
static constexpr int BARLINE_SPAN_SHORT1_TO         = -2;
static constexpr int BARLINE_SPAN_SHORT2_FROM       = 1;
static constexpr int BARLINE_SPAN_SHORT2_TO         = -1;

//---------------------------------------------------------
//   BarLineTableItem
//---------------------------------------------------------

struct BarLineTableItem {
    BarLineType type;
    const char* userName;         // user name, translatable
};

//---------------------------------------------------------
//   @@ BarLine
//
//   @P barLineType  enum  (BarLineType.NORMAL, .DOUBLE, .START_REPEAT, .END_REPEAT, .BROKEN, .END, .END_START_REPEAT, .DOTTED)
//---------------------------------------------------------

class BarLine final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, BarLine)
    DECLARE_CLASSOF(ElementType::BAR_LINE)

public:

    virtual ~BarLine();

    KerningType doComputeKerningType(const EngravingItem*) const override { return KerningType::NON_KERNING; }

    BarLine& operator=(const BarLine&) = delete;

    void setParent(Segment* parent);

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    BarLine* clone() const override { return new BarLine(*this); }
    Fraction playTick() const override;
    void draw(mu::draw::Painter*) const override;
    mu::PointF canvasPos() const override;      ///< position in canvas coordinates
    mu::PointF pagePos() const override;        ///< position in page coordinates

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;
    void setTrack(track_idx_t t) override;
    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;
    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;
    bool isEditable() const override { return true; }

    Segment* segment() const { return toSegment(explicitParent()); }
    Measure* measure() const { return toMeasure(explicitParent()->explicitParent()); }

    double y1() const { return m_y1; }
    double y2() const { return m_y2; }
    void setY1(double v) { m_y1 = v; }
    void setY2(double v) { m_y2 = v; }

    void setSpanStaff(int val) { _spanStaff = val; }
    void setSpanFrom(int val) { _spanFrom = val; }
    void setSpanTo(int val) { _spanTo = val; }
    void setShowTips(bool val);
    int spanStaff() const { return _spanStaff; }
    int spanFrom() const { return _spanFrom; }
    int spanTo() const { return _spanTo; }
    bool showTips() const;

    void startEdit(EditData& ed) override;
    bool isEditAllowed(EditData&) const override;
    bool edit(EditData& ed) override;
    void editDrag(EditData&) override;
    void endEditDrag(EditData&) override;
    Shape shape() const override;

    const ElementList* el() const { return &_el; }

    static String translatedUserTypeName(BarLineType);

    void setBarLineType(BarLineType i) { _barLineType = i; }
    BarLineType barLineType() const { return _barLineType; }

    int subtype() const override { return int(_barLineType); }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid propertyId) const override;
    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;
    using EngravingObject::undoChangeProperty;

    mu::RectF layoutRect() const;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;

    String accessibleInfo() const override;
    String accessibleExtraInfo() const override;

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

    static const std::vector<BarLineTableItem> barLineTable;

private:

    friend class v0::TLayout;
    friend class Factory;
    BarLine(Segment* parent);
    BarLine(const BarLine&);

    void getY() const;
    void drawDots(mu::draw::Painter* painter, double x) const;
    void drawTips(mu::draw::Painter* painter, bool reversed, double x) const;
    bool isTop() const;
    bool isBottom() const;
    void drawEditMode(mu::draw::Painter* painter, EditData& editData, double currentViewScaling) override;

    bool neverKernable() const override { return true; }

    int _spanStaff          { 0 };         // span barline to next staff if true, values > 1 are used for importing from 2.x
    int _spanFrom           { 0 };         // line number on start and end staves
    int _spanTo             { 0 };
    BarLineType _barLineType { BarLineType::NORMAL };
    mutable double m_y1 = 0.0;
    mutable double m_y2 = 0.0;
    ElementList _el;          ///< fermata or other articulations
};
} // namespace mu::engraving

#endif
