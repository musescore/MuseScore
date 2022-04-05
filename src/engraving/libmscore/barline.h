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
#include "mscore.h"

namespace mu::engraving {
class Factory;
}

namespace Ms {
class MuseScoreView;
class Segment;

static const int MIN_BARLINE_FROMTO_DIST        = 2;
static const int MIN_BARLINE_SPAN_FROMTO        = -2;

// bar line span for 1-line staves is special: goes from 2sp above the line to 2sp below the line;
static const int BARLINE_SPAN_1LINESTAFF_FROM   = -4;
static const int BARLINE_SPAN_1LINESTAFF_TO     = 4;

// data for some preset bar line span types
static const int BARLINE_SPAN_TICK1_FROM        = -1;
static const int BARLINE_SPAN_TICK1_TO          = -7;
static const int BARLINE_SPAN_TICK2_FROM        = -2;
static const int BARLINE_SPAN_TICK2_TO          = -6;
static const int BARLINE_SPAN_SHORT1_FROM       = 2;
static const int BARLINE_SPAN_SHORT1_TO         = -2;
static const int BARLINE_SPAN_SHORT2_FROM       = 1;
static const int BARLINE_SPAN_SHORT2_TO         = -1;

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
    int _spanStaff          { 0 };         // span barline to next staff if true, values > 1 are used for importing from 2.x
    int _spanFrom           { 0 };         // line number on start and end staves
    int _spanTo             { 0 };
    BarLineType _barLineType { BarLineType::NORMAL };
    mutable qreal y1;
    mutable qreal y2;
    ElementList _el;          ///< fermata or other articulations

    friend class mu::engraving::Factory;
    BarLine(Segment* parent);
    BarLine(const BarLine&);

    void getY() const;
    void drawDots(mu::draw::Painter* painter, qreal x) const;
    void drawTips(mu::draw::Painter* painter, bool reversed, qreal x) const;
    bool isTop() const;
    bool isBottom() const;
    void drawEditMode(mu::draw::Painter* painter, EditData& editData, qreal currentViewScaling) override;

public:

    virtual ~BarLine();

    BarLine& operator=(const BarLine&) = delete;

    void setParent(Segment* parent);

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObject* scanChild(int idx) const override;
    int scanChildCount() const override;

    BarLine* clone() const override { return new BarLine(*this); }
    Fraction playTick() const override;
    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;
    void draw(mu::draw::Painter*) const override;
    mu::PointF canvasPos() const override;      ///< position in canvas coordinates
    mu::PointF pagePos() const override;        ///< position in page coordinates
    void layout() override;
    void layout2();
    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;
    void setTrack(int t) override;
    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;
    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;
    bool isEditable() const override { return true; }

    Segment* segment() const { return toSegment(explicitParent()); }
    Measure* measure() const { return toMeasure(explicitParent()->explicitParent()); }

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

    static QString userTypeName(BarLineType);
    static const BarLineTableItem* barLineTableItem(unsigned);

    void setBarLineType(BarLineType i) { _barLineType = i; }
    BarLineType barLineType() const { return _barLineType; }

    int subtype() const override { return int(_barLineType); }

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid propertyId) const override;
    Pid propertyId(const QStringRef& xmlName) const override;
    void undoChangeProperty(Pid id, const mu::engraving::PropertyValue&, PropertyFlags ps) override;
    using EngravingObject::undoChangeProperty;

    static qreal layoutWidth(Score*, BarLineType);
    mu::RectF layoutRect() const;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;

    QString accessibleInfo() const override;
    QString accessibleExtraInfo() const override;

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return 1; }
    Grip initialEditModeGrip() const override { return Grip::START; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

    static const std::vector<BarLineTableItem> barLineTable;
};
}     // namespace Ms

#endif
