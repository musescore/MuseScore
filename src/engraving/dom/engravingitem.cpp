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

/**
 \file
 Implementation of EngravingItem, ElementList
*/

#include "engravingitem.h"

#include <cmath>

#include "containers.h"
#include "io/buffer.h"
#include "translation.h"

#include "draw/types/pen.h"
#include "iengravingfont.h"

#include "rw/rwregister.h"

#include "types/typesconv.h"

#include "rendering/score/autoplace.h"
#include "rendering/score/chordlayout.h"
#include "rendering/score/tlayout.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#include "accessibility/accessibleroot.h"
#endif

#include "chord.h"
#include "factory.h"
#include "linkedobjects.h"
#include "masterscore.h"
#include "measure.h"
#include "mscore.h"
#include "note.h"
#include "page.h"
#include "parenthesis.h"
#include "score.h"
#include "segment.h"
#include "shape.h"
#include "sig.h"
#include "staff.h"
#include "stafflines.h"
#include "stafftype.h"
#include "system.h"
#include "undo.h"

#include "log.h"
#define LOG_PROP() if (0) LOGD()

using namespace mu;
using namespace muse::io;
using namespace muse::draw;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

namespace mu::engraving {
EngravingItem* EngravingItemList::at(size_t i) const
{
    return *std::next(begin(), i);
}

EngravingItem::EngravingItem(const ElementType& type, EngravingObject* parent, ElementFlags f)
    : EngravingObject(type, parent)
{
    m_flags         = f;
    m_color         = configuration()->defaultColor();
    m_z             = -1;
    m_minDistance   = Spatium(0.0);
}

EngravingItem::EngravingItem(const EngravingItem& e)
    : EngravingObject(e)
{
    m_offset     = e.m_offset;
    m_track      = e.m_track;
    m_flags      = e.m_flags;
    setFlag(ElementFlag::SELECTED, false);
    m_z          = e.m_z;
    m_color      = e.m_color;
    m_minDistance = e.m_minDistance;
    m_excludeVerticalAlign = e.m_excludeVerticalAlign;
    itemDiscovered = false;

    m_accessibleEnabled = e.m_accessibleEnabled;
}

EngravingItem::~EngravingItem()
{
    Score::onElementDestruction(this);

    delete m_layoutData;
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
void EngravingItem::setupAccessible()
{
    if (m_accessible) {
        return;
    }

    static const std::set<ElementType> accessibleDisabled = {
        ElementType::LEDGER_LINE
    };

    if (score() && !score()->isPaletteScore()) {
        if (!muse::contains(accessibleDisabled, type())) {
            m_accessible = createAccessible();
            m_accessible->setup();
        }
    }
}

#endif

bool EngravingItem::accessibleEnabled() const
{
    return m_accessibleEnabled;
}

void EngravingItem::setAccessibleEnabled(bool enabled)
{
    m_accessibleEnabled = enabled;
}

EngravingItem* EngravingItem::parentItem(bool explicitParent) const
{
    EngravingObject* p = explicitParent ? this->explicitParent() : parent();
    if (p && p->isEngravingItem()) {
        return static_cast<EngravingItem*>(p);
    }

    return nullptr;
}

static void collectChildrenItems(const EngravingObject* item, EngravingItemList& list, bool all)
{
    for (EngravingObject* ch : item->children()) {
        if (ch->isEngravingItem()) {
            list.push_back(static_cast<EngravingItem*>(ch));

            if (all) {
                collectChildrenItems(ch, list, all);
            }
        }
    }
}

EngravingItemList EngravingItem::childrenItems(bool all) const
{
    EngravingItemList list;
    collectChildrenItems(this, list, all);
    return list;
}

const muse::modularity::ContextPtr& EngravingItem::iocContext() const
{
    return score()->iocContext();
}

const std::shared_ptr<IEngravingConfiguration>& EngravingItem::configuration() const
{
    return score()->configuration.get();
}

const std::shared_ptr<rendering::IScoreRenderer>& EngravingItem::renderer() const
{
    return score()->renderer.get();
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
AccessibleItemPtr EngravingItem::createAccessible()
{
    return std::make_shared<AccessibleItem>(this);
}

void EngravingItem::notifyAboutNameChanged()
{
    if (!selected()) {
        return;
    }

    if (m_accessible) {
        doInitAccessible();
        m_accessible->accessibleRoot()->notifyAboutFocusedElementNameChanged();
    }
}

#endif

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void EngravingItem::spatiumChanged(double oldValue, double newValue)
{
    if (offsetIsSpatiumDependent()) {
        m_offset *= (newValue / oldValue);
    }
}

//---------------------------------------------------------
//   localSpatiumChanged
//    the scale of a staff changed
//---------------------------------------------------------

void EngravingItem::localSpatiumChanged(double oldValue, double newValue)
{
    if (offsetIsSpatiumDependent()) {
        m_offset *= (newValue / oldValue);
    }
}

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

double EngravingItem::spatium() const
{
    if (systemFlag() || (explicitParent() && parentItem()->systemFlag())) {
        return style().spatium();
    }
    Staff* s = staff();
    return s ? s->spatium(this) : style().spatium();
}

bool EngravingItem::isInteractionAvailable() const
{
    if (!getProperty(Pid::VISIBLE).toBool() && (score()->printing() || !score()->isShowInvisible())) {
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   offsetIsSpatiumDependent
//---------------------------------------------------------

bool EngravingItem::offsetIsSpatiumDependent() const
{
    return sizeIsSpatiumDependent() || (m_flags & ElementFlag::ON_STAFF);
}

//---------------------------------------------------------
//   magS
//---------------------------------------------------------

double EngravingItem::magS() const
{
    return mag() * (style().spatium() / SPATIUM20);
}

//---------------------------------------------------------
//   name
//---------------------------------------------------------

TranslatableString EngravingItem::subtypeUserName() const
{
    return {};
}

String EngravingItem::translatedSubtypeUserName() const
{
    return subtypeUserName().translated();
}

//---------------------------------------------------------
//   linkedClone
//---------------------------------------------------------

EngravingItem* EngravingItem::linkedClone()
{
    EngravingItem* e = clone();
    score()->undo(new Link(e, this));
    return e;
}

//---------------------------------------------------------
//   deleteLater
//---------------------------------------------------------

void EngravingItem::deleteLater()
{
    if (selected()) {
        score()->deselect(this);
    }
    masterScore()->deleteLater(this);
}

//---------------------------------------------------------
//   scanElements
/// If leaf node, apply `func` on this element (after checking if it is visible).
/// Otherwise, recurse over all children (see ScoreElement::scanElements).
/// Note: This function is overridden in some classes to skip certain children,
/// or to apply `func` even to non-leaf nodes.
//---------------------------------------------------------

void EngravingItem::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    if (m_leftParenthesis) {
        func(data, m_leftParenthesis);
    }

    if (m_rightParenthesis) {
        func(data, m_rightParenthesis);
    }

    if (scanChildren().size() == 0) {
        if (all || visible() || score()->isShowInvisible()) {
            func(data, this);
        }
    } else {
        EngravingObject::scanElements(data, func, all);
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void EngravingItem::reset()
{
    undoResetProperty(Pid::AUTOPLACE);
    undoResetProperty(Pid::PLACEMENT);
    undoResetProperty(Pid::MIN_DISTANCE);
    undoResetProperty(Pid::OFFSET);
    undoResetProperty(Pid::LEADING_SPACE);
    setOffsetChanged(false);
    EngravingObject::reset();
}

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void EngravingItem::change(EngravingItem* o, EngravingItem* n)
{
    remove(o);
    add(n);
}

//---------------------------------------------------------
//   staff
//---------------------------------------------------------

Staff* EngravingItem::staff() const
{
    if (!hasStaff() || score()->staves().empty()) {
        return nullptr;
    }

    return score()->staff(staffIdx());
}

bool EngravingItem::hasStaff() const
{
    return m_track != muse::nidx;
}

//---------------------------------------------------------
//   staffType
//---------------------------------------------------------

const StaffType* EngravingItem::staffType() const
{
    Staff* s = staff();
    return s ? s->staffTypeForElement(this) : nullptr;
}

//---------------------------------------------------------
//   onTabStaff
//---------------------------------------------------------

bool EngravingItem::onTabStaff() const
{
    const StaffType* stt = staffType();
    return stt ? stt->isTabStaff() : false;
}

bool EngravingItem::hasGrips() const
{
    return gripsCount() > 0;
}

track_idx_t EngravingItem::track() const
{
    return m_track;
}

void EngravingItem::setTrack(track_idx_t val)
{
    m_track = val;
}

//---------------------------------------------------------
//   z
//---------------------------------------------------------

int EngravingItem::z() const
{
    if (m_z == -1) {
        m_z = int(type()) * 100;
    }
    return m_z;
}

void EngravingItem::setZ(int val)
{
    m_z = val;
}

staff_idx_t EngravingItem::staffIdx() const
{
    return track2staff(m_track);
}

void EngravingItem::setStaffIdx(staff_idx_t val)
{
    voice_idx_t voiceIdx = voice();
    m_track = staff2track(val, voiceIdx == muse::nidx ? 0 : voiceIdx);
}

staff_idx_t EngravingItem::effectiveStaffIdx() const
{
    if (!systemFlag()) {
        return vStaffIdx();
    }

    const System* system = toSystem(findAncestor(ElementType::SYSTEM));
    if (!system || system->vbox()) {
        return vStaffIdx();
    }

    staff_idx_t originalStaffIdx = staffIdx();
    if (originalStaffIdx == muse::nidx) {
        return muse::nidx;
    }

    const std::vector<Staff*>& systemObjectStaves = m_score->systemObjectStaves(); // CAUTION: may not be ordered
    if (originalStaffIdx > 0) {
        staff_idx_t prevSysObjStaffIdx = 0;
        for (Staff* sysObjStaff : systemObjectStaves) {
            staff_idx_t sysObjStaffIdx = sysObjStaff->idx();
            if (sysObjStaffIdx > prevSysObjStaffIdx && sysObjStaffIdx < originalStaffIdx) {
                prevSysObjStaffIdx = sysObjStaffIdx;
            }
        }

        bool omitObject = true;
        for (staff_idx_t stfIdx = prevSysObjStaffIdx; stfIdx < originalStaffIdx; ++stfIdx) {
            if (m_score->staff(stfIdx)->show() && system->staff(stfIdx)->show()) {
                omitObject = false;
                break;
            }
        }

        if (omitObject) {
            // If all staves between this and the previous system object are hidden
            // we omit this object because it will be replaced by the upper one
            return muse::nidx;
        }
    }

    if (m_score->staff(originalStaffIdx)->show() && system->staff(originalStaffIdx)->show()) {
        return originalStaffIdx;
    }

    return system->nextVisibleStaff(originalStaffIdx);
}

bool EngravingItem::isTopSystemObject() const
{
    return systemFlag() && track() == 0;
}

staff_idx_t EngravingItem::vStaffIdx() const
{
    return staffIdx();
}

voice_idx_t EngravingItem::voice() const
{
    return track2voice(m_track);
}

void EngravingItem::setVoice(voice_idx_t v)
{
    m_track = (m_track / VOICES) * VOICES + v;
}

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

Fraction EngravingItem::tick() const
{
    const EngravingItem* e = this;
    while (e->explicitParent()) {
        if (e->explicitParent()->isSegment()) {
            return toSegment(e->explicitParent())->tick();
        } else if (e->explicitParent()->isMeasureBase()) {
            return toMeasureBase(e->explicitParent())->tick();
        }
        e = e->parentItem();
    }
    return Fraction(0, 1);
}

//---------------------------------------------------------
//   rtick
//---------------------------------------------------------

Fraction EngravingItem::rtick() const
{
    const EngravingItem* e = this;
    while (e->explicitParent()) {
        if (e->explicitParent()->isSegment()) {
            return toSegment(e->explicitParent())->rtick();
        }
        e = e->parentItem();
    }
    return Fraction(0, 1);
}

//---------------------------------------------------------
//   playTick
//---------------------------------------------------------

Fraction EngravingItem::playTick() const
{
    // Play from the element's tick position by default.
    return tick();
}

//---------------------------------------------------------
//   beat
//---------------------------------------------------------

Fraction EngravingItem::beat() const
{
    // Returns an appropriate fraction of ticks for use as a "Beat" reference
    // in the Select All Similar filter.
    int bar, beat, ticks;
    TimeSigMap* tsm = score()->sigmap();
    tsm->tickValues(tick().ticks(), &bar, &beat, &ticks);
    int ticksB = ticks_beat(tsm->timesig(tick().ticks()).timesig().denominator());

    Fraction complexFraction((++beat * ticksB) + ticks, ticksB);
    return complexFraction.reduced();
}

//---------------------------------------------------------
//   part
//---------------------------------------------------------

Part* EngravingItem::part() const
{
    Staff* s = staff();
    return s ? s->part() : 0;
}

void EngravingItem::setColor(const Color& c)
{
    m_color = c;
}

Color EngravingItem::color() const
{
    return m_color;
}

//---------------------------------------------------------
//   curColor
//---------------------------------------------------------

Color EngravingItem::curColor() const
{
    return curColor(getProperty(Pid::VISIBLE).toBool());
}

//---------------------------------------------------------
//   curColor
//---------------------------------------------------------

Color EngravingItem::curColor(bool isVisible) const
{
    return curColor(isVisible, color());
}

Color EngravingItem::curColor(bool isVisible, Color normalColor) const
{
    // the default element color is always interpreted as black in printing
    if (score() && score()->printing()) {
        return (normalColor == configuration()->defaultColor()) ? Color::BLACK : normalColor;
    }

    if (flag(ElementFlag::DROP_TARGET)) {
        return configuration()->highlightSelectionColor(track() == muse::nidx ? 0 : voice());
    }

    bool marked = false;
    if (isNote()) {
        marked = toNote(this)->mark();
    }

    if (selected() || marked) {
        voice_idx_t voiceForColorChoice = track() == muse::nidx ? 0 : voice();
        if (hasVoiceAssignmentProperties()) {
            VoiceAssignment voiceAssignment = getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
            if (voiceAssignment != VoiceAssignment::CURRENT_VOICE_ONLY) {
                voiceForColorChoice = VOICES;
            }
        }
        return configuration()->selectionColor(voiceForColorChoice, isVisible, isUnlinkedFromMaster());
    }

    if (!isVisible) {
        return configuration()->invisibleColor();
    }

    if (m_colorsInversionEnabled && configuration()->scoreInversionEnabled()) {
        return normalColor.inverted();
    }

    return normalColor;
}

PointF EngravingItem::systemPos() const
{
    // Returns position in system coordinates. Only applicable to items
    // that have a System ancestor.

    IF_ASSERT_FAILED(findAncestor(ElementType::SYSTEM)) {
        return PointF();
    }

    PointF result = pos();
    EngravingItem* ancestor = parentItem();
    while (ancestor && !ancestor->isSystem()) {
        result += ancestor->pos();
        ancestor = ancestor->parentItem();
    }

    return result;
}

//---------------------------------------------------------
//   pagePos
//    return position in canvas coordinates
//---------------------------------------------------------

PointF EngravingItem::pagePos() const
{
    PointF p(pos());
    if (explicitParent() == nullptr) {
        return p;
    }

    staff_idx_t idx = effectiveStaffIdx();

    if (idx == muse::nidx) {
        idx = vStaffIdx();
    }

    if (m_flags & ElementFlag::ON_STAFF) {
        System* system = nullptr;
        Measure* measure = nullptr;
        if (explicitParent()->isSegment()) {
            measure = toSegment(explicitParent())->measure();
        } else if (explicitParent()->isMeasure()) {           // used in measure number
            measure = toMeasure(explicitParent());
        } else if (explicitParent()->isSystem()) {
            system = toSystem(explicitParent());
        } else if (explicitParent()->isFretDiagram()) {
            return p + parentItem()->pagePos();
        } else if (explicitParent()->isFBox()) {
            return p + parentItem()->pagePos();
        } else {
            ASSERT_X(String(u"this %1 parent %2\n").arg(String::fromAscii(typeName()), String::fromAscii(explicitParent()->typeName())));
        }
        if (measure) {
            system = measure->system();
            p.ry() += measure->staffLines(idx)->y();
        }
        if (system) {
            if (system->staves().size() <= idx) {
                LOGD("staffIdx out of bounds: %s", typeName());
            }
            p.ry() += system->staffYpage(idx);
        }
        p.rx() = pageX();
    } else {
        if (explicitParent()->explicitParent()) {
            p += parentItem()->pagePos();
        }
    }
    return p;
}

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

PointF EngravingItem::canvasPos() const
{
    PointF p(pos());
    if (explicitParent() == nullptr) {
        return p;
    }

    staff_idx_t idx = effectiveStaffIdx();

    if (idx == muse::nidx) {
        idx = vStaffIdx();
    }

    if (m_flags & ElementFlag::ON_STAFF) {
        System* system = nullptr;
        Measure* measure = nullptr;
        if (explicitParent()->isSegment()) {
            measure = toSegment(explicitParent())->measure();
        } else if (explicitParent()->isMeasure()) {     // used in measure number
            measure = toMeasure(explicitParent());
        }
        // system = toMeasure(parent())->system();
        else if (explicitParent()->isSystem()) {
            system = toSystem(explicitParent());
        } else if (explicitParent()->isChord()) {       // grace chord
            measure = toSegment(explicitParent()->explicitParent())->measure();
        } else if (explicitParent()->isFretDiagram()) {
            return p + parentItem()->canvasPos();
        } else {
            ASSERT_X(String(u"this %1 parent %2\n").arg(String::fromAscii(typeName()), String::fromAscii(explicitParent()->typeName())));
        }
        if (measure) {
            const StaffLines* lines = measure->staffLines(idx);
            p.ry() += lines ? lines->y() : 0;

            system = measure->system();

            if (system) {
                Page* page = system->page();
                if (page) {
                    p.ry() += page->y();
                }
            }
        }
        if (system) {
            p.ry() += system->staffYpage(idx);
        }
        p.rx() = canvasX();
    } else {
        p += parentItem()->canvasPos();
    }
    return p;
}

//---------------------------------------------------------
//   pageX
//---------------------------------------------------------

double EngravingItem::pageX() const
{
    double xp = x();
    for (EngravingItem* e = parentItem(); e && e->parentItem(); e = e->parentItem()) {
        xp += e->x();
    }
    return xp;
}

//---------------------------------------------------------
//    canvasX
//---------------------------------------------------------

double EngravingItem::canvasX() const
{
    double xp = x();
    for (EngravingItem* e = parentItem(); e; e = e->parentItem()) {
        xp += e->x();
    }
    return xp;
}

//---------------------------------------------------------
//   contains
//    Return true if p is inside the shape of the object.
//    Note: p is in page coordinates
//---------------------------------------------------------

bool EngravingItem::contains(const PointF& p) const
{
    return shape().contains(p - pagePos());
}

//---------------------------------------------------------
//  intersects
//    Return true if \a rr intersects bounding box of object.
//    Note: \a rr is in page coordinates
//---------------------------------------------------------

bool EngravingItem::intersects(const RectF& rr) const
{
    return shape().intersects(rr.translated(-pagePos()));
}

bool EngravingItem::hitShapeContains(const PointF& p) const
{
    return hitShape().contains(p - pagePos());
}

bool EngravingItem::hitShapeIntersects(const RectF& rr) const
{
    return hitShape().intersects(rr.translated(-pagePos()));
}

//---------------------------------------------------------
//   remove
///   Remove \a el from the list. Return true on success.
//---------------------------------------------------------

bool ElementList::remove(EngravingItem* el)
{
    auto i = find(begin(), end(), el);
    if (i == end()) {
        return false;
    }
    erase(i);
    return true;
}

//---------------------------------------------------------
//   replace
//---------------------------------------------------------

void ElementList::replace(EngravingItem* o, EngravingItem* n)
{
    auto i = find(begin(), end(), o);
    if (i == end()) {
        LOGD("ElementList::replace: element not found");
        return;
    }
    *i = n;
}

//---------------------------------------------------------
//   Compound
//---------------------------------------------------------

Compound::Compound(const ElementType& type, Score* s)
    : EngravingItem(type, s)
{
}

Compound::Compound(const Compound& c)
    : EngravingItem(c)
{
    m_elements.clear();
    for (EngravingItem* e : c.m_elements) {
        m_elements.push_back(e->clone());
    }
}

//---------------------------------------------------------
//   addElement
//---------------------------------------------------------

/**
 offset \a x and \a y are in Point units
*/

void Compound::addElement(EngravingItem* e, double x, double y)
{
    e->setPos(x, y);
    e->setParent(this);
    m_elements.push_back(e);
}

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Compound::setSelected(bool f)
{
    EngravingItem::setSelected(f);
    for (auto i = m_elements.begin(); i != m_elements.end(); ++i) {
        (*i)->setSelected(f);
    }
}

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Compound::setVisible(bool f)
{
    EngravingItem::setVisible(f);
    for (auto i = m_elements.begin(); i != m_elements.end(); ++i) {
        (*i)->setVisible(f);
    }
}

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Compound::clear()
{
    for (EngravingItem* e : m_elements) {
        if (e->selected()) {
            score()->deselect(e);
        }
        delete e;
    }
    m_elements.clear();
}

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void EngravingItem::dump() const
{
    const LayoutData* ldata = this->ldata();
    LOGD("---EngravingItem: %s, pos(%4.2f,%4.2f)"
         "\n   bbox(%g,%g,%g,%g)"
         "\n   abox(%g,%g,%g,%g)"
         "\n  parent: %p",
         typeName(), ldata->pos().x(), ldata->pos().y(),
         ldata->bbox().x(), ldata->bbox().y(), ldata->bbox().width(), ldata->bbox().height(),
         pageBoundingRect().x(), pageBoundingRect().y(), pageBoundingRect().width(), pageBoundingRect().height(),
         explicitParent());
}

//---------------------------------------------------------
//   mimeData
//---------------------------------------------------------

muse::ByteArray EngravingItem::mimeData(const PointF& dragOffset) const
{
    Buffer buffer;
    buffer.open(IODevice::WriteOnly);
    XmlWriter xml(&buffer);

    xml.startElement("EngravingItem");
    if (isNote()) {
        xml.tagFraction("duration", toNote(this)->chord()->ticks());
    }
    if (!dragOffset.isNull()) {
        xml.tagPoint("dragOffset", dragOffset);
    }

    rw::RWRegister::writer(iocContext())->writeItem(this, xml);

    xml.endElement();
    buffer.close();
    return buffer.data();
}

//---------------------------------------------------------
//   readType
//    return new position of QDomElement in e
//---------------------------------------------------------

ElementType EngravingItem::readType(XmlReader& e, PointF* dragOffset, Fraction* duration)
{
    while (e.readNextStartElement()) {
        if (e.name() == "EngravingItem") {
            while (e.readNextStartElement()) {
                const AsciiStringView tag = e.name();
                if (tag == "dragOffset") {
                    *dragOffset = e.readPoint();
                } else if (tag == "duration") {
                    *duration = e.readFraction();
                } else {
                    ElementType type = TConv::fromXml(tag, ElementType::INVALID);
                    if (type == ElementType::INVALID) {
                        break;
                    }
                    return type;
                }
            }
        } else {
            e.unknown();
        }
    }
    return ElementType::INVALID;
}

//---------------------------------------------------------
//   readMimeData
//---------------------------------------------------------

EngravingItem* EngravingItem::readMimeData(Score* score, const muse::ByteArray& data, PointF* dragOffset, Fraction* duration)
{
    XmlReader e(data);

    const ElementType type = EngravingItem::readType(e, dragOffset, duration);
    if (type == ElementType::INVALID) {
        LOGD("cannot read type");
        return nullptr;
    }

    EngravingItem* el = Factory::createItem(type, score->dummy(), false);
    if (el) {
        rw::RWRegister::reader()->readItem(el, e);
    }

    return el;
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void EngravingItem::add(EngravingItem* e)
{
    switch (e->type()) {
    case ElementType::PARENTHESIS: {
        Parenthesis* p = toParenthesis(e);
        if (p->direction() == DirectionH::LEFT) {
            m_leftParenthesis = p;
        } else if (p->direction() == DirectionH::RIGHT) {
            m_rightParenthesis = p;
        }
        break;
    }
    default:
        LOGD("EngravingItem: cannot add %s to %s", e->typeName(), typeName());
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void EngravingItem::remove(EngravingItem* e)
{
    switch (e->type()) {
    case ElementType::PARENTHESIS: {
        if (e == m_leftParenthesis) {
            m_leftParenthesis = nullptr;
        }
        if (e == m_rightParenthesis) {
            m_rightParenthesis = nullptr;
        }
        break;
    }
    default:
        ASSERT_X(String(u"EngravingItem: cannot remove %1 from %2").arg(String::fromAscii(e->typeName()), String::fromAscii(typeName())));
    }
}

//---------------------------------------------------------
//   elementLessThan
//---------------------------------------------------------

bool elementLessThan(const EngravingItem* const e1, const EngravingItem* const e2)
{
    if (e1->z() == e2->z()) {
        if (e1->selected() && !e2->selected()) {
            return false;
        }
        if (!e1->selected() && e2->selected()) {
            return true;
        }
        if (e1->visible() && !e2->visible()) {
            return false;
        }
        if (!e1->visible() && e2->visible()) {
            return true;
        }

        return e1->track() < e2->track();
    }

    return e1->z() < e2->z();
}

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void collectElements(void* data, EngravingItem* e)
{
    std::vector<EngravingItem*>* el = static_cast<std::vector<EngravingItem*>*>(data);
    el->push_back(e);
}

//---------------------------------------------------------
//   autoplace
//---------------------------------------------------------

bool EngravingItem::autoplace() const
{
    if (!score() || !style().styleB(Sid::autoplaceEnabled)) {
        return false;
    }
    return !flag(ElementFlag::NO_AUTOPLACE);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue EngravingItem::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TICK:
        return tick();
    case Pid::TRACK:
        return track();
    case Pid::VOICE:
        return voice();
    case Pid::GENERATED:
        return generated();
    case Pid::COLOR:
        return PropertyValue::fromValue(color());
    case Pid::VISIBLE:
        return visible();
    case Pid::SELECTED:
        return selected();
    case Pid::OFFSET:
        return PropertyValue::fromValue(m_offset);
    case Pid::MIN_DISTANCE:
        return m_minDistance;
    case Pid::PLACEMENT:
        return placement();
    case Pid::AUTOPLACE:
        return autoplace();
    case Pid::Z:
        return z();
    case Pid::SYSTEM_FLAG:
        return systemFlag();
    case Pid::SIZE_SPATIUM_DEPENDENT:
        return sizeIsSpatiumDependent();
    case Pid::POSITION_LINKED_TO_MASTER:
        return _isPositionLinkedToMaster;
    case Pid::APPEARANCE_LINKED_TO_MASTER:
        return _isAppearanceLinkedToMaster;
    case Pid::EXCLUDE_FROM_OTHER_PARTS:
        return _excludeFromOtherParts;
    case Pid::EXCLUDE_VERTICAL_ALIGN:
        return m_excludeVerticalAlign;
    case Pid::HAS_PARENTHESES:
        return parenthesesMode();
    default:
        if (explicitParent()) {
            return explicitParent()->getProperty(propertyId);
        }

        return PropertyValue();
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool EngravingItem::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::TRACK:
        setTrack(v.value<track_idx_t>());
        break;
    case Pid::VOICE:
        setVoice(v.toInt());
        break;
    case Pid::GENERATED:
        setGenerated(v.toBool());
        break;
    case Pid::COLOR:
        setColor(v.value<Color>());
        break;
    case Pid::VISIBLE:
        setVisible(v.toBool());
        break;
    case Pid::SELECTED:
        setSelected(v.toBool());
        break;
    case Pid::OFFSET:
        m_offset = v.value<PointF>();
        break;
    case Pid::MIN_DISTANCE:
        setMinDistance(v.value<Spatium>());
        break;
    case Pid::PLACEMENT:
        setPlacement(v.value<PlacementV>());
        break;
    case Pid::AUTOPLACE:
        setAutoplace(v.toBool());
        break;
    case Pid::Z:
        setZ(v.toInt());
        break;
    case Pid::SYSTEM_FLAG:
        setSystemFlag(v.toBool());
        break;
    case Pid::SIZE_SPATIUM_DEPENDENT:
        setSizeIsSpatiumDependent(v.toBool());
        break;
    case Pid::POSITION_LINKED_TO_MASTER:
        if (_isPositionLinkedToMaster == v.toBool()) {
            break;
        }
        if (!_isPositionLinkedToMaster) {
            relinkPropertiesToMaster(PropertyGroup::POSITION);
        }
        setPositionLinkedToMaster(v.toBool());
        break;
    case Pid::APPEARANCE_LINKED_TO_MASTER:
        if (_isAppearanceLinkedToMaster == v.toBool()) {
            break;
        }
        if (!_isAppearanceLinkedToMaster) {
            relinkPropertiesToMaster(PropertyGroup::APPEARANCE);
        }
        setAppearanceLinkedToMaster(v.toBool());
        break;
    case Pid::EXCLUDE_FROM_OTHER_PARTS:
        setExcludeFromOtherParts(v.toBool());
        break;
    case Pid::EXCLUDE_VERTICAL_ALIGN:
        setExcludeVerticalAlign(v.toBool());
        break;
    case Pid::HAS_PARENTHESES:
        setParenthesesMode(v.value<ParenthesesMode>());
        if (links()) {
            for (EngravingObject* scoreElement : *links()) {
                Note* note = toNote(scoreElement);
                Staff* linkedStaff = note ? note->staff() : nullptr;
                if (linkedStaff && linkedStaff->isTabStaff(tick())) {
                    note->setGhost(v.toBool());
                }
            }
        }
        break;
    default:
        if (explicitParent()) {
            return explicitParent()->setProperty(propertyId, v);
        }

        LOG_PROP() << typeName() << " unknown <" << propertyName(propertyId) << ">(" << int(propertyId) << "), data: " << v.value<String>();
        return false;
    }
    triggerLayout();
    return true;
}

void EngravingItem::manageExclusionFromParts(bool exclude)
{
    if (exclude) {
        const std::list<EngravingObject*> links = linkList();
        for (EngravingObject* linkedObject : links) {
            if (linkedObject->score() == score()) {
                continue;
            }
            EngravingItem* linkedItem = toEngravingItem(linkedObject);
            if (linkedItem->selected()) {
                linkedItem->score()->deselect(linkedItem);
            }
            linkedItem->score()->undoRemoveElement(linkedItem, false);
            linkedItem->undoUnlink();
        }
    } else {
        score()->undoAddElement(this, /*addToLinkedStaves*/ true, /*ctrlModifier*/ false, this);
    }
}

bool EngravingItem::isBefore(const EngravingItem* item) const
{
    if (!item) {
        return false;
    }
    if (tick() != item->tick()) {
        return tick() < item->tick();
    }

    const Measure* thisMeasure = findMeasure();
    const Measure* otherMeasure = item->findMeasure();
    if (thisMeasure != otherMeasure) {
        return thisMeasure->isBefore(otherMeasure);
    }

    const EngravingItem* thisSeg = findAncestor(ElementType::SEGMENT);
    const EngravingItem* otherSeg = item->findAncestor(ElementType::SEGMENT);
    if (!thisSeg || !otherSeg || !thisSeg->isSegment() || !otherSeg->isSegment()) {
        return false;
    }

    return toSegment(thisSeg)->goesBefore(toSegment(otherSeg));
}

bool EngravingItem::appliesToAllVoicesInInstrument() const
{
    return hasVoiceAssignmentProperties()
           && getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>() == VoiceAssignment::ALL_VOICE_IN_INSTRUMENT;
}

void EngravingItem::setInitialTrackAndVoiceAssignment(track_idx_t track, bool curVoiceOnlyOverride)
{
    IF_ASSERT_FAILED(track != muse::nidx) {
        return;
    }

    if (configuration()->dynamicsApplyToAllVoices() && !curVoiceOnlyOverride) {
        setTrack(trackZeroVoice(track));
        setProperty(Pid::VOICE_ASSIGNMENT, VoiceAssignment::ALL_VOICE_IN_INSTRUMENT);
    } else {
        setTrack(track);
        setProperty(Pid::VOICE_ASSIGNMENT, VoiceAssignment::CURRENT_VOICE_ONLY);
    }
}

void EngravingItem::checkVoiceAssignmentCompatibleWithTrack()
{
    voice_idx_t currentVoice = voice();
    VoiceAssignment voiceAssignment = getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();

    if (voiceAssignment != VoiceAssignment::CURRENT_VOICE_ONLY && currentVoice != 0) {
        setProperty(Pid::TRACK, trackZeroVoice(track()));
    }
}

bool EngravingItem::elementAppliesToTrack(const track_idx_t refTrack) const
{
    if (!hasVoiceAssignmentProperties()) {
        return refTrack == track();
    }
    const VoiceAssignment voiceAssignment = getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();

    return elementAppliesToTrack(track(), refTrack, voiceAssignment, part());
}

void EngravingItem::setPlacementBasedOnVoiceAssignment(DirectionV styledDirection)
{
    PlacementV oldPlacement = placement();
    bool offsetIsStyled = isStyled(Pid::OFFSET);

    PlacementV newPlacement = PlacementV::BELOW;

    DirectionV internalDirectionProperty = getProperty(Pid::DIRECTION).value<DirectionV>();
    if (internalDirectionProperty != DirectionV::AUTO) {
        newPlacement = internalDirectionProperty == DirectionV::UP ? PlacementV::ABOVE : PlacementV::BELOW;
    } else if (styledDirection != DirectionV::AUTO) {
        newPlacement = styledDirection == DirectionV::UP ? PlacementV::ABOVE : PlacementV::BELOW;
    } else if (part()->nstaves() > 1 && getProperty(Pid::CENTER_BETWEEN_STAVES).value<AutoOnOff>() == AutoOnOff::ON) {
        bool isOnLastStaffOfInstrument = staffIdx() == part()->staves().back()->idx();
        newPlacement = isOnLastStaffOfInstrument ? PlacementV::ABOVE : PlacementV::BELOW;
    } else {
        VoiceAssignment voiceAssignment = getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
        if (voiceAssignment == VoiceAssignment::ALL_VOICE_IN_INSTRUMENT || voiceAssignment == VoiceAssignment::ALL_VOICE_IN_STAFF) {
            if (style().styleB(Sid::dynamicsHairpinsAboveForVocalStaves) && part()->instrument()->isVocalInstrument()) {
                newPlacement = PlacementV::ABOVE;
            } else {
                newPlacement = PlacementV::BELOW;
            }
        } else if (voice() == 0) {
            // Put above the staff only in case of multiple voices at this tick (similar to stem directions)
            const Measure* measure = score()->tick2measure(tick());
            Fraction startTick = Fraction(-1, 1);
            Fraction length = Fraction(-1, 1);
            if (isSpanner()) {
                startTick = tick();
                length = toSpanner(this)->ticks();
            } else if (const Segment* segment = toSegment(findAncestor(ElementType::SEGMENT))) {
                if (segment && segment->isTimeTickType() && segment->measure() != measure) {
                    // Edge case: this is a TimeTick segment at the end of previous measure. Happens only
                    // when dynamic is anchorToEndOfPrevious. In this case look for preceding segment.
                    segment = segment->prev1ChordRestOrTimeTick();
                    assert(segment);
                    measure = segment->measure();
                }
                startTick = segment->tick();
                length = segment->ticks();
            } else if (measure) {
                startTick = measure->tick();
                length = measure->ticks();
            }
            if (measure && measure->hasVoices(staffIdx(), startTick, length)) {
                newPlacement = PlacementV::ABOVE;
            } else {
                newPlacement = PlacementV::BELOW;
            }
        } else {
            newPlacement = voice() % 2 ? PlacementV::BELOW : PlacementV::ABOVE;
        }
    }

    if (newPlacement != oldPlacement) {
        setPlacement(newPlacement);
        if (offsetIsStyled) {
            resetProperty(Pid::OFFSET);
        }
    }
}

void EngravingItem::relinkPropertiesToMaster(PropertyGroup propGroup)
{
    assert(!score()->isMaster());

    const std::list<EngravingObject*> linkedElements = linkListForPropertyPropagation();
    EngravingObject* masterElement = nullptr;
    for (EngravingObject* element : linkedElements) {
        if (element->score()->isMaster()) {
            masterElement = element;
            break;
        }
    }

    if (!masterElement) {
        return;
    }

    for (int i = 0; i < static_cast<int>(Pid::END); ++i) {
        const Pid propertyId = static_cast<Pid>(i);
        if (propertyGroup(propertyId) != propGroup) {
            continue;
        }
        const PropertyValue masterValue = masterElement->getProperty(propertyId);
        const PropertyFlags masterFlags = masterElement->propertyFlags(propertyId);
        if (getProperty(propertyId) != masterValue) {
            setProperty(propertyId, masterValue);
        }
        if (propertyFlags(propertyId) != masterFlags) {
            setPropertyFlags(propertyId, masterFlags);
        }
    }
}

void EngravingItem::relinkPropertyToMaster(Pid propertyId)
{
    assert(!score()->isMaster());

    const std::list<EngravingObject*> linkedElements = linkListForPropertyPropagation();
    EngravingObject* masterElement = nullptr;
    for (EngravingObject* element : linkedElements) {
        if (element->score()->isMaster()) {
            masterElement = element;
            break;
        }
    }

    if (!masterElement) {
        return;
    }

    setProperty(propertyId, masterElement->getProperty(propertyId));
    setPropertyFlags(propertyId, masterElement->propertyFlags(propertyId));
}

PropertyPropagation EngravingItem::propertyPropagation(const EngravingItem* destinationItem, Pid propertyId) const
{
    assert(destinationItem != this);

    if (propertyLink(propertyId)) {
        return PropertyPropagation::PROPAGATE; // These properties are always linked, no matter what
    }

    if (propertyGroup(propertyId) == PropertyGroup::NONE) {
        return PropertyPropagation::NONE;
    }

    const Score* sourceScore = score();
    const Score* destinationScore = destinationItem->score();
    const Staff* sourceStaff = staff();
    const Staff* destinationStaff = destinationItem->staff();

    if (sourceScore == destinationScore) {
        const bool diffStaff = sourceStaff != destinationStaff;
        const bool visiblePositionOrColor = propertyId == Pid::VISIBLE || propertyId == Pid::COLOR
                                            || propertyGroup(propertyId) == PropertyGroup::POSITION;
        const bool linkSameScore = propertyLinkSameScore(propertyId);
        if ((diffStaff && visiblePositionOrColor) || !linkSameScore) {
            // Allow visibility and position to stay independent
            return PropertyPropagation::NONE;
        }
        // Maintain every other property synced
        return PropertyPropagation::PROPAGATE;
    }

    const bool isTextProperty = propertyGroup(propertyId) == PropertyGroup::TEXT;
    if (isTextProperty) {
        if (sourceScore->isMaster() && destinationItem->isPropertyLinkedToMaster(propertyId)) {
            // From master score - check if destination part follows master
            return PropertyPropagation::PROPAGATE;
        }

        if (!sourceScore->isMaster() && isPropertyLinkedToMaster(propertyId)) {
            // From part - check if source part follows master
            return PropertyPropagation::PROPAGATE;
        }
    }

    if (!sourceScore->isMaster()) {
        // Properties are only propagated when being edited from master. If this is being edited
        // from a part score, we mark it as unlinked so it becomes independent in the part.
        return PropertyPropagation::UNLINK;
    }

    if (systemFlag() && !isTopSystemObject()) {
        // Let only the top system object propagate
        return PropertyPropagation::NONE;
    }

    if (destinationItem->isPropertyLinkedToMaster(propertyId)) {
        return PropertyPropagation::PROPAGATE;
    }

    return PropertyPropagation::NONE;
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void EngravingItem::undoChangeProperty(Pid pid, const PropertyValue& val, PropertyFlags ps)
{
    if (pid == Pid::AUTOPLACE && (val.toBool() == true && !autoplace())) {
        // Switching autoplacement on. Save user-defined
        // placement properties to undo stack.
        undoPushProperty(Pid::PLACEMENT);
        undoPushProperty(Pid::OFFSET);
    }
    EngravingObject::undoChangeProperty(pid, val, ps);
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue EngravingItem::propertyDefault(Pid pid) const
{
    switch (pid) {
    case Pid::GENERATED:
        return false;
    case Pid::VISIBLE:
        return true;
    case Pid::COLOR:
        return PropertyValue::fromValue(configuration()->defaultColor());
    case Pid::PLACEMENT: {
        PropertyValue v = EngravingObject::propertyDefault(pid);
        if (v.isValid()) {        // if it's a styled property
            return v;
        }
        return PlacementV::BELOW;
    }
    case Pid::SELECTED:
        return false;
    case Pid::OFFSET: {
        PropertyValue v = EngravingObject::propertyDefault(pid);
        if (v.isValid()) {        // if it's a styled property
            return v;
        }
        return PropertyValue::fromValue(PointF());
    }
    case Pid::MIN_DISTANCE: {
        PropertyValue v = EngravingObject::propertyDefault(pid);
        if (v.isValid()) {
            return v;
        }
        return 0.0;
    }
    case Pid::AUTOPLACE:
        return true;
    case Pid::Z:
        return int(type()) * 100;
    case Pid::POSITION_LINKED_TO_MASTER:
        return true;
    case Pid::APPEARANCE_LINKED_TO_MASTER:
        return true;
    case Pid::EXCLUDE_FROM_OTHER_PARTS:
        return false;
    case Pid::EXCLUDE_VERTICAL_ALIGN:
        return false;
    case Pid::HAS_PARENTHESES:
        return ParenthesesMode::NONE;
    default: {
        PropertyValue v = EngravingObject::propertyDefault(pid);

        if (v.isValid()) {
            return v;
        }

        if (explicitParent()) {
            return explicitParent()->propertyDefault(pid);
        }

        return PropertyValue();
    }
    }
}

//---------------------------------------------------------
//   custom
//    check if property is != default
//---------------------------------------------------------

bool EngravingItem::custom(Pid id) const
{
    return propertyDefault(id) != getProperty(id);
}

//---------------------------------------------------------
//   isPrintable
//---------------------------------------------------------

bool EngravingItem::isPrintable() const
{
    switch (type()) {
    case ElementType::PAGE:
    case ElementType::SYSTEM:
    case ElementType::MEASURE:
    case ElementType::SEGMENT:
    case ElementType::VBOX:
    case ElementType::HBOX:
    case ElementType::TBOX:
    case ElementType::FBOX:
    case ElementType::SPACER:
    case ElementType::SHADOW_NOTE:
    case ElementType::LASSO:
        return false;
    default:
        return true;
    }
}

bool EngravingItem::isPlayable() const
{
    switch (type()) {
    case ElementType::NOTE:
    case ElementType::CHORD:
        return true;
    case ElementType::HARMONY:
        return explicitParent() && explicitParent()->isSegment();
    default:
        return false;
    }
}

//---------------------------------------------------------
//   findAncestor
//---------------------------------------------------------

EngravingItem* EngravingItem::findAncestor(ElementType t)
{
    EngravingItem* e = this;
    while (e && e->type() != t) {
        e = e->parentItem();
    }
    return e;
}

const EngravingItem* EngravingItem::findAncestor(ElementType t) const
{
    const EngravingItem* e = this;
    while (e && e->type() != t) {
        e = e->parentItem();
    }
    return e;
}

//---------------------------------------------------------
//   findMeasure
//---------------------------------------------------------

Measure* EngravingItem::findMeasure()
{
    if (isMeasure()) {
        return toMeasure(this);
    } else if (explicitParent()) {
        return parentItem()->findMeasure();
    } else {
        return 0;
    }
}

//---------------------------------------------------------
//   findMeasure
//---------------------------------------------------------

const Measure* EngravingItem::findMeasure() const
{
    EngravingItem* e = const_cast<EngravingItem*>(this);
    return e->findMeasure();
}

//---------------------------------------------------------
//   findMeasureBase
//---------------------------------------------------------

MeasureBase* EngravingItem::findMeasureBase()
{
    if (isMeasureBase()) {
        return toMeasureBase(this);
    } else if (explicitParent()) {
        return parentItem()->findMeasureBase();
    } else {
        return 0;
    }
}

//---------------------------------------------------------
//   findMeasureBase
//---------------------------------------------------------

const MeasureBase* EngravingItem::findMeasureBase() const
{
    EngravingItem* e = const_cast<EngravingItem*>(this);
    return e->findMeasureBase();
}

//---------------------------------------------------------
//   undoSetColor
//---------------------------------------------------------

void EngravingItem::undoSetColor(const Color& c)
{
    undoChangeProperty(Pid::COLOR, PropertyValue::fromValue(c));
}

//---------------------------------------------------------
//   undoSetVisible
//---------------------------------------------------------

void EngravingItem::undoSetVisible(bool v)
{
    undoChangeProperty(Pid::VISIBLE, v);
}

void EngravingItem::undoAddElement(EngravingItem* element, bool addToLinkedStaves)
{
    score()->undoAddElement(element, addToLinkedStaves);
}

//---------------------------------------------------------
//   drawSymbol
//---------------------------------------------------------

void EngravingItem::drawSymbol(SymId id, Painter* p, const PointF& o, double scale) const
{
    score()->engravingFont()->draw(id, p, magS() * scale, o);
}

void EngravingItem::drawSymbols(const SymIdList& symbols, Painter* p, const PointF& o, double scale) const
{
    score()->engravingFont()->draw(symbols, p, magS() * scale, o);
}

void EngravingItem::drawSymbols(const SymIdList& symbols, Painter* p, const PointF& o, const SizeF& scale) const
{
    score()->engravingFont()->draw(symbols, p, SizeF(magS() * scale), o);
}

//---------------------------------------------------------
//   symHeight
//---------------------------------------------------------

double EngravingItem::symHeight(SymId id) const
{
    return score()->engravingFont()->height(id, magS());
}

//---------------------------------------------------------
//   symWidth
//---------------------------------------------------------

double EngravingItem::symWidth(SymId id) const
{
    return score()->engravingFont()->width(id, magS());
}

double EngravingItem::symWidth(const SymIdList& symbols) const
{
    return score()->engravingFont()->width(symbols, magS());
}

//---------------------------------------------------------
//   symAdvance
//---------------------------------------------------------

double EngravingItem::symAdvance(SymId id) const
{
    return score()->engravingFont()->advance(id, magS());
}

//---------------------------------------------------------
//   symBbox
//---------------------------------------------------------

RectF EngravingItem::symBbox(SymId id) const
{
    return score()->engravingFont()->bbox(id, magS());
}

RectF EngravingItem::symBbox(const SymIdList& symbols) const
{
    return score()->engravingFont()->bbox(symbols, magS());
}

Shape EngravingItem::symShapeWithCutouts(SymId id) const
{
    Shape shape = score()->engravingFont()->shapeWithCutouts(id, magS());
    for (ShapeElement& element : shape.elements()) {
        element.setItem(this);
    }

    return shape;
}

//---------------------------------------------------------
//   symSmuflAnchor
//---------------------------------------------------------

PointF EngravingItem::symSmuflAnchor(SymId symId, SmuflAnchorId anchorId) const
{
    return score()->engravingFont()->smuflAnchor(symId, anchorId, magS());
}

//---------------------------------------------------------
//   symIsValid
//---------------------------------------------------------

bool EngravingItem::symIsValid(SymId id) const
{
    return score()->engravingFont()->isValid(id);
}

//---------------------------------------------------------
//   concertPitch
//---------------------------------------------------------

bool EngravingItem::concertPitch() const
{
    return style().styleB(Sid::concertPitch);
}

//---------------------------------------------------------
//   nextElement
//   selects the next score element
//---------------------------------------------------------

EngravingItem* EngravingItem::nextElement()
{
    EngravingItem* e = score()->selection().element();
    if (!e && !score()->selection().elements().empty()) {
        e = score()->selection().elements().front();
    }
    if (e) {
        switch (e->type()) {
        case ElementType::SEGMENT: {
            Segment* s = toSegment(e);
            return s->nextElement(staffIdx());
        }
        case ElementType::MEASURE: {
            Measure* m = toMeasure(e);
            return m->nextElementStaff(staffIdx());
        }
        case ElementType::CLEF:
        case ElementType::KEYSIG:
        case ElementType::TIMESIG:
        case ElementType::BAR_LINE:
            return nextSegmentElement();
        default: {
            return e->parentItem()->nextElement();
        }
        }
    }
    return nullptr;
}

//---------------------------------------------------------
//   prevElement
//   selects the previous score element
//---------------------------------------------------------

EngravingItem* EngravingItem::prevElement()
{
    EngravingItem* e = score()->selection().element();
    if (!e && !score()->selection().elements().empty()) {
        e = score()->selection().elements().back();
    }
    if (e) {
        switch (e->type()) {
        case ElementType::SEGMENT: {
            Segment* s = toSegment(e);
            return s->prevElement(staffIdx());
        }
        case ElementType::MEASURE: {
            Measure* m = toMeasure(e);
            return m->prevElementStaff(staffIdx());
        }
        case ElementType::CLEF:
        case ElementType::KEYSIG:
        case ElementType::TIMESIG:
        case ElementType::BAR_LINE:
            return prevSegmentElement();
        default: {
            return e->parentItem()->prevElement();
        }
        }
    }
    return nullptr;
}

//------------------------------------------------------------------------------------------
//   nextSegmentElement
//   This function is used in for the next-element command to navigate between main elements
//   of segments. (Note, Rest, Clef, Time Signature, Key Signature, Barline, Ambitus, Breath, etc.)
//   The default implementation is to look for the first such element. After it is found each
//   element knows how to find the next one and overrides this method
//------------------------------------------------------------------------------------------

EngravingItem* EngravingItem::nextSegmentElement()
{
    EngravingItem* p = this;
    while (p) {
        switch (p->type()) {
        case ElementType::NOTE:
            if (toNote(p)->chord()->isGrace()) {
                break;
            }
            return p;
        case ElementType::REST:
        case ElementType::MMREST:
            return p;
        case ElementType::CHORD: {
            Chord* c = toChord(p);
            if (!c->isGrace()) {
                return c->notes().back();
            }
        }
        break;
        case ElementType::SEGMENT: {
            Segment* s = toSegment(p);
            return s->firstElementForNavigation(staffIdx());
        }
        case ElementType::MEASURE: {
            Measure* m = toMeasure(p);
            return m->nextElementStaff(staffIdx());
        }
        case ElementType::SYSTEM: {
            System* sys = toSystem(p);
            return sys->nextSegmentElement();
        }
        default:
            break;
        }
        p = p->parentItem();
    }
    return score()->firstElement();
}

//------------------------------------------------------------------------------------------
//   prevSegmentElement
//   This function is used in for the prev-element command to navigate between main elements
//   of segments. (Note, Rest, Clef, Time Signature, Key Signature, Barline, Ambitus, Breath, etc.)
//   The default implementation is to look for the first such element. After it is found each
//   element knows how to find the previous one and overrides this method
//------------------------------------------------------------------------------------------

EngravingItem* EngravingItem::prevSegmentElement()
{
    EngravingItem* p = this;
    while (p) {
        switch (p->type()) {
        case ElementType::NOTE:
            if (toNote(p)->chord()->isGrace()) {
                break;
            }
            return p;
        case ElementType::REST:
        case ElementType::MMREST:
            return p;
        case ElementType::CHORD: {
            Chord* c = toChord(p);
            if (!c->isGrace()) {
                return c->notes().front();
            }
        }
        break;
        case ElementType::SEGMENT: {
            Segment* s = toSegment(p);
            return s->lastElementForNavigation(staffIdx());
        }
        case ElementType::MEASURE: {
            Measure* m = toMeasure(p);
            return m->prevElementStaff(staffIdx());
        }
        case ElementType::SYSTEM: {
            System* sys = toSystem(p);
            return sys->prevSegmentElement();
        }
        default:
            break;
        }
        p = p->parentItem();
    }
    return score()->firstElement();
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
AccessibleItemPtr EngravingItem::accessible() const
{
    return m_accessible;
}

#endif

String EngravingItem::accessibleInfo() const
{
    return EngravingItem::translatedTypeUserName();
}

//---------------------------------------------------------
//   nextGrip
//---------------------------------------------------------

bool EngravingItem::nextGrip(EditData& ed) const
{
    int i = int(ed.curGrip) + 1;
    if (i >= ed.grips) {
        ed.curGrip = Grip(0);
        return false;
    }
    ed.curGrip = Grip(i);
    return true;
}

//---------------------------------------------------------
//   prevGrip
//---------------------------------------------------------

bool EngravingItem::prevGrip(EditData& ed) const
{
    int i = int(ed.curGrip) - 1;
    if (i < 0) {
        ed.curGrip = Grip(ed.grips - 1);
        return false;
    }
    ed.curGrip = Grip(i);
    return true;
}

//---------------------------------------------------------
//   isUserModified
//    Check if this element was modified by user and
//    therefore must be saved.
//---------------------------------------------------------

bool EngravingItem::isUserModified() const
{
    for (const StyledProperty& spp : *styledProperties()) {
        Pid pid = spp.pid;
        PropertyValue val = getProperty(pid);
        PropertyValue defaultValue = propertyDefault(pid);

        if (propertyType(pid) == P_TYPE::MILLIMETRE) {
            if (std::abs(val.value<Millimetre>() - defaultValue.value<Millimetre>()) > 0.0001) {         // we don’t care spatium diffs that small
                return true;
            }
        } else {
            if (getProperty(pid) != propertyDefault(pid)) {
                return true;
            }
        }
    }
    for (Pid p : { Pid::VISIBLE, Pid::OFFSET, Pid::COLOR, Pid::Z, Pid::AUTOPLACE }) {
        if (getProperty(p) != propertyDefault(p)) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void EngravingItem::triggerLayout() const
{
    if (explicitParent()) {
        score()->setLayout(tick(), staffIdx(), this);
    }
}

//----------------------------------------------------------------------
//   triggerLayoutAll
//
//   *************************** CAUTION *******************************
//   This causes a layout of the entire score: extremely expensive and
//   likely unnecessary! Consider overriding triggerLayout() instead.
//----------------------------------------------------------------------

void EngravingItem::triggerLayoutAll() const
{
    if (explicitParent()) {
        score()->setLayoutAll(staffIdx(), this);
    }
}

void EngravingItem::triggerLayoutToEnd() const
{
    if (explicitParent()) {
        score()->setLayout(tick(), score()->endTick(), staffIdx(), staffIdx(), this);
    }
}

//---------------------------------------------------------
//   addData
//---------------------------------------------------------

void EditData::addData(std::shared_ptr<ElementEditData> ed)
{
    m_data.push_back(ed);
}

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void EngravingItem::startDrag(EditData& ed)
{
    if (!isMovable()) {
        return;
    }
    std::shared_ptr<ElementEditData> eed = std::make_shared<ElementEditData>();
    eed->e = this;
    eed->pushProperty(Pid::OFFSET);
    eed->pushProperty(Pid::AUTOPLACE);
    ed.addData(eed);
    if (ed.modifiers & AltModifier) {
        setAutoplace(false);
    }
}

//---------------------------------------------------------
//   drag
///   Return update Rect relative to canvas.
//---------------------------------------------------------

RectF EngravingItem::drag(EditData& ed)
{
    if (!isMovable()) {
        return RectF();
    }

    const RectF r0(canvasBoundingRect());

    const PointF offset0 = ed.evtDelta + offset();
    double x = offset0.x();
    double y = offset0.y();

    double _spatium = spatium();
    if (ed.hRaster) {
        double hRaster = _spatium / MScore::hRaster();
        int n = lrint(x / hRaster);
        x = hRaster * n;
    }
    if (ed.vRaster) {
        double vRaster = _spatium / MScore::vRaster();
        int n = lrint(y / vRaster);
        y = vRaster * n;
    }

    setOffset(PointF(x, y));
    setOffsetChanged(true);
//      setGenerated(false);

    if (isTextBase()) {           // TODO: check for other types
        //
        // restrict move to page boundaries
        //
        const RectF r(canvasBoundingRect());
        Page* p = 0;
        EngravingItem* e = this;
        while (e) {
            if (e->isPage()) {
                p = toPage(e);
                break;
            }
            e = e->parentItem();
        }
        if (p) {
            bool move = false;
            RectF pr(p->canvasBoundingRect());
            if (r.right() > pr.right()) {
                x -= r.right() - pr.right();
                move = true;
            } else if (r.left() < pr.left()) {
                x += pr.left() - r.left();
                move = true;
            }
            if (r.bottom() > pr.bottom()) {
                y -= r.bottom() - pr.bottom();
                move = true;
            } else if (r.top() < pr.top()) {
                y += pr.top() - r.top();
                move = true;
            }
            if (move) {
                setOffset(PointF(x, y));
            }
        }
    }
    return canvasBoundingRect().united(r0);
}

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void EngravingItem::endDrag(EditData& ed)
{
    if (!isMovable()) {
        return;
    }
    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        return;
    }
    for (const PropertyData& pd : eed->propertyData) {
        setPropertyFlags(pd.id, pd.f);     // reset initial property flags state
        PropertyFlags f = pd.f;
        if (f == PropertyFlags::STYLED) {
            f = PropertyFlags::UNSTYLED;
        }
        score()->undoPropertyChanged(this, pd.id, pd.data, f);
        setGenerated(false);
    }
    score()->hideAnchors();
}

//---------------------------------------------------------
//   genericDragAnchorLines
//---------------------------------------------------------

std::vector<LineF> EngravingItem::genericDragAnchorLines() const
{
    double xp = 0.0;
    for (EngravingItem* e = parentItem(); e; e = e->parentItem()) {
        xp += e->x();
    }
    double yp;
    if (explicitParent()->isSegment() || explicitParent()->isMeasure()) {
        Measure* meas = explicitParent()->isSegment() ? toSegment(explicitParent())->measure() : toMeasure(explicitParent());
        System* system = meas->system();
        const staff_idx_t stIdx = effectiveStaffIdx();
        if (stIdx == muse::nidx) {
            return { LineF() };
        }
        yp = system ? system->staffCanvasYpage(stIdx) : 0.0;
        if (placement() == PlacementV::BELOW) {
            yp += system ? system->staff(stIdx)->bbox().height() : 0.0;
        }
        //adjust anchor Y positions to staffType offset
        if (staff()) {
            yp += staff()->staffTypeForElement(this)->yoffset().val() * spatium();
        }
    } else {
        yp = parentItem()->canvasPos().y();
    }
    PointF p1(xp, yp);
    LineF anchorLine(p1, canvasPos());
    return { anchorLine };
}

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void EngravingItem::updateGrips(EditData& ed) const
{
    const auto positions(gripsPositions(ed));
    const size_t ngrips = positions.size();
    for (int i = 0; i < int(ngrips); ++i) {
        ed.grip[i].translate(positions[i]);
    }
}

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void EngravingItem::startEdit(EditData& ed)
{
    std::shared_ptr<ElementEditData> elementData = std::make_shared<ElementEditData>();
    elementData->e = this;
    ed.addData(elementData);
}

//---------------------------------------------------------
//   isEditAllowed
//---------------------------------------------------------

bool EngravingItem::isEditAllowed(EditData& ed) const
{
    return ed.key == Key_Home;
}

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool EngravingItem::edit(EditData& ed)
{
    if (ed.key == Key_Home) {
        setOffset(PointF());
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void EngravingItem::startEditDrag(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        eed = std::make_shared<ElementEditData>();
        eed->e = this;
        ed.addData(eed);
    }
    eed->pushProperty(Pid::OFFSET);
    eed->pushProperty(Pid::AUTOPLACE);
    if (ed.modifiers & AltModifier) {
        setAutoplace(false);
    }
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void EngravingItem::editDrag(EditData& ed)
{
    score()->addRefresh(canvasBoundingRect());
    setOffset(offset() + ed.delta);
    setOffsetChanged(true);
    score()->addRefresh(canvasBoundingRect());
}

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void EngravingItem::endEditDrag(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    bool changed = false;
    if (eed) {
        for (const PropertyData& pd : eed->propertyData) {
            setPropertyFlags(pd.id, pd.f);       // reset initial property flags state
            PropertyFlags f = pd.f;
            if (f == PropertyFlags::STYLED) {
                f = PropertyFlags::UNSTYLED;
            }
            if (score()->undoPropertyChanged(this, pd.id, pd.data, f)) {
                changed = true;
            }
        }
        eed->propertyData.clear();
    }
    if (changed) {
        undoChangeProperty(Pid::GENERATED, false);
    }
    score()->hideAnchors();
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void EngravingItem::endEdit(EditData&)
{
    score()->hideAnchors();
}

//---------------------------------------------------------
//   styleP
//---------------------------------------------------------

double EngravingItem::styleP(Sid idx) const
{
    return style().styleMM(idx);
}

bool EngravingItem::colorsInversionEnabled() const
{
    return m_colorsInversionEnabled;
}

void EngravingItem::setColorsInverionEnabled(bool enabled)
{
    m_colorsInversionEnabled = enabled;
}

void EngravingItem::setParenthesesMode(const ParenthesesMode& v, bool addToLinked, bool generated)
{
    setHasLeftParenthesis(v & ParenthesesMode::LEFT, addToLinked, generated);
    setHasRightParenthesis(v & ParenthesesMode::RIGHT, addToLinked, generated);
}

ParenthesesMode EngravingItem::parenthesesMode() const
{
    ParenthesesMode p = ParenthesesMode::NONE;
    if (m_leftParenthesis) {
        p |= ParenthesesMode::LEFT;
    }
    if (m_leftParenthesis) {
        p |= ParenthesesMode::RIGHT;
    }

    return p;
}

void EngravingItem::setHasLeftParenthesis(bool v, bool addToLinked, bool generated)
{
    const bool hasGeneratedParen = m_leftParenthesis && m_leftParenthesis->generated();
    const bool hasUserParen = m_leftParenthesis && !m_leftParenthesis->generated();

    if (generated && v == hasGeneratedParen) {
        return;
    }

    if (!generated && v == hasUserParen) {
        return;
    }

    if (v) {
        if (!m_leftParenthesis) {
            Parenthesis* paren = Factory::createParenthesis(this);
            paren->setParent(this);
            paren->setTrack(track());
            paren->setDirection(DirectionH::LEFT);
            paren->setGenerated(generated);

            score()->undoAddElement(paren, addToLinked);
        }
    } else {
        score()->undoRemoveElement(m_leftParenthesis, addToLinked);
        assert(m_leftParenthesis == nullptr);
    }
}

void EngravingItem::setHasRightParenthesis(bool v, bool addToLinked, bool generated)
{
    const bool hasGeneratedParen = m_rightParenthesis && m_rightParenthesis->generated();
    const bool hasUserParen = m_rightParenthesis && !m_rightParenthesis->generated();

    if (generated && v == hasGeneratedParen) {
        return;
    }

    if (!generated && v == hasUserParen) {
        return;
    }

    if (v) {
        if (!m_rightParenthesis) {
            Parenthesis* paren = Factory::createParenthesis(this);
            paren->setParent(this);
            paren->setTrack(track());
            paren->setDirection(DirectionH::RIGHT);
            paren->setGenerated(generated);

            score()->undoAddElement(paren, addToLinked);
        }
    } else {
        score()->undoRemoveElement(m_rightParenthesis, addToLinked);
        assert(m_rightParenthesis == nullptr);
    }
}

EngravingItem::BarBeat EngravingItem::barbeat() const
{
    EngravingItem::BarBeat barBeat = { 0, 0, 0.0F };
    const EngravingItem* parent = this;
    while (parent && parent->type() != ElementType::SEGMENT && parent->type() != ElementType::MEASURE) {
        parent = parent->parentItem();
    }

    if (!parent) {
        return barBeat;
    }

    int bar = 0;
    int displayedBar = 0;
    int beat = 0;
    int ticks = 0;
    const Measure* measure = nullptr;

    const TimeSigMap* timeSigMap = score()->sigmap();
    int ticksB = ticks_beat(timeSigMap->timesig(0).timesig().denominator());

    if (parent->type() == ElementType::SEGMENT) {
        const Segment* segment = static_cast<const Segment*>(parent);
        timeSigMap->tickValues(segment->tick().ticks(), &bar, &beat, &ticks);
        ticksB = ticks_beat(timeSigMap->timesig(segment->tick().ticks()).timesig().denominator());
        measure = segment->findMeasure();
        if (measure) {
            displayedBar = measure->no();
        }
    } else if (parent->type() == ElementType::MEASURE) {
        measure = static_cast<const Measure*>(parent);
        bar = measure->no();
        displayedBar = bar;
        beat = -1;
        ticks = 0;
    }

    barBeat = { bar + 1, displayedBar + 1, beat + 1 + ticks / static_cast<float>(ticksB) };
    return barBeat;
}

EngravingItem* EngravingItem::findLinkedInScore(const Score* score) const
{
    if (!score || !staff() || !links() || links()->empty()) {
        return nullptr;
    }

    // If this is a system element it may not be on the same stave, so just check if linked elements are in the score
    if (systemFlag() && track() == 0) {
        for (EngravingObject* linked : *links()) {
            if (linked != this && linked->score() == score) {
                return toEngravingItem(linked);
            }
        }
        return nullptr;
    }

    Staff* linkedStaffInScore = staff()->findLinkedInScore(score);

    if (!linkedStaffInScore) {
        return nullptr;
    }

    return findLinkedInStaff(linkedStaffInScore);
}

EngravingItem* EngravingItem::findLinkedInStaff(const Staff* staff) const
{
    if (!links() || links()->empty()) {
        return nullptr;
    }

    for (EngravingObject* linked : *links()) {
        if (toEngravingItem(linked)->staff() == staff) {
            return toEngravingItem(linked);
        }
    }
    return nullptr;
}

bool EngravingItem::selected() const
{
    return flag(ElementFlag::SELECTED);
}

void EngravingItem::setSelected(bool f)
{
    setFlag(ElementFlag::SELECTED, f);
}

void EngravingItem::setVisible(bool f)
{
    setFlag(ElementFlag::INVISIBLE, !f);
    if (m_leftParenthesis) {
        m_leftParenthesis->setVisible(f);
    }
    if (m_rightParenthesis) {
        m_rightParenthesis->setVisible(f);
    }
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
void EngravingItem::initAccessibleIfNeed()
{
    if (!configuration()->isAccessibleEnabled()) {
        return;
    }

    if (!m_accessibleEnabled) {
        return;
    }

    doInitAccessible();
}

void EngravingItem::doInitAccessible()
{
    EngravingItemList parents;
    auto parent = parentItem(false /*not explicit*/);
    while (parent) {
        parents.push_front(parent);
        parent = parent->parentItem(false /*not explicit*/);
    }

    for (EngravingItem* parent2 : parents) {
        parent2->setupAccessible();
    }

    setupAccessible();
}

#endif // ENGRAVING_NO_ACCESSIBILITY

String EngravingItem::formatBarsAndBeats() const
{
    String result;
    EngravingItem::BarBeat barbeat = this->barbeat();

    if (barbeat.bar != 0) {
        result = muse::mtrc("engraving", "Measure: %1").arg(barbeat.bar);

        if (barbeat.displayedBar != barbeat.bar) {
            result += u"; " + muse::mtrc("engraving", "Displayed measure: %1").arg(barbeat.displayedBar);
        }

        if (!muse::RealIsNull(barbeat.beat)) {
            result += u"; " + muse::mtrc("engraving", "Beat: %1").arg(barbeat.beat);
        }
    }

    return result;
}

bool EngravingItem::isPropertyLinkedToMaster(Pid id) const
{
    if (propertyGroup(id) == PropertyGroup::POSITION) {
        return isPositionLinkedToMaster();
    }

    if (propertyGroup(id) == PropertyGroup::APPEARANCE) {
        return isAppearanceLinkedToMaster();
    }

    return true;
}

bool EngravingItem::isUnlinkedFromMaster() const
{
    return !(getProperty(Pid::POSITION_LINKED_TO_MASTER).toBool()
             && getProperty(Pid::APPEARANCE_LINKED_TO_MASTER).toBool());
}

void EngravingItem::unlinkPropertyFromMaster(Pid id)
{
    if (propertyGroup(id) == PropertyGroup::POSITION) {
        score()->undo(new ChangeProperty(this, Pid::POSITION_LINKED_TO_MASTER, false));
    } else if (propertyGroup(id) == PropertyGroup::APPEARANCE) {
        score()->undo(new ChangeProperty(this, Pid::APPEARANCE_LINKED_TO_MASTER, false));
    }
}

EngravingItem::LayoutData* EngravingItem::createLayoutData() const
{
    return new EngravingItem::LayoutData();
}

const EngravingItem::LayoutData* EngravingItem::ldata() const
{
    return ldataInternal();
}

EngravingItem::LayoutData* EngravingItem::mutldata()
{
    return mutldataInternal();
}

const EngravingItem::LayoutData* EngravingItem::ldataInternal() const
{
    if (!m_layoutData) {
        m_layoutData = createLayoutData();
        m_layoutData->m_item = this;
    }
    return m_layoutData;
}

EngravingItem::LayoutData* EngravingItem::mutldataInternal()
{
    if (!m_layoutData) {
        m_layoutData = createLayoutData();
        m_layoutData->m_item = this;
    }
    return m_layoutData;
}

bool EngravingItem::elementAppliesToTrack(const track_idx_t elementTrack, const track_idx_t refTrack,
                                          const VoiceAssignment voiceAssignment, const Part* part)
{
    if (voiceAssignment == VoiceAssignment::CURRENT_VOICE_ONLY && elementTrack == refTrack) {
        return true;
    }

    if (voiceAssignment == VoiceAssignment::ALL_VOICE_IN_STAFF && track2staff(elementTrack) == track2staff(refTrack)) {
        return true;
    }

    if (!part) {
        return false;
    }

    if (voiceAssignment == VoiceAssignment::ALL_VOICE_IN_INSTRUMENT && (part->startTrack() <= refTrack
                                                                        && part->endTrack() - 1 >= refTrack)) {
        return true;
    }
    return false;
}

void EngravingItem::LayoutData::setBbox(const RectF& r)
{
    DO_ASSERT(!std::isnan(r.x()) && !std::isinf(r.x()));
    DO_ASSERT(!std::isnan(r.y()) && !std::isinf(r.y()));
    DO_ASSERT(!std::isnan(r.width()) && !std::isinf(r.width()));
    DO_ASSERT(!std::isnan(r.height()) && !std::isinf(r.height()));

    //DO_ASSERT(!isShapeComposite());
    m_shape.set_value(Shape(r, m_item, Shape::Type::Fixed));
}

void EngravingItem::LayoutData::connectItemSnappedBefore(EngravingItem* itemBefore)
{
    IF_ASSERT_FAILED(itemBefore && itemBefore != m_item && itemBefore != m_itemSnappedAfter
                     && itemBefore->ldata()->itemSnappedBefore() != m_item) {
        return;
    }
    m_itemSnappedBefore = itemBefore;
    itemBefore->mutldata()->m_itemSnappedAfter = const_cast<EngravingItem*>(m_item);
}

void EngravingItem::LayoutData::disconnectItemSnappedBefore()
{
    if (!m_itemSnappedBefore) {
        return;
    }
    m_itemSnappedBefore->mutldata()->m_itemSnappedAfter = nullptr;
    m_itemSnappedBefore = nullptr;
}

void EngravingItem::LayoutData::connectItemSnappedAfter(EngravingItem* itemAfter)
{
    IF_ASSERT_FAILED(itemAfter && itemAfter != m_item && itemAfter != m_itemSnappedBefore
                     && itemAfter->ldata()->itemSnappedAfter() != m_item) {
        return;
    }
    m_itemSnappedAfter = itemAfter;
    itemAfter->mutldata()->m_itemSnappedBefore = const_cast<EngravingItem*>(m_item);
}

void EngravingItem::LayoutData::disconnectItemSnappedAfter()
{
    if (!m_itemSnappedAfter) {
        return;
    }
    m_itemSnappedAfter->mutldata()->m_itemSnappedBefore = nullptr;
    m_itemSnappedAfter = nullptr;
}

const RectF& EngravingItem::LayoutData::bbox(LD_ACCESS mode) const
{
    //! NOTE Temporary disabled CHECK - a lot of messages
    UNUSED(mode);
    const Shape& sh = m_shape.value(LD_ACCESS::MAYBE_NOTINITED);

    //! NOTE Temporary
    {
        static const RectF _dummy;

        switch (m_item->type()) {
        case ElementType::NOTE:
        case ElementType::ORNAMENT:
            return !sh.elements().empty() ? sh.elements().at(0) : _dummy;
        default:
            break;
        }
    }

    return sh.bbox();
}

Shape EngravingItem::LayoutData::shape(LD_ACCESS mode) const
{
    //! NOTE Temporary disabled CHECK - a lot of messages
    Shape sh = m_shape.value(LD_ACCESS::MAYBE_NOTINITED);

    const Parenthesis* leftParen = m_item->leftParen();
    if (leftParen && leftParen->addToSkyline()) {
        sh.add(leftParen->ldata()->shape().translated(leftParen->pos()));
    }
    const Parenthesis* rightParen = m_item->rightParen();
    if (rightParen && rightParen->addToSkyline()) {
        sh.add(rightParen->ldata()->shape().translated(rightParen->pos()));
    }

    //! NOTE Temporary
    //! Reimplementation: done
    {
        switch (m_item->type()) {
        case ElementType::BEAM:
        case ElementType::GRACE_NOTES_GROUP:
        case ElementType::ORNAMENT:
        case ElementType::OTTAVA_SEGMENT:
        case ElementType::VOLTA_SEGMENT:
        case ElementType::PEDAL_SEGMENT:
        case ElementType::TEXTLINE_SEGMENT:
        case ElementType::HARMONIC_MARK_SEGMENT:
        case ElementType::PALM_MUTE_SEGMENT:
        case ElementType::LET_RING_SEGMENT:
        case ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT:
        case ElementType::RASGUEADO_SEGMENT:
        case ElementType::WHAMMY_BAR_SEGMENT:
        case ElementType::SLUR_SEGMENT:
        case ElementType::TIE_SEGMENT:
        case ElementType::LAISSEZ_VIB_SEGMENT:
        case ElementType::PARTIAL_TIE_SEGMENT:
            return sh;
        case ElementType::CHORD:
        case ElementType::REST:
        case ElementType::MEASURE_REPEAT:
        case ElementType::MMREST: {
            if (mode == LD_ACCESS::CHECK) {
                //! NOTE Temporary fix
                //! We can remove it the moment we figure out the layout order of the elements
                LayoutContext ctx(m_item->score());
                ChordRest::LayoutData* ldata = static_cast<ChordRest::LayoutData*>(const_cast<LayoutData*>(this));
                ChordLayout::checkAndFillShape(toChordRest(m_item), ldata, ctx.conf());
            }
            return m_shape.value(LD_ACCESS::CHECK);
        } break;
        case ElementType::NOTE: {
            //! NOTE Temporary fix
            //! We can remove it the moment we figure out the layout order of the elements
            TLayout::fillNoteShape(toNote(m_item), static_cast<Note::LayoutData*>(const_cast<LayoutData*>(this)));
            return m_shape.value(LD_ACCESS::CHECK);
        } break;
        case ElementType::GUITAR_BEND_SEGMENT: {
            //! NOTE Temporary fix
            //! We can remove it the moment we figure out the layout order of the elements
            TLayout::fillGuitarBendSegmentShape(toGuitarBendSegment(m_item),
                                                static_cast<GuitarBendSegment::LayoutData*>(const_cast<LayoutData*>(this)));
            return m_shape.value(LD_ACCESS::CHECK);
        } break;
        case ElementType::HAIRPIN_SEGMENT: {
            //! NOTE Temporary fix
            //! We can remove it the moment we figure out the layout order of the elements
            TLayout::fillHairpinSegmentShape(toHairpinSegment(m_item),
                                             static_cast<HairpinSegment::LayoutData*>(const_cast<LayoutData*>(this)));
            return m_shape.value(LD_ACCESS::CHECK);
        } break;
        case ElementType::TRILL_SEGMENT: {
            //! NOTE Temporary fix
            //! We can remove it the moment we figure out the layout order of the elements
            LayoutContext ctx(m_item->score());
            TLayout::fillTrillSegmentShape(toTrillSegment(m_item),
                                           static_cast<HairpinSegment::LayoutData*>(const_cast<LayoutData*>(this)),
                                           ctx.conf());
            return m_shape.value(LD_ACCESS::CHECK);
        } break;
        case ElementType::TUPLET: {
            //! NOTE Temporary fix
            //! We can remove it the moment we figure out the layout order of the elements
            TLayout::fillTupletShape(toTuplet(m_item), static_cast<Tuplet::LayoutData*>(const_cast<LayoutData*>(this)));
            return m_shape.value(LD_ACCESS::CHECK);
        } break;
        default:
            break;
        }
    }

    return sh;
}

#ifndef NDEBUG
void EngravingItem::LayoutData::doSetPosDebugHook(double x, double y)
{
    UNUSED(x);
    UNUSED(y);
}

void EngravingItem::LayoutData::setWidthDebugHook(double w)
{
    UNUSED(w);
}

#endif

void EngravingItem::LayoutData::dump(std::stringstream& ss) const
{
    ss << "\n";
    ss << m_item->typeName() << " id: " << m_item->eid().toStdString() << "\n";

    ss << "skip: " << (m_isSkipDraw ? "yes" : "no") << "\n";
    ss << "mag: " << m_mag << "\n";

    ss << "pos: ";
    mu::engraving::dump(m_pos, ss);
    ss << "\n";

    ss << "shape: ";
    mu::engraving::dump(m_shape, ss);
    ss << "\n";

    supDump(ss);
}

double EngravingItem::mag() const
{
    if (!ldata()) {
        //LOGD() << "no layout data, will be returned default (1.0)";
        return 1.0;
    }
    return ldata()->mag();
}

PointF EngravingItem::staffOffset() const
{
    const StaffType* st = staffType();
    const double yOffset = st ? st->yoffset().val() * spatium() : 0.0;
    return PointF(0.0, yOffset);
}

void EngravingItem::setOffsetChanged(bool val, bool absolute, const PointF& diff)
{
    rendering::score::Autoplace::setOffsetChanged(this, mutldata(), val, absolute, diff);
}
}
