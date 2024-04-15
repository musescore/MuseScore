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
#include "accessibleroot.h"

#include "../dom/score.h"
#include "../dom/staff.h"
#include "../dom/part.h"
#include "../dom/segment.h"

#include "log.h"
#include "translation.h"

using namespace mu::engraving;
using namespace muse::accessibility;

AccessibleRoot::AccessibleRoot(RootItem* e, Role role)
    : AccessibleItem(e, role)
{
}

AccessibleRoot::~AccessibleRoot()
{
    if (m_registred && accessibilityController()) {
        accessibilityController()->unreg(this);
        m_registred = false;
    }

    m_element = nullptr;
}

void AccessibleRoot::setFocusedElement(AccessibleItemPtr e, bool voiceStaffInfoChange)
{
    AccessibleItemWeakPtr old = m_focusedElement;
    updateStaffInfo(e, old, voiceStaffInfoChange);

    if (auto oldItem = old.lock()) {
        oldItem->notifyAboutFocus(false);
    }

    m_focusedElement = e;
    if (auto newItem = m_focusedElement.lock()) {
        newItem->notifyAboutFocus(true);
    }
}

AccessibleItemWeakPtr AccessibleRoot::focusedElement() const
{
    return m_focusedElement;
}

void AccessibleRoot::notifyAboutFocusedElementNameChanged()
{
    m_staffInfo = "";

    if (auto focusedElement = m_focusedElement.lock()) {
        focusedElement->accessiblePropertyChanged().send(IAccessible::Property::Name, muse::Val());
    }
}

RectF AccessibleRoot::toScreenRect(const RectF& rect, bool* ok) const
{
    RectF result;
    if (m_accessibleMapToScreenFunc) {
        result = m_accessibleMapToScreenFunc(rect);
    }

    if (ok) {
        *ok = m_accessibleMapToScreenFunc != nullptr;
    }

    return result;
}

const IAccessible* AccessibleRoot::accessibleParent() const
{
    return accessibilityController()->accessibleRoot();
}

IAccessible::Role AccessibleRoot::accessibleRole() const
{
    return IAccessible::Panel;
}

QString AccessibleRoot::accessibleName() const
{
    return muse::qtrc("engraving", "Score") + " " + element()->score()->name();
}

bool AccessibleRoot::enabled() const
{
    return m_enabled;
}

void AccessibleRoot::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

QString AccessibleRoot::staffInfo() const
{
    return m_staffInfo;
}

void AccessibleRoot::updateStaffInfo(const AccessibleItemWeakPtr newAccessibleItem, const AccessibleItemWeakPtr oldAccessibleItem,
                                     bool voiceStaffInfoChange)
{
    m_staffInfo = "";

    if (!voiceStaffInfoChange) {
        return;
    }

    AccessibleItemPtr newItem = newAccessibleItem.lock();
    AccessibleItemPtr oldItem = oldAccessibleItem.lock();

    if (newItem && newItem->element()->hasStaff()) {
        staff_idx_t newStaffIdx = newItem->element()->staffIdx();
        staff_idx_t oldStaffIdx = oldItem ? oldItem->element()->staffIdx() : muse::nidx;

        if (newStaffIdx != oldStaffIdx) {
            auto element = newItem->element();
            QString staff = muse::qtrc("engraving", "Staff %1").arg(QString::number(element->staffIdx() + 1));

            QString staffName = element->staff()->part()->longName(element->tick());
            if (staffName.isEmpty()) {
                staffName = element->staff()->partName();
            }

            if (staffName.isEmpty()) {
                m_staffInfo = staff;
            } else {
                m_staffInfo = QString("%2 (%3)").arg(staff, staffName);
            }
        }
    }
}

QString AccessibleRoot::commandInfo() const
{
    return m_commandInfo;
}

void AccessibleRoot::setCommandInfo(const QString& command)
{
    m_commandInfo = command;

    if (!m_commandInfo.isEmpty()) {
        notifyAboutFocusedElementNameChanged();
    }
}

void AccessibleRoot::setMapToScreenFunc(const AccessibleMapToScreenFunc& func)
{
    m_accessibleMapToScreenFunc = func;
}

bool AccessibleRoot::isRangeSelection() const
{
    return element()->score()->selection().isRange();
}

QString AccessibleRoot::rangeSelectionInfo()
{
    Score* score = element()->score();
    Selection selection = score->selection();
    Segment* startSegment = selection.startSegment();
    Segment* endSegment = selection.endSegment();

    if (endSegment) {
        // Make end beat match status bar text
        endSegment = endSegment->prev1(SegmentType::ChordRest);
    }

    IF_ASSERT_FAILED(selection.isRange() && startSegment && endSegment) {
        return QString();
    }

    String startBarBeat = startSegment->formatBarsAndBeats();
    String endBarBeat = endSegment->formatBarsAndBeats();

    startBarBeat.remove(u';'); // Too many pauses in speech
    endBarBeat.remove(u';');

    QString staffInstrument1, staffInstrument2;
    staff_idx_t startStaff = selection.staffStart();
    staff_idx_t endStaff = selection.staffEnd() - 1;

    if (startStaff != endStaff) {
        Staff* staff1 = score->staff(startStaff);
        Staff* staff2 = score->staff(endStaff);
        if (staff1 && staff2) {
            staffInstrument1 = muse::qtrc("engraving", "Staff %1 (%2)")
                               .arg(QString::number(startStaff + 1))
                               .arg(staff1 ? staff1->partName().toQString() : "");
            staffInstrument2 = muse::qtrc("engraving", "Staff %1 (%2)")
                               .arg(QString::number(endStaff + 1))
                               .arg(staff2 ? staff2->partName().toQString() : "");
        }
    }

    return muse::qtrc("engraving", "Range selection starts %1%2 ends %3%4")
           .arg(startBarBeat)
           .arg(!staffInstrument1.isEmpty() ? (" " + staffInstrument1) : "")
           .arg(endBarBeat)
           .arg(!staffInstrument2.isEmpty() ? (" " + staffInstrument2) : "");
}
