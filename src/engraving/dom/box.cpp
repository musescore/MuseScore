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

#include "box.h"

#include <cmath>

#include "actionicon.h"
#include "factory.h"
#include "layoutbreak.h"
#include "masterscore.h"
#include "mscore.h"
#include "score.h"
#include "stafftext.h"
#include "system.h"
#include "text.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace muse::draw;

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
    setWidth(absoluteFromSpatium(boxWidth() + topGap() + bottomGap()));    // top/bottom is really left/right
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
    const double sp = sizeIsSpatiumDependent() ? spatium() : style().defaultSpatium();
    if (isVBox()) {
        m_boxHeight += Spatium(ed.delta.y() / sp);
        if (ed.vRaster) {
            double vRaster = 1.0 / MScore::vRaster();
            int n = lrint(m_boxHeight.val() / vRaster);
            m_boxHeight = Spatium(vRaster * n);
        }
        mutldata()->setBbox(0.0, 0.0, system()->width(), absoluteFromSpatium(boxHeight()));
        system()->setHeight(height());
    } else {
        m_boxWidth += Spatium(ed.delta.x() / sp);
        if (ed.hRaster) {
            double hRaster = 1.0 / MScore::hRaster();
            int n = lrint(m_boxWidth.val() / hRaster);
            m_boxWidth = Spatium(hRaster * n);
        }
    }
    triggerLayout();
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> HBox::gripsPositions(const EditData&) const
{
    RectF r(pageBoundingRect());
    return { PointF(r.right(), r.top() + r.height() * .5) };
}

