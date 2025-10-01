/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "playcounttext.h"

#include "../editing/textedit.h"
#include "../editing/undo.h"
#include "../types/typesconv.h"

#include "barline.h"
#include "score.h"

using namespace mu;
using namespace mu::engraving;

static ElementStyle playCountStyle {
    { Sid::repeatPlayCountPlacement, Pid::PLACEMENT },
    { Sid::repeatPlayCountMinDistance, Pid::MIN_DISTANCE },
};

PlayCountText::PlayCountText(Segment* parent, TextStyleType tid)
    : TextBase(ElementType::PLAY_COUNT_TEXT, parent, tid, ElementFlag::SYSTEM | ElementFlag::ON_STAFF)
{
    initElementStyle(&playCountStyle);
}

void PlayCountText::endEdit(EditData& ed)
{
    UndoStack* undo = score()->undoStack();
    TextEditData* ted = static_cast<TextEditData*>(ed.getData(this).get());
    const bool textWasEdited = undo->currentIndex() > ted->startUndoIdx;

    if (textWasEdited) {
        score()->startCmd(TranslatableString("undoableAction", "Update play count text"));
        barline()->undoChangeProperty(Pid::PLAY_COUNT_TEXT, xmlText());
        barline()->undoChangeProperty(Pid::PLAY_COUNT_TEXT_SETTING, AutoCustomHide::CUSTOM);
        score()->endCmd();
    }
    TextBase::endEdit(ed);
}

PropertyValue PlayCountText::getProperty(Pid id) const
{
    switch (id) {
    case Pid::PLAY_COUNT_TEXT_SETTING:
        return m_playCountTextSetting;
    case Pid::PLAY_COUNT_TEXT:
        return m_playCountCustomText;
    default:
        return TextBase::getProperty(id);
    }
}

bool PlayCountText::setProperty(Pid id, const PropertyValue& v)
{
    Measure* m = segment()->measure();

    switch (id) {
    case Pid::PLAY_COUNT_TEXT_SETTING:
        m_playCountTextSetting = v.value<AutoCustomHide>();
        if (playCountTextSetting() == AutoCustomHide::CUSTOM && playCountCustomText().isEmpty()) {
            int repeatCount = m ? m->repeatCount() : 2;
            String text = TConv::translatedUserName(style().styleV(Sid::repeatPlayCountPreset).value<RepeatPlayCountPreset>()).arg(
                repeatCount);
            undoChangeProperty(Pid::PLAY_COUNT_TEXT, text);
        }
        break;
    case Pid::PLAY_COUNT_TEXT:
        m_playCountCustomText = v.value<String>();
        break;
    default:
        return TextBase::setProperty(id, v);
    }
    triggerLayout();
    return true;
}

PropertyValue PlayCountText::propertyDefault(Pid propertyId) const
{
    Measure* m = segment()->measure();

    switch (propertyId) {
    case Pid::PLAY_COUNT_TEXT_SETTING:
        return AutoCustomHide::AUTO;
    case Pid::PLAY_COUNT_TEXT: {
        int repeatCount = m ? m->repeatCount() : 2;
        return TConv::translatedUserName(style().styleV(Sid::repeatPlayCountPreset).value<RepeatPlayCountPreset>()).arg(repeatCount);
    }
    default:
        return TextBase::propertyDefault(propertyId);
    }
}

RectF PlayCountText::drag(EditData& ed)
{
    // Not TextBase::drag because we don't allow reanchoring on drag
    return EngravingItem::drag(ed);
}

mu::engraving::BarLine* mu::engraving::PlayCountText::barline() const
{
    Segment* seg = segment();
    EngravingItem* el = seg ? seg->element(track()) : nullptr;

    return toBarLine(el);
}
