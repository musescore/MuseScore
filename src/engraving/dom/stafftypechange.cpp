/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "stafftypechange.h"

#include "iengravingconfiguration.h" // IWYU pragma: keep

#include "score.h"
#include "measure.h"
#include "page.h"
#include "system.h"
#include "staff.h"
#include "../editing/editdata.h"
#include "../editing/mscoreview.h"
#include "../editing/transaction/undoablecommand.h"

using namespace mu::engraving;

namespace {
//! Moves a staff-type marker while retaining the initial staff type for undo.
class MoveStaffTypeChange : public UndoableCommand
{
public:
    MoveStaffTypeChange(StaffTypeChange* item, Measure* target)
        : m_item(item), m_target(target), m_initialType(*item->staff()->staffType(Fraction(0, 1))) {}

    UNDO_NAME("MoveStaffTypeChange")
    UNDO_CHANGED_OBJECTS({ m_item, m_target })

private:
    void flip() override
    {
        Measure* source = m_item->measure();
        Staff* staff = m_item->staff();
        const StaffType initialType = *staff->staffType(Fraction(0, 1));
        source->remove(m_item);
        if (source->tick().isZero()) {
            staff->setStaffType(Fraction(0, 1), m_initialType);
            staff->staffTypeListChanged(Fraction(0, 1));
        }
        m_item->setOwnershipParent(m_target);
        m_target->add(m_item);
        m_target = source;
        m_initialType = initialType;
        m_item->triggerLayout();
    }

    StaffTypeChange* m_item;
    Measure* m_target;
    StaffType m_initialType;
};
}

//---------------------------------------------------------
//   StaffTypeChange
//---------------------------------------------------------

StaffTypeChange::StaffTypeChange(MeasureBase* parent)
    : EngravingItem(ElementType::STAFFTYPE_CHANGE, parent, ElementFlag::MOVABLE)
{
    m_lw = spatium() * 0.3;
}

StaffTypeChange::StaffTypeChange(const StaffTypeChange& lb)
    : EngravingItem(lb)
{
    m_lw = lb.m_lw;
    m_ownsStaffType = lb.m_ownsStaffType;
    if (lb.m_ownsStaffType && lb.m_staffType) {
        m_staffType = new StaffType(*lb.m_staffType);
    } else {
        m_staffType = lb.m_staffType;
    }
}

StaffTypeChange::~StaffTypeChange()
{
    if (m_staffType && m_ownsStaffType) {
        delete m_staffType;
    }
}

void StaffTypeChange::setStaffType(StaffType* st, bool owned)
{
    if (m_staffType && m_ownsStaffType) {
        delete m_staffType;
    }

    m_staffType = st;
    m_ownsStaffType = owned && (st != nullptr);
}

//! Keeps the original offset so dragging changes the measure, not the marker's layout.
void StaffTypeChange::startDrag(EditData& ed)
{
    m_dragTarget = nullptr;
    m_dragOffset = offset();
    EngravingItem::startDrag(ed);
}

//! Previews attachment to the original measure or an unoccupied measure on the same staff.
RectF StaffTypeChange::drag(EditData& ed)
{
    const RectF dirty = EngravingItem::drag(ed);
    Measure* target = score()->searchMeasure(ed.pos);
    m_dragTarget = measure() && target && (target == measure() || target->canAddStaffTypeChange(staffIdx())) ? target : nullptr;
    if (ed.view()) {
        if (m_dragTarget && m_dragTarget->system()) {
            const RectF rect = m_dragTarget->staffPageBoundingRect(staffIdx()).translated(m_dragTarget->system()->page()->pos());
            ed.view()->setDropRectangles({ rect });
        } else {
            ed.view()->setDropRectangles({});
        }
    }
    return dirty;
}

//! Connects the dragged marker to the start of its prospective destination measure.
std::vector<LineF> StaffTypeChange::dragAnchorLines() const
{
    if (!m_dragTarget || !m_dragTarget->system()) {
        return {};
    }
    const PointF anchor = m_dragTarget->staffPageBoundingRect(staffIdx()).topLeft()
                          + m_dragTarget->system()->page()->pos();
    return { LineF(anchor, canvasPos()) };
}