std::vector<PointF> VBox::gripsPositions(const EditData&) const
{
    RectF r(pageBoundingRect());
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

double Box::absoluteFromSpatium(const Spatium& val) const
{
    const double sp = sizeIsSpatiumDependent() ? spatium() : style().defaultSpatium();
    return val.val() * sp;
}

RectF Box::contentRect() const
{
    RectF result;

    for (const EngravingItem* element : el()) {
        result = result.united(element->ldata()->bbox());
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
        return m_boxHeight;
    case Pid::BOX_WIDTH:
        return m_boxWidth;
    case Pid::TOP_GAP:
        return m_topGap;
    case Pid::BOTTOM_GAP:
        return m_bottomGap;
    case Pid::LEFT_MARGIN:
        return m_leftMargin;
    case Pid::RIGHT_MARGIN:
        return m_rightMargin;
    case Pid::TOP_MARGIN:
        return m_topMargin;
    case Pid::BOTTOM_MARGIN:
        return m_bottomMargin;
    case Pid::BOX_AUTOSIZE:
        return (score()->mscVersion() >= 302) ? m_isAutoSizeEnabled : false;
    default:
        return MeasureBase::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Box::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::BOX_HEIGHT:
        m_boxHeight = v.value<Spatium>();
        break;
    case Pid::BOX_WIDTH:
        m_boxWidth = v.value<Spatium>();
        break;
    case Pid::TOP_GAP:
        m_topGap = v.value<Spatium>();
        break;
    case Pid::BOTTOM_GAP:
        m_bottomGap = v.value<Spatium>();
        break;
    case Pid::LEFT_MARGIN:
        m_leftMargin = v.toDouble();
        break;
    case Pid::RIGHT_MARGIN:
        m_rightMargin = v.toDouble();
        break;
    case Pid::TOP_MARGIN:
        m_topMargin = v.toDouble();
        break;
    case Pid::BOTTOM_MARGIN:
        m_bottomMargin = v.toDouble();
        break;
    case Pid::BOX_AUTOSIZE:
        m_isAutoSizeEnabled = v.toBool();
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
        return isHBox() ? Spatium(0.0) : style().styleS(Sid::systemFrameDistance);
    case Pid::BOTTOM_GAP:
        return isHBox() ? Spatium(0.0) : style().styleS(Sid::frameSystemDistance);

    case Pid::LEFT_MARGIN:
    case Pid::RIGHT_MARGIN:
    case Pid::TOP_MARGIN:
    case Pid::BOTTOM_MARGIN:
        return 0.0;
    case Pid::BOX_AUTOSIZE:
        return true;
    case Pid::SIZE_SPATIUM_DEPENDENT:
        return !isTitleFrame();
    default:
        return MeasureBase::propertyDefault(id);
    }
}

bool Box::isTitleFrame() const
{
    return this == score()->first() && type() == ElementType::VBOX;
}

//---------------------------------------------------------
//   copyValues
//---------------------------------------------------------

void Box::copyValues(Box* origin)
{
    m_boxHeight    = origin->boxHeight();
    m_boxWidth     = origin->boxWidth();

    double factor  = magS() / origin->magS();
    m_bottomGap    = origin->bottomGap() * factor;
    m_topGap       = origin->topGap() * factor;
    m_bottomMargin = origin->bottomMargin() * factor;
    m_topMargin    = origin->topMargin() * factor;
    m_leftMargin   = origin->leftMargin() * factor;
    m_rightMargin  = origin->rightMargin() * factor;

    setSizeIsSpatiumDependent(origin->sizeIsSpatiumDependent());
}

//---------------------------------------------------------
//   HBox
//---------------------------------------------------------

HBox::HBox(System* parent)
    : Box(ElementType::HBOX, parent)
{
    initElementStyle(&hBoxStyle);
    resetProperty(Pid::BOX_WIDTH);
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
        lb->setTrack(0);
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
            score()->insertBox(ElementType::VBOX, this);
            break;
        case ActionIconType::TFRAME:
            score()->insertBox(ElementType::TBOX, this);
            break;
        case ActionIconType::FFRAME:
            score()->insertBox(ElementType::FBOX, this);
            break;
        case ActionIconType::HFRAME:
            score()->insertBox(ElementType::HBOX, this);
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

void Box::manageExclusionFromParts(bool exclude)
{
    // manage Layout Breaks - remove old ones first
    LayoutBreak* sectionBreak = sectionBreakElement();
    if (sectionBreak) {
        toEngravingItem(sectionBreak)->manageExclusionFromParts(true);
    }

    bool titleFrame = isTitleFrame();
    if (exclude) {
        const std::list<EngravingObject*> links = linkList();
        for (EngravingObject* linkedObject : links) {
            // Only remove title frame from score
            if (linkedObject->score() == score() || (!this->score()->isMaster() && titleFrame && !linkedObject->score()->isMaster())) {
                continue;
            }
            EngravingItem* linkedItem = toEngravingItem(linkedObject);
            if (linkedItem->selected()) {
                linkedItem->score()->deselect(linkedItem);
            }
            linkedItem->score()->undoRemoveElement(linkedItem, false);
            linkedItem->undoUnlink();
        }

        // manage Layout Breaks - there are no linked boxes, so add linked Line Breaks to previous measure
        if (sectionBreak && !titleFrame) {
            if (MeasureBase* prevMeasure = this->prevMeasure()) {
                for (Score* score : masterScore()->scoreList()) {
                    if (score == this->score()) {
                        continue;
                    }
                    if (MeasureBase* localPrevMeasure = score->tick2measure(prevMeasure->tick())) {
                        EngravingItem* newSectionBreak = sectionBreak->linkedClone();
                        newSectionBreak->setScore(score);
                        newSectionBreak->setParent(localPrevMeasure);
                        score->doUndoAddElement(newSectionBreak);
                    }
                }
            }
        }
    } else {
        for (Score* score : masterScore()->scoreList()) {
            if (score == this->score() || (titleFrame && !this->score()->isMaster() && !score->isMaster())) {
                continue;
            }

            MeasureBase* newMB = next() ? next()->getInScore(score, true) : nullptr;
            Score::InsertMeasureOptions options;
            options.cloneBoxToAllParts = false;
            MeasureBase* newFrame = score->insertBox(type(), newMB, options);
            newFrame->setExcludeFromOtherParts(false);
            // newFrame->setSizeIsSpatiumDependent(!titleFrame);

            // Clear auto generated diagrams inside fret box
            if (newFrame->isFBox()) {
                toFBox(newFrame)->clearElements();
            }

            for (EngravingItem* item : el()) {
                // Don't add instrument name from current part
                if (item->isText() && toText(item)->textStyleType() == TextStyleType::INSTRUMENT_EXCERPT) {
                    continue;
                }
                // add frame items (Layout Break, Title, ...)
                newFrame->add(item->linkedClone());
            }

            if (isTBox()) {
                Text* thisText = toTBox(this)->text();
                Text* newText = toText(thisText->linkedClone());
                toTBox(newFrame)->resetText(newText);
            }

            if (!score->isMaster() && titleFrame) {
                // Title frame - add part name
                String partLabel = score->name();
                if (!partLabel.empty()) {
                    Text* txt = Factory::createText(newFrame, TextStyleType::INSTRUMENT_EXCERPT);
                    txt->setPlainText(partLabel);
                    newFrame->add(txt);

                    score->setMetaTag(u"partName", partLabel);
                }
            }

            newFrame->linkTo(this);
        }
    }
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
    case Pid::BOX_WIDTH:
        return Spatium(5.0);
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
    resetProperty(Pid::BOX_HEIGHT);
    setLineBreak(true);
}

VBox::VBox(System* parent)
    : VBox(ElementType::VBOX, parent)
{
}

double VBox::minHeight() const
{
    return absoluteFromSpatium(Spatium(10));
}

double VBox::maxHeight() const
{
    return absoluteFromSpatium(Spatium(30));
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
//   propertyDefault
//---------------------------------------------------------

PropertyValue VBox::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::BOX_HEIGHT:
        return Spatium(10.0);
    default:
        return Box::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void VBox::startEditDrag(EditData& ed)
{
    const double sp = sizeIsSpatiumDependent() ? spatium() : style().defaultSpatium();
    if (isAutoSizeEnabled()) {
        setAutoSizeEnabled(false);
        setBoxHeight(Spatium(height() / sp));
    }
    Box::startEditDrag(ed);
}

//---------------------------------------------------------
//   add
///   Add new EngravingItem \a e to fret diagram box
//---------------------------------------------------------

FBox::FBox(System* parent)
    : VBox(ElementType::FBOX, parent)
{
    init();

    resetProperty(Pid::FRET_FRAME_TEXT_SCALE);
    resetProperty(Pid::FRET_FRAME_DIAGRAM_SCALE);
    resetProperty(Pid::FRET_FRAME_COLUMN_GAP);
    resetProperty(Pid::FRET_FRAME_ROW_GAP);
    resetProperty(Pid::FRET_FRAME_CHORDS_PER_ROW);
    resetProperty(Pid::FRET_FRAME_H_ALIGN);

    resetProperty(Pid::LEFT_MARGIN);
    resetProperty(Pid::RIGHT_MARGIN);
    resetProperty(Pid::TOP_MARGIN);
    resetProperty(Pid::BOTTOM_MARGIN);
    resetProperty(Pid::TOP_GAP);
    resetProperty(Pid::BOTTOM_GAP);
}

void FBox::init()
{
    clearElements();

    std::set<String> usedDiagrams;

    for (mu::engraving::Segment* segment = score()->firstSegment(mu::engraving::SegmentType::ChordRest); segment;
         segment = segment->next1(mu::engraving::SegmentType::ChordRest)) {
        for (EngravingItem* item : segment->annotations()) {
            if (!item || !item->part()) {
                continue;
            }

            FretDiagram* fretDiagram = FretDiagram::makeFromHarmonyOrFretDiagram(item);
            if (!fretDiagram) {
                continue;
            }

            String harmonyName = fretDiagram->harmony()->harmonyName().toLower();
            if (muse::contains(usedDiagrams, harmonyName)) {
                delete fretDiagram;
                fretDiagram = nullptr;

                continue;
            }

            add(fretDiagram);

            usedDiagrams.insert(harmonyName);
        }
    }
}

void FBox::add(EngravingItem* e)
{
    e->setParent(this);
    if (e->isFretDiagram()) {
        FretDiagram* fretDiagram = toFretDiagram(e);
        fretDiagram->setFlag(ElementFlag::MOVABLE, false);
        fretDiagram->setFlag(ElementFlag::ON_STAFF, false);

        Harmony* harmony = fretDiagram->harmony();
        harmony->setFlag(ElementFlag::MOVABLE, false);
        harmony->setFlag(ElementFlag::ON_STAFF, false);

        if (!e->eid().isValid()) {
            e->assignNewEID();
        }

        VBox::add(e);
    } else {
        LOGD("FBox::add: element not allowed");
        return;
    }
    e->added();
}

PropertyValue FBox::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::FRET_FRAME_TEXT_SCALE:
        return m_textScale;
    case Pid::FRET_FRAME_DIAGRAM_SCALE:
        return m_diagramScale;
    case Pid::FRET_FRAME_COLUMN_GAP:
        return m_columnGap;
    case Pid::FRET_FRAME_ROW_GAP:
        return m_rowGap;
    case Pid::FRET_FRAME_CHORDS_PER_ROW:
        return m_chordsPerRow;
    case Pid::FRET_FRAME_H_ALIGN:
        return static_cast<int>(m_contentAlignmentH);
    case Pid::LEFT_MARGIN:
        return m_contentAlignmentH == AlignH::LEFT ? VBox::getProperty(propertyId) : PropertyValue();
    case Pid::RIGHT_MARGIN:
        return m_contentAlignmentH == AlignH::RIGHT ? VBox::getProperty(propertyId) : PropertyValue();
    default:
        return VBox::getProperty(propertyId);
    }
}

bool FBox::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::FRET_FRAME_TEXT_SCALE:
        m_textScale = val.toDouble();
        break;
    case Pid::FRET_FRAME_DIAGRAM_SCALE:
        m_diagramScale = val.toDouble();
        break;
    case Pid::FRET_FRAME_COLUMN_GAP:
        m_columnGap = val.value<Spatium>();
        break;
    case Pid::FRET_FRAME_ROW_GAP:
        m_rowGap = val.value<Spatium>();
        break;
    case Pid::FRET_FRAME_CHORDS_PER_ROW:
        m_chordsPerRow = val.toInt();
        break;
    case Pid::FRET_FRAME_H_ALIGN:
        m_contentAlignmentH = static_cast<AlignH>(val.toInt());
        resetProperty(Pid::LEFT_MARGIN);
        resetProperty(Pid::RIGHT_MARGIN);
        break;
    default:
        return VBox::setProperty(propertyId, val);
    }

    triggerLayout();
    return true;
}

