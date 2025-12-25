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
#include "noteinputbarcustomiseitem.h"

using namespace mu::notation;
using namespace muse::uicomponents;

NoteInputBarCustomiseItem::NoteInputBarCustomiseItem(const ItemType& type, QObject* parent)
    : Item(parent), m_type(type)
{
}

QString NoteInputBarCustomiseItem::id() const
{
    return m_id;
}

QString NoteInputBarCustomiseItem::title() const
{
    return m_title;
}

NoteInputBarCustomiseItem::ItemType NoteInputBarCustomiseItem::type() const
{
    return m_type;
}

void NoteInputBarCustomiseItem::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged();
}

void NoteInputBarCustomiseItem::setId(const QString& id)
{
    m_id = id;
}

int NoteInputBarCustomiseItem::icon() const
{
    return static_cast<int>(m_icon);
}

bool NoteInputBarCustomiseItem::checked() const
{
    return m_checked;
}

void NoteInputBarCustomiseItem::setIcon(muse::ui::IconCode::Code icon)
{
    if (m_icon == icon) {
        return;
    }

    m_icon = icon;
    emit iconChanged();
}

void NoteInputBarCustomiseItem::setChecked(bool checked)
{
    if (m_checked == checked) {
        return;
    }

    m_checked = checked;
    emit checkedChanged(m_checked);
}