//! Commits the move, including linked parts, as part of the current drag transaction.
void StaffTypeChange::endDrag(EditData& ed)
{
    setOffset(m_dragOffset);
    EngravingItem::endDrag(ed);
    if (ed.view()) {
        ed.view()->setDropRectangles({});
    }
    Measure* target = m_dragTarget;
    m_dragTarget = nullptr;
    if (!target || target == measure()) {
        return;
    }
    std::vector<std::pair<StaffTypeChange*, Measure*> > moves;
    for (EngravingObject* linked : linkList()) {
        StaffTypeChange* item = toStaffTypeChange(linked);
        Measure* destination = item->score()->tick2measure(target->tick());
        if (!destination || !destination->canAddStaffTypeChange(item->staffIdx())) {
            return;
        }
        moves.emplace_back(item, destination);
    }
    for (const auto& [item, destination] : moves) {
        item->score()->undo(new MoveStaffTypeChange(item, destination));
    }
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void StaffTypeChange::spatiumChanged(double, double)
{
    m_lw = spatium() * 0.3;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue StaffTypeChange::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::STEP_OFFSET:
        return m_staffType->stepOffset();
    case Pid::STAFF_LINES:
        return m_staffType->lines();
    case Pid::LINE_DISTANCE:
        return m_staffType->lineDistance();
    case Pid::STAFF_SHOW_BARLINES:
        return m_staffType->showBarlines();
    case Pid::STAFF_SHOW_LEDGERLINES:
        return m_staffType->showLedgerLines();
    case Pid::STAFF_STEMLESS:
        return m_staffType->stemless();
    case Pid::HEAD_SCHEME:
        return m_staffType->noteHeadScheme();
    case Pid::STAFF_GEN_CLEF:
        return m_staffType->genClef();
    case Pid::STAFF_GEN_TIMESIG:
        return m_staffType->genTimesig();
    case Pid::STAFF_GEN_KEYSIG:
        return m_staffType->genKeysig();
    case Pid::MAG:
        return m_staffType->userMag();
    case Pid::SMALL:
        return m_staffType->isSmall();
    case Pid::STAFF_INVISIBLE:
        return m_staffType->invisible();
    case Pid::STAFF_COLOR:
        return PropertyValue::fromValue(m_staffType->color());
    case Pid::STAFF_YOFFSET:
        return m_staffType->yoffset();
    case Pid::STAFF_LONG_NAME:
        return m_staffType->longName();
    case Pid::STAFF_SHORT_NAME:
        return m_staffType->shortName();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool StaffTypeChange::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::STEP_OFFSET:
        m_staffType->setStepOffset(v.toInt());
        break;
    case Pid::STAFF_LINES:
        m_staffType->setLines(v.toInt());
        break;
    case Pid::LINE_DISTANCE:
        m_staffType->setLineDistance(v.value<Spatium>());
        break;
    case Pid::STAFF_SHOW_BARLINES:
        m_staffType->setShowBarlines(v.toBool());
        break;
    case Pid::STAFF_SHOW_LEDGERLINES:
        m_staffType->setShowLedgerLines(v.toBool());
        break;
    case Pid::STAFF_STEMLESS:
        m_staffType->setStemless(v.toBool());
        break;
    case Pid::HEAD_SCHEME:
        m_staffType->setNoteHeadScheme(v.value<NoteHeadScheme>());
        break;
    case Pid::STAFF_GEN_CLEF:
        m_staffType->setGenClef(v.toBool());
        break;
    case Pid::STAFF_GEN_TIMESIG:
        m_staffType->setGenTimesig(v.toBool());
        break;
    case Pid::STAFF_GEN_KEYSIG:
        m_staffType->setGenKeysig(v.toBool());
        break;
    case Pid::MAG: {
        double _spatium = spatium();
        m_staffType->setUserMag(v.toDouble());
        Staff* _staff = staff();
        if (_staff) {
            _staff->setLocalSpatium(_spatium, spatium(), tick());
        }
    }
    break;
    case Pid::SMALL: {
        double _spatium = spatium();
        m_staffType->setSmall(v.toBool());
        Staff* _staff = staff();
        if (_staff) {
            _staff->setLocalSpatium(_spatium, spatium(), tick());
        }
    }
    break;
    case Pid::STAFF_INVISIBLE:
        m_staffType->setInvisible(v.toBool());
        break;
    case Pid::STAFF_COLOR:
        m_staffType->setColor(v.value<Color>());
        break;
    case Pid::STAFF_YOFFSET:
        m_staffType->setYoffset(v.value<Spatium>());
        break;
    case Pid::STAFF_LONG_NAME:
        m_staffType->setLongName(v.value<String>());
        break;
    case Pid::STAFF_SHORT_NAME:
        m_staffType->setShortName(v.value<String>());
        break;
    default:
        if (!EngravingItem::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }

    if (ownershipParent()) {
        staff()->staffTypeListChanged(measure()->tick());
    }
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue StaffTypeChange::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::STEP_OFFSET:
        return 0;
    case Pid::STAFF_LINES:
        return 5;
    case Pid::LINE_DISTANCE:
        return 1.0_sp;
    case Pid::STAFF_SHOW_BARLINES:
        return true;
    case Pid::STAFF_SHOW_LEDGERLINES:
        return true;
    case Pid::STAFF_STEMLESS:
        return false;
    case Pid::HEAD_SCHEME:
        return NoteHeadScheme::HEAD_NORMAL;
    case Pid::STAFF_GEN_CLEF:
        return true;
    case Pid::STAFF_GEN_TIMESIG:
        return true;
    case Pid::STAFF_GEN_KEYSIG:
        return true;
    case Pid::MAG:
        return 1.0;
    case Pid::SMALL:
        return false;
    case Pid::STAFF_INVISIBLE:
        return false;
    case Pid::STAFF_COLOR:
        return PropertyValue::fromValue(configuration()->defaultColor());
    case Pid::STAFF_YOFFSET:
        return 0.0_sp;
    case Pid::STAFF_LONG_NAME:
        return String();
    case Pid::STAFF_SHORT_NAME:
        return String();
    default:
        return EngravingItem::propertyDefault(id);
    }
}