PropertyValue FBox::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::FRET_FRAME_TEXT_SCALE:
    case Pid::FRET_FRAME_DIAGRAM_SCALE:
        return 1.0;
    case Pid::FRET_FRAME_COLUMN_GAP:
    case Pid::FRET_FRAME_ROW_GAP:
        return Spatium(3.0);
    case Pid::FRET_FRAME_CHORDS_PER_ROW:
        return 8;
    case Pid::FRET_FRAME_H_ALIGN:
        return static_cast<int>(AlignH::HCENTER);
    case Pid::TOP_GAP:
    case Pid::BOTTOM_GAP:
        return 4.0;
    default:
        return VBox::propertyDefault(propertyId);
    }
}

std::vector<PointF> FBox::gripsPositions(const EditData&) const
{
    return {};
}

void FBox::undoReorderElements(const std::vector<EID>& newOrderElementsIds)
{
    score()->undo(new ReorderFBox(this, newOrderElementsIds));
    triggerLayout();
}

//---------------------------------------------------------
//   TBox
//---------------------------------------------------------

TBox::TBox(System* parent)
    : VBox(ElementType::TBOX, parent)
{
    resetProperty(Pid::BOX_HEIGHT);
    m_text  = Factory::createText(this, TextStyleType::FRAME);
    m_text->setLayoutToParentWidth(true);
    m_text->setParent(this);
}

