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

#include "element.h"
#include "mscore.h"

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
    const char* name;
};

//---------------------------------------------------------
//   @@ BarLine
//
//   @P barLineType  enum  (BarLineType.NORMAL, .DOUBLE, .START_REPEAT, .END_REPEAT, .BROKEN, .END, .END_START_REPEAT, .DOTTED)
//---------------------------------------------------------

class BarLine final : public Element
{
    int _spanStaff          { 0 };         // span barline to next staff if true, values > 1 are used for importing from 2.x
    int _spanFrom           { 0 };         // line number on start and end staves
    int _spanTo             { 0 };
    BarLineType _barLineType { BarLineType::NORMAL };
    mutable qreal y1;
    mutable qreal y2;
    ElementList _el;          ///< fermata or other articulations

    void getY() const;
    void drawDots(mu::draw::Painter* painter, qreal x) const;
    void drawTips(mu::draw::Painter* painter, bool reversed, qreal x) const;
    bool isTop() const;
    bool isBottom() const;
    void drawEditMode(mu::draw::Painter*, EditData&) override;

public:
    BarLine(Score* s = 0);
    virtual ~BarLine();
    BarLine(const BarLine&);
    BarLine& operator=(const BarLine&) = delete;

    // Score Tree functions
    ScoreElement* treeParent() const override;
    ScoreElement* treeChild(int idx) const override;
    int treeChildCount() const override;

    BarLine* clone() const override { return new BarLine(*this); }
    ElementType type() const override { return ElementType::BAR_LINE; }
    Fraction playTick() const override;
    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;
    void draw(mu::draw::Painter*) const override;
    mu::PointF canvasPos() const override;      ///< position in canvas coordinates
    mu::PointF pagePos() const override;        ///< position in page coordinates
    void layout() override;
    void layout2();
    void scanElements(void* data, void (* func)(void*, Element*), bool all=true) override;
    void setTrack(int t) override;
    void setScore(Score* s) override;
    void add(Element*) override;
    void remove(Element*) override;
    bool acceptDrop(EditData&) const override;
    Element* drop(EditData&) override;
    bool isEditable() const override { return true; }

    Segment* segment() const { return toSegment(parent()); }
    Measure* measure() const { return toMeasure(parent()->parent()); }

    void setSpanStaff(int val) { _spanStaff = val; }
    void setSpanFrom(int val) { _spanFrom = val; }
    void setSpanTo(int val) { _spanTo = val; }
    void setShowTips(bool val);
    int spanStaff() const { return _spanStaff; }
    int spanFrom() const { return _spanFrom; }
    int spanTo() const { return _spanTo; }
    bool showTips() const;

    void startEdit(EditData& ed) override;
    void editDrag(EditData&) override;
    void endEditDrag(EditData&) override;
    Shape shape() const override;

    ElementList* el() { return &_el; }
    const ElementList* el() const { return &_el; }

    static QString userTypeName(BarLineType);
    static const BarLineTableItem* barLineTableItem(unsigned);

    QString barLineTypeName() const;
    static QString barLineTypeName(BarLineType t);
    void setBarLineType(const QString& s);
    void setBarLineType(BarLineType i) { _barLineType = i; }
    BarLineType barLineType() const { return _barLineType; }
    static BarLineType barLineType(const QString&);

    int subtype() const override { return int(_barLineType); }

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid propertyId) const override;
    Pid propertyId(const QStringRef& xmlName) const override;
    void undoChangeProperty(Pid id, const QVariant&, PropertyFlags ps) override;
    using ScoreElement::undoChangeProperty;

    static qreal layoutWidth(Score*, BarLineType);
    mu::RectF layoutRect() const;

    Element* nextSegmentElement() override;
    Element* prevSegmentElement() override;

    QString accessibleInfo() const override;
    QString accessibleExtraInfo() const override;

    EditBehavior normalModeEditBehavior() const override { return EditBehavior::Edit; }
    int gripsCount() const override { return 2; }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::START; }
    std::vector<mu::PointF> gripsPositions(const EditData&) const override;

    static const std::vector<BarLineTableItem> barLineTable;
};
}     // namespace Ms

#endif
