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
#include "notationaccessibility.h"

#include "translation.h"

#include "igetscore.h"
#include "notation.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/part.h"
#include "engraving/dom/sig.h"
#include "engraving/dom/measure.h"

#include "accessibility/accessibleroot.h"

using namespace mu::notation;
using namespace muse::async;
using namespace mu::engraving;
using namespace muse::accessibility;

NotationAccessibility::NotationAccessibility(const Notation* notation)
    : m_getScore(notation)
{
    notation->interaction()->selectionChanged().onNotify(this, [this]() {
        setTriggeredCommand("");
        updateAccessibilityInfo();
    });

    notation->notationChanged().onNotify(this, [this]() {
        updateAccessibilityInfo();
    });
}

const mu::engraving::Score* NotationAccessibility::score() const
{
    return m_getScore->score();
}

const mu::engraving::Selection* NotationAccessibility::selection() const
{
    return &score()->selection();
}

muse::ValCh<std::string> NotationAccessibility::accessibilityInfo() const
{
    return m_accessibilityInfo;
}

void NotationAccessibility::setMapToScreenFunc(const AccessibleMapToScreenFunc& func)
{
#ifndef ENGRAVING_NO_ACCESSIBILITY
    score()->rootItem()->accessible()->accessibleRoot()->setMapToScreenFunc(func);
    score()->dummy()->rootItem()->accessible()->accessibleRoot()->setMapToScreenFunc(func);
#else
    UNUSED(func)
#endif
}

void NotationAccessibility::setEnabled(bool enabled)
{
#ifndef ENGRAVING_NO_ACCESSIBILITY
    std::vector<AccessibleRoot*> roots {
        score()->rootItem()->accessible()->accessibleRoot(),
        score()->dummy()->rootItem()->accessible()->accessibleRoot()
    };

    EngravingItem* selectedElement = selection()->element();
    AccessibleItemPtr selectedElementAccItem = selectedElement ? selectedElement->accessible() : nullptr;

    for (AccessibleRoot* root : roots) {
        root->setEnabled(enabled);

        if (!enabled) {
            root->setFocusedElement(nullptr);
            continue;
        }

        if (!selectedElementAccItem) {
            continue;
        }

        if (selectedElementAccItem->accessibleRoot() == root) {
            root->setFocusedElement(selectedElementAccItem);
        }
    }
#else
    UNUSED(enabled)
#endif
}

void NotationAccessibility::setTriggeredCommand(const std::string& command)
{
#ifndef ENGRAVING_NO_ACCESSIBILITY
    score()->rootItem()->accessible()->accessibleRoot()->setCommandInfo(QString::fromStdString(command));
    score()->dummy()->rootItem()->accessible()->accessibleRoot()->setCommandInfo(QString::fromStdString(command));
#else
    UNUSED(command)
#endif
}

void NotationAccessibility::updateAccessibilityInfo()
{
    if (!score()) {
        return;
    }

    QString newAccessibilityInfo;

    if (selection()->isSingle()) {
        newAccessibilityInfo = singleElementAccessibilityInfo();
    } else if (selection()->isRange()) {
        newAccessibilityInfo = rangeAccessibilityInfo();
    } else if (selection()->isList()) {
        newAccessibilityInfo = muse::qtrc("notation", "List selection");
    }

    // Simplify whitespace and remove newlines
    newAccessibilityInfo = newAccessibilityInfo.simplified();

    setAccessibilityInfo(newAccessibilityInfo);
}

void NotationAccessibility::setAccessibilityInfo(const QString& info)
{
    std::string infoStd = info.toStdString();

    if (m_accessibilityInfo.val == infoStd) {
        return;
    }

    m_accessibilityInfo.set(infoStd);
}

QString NotationAccessibility::rangeAccessibilityInfo() const
{
    const mu::engraving::Segment* endSegment = selection()->endSegment();

    if (!endSegment) {
        endSegment = score()->lastSegment();
    } else {
        endSegment = endSegment->prev1MM();
    }

    EngravingItem::BarBeat startBarbeat = selection()->startSegment()->barbeat();

    QString start = muse::qtrc("engraving", "Start measure: %1").arg(String::number(startBarbeat.bar));
    if (startBarbeat.displayedBar != startBarbeat.bar) {
        start += "; " + muse::qtrc("engraving", "Start displayed measure: %1").arg(startBarbeat.displayedBar);
    }
    start += "; " + muse::qtrc("engraving", "Start beat: %1").arg(startBarbeat.beat);

    EngravingItem::BarBeat endBarbeat = endSegment->barbeat();
    QString end = muse::qtrc("engraving", "End measure: %1").arg(String::number(endBarbeat.bar));
    if (endBarbeat.displayedBar != endBarbeat.bar) {
        end += "; " + muse::qtrc("engraving", "End displayed measure: %1").arg(endBarbeat.displayedBar);
    }
    end += "; " + muse::qtrc("engraving", "End beat: %1").arg(endBarbeat.beat);

    return muse::qtrc("notation", "Range selection; %1; %2")
           .arg(start)
           .arg(end);
}

QString NotationAccessibility::singleElementAccessibilityInfo() const
{
    const EngravingItem* element = selection()->element();
    if (!element) {
        return QString();
    }

    QString accessibilityInfo = element->accessibleInfo();
    QString barsAndBeats = element->formatBarsAndBeats();

    if (!barsAndBeats.isEmpty()) {
        accessibilityInfo += "; " + barsAndBeats;
    }

    if (element->hasStaff()) {
        QString staff = muse::qtrc("notation", "Staff %1").arg(QString::number(element->staffIdx() + 1));

        QString staffName = element->staff()->part()->longName(element->tick());
        if (staffName.isEmpty()) {
            staffName = element->staff()->partName();
        }

        if (staffName.isEmpty()) {
            accessibilityInfo = QString("%1; %2").arg(accessibilityInfo).arg(staff);
        } else {
            accessibilityInfo = QString("%1; %2 (%3)").arg(accessibilityInfo).arg(staff).arg(staffName);
        }
    }

    return accessibilityInfo;
}
