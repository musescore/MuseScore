/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "tableviewheader.h"

using namespace muse::uicomponents;

TableViewHeader::TableViewHeader(QObject* parent)
    : QObject(parent)
{
}

QString TableViewHeader::title() const
{
    return m_title;
}

void TableViewHeader::setTitle(const QString& title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged();
}

int TableViewHeader::preferredWidth() const
{
    return m_preferredWidth;
}

void TableViewHeader::setPreferredWidth(int width)
{
    if (m_preferredWidth == width) {
        return;
    }

    m_preferredWidth = width;
    emit preferredWidthChanged();
}

TableViewCellType::Type TableViewHeader::cellType() const
{
    return m_cellType;
}

void TableViewHeader::setCellType(TableViewCellType::Type type)
{
    if (m_cellType == type) {
        return;
    }

    m_cellType = type;
    emit cellTypeChanged();
}

TableViewCellEditMode::Mode TableViewHeader::cellEditMode() const
{
    return m_cellEditMode;
}

void TableViewHeader::setCellEditMode(TableViewCellEditMode::Mode mode)
{
    if (m_cellEditMode == mode) {
        return;
    }

    m_cellEditMode = mode;
    emit cellEditModeChanged();
}

MenuItemList TableViewHeader::availableFormats() const
{
    return m_availableFormats;
}

void TableViewHeader::setAvailableFormats(const MenuItemList& formats)
{
    if (m_availableFormats == formats) {
        return;
    }

    m_availableFormats = formats;
    emit availableFormatsChanged();
}

QString TableViewHeader::currentFormatId() const
{
    return m_currentFormatId;
}

void TableViewHeader::setCurrentFormatId(const QString& id)
{
    if (m_currentFormatId == id) {
        return;
    }

    for (muse::uicomponents::MenuItem* item : std::as_const(m_availableFormats)) {
        item->setChecked(item->id() == id);
    }

    m_currentFormatId = id;
    emit currentFormatIdChanged();
}
