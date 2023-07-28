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

#include "box.h"

#include <cmath>

#include "actionicon.h"
#include "factory.h"
#include "layoutbreak.h"
#include "mscore.h"
#include "score.h"
#include "stafftext.h"
#include "system.h"
#include "text.h"
#include "textframe.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::draw;

namespace mu::engraving {
static const ElementStyle boxStyle {
    { Sid::systemFrameDistance,                Pid::TOP_GAP },
    { Sid::frameSystemDistance,                Pid::BOTTOM_GAP },
};

static const ElementStyle hBoxStyle {
};

Box::Box(const ElementType& type, System* parent)
    : MeasureBase(type, parent)
{
}

//---------------------------------------------------------
//   computeMinWidth
//---------------------------------------------------------

void HBox::computeMinWidth()
{
    setWidth(point(boxWidth()) + topGap() + bottomGap());    // top/bottom is really left/right
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Box::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    if (score() && score()->printing()) {
        return;
    }

    const bool showHighlightedFrame = selected() || dropTarget();
    const bool showFrame = showHighlightedFrame || (score() ? score()->showFrames() : false);

    if (showFrame) {
        double lineWidth = spatium() * .15;
        Pen pen;
        pen.setWidthF(lineWidth);
        pen.setJoinStyle(PenJoinStyle::MiterJoin);
        pen.setCapStyle(PenCapStyle::SquareCap);
        pen.setColor(showHighlightedFrame ? engravingConfiguration()->selectionColor() : engravingConfiguration()->formattingMarksColor());
        pen.setDashPattern({ 1, 3 });

        painter->setBrush(BrushStyle::NoBrush);
        painter->setPen(pen);
        lineWidth *= 0.5;
        painter->drawRect(bbox().adjusted(lineWidth, lineWidth, -lineWidth, -lineWidth));
    }
}

bool Box::isEditAllowed(EditData&) const
{
    return false;
}

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Box::edit(EditData&)
{
    return false;
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void Box::startEditDrag(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    if (isHBox()) {
        eed->pushProperty(Pid::BOX_WIDTH);
    } else {
        eed->pushProperty(Pid::BOX_HEIGHT);
    }
}

void Box::editDrag(EditData& ed)
{
    if (isVBox()) {
        _boxHeight += Spatium(ed.delta.y() / spatium());
        if (ed.vRaster) {
            double vRaster = 1.0 / MScore::vRaster();
            int n = lrint(_boxHeight.val() / vRaster);
            _boxHeight = Spatium(vRaster * n);
        }
        bbox().setRect(0.0, 0.0, system()->width(), point(boxHeight()));
        system()->setHeight(height());
        triggerLayout();
    } else {
        _boxWidth += Spatium(ed.delta.x() / spatium());
        if (ed.hRaster) {
            double hRaster = 1.0 / MScore::hRaster();
            int n = lrint(_boxWidth.val() / hRaster);
            _boxWidth = Spatium(hRaster * n);
        }
        triggerLayout();
    }

    rendering()->layoutItem(this);
}

void Box::endEdit(EditData&)
{
    rendering()->layoutItem(this);
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<mu::PointF> HBox::gripsPositions(const EditData&) const
{
    RectF r(abbox());
    return { PointF(r.right(), r.top() + r.height() * .5) };
}

std::vector<PointF> VBox::gripsPositions(const EditData&) const
{
    RectF r(abbox());
    return { PointF(r.x() + r.width() * .5, r.bottom()) };
}

//---------------------------------------------------------
//   add
///   Add new EngravingItem \a el to Box
//---------------------------------------------------------

void Box::add(EngravingItem* e)
{
    if (e->isText()) {
        toText(e)->setLayoutToParentWidth(true);
    }
    MeasureBase::add(e);
}

RectF Box::contentRect() const
{
    RectF result;

    for (const EngravingItem* element : el()) {
        result = result.united(element->bbox());
    }

    return result;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Box::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BOX_HEIGHT:
        return _boxHeight;
    case Pid::BOX_WIDTH:
        return _boxWidth;
    case Pid::TOP_GAP:
        return _topGap;
    case Pid::BOTTOM_GAP:
        return _bottomGap;
    case Pid::LEFT_MARGIN:
        return _leftMargin;
    case Pid::RIGHT_MARGIN:
        return _rightMargin;
    case Pid::TOP_MARGIN:
        return _topMargin;
    case Pid::BOTTOM_MARGIN:
        return _bottomMargin;
    case Pid::BOX_AUTOSIZE:
        return (score()->mscVersion() >= 302) ? _isAutoSizeEnabled : false;
    default:
        return MeasureBase::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Box::setProperty(Pid propertyId, const PropertyValue& v)
{
    score()->addRefresh(canvasBoundingRect());
    switch (propertyId) {
    case Pid::BOX_HEIGHT:
        _boxHeight = v.value<Spatium>();
        break;
    case Pid::BOX_WIDTH:
        _boxWidth = v.value<Spatium>();
        break;
    case Pid::TOP_GAP:
        _topGap = v.value<Millimetre>();
        break;
    case Pid::BOTTOM_GAP:
        _bottomGap = v.value<Millimetre>();
        break;
    case Pid::LEFT_MARGIN:
        _leftMargin = v.toDouble();
        break;
    case Pid::RIGHT_MARGIN:
        _rightMargin = v.toDouble();
        break;
    case Pid::TOP_MARGIN:
        _topMargin = v.toDouble();
        break;
    case Pid::BOTTOM_MARGIN:
        _bottomMargin = v.toDouble();
        break;
    case Pid::BOX_AUTOSIZE:
        _isAutoSizeEnabled = v.toBool();
        break;
    default:
        return MeasureBase::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Box::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::BOX_HEIGHT:
    case Pid::BOX_WIDTH:
        return Spatium(0.0);

    case Pid::TOP_GAP:
        return isHBox() ? Millimetre(0.0) : style().styleMM(Sid::systemFrameDistance);
    case Pid::BOTTOM_GAP:
        return isHBox() ? Millimetre(0.0) : style().styleMM(Sid::frameSystemDistance);

    case Pid::LEFT_MARGIN:
    case Pid::RIGHT_MARGIN:
    case Pid::TOP_MARGIN:
    case Pid::BOTTOM_MARGIN:
        return 0.0;
    case Pid::BOX_AUTOSIZE:
        return true;
    default:
        return MeasureBase::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   copyValues
//---------------------------------------------------------

void Box::copyValues(Box* origin)
{
    _boxHeight    = origin->boxHeight();
    _boxWidth     = origin->boxWidth();

    double factor  = magS() / origin->magS();
    _bottomGap    = origin->bottomGap() * factor;
    _topGap       = origin->topGap() * factor;
    _bottomMargin = origin->bottomMargin() * factor;
    _topMargin    = origin->topMargin() * factor;
    _leftMargin   = origin->leftMargin() * factor;
    _rightMargin  = origin->rightMargin() * factor;
}

//---------------------------------------------------------
//   HBox
//---------------------------------------------------------

HBox::HBox(System* parent)
    : Box(ElementType::HBOX, parent)
{
    initElementStyle(&hBoxStyle);
    setBoxWidth(Spatium(5.0));
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Box::acceptDrop(EditData& data) const
{
    if (data.dropElement->flag(ElementFlag::ON_STAFF)) {
        return false;
    }
    if (MScore::debugMode) {
        LOGD("<%s>", data.dropElement->typeName());
    }
    ElementType t = data.dropElement->type();
    switch (t) {
    case ElementType::LAYOUT_BREAK:
    case ElementType::TEXT:
    case ElementType::STAFF_TEXT:
    case ElementType::IMAGE:
    case ElementType::SYMBOL:
        return true;
    case ElementType::ACTION_ICON:
        switch (toActionIcon(data.dropElement)->actionType()) {
        case ActionIconType::VFRAME:
        case ActionIconType::TFRAME:
        case ActionIconType::FFRAME:
        case ActionIconType::HFRAME:
        case ActionIconType::MEASURE:
            return true;
        default:
            break;
        }
        break;
    case ElementType::BAR_LINE:
        return isHBox();
    default:
        break;
    }
    return false;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* Box::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    if (e->flag(ElementFlag::ON_STAFF)) {
        return 0;
    }
    if (MScore::debugMode) {
        LOGD("<%s>", e->typeName());
    }
    switch (e->type()) {
    case ElementType::LAYOUT_BREAK:
    {
        LayoutBreak* lb = toLayoutBreak(e);
        if (pageBreak() || lineBreak()) {
            if (
                (lb->isPageBreak() && pageBreak())
                || (lb->isLineBreak() && lineBreak())
                || (lb->isSectionBreak() && sectionBreak())
                ) {
                //
                // if break already set
                //
                delete lb;
                break;
            }
            for (EngravingItem* elem : el()) {
                if (elem->type() == ElementType::LAYOUT_BREAK) {
                    score()->undoChangeElement(elem, e);
                    break;
                }
            }
            break;
        }
        lb->setTrack(mu::nidx);                 // these are system elements
        lb->setParent(this);
        score()->undoAddElement(lb);
        return lb;
    }

    case ElementType::STAFF_TEXT:
    {
        Text* text = Factory::createText(this, TextStyleType::FRAME);
        text->setParent(this);
        text->setXmlText(toStaffText(e)->xmlText());
        score()->undoAddElement(text);
        delete e;
        return text;
    }

    case ElementType::ACTION_ICON:
        switch (toActionIcon(e)->actionType()) {
        case ActionIconType::VFRAME:
            score()->insertMeasure(ElementType::VBOX, this);
            break;
        case ActionIconType::TFRAME:
            score()->insertMeasure(ElementType::TBOX, this);
            break;
        case ActionIconType::FFRAME:
            score()->insertMeasure(ElementType::FBOX, this);
            break;
        case ActionIconType::HFRAME:
            score()->insertMeasure(ElementType::HBOX, this);
            break;
        case ActionIconType::MEASURE:
            score()->insertMeasure(ElementType::MEASURE, this);
            break;
        default:
            break;
        }
        break;

    case ElementType::TEXT:
    case ElementType::IMAGE:
    case ElementType::SYMBOL:
        e->setParent(this);
        score()->undoAddElement(e);
        return e;
    default:
        return 0;
    }
    return 0;
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

RectF HBox::drag(EditData& data)
{
    RectF r(canvasBoundingRect());
    double diff = data.evtDelta.x();
    double x1   = offset().x() + diff;
    if (explicitParent()->type() == ElementType::VBOX) {
        VBox* vb = toVBox(explicitParent());
        double x2 = parentItem()->width() - width() - (vb->leftMargin() + vb->rightMargin()) * DPMM;
        if (x1 < 0.0) {
            x1 = 0.0;
        } else if (x1 > x2) {
            x1 = x2;
        }
    }
    setOffset(PointF(x1, 0.0));
//      setStartDragPosition(data.delta);
    return canvasBoundingRect().united(r);
}

//---------------------------------------------------------
//   isMovable
//---------------------------------------------------------

bool HBox::isMovable() const
{
    return explicitParent() && (explicitParent()->isHBox() || explicitParent()->isVBox());
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue HBox::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::CREATE_SYSTEM_HEADER:
        return createSystemHeader();
    default:
        return Box::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool HBox::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::CREATE_SYSTEM_HEADER:
        setCreateSystemHeader(v.toBool());
        triggerLayout();
        break;
    default:
        return Box::setProperty(propertyId, v);
    }
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue HBox::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::CREATE_SYSTEM_HEADER:
        return true;
    default:
        return Box::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   VBox
//---------------------------------------------------------

VBox::VBox(const ElementType& type, System* parent)
    : Box(type, parent)
{
    initElementStyle(&boxStyle);
    setBoxHeight(Spatium(10.0));
    setLineBreak(true);
}

VBox::VBox(System* parent)
    : VBox(ElementType::VBOX, parent)
{
}

double VBox::minHeight() const
{
    return point(Spatium(10));
}

double VBox::maxHeight() const
{
    return point(Spatium(30));
}

PropertyValue VBox::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BOX_AUTOSIZE:
        return isAutoSizeEnabled();
    default:
        return Box::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void VBox::startEditDrag(EditData& ed)
{
    if (isAutoSizeEnabled()) {
        setAutoSizeEnabled(false);
        setBoxHeight(Spatium(height() / spatium()));
    }
    Box::startEditDrag(ed);
}

//---------------------------------------------------------
//   add
///   Add new EngravingItem \a e to fret diagram box
//---------------------------------------------------------

void FBox::add(EngravingItem* e)
{
    e->setParent(this);
    if (e->isFretDiagram()) {
//            FretDiagram* fd = toFretDiagram(e);
//            fd->setFlag(ElementFlag::MOVABLE, false);
    } else {
        LOGD("FBox::add: element not allowed");
        return;
    }
    el().push_back(e);
    e->added();
}

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

String Box::accessibleExtraInfo() const
{
    String rez;
    for (EngravingItem* e : el()) {
        rez += u' ' + e->screenReaderInfo();
    }
    return rez;
}

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

String TBox::accessibleExtraInfo() const
{
    String rez = m_text->screenReaderInfo();
    return rez;
}

int TBox::gripsCount() const
{
    return 0;
}

Grip TBox::initialEditModeGrip() const
{
    return Grip::NO_GRIP;
}

Grip TBox::defaultGrip() const
{
    return Grip::NO_GRIP;
}
}
