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
    const muse::TranslatableString& userName;
};

//---------------------------------------------------------
//   BarLineEditData
//---------------------------------------------------------

class BarLineEditData : public ElementEditData
{
    OBJECT_ALLOCATOR(engraving, BarLineEditData)
public:
    double yoff1;
    double yoff2;
    virtual EditDataType type() override { return EditDataType::BarLineEditData; }
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

    BarLine& operator=(const BarLine&) = delete;

    void setParent(Segment* parent);

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    BarLine* clone() const override { return new BarLine(*this); }
    Fraction playTick() const override;
    PointF canvasPos() const override;      ///< position in canvas coordinates
    PointF pagePos() const override;        ///< position in page coordinates

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;
    void setTrack(track_idx_t t) override;
    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;
    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;
    bool isEditable() const override { return true; }

    Segment* segment() const { return toSegment(explicitParent()); }
    Measure* measure() const { return toMeasure(explicitParent()->explicitParent()); }

    void setSpanStaff(int val) { m_spanStaff = val; }
    void setSpanFrom(int val) { m_spanFrom = val; }
    void setSpanTo(int val) { m_spanTo = val; }
    void setShowTips(bool val);
    int spanStaff() const { return m_spanStaff; }
    int spanFrom() const { return m_spanFrom; }
    int spanTo() const { return m_spanTo; }
    bool showTips() const;

    void startEdit(EditData& ed) override;
    bool isEditAllowed(EditData&) const override;
    bool edit(EditData& ed) override;
    void editDrag(EditData&) override;
    void endEditDrag(EditData&) override;

    const ElementList* el() const { return &m_el; }

    static String translatedUserTypeName(BarLineType);

    void setBarLineType(BarLineType i) { m_barLineType = i; }
    BarLineType barLineType() const { return m_barLineType; }

    bool isTop() const;
    bool isBottom() const;

    void setPlayCountTextSetting(const AutoCustomHide& v) { m_playCountTextSetting = v; }
    AutoCustomHide playCountTextSetting() const { return m_playCountTextSetting; }

    int subtype() const override { return int(m_barLineType); }
    TranslatableString subtypeUserName() const override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid propertyId) const override;
    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;
    using EngravingObject::undoChangeProperty;
    EngravingItem* propertyDelegate(Pid) override;

    Text* playCountText() const { return m_playCountText; }
    void setPlayCountText(Text* text);
    String playCountCustomText() const { return m_playCountCustomText; }

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;

    String accessibleInfo() const override;
    String accessibleExtraInfo() const override;

    void setSelected(bool f) override;
    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<PointF> gripsPositions(const EditData&) const override;

    void styleChanged() override;

    static const std::vector<BarLineTableItem> barLineTable;

    void calcY();

    struct LayoutData : public EngravingItem::LayoutData {
        double y1 = 0.0;
        double y2 = 0.0;
    };

    DECLARE_LAYOUTDATA_METHODS(BarLine)

private:

    friend class Factory;
    BarLine(Segment* parent);
    BarLine(const BarLine&);

    int m_spanStaff = 0;         // span barline to next staff if true, values > 1 are used for importing from 2.x
    int m_spanFrom = 0;         // line number on start and end staves
    int m_spanTo = 0;
    BarLineType m_barLineType = BarLineType::NORMAL;

    ElementList m_el;          ///< fermata or other articulations

    Text* m_playCountText = nullptr;     // Play count text for barlines on system object staves
    AutoCustomHide m_playCountTextSetting = AutoCustomHide::AUTO;
    String m_playCountCustomText = u"";
};
} // namespace mu::engraving
