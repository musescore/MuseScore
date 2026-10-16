/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#include "automationuiutils.h"

#include "types/translatablestring.h"

#include "log.h"

using namespace muse;
using namespace muse::uicomponents;
using mu::engraving::AutomationType;

namespace mu::notation::automationui {
static const QString RESET_ITEM_ID_PREFIX("automation-reset-");

String typeTitle(AutomationType type)
{
    switch (type) {
    case AutomationType::Dynamics: return u"Dynamics";
    case AutomationType::Volume: return u"Volume";
    case AutomationType::Pan: return u"Pan";
    case AutomationType::Tempo: return u"Tempo";
    case AutomationType::Unknown: break;
    }

    return String();
}

String voiceTitle(const std::optional<int>& voiceIdx)
{
    return voiceIdx ? String(u"Voice %1").arg(voiceIdx.value() + 1) : String(u"All voices");
}

MenuItem* makeItem(const QString& id, const String& title, QObject* parent, bool checkable, bool checked)
{
    MenuItem* item = new MenuItem(parent);
    item->setId(id);
    item->setTitle(TranslatableString::untranslatable(title));
    item->setEnabled(true);
    item->setCheckable(checkable);
    item->setChecked(checked);

    return item;
}

MenuItemList makeResetItems(AutomationType type, QObject* parent)
{
    const String title = typeTitle(type);

    return {
        makeItem(RESET_ITEM_ID_PREFIX + "selection", u"Selection", parent),
        makeItem(RESET_ITEM_ID_PREFIX + "type-on-staff", String(u"%1 automation on this staff").arg(title), parent),
        makeItem(RESET_ITEM_ID_PREFIX + "all-on-staff", u"All automation on this staff", parent),
        makeItem(RESET_ITEM_ID_PREFIX + "type-everywhere", String(u"All %1 automation").arg(title.toLower()), parent),
    };
}

bool isResetItem(const QString& itemId)
{
    return itemId.startsWith(RESET_ITEM_ID_PREFIX);
}

void handleResetItem(const QString& itemId)
{
    //! TODO: not implemented yet
    LOGI() << "Automation reset requested: " << itemId;
}
}
