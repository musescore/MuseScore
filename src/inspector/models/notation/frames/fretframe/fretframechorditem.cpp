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
#include "fretframechorditem.h"

using namespace mu::inspector;

FretFrameChordItem::FretFrameChordItem(QObject* parent)
    : muse::uicomponents::SelectableItemListModel::Item(parent)
{
}

QString FretFrameChordItem::id() const
{
    return m_id;
}

void FretFrameChordItem::setId(const QString& id)
{
    if (m_id == id) {
        return;
    }

    m_id = id;
    emit idChanged();
}

QString FretFrameChordItem::title() const
{
    return m_title.toUpper();
}

void FretFrameChordItem::setTitle(const QString& title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged();
}

bool FretFrameChordItem::isVisible() const
{
    return m_isVisible;
}

void FretFrameChordItem::setIsVisible(bool visible)
{
    if (m_isVisible == visible) {
        return;
    }

    m_isVisible = visible;
    emit isVisibleChanged();
}