TBox::TBox(const TBox& tbox)
    : VBox(tbox)
{
    m_text = Factory::copyText(*(tbox.m_text));
}

TBox::~TBox()
{
    delete m_text;
}

void TBox::resetText(Text* text)
{
    if (m_text) {
        delete m_text;
    }
    m_text = text;
    text->setParent(this);
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* TBox::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    switch (e->type()) {
    case ElementType::TEXT:
        m_text->undoChangeProperty(Pid::TEXT, toText(e)->xmlText());
        delete e;
        return m_text;
    default:
        return VBox::drop(data);
    }
}

//---------------------------------------------------------
//   add
///   Add new EngravingItem \a el to TBox
//---------------------------------------------------------

void TBox::add(EngravingItem* e)
{
    if (e->isText()) {
        // does not normally happen, since drop() handles this directly
        m_text->undoChangeProperty(Pid::TEXT, toText(e)->xmlText());
        e->setParent(this);
        e->added();
    } else {
        VBox::add(e);
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TBox::remove(EngravingItem* el)
{
    if (el == m_text) {
        // does not normally happen, since Score::deleteItem() handles this directly
        // but if it does:
        // replace with new empty text element
        // this keeps undo/redo happier than just clearing the text
        LOGD("TBox::remove() - replacing _text");
        m_text = Factory::createText(this, TextStyleType::FRAME);
        m_text->setLayoutToParentWidth(true);
        m_text->setParent(this);
        el->removed();
    } else {
        VBox::remove(el);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue TBox::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::BOX_HEIGHT:
        return Spatium(1);
    default:
        return VBox::propertyDefault(id);
    }
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
