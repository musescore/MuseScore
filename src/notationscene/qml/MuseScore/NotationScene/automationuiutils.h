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
#pragma once

#include <optional>

#include "global/types/string.h"
#include "engraving/automation/automationtypes.h"
#include "uicomponents/qml/Muse/UiComponents/menuitem.h"

//! NOTE: shared between the automation UI variants (Widget and Toolbar)
namespace mu::notation::automationui {
static constexpr int VOICES_COUNT = 4;

inline const std::string SET_TEMPO_RANGE_DIALOG_URI("musescore://notation/settemporange");

muse::String typeTitle(engraving::AutomationType type);

//! NOTE: nullopt means all voices
muse::String voiceTitle(const std::optional<int>& voiceIdx);

muse::uicomponents::MenuItem* makeItem(const QString& id, const muse::String& title, QObject* parent, bool checkable = false,
                                       bool checked = false);

muse::uicomponents::MenuItemList makeResetItems(engraving::AutomationType type, QObject* parent);
bool isResetItem(const QString& itemId);

//! NOTE: UI only for now
void handleResetItem(const QString& itemId);
}
