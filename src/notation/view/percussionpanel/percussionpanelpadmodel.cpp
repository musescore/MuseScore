/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "percussionpanelpadmodel.h"

#include "ui/view/iconcodes.h"

static const QString DUPLICATE_PAD_CODE("duplicate-pad");
static const QString DELETE_PAD_CODE("delete-pad");
static const QString DEFINE_PAD_SHORTCUT_CODE("define-pad-shortcut");

using namespace mu::notation;
using namespace muse::ui;

PercussionPanelPadModel::PercussionPanelPadModel(QObject* parent)
    : QObject(parent)
{
}

void PercussionPanelPadModel::setPadName(const QString& padName)
{
    if (m_padName == padName) {
        return;
    }

    m_padName = padName;
    emit padNameChanged();
}

void PercussionPanelPadModel::setKeyboardShortcut(const QString& keyboardShortcut)
{
    if (m_keyboardShortcut == keyboardShortcut) {
        return;
    }

    m_keyboardShortcut = keyboardShortcut;
    emit keyboardShortcutChanged();
}

void PercussionPanelPadModel::setPitch(int pitch)
{
    if (m_pitch == pitch) {
        return;
    }

    m_pitch = pitch;
    emit midiNoteChanged();
}

void PercussionPanelPadModel::setNotationPreviewItem(mu::engraving::ElementPtr item)
{
    if (m_notationPreviewItem == item) {
        return;
    }

    m_notationPreviewItem = item;
    emit notationPreviewItemChanged();
}

const QVariant PercussionPanelPadModel::notationPreviewItemVariant() const
{
    return QVariant::fromValue(m_notationPreviewItem);
}

QList<QVariantMap> PercussionPanelPadModel::footerContextMenuItems() const
{
    static constexpr int duplicatePadIcon = static_cast<int>(IconCode::Code::COPY);
    static constexpr int deletePadIcon = static_cast<int>(IconCode::Code::DELETE_TANK);
    static constexpr int definePadShortcutIcon = static_cast<int>(IconCode::Code::SHORTCUTS);

    QList<QVariantMap> menuItems = {
        { { "id", DUPLICATE_PAD_CODE }, { "title", muse::qtrc("global", "Duplicate") },
            { "icon", duplicatePadIcon }, { "enabled", true } },

        { { "id", DELETE_PAD_CODE }, { "title", muse::qtrc("global", "Delete") },
            { "icon", deletePadIcon }, { "enabled", true } },

        { }, // separator

        { { "id", DEFINE_PAD_SHORTCUT_CODE }, { "title", muse::qtrc("shortcuts", "Define keyboard shortcut") },
            { "icon", definePadShortcutIcon }, { "enabled", true } },
    };

    return menuItems;
}

void PercussionPanelPadModel::handleMenuItem(const QString& itemId)
{
    if (itemId == DUPLICATE_PAD_CODE) {
        m_padActionTriggered.send(PadAction::DUPLICATE);
    } else if (itemId == DELETE_PAD_CODE) {
        m_padActionTriggered.send(PadAction::DELETE);
    } else if (itemId == DEFINE_PAD_SHORTCUT_CODE) {
        m_padActionTriggered.send(PadAction::DEFINE_SHORTCUT);
    }
}

void PercussionPanelPadModel::triggerPad()
{
    m_padActionTriggered.send(PadAction::TRIGGER);
}
