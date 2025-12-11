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
#include "tableviewcell.h"

using namespace muse::uicomponents;

TableViewCell::TableViewCell(QObject* parent)
    : QObject(parent)
{
}

muse::Val TableViewCell::value() const
{
    return m_val;
}

void TableViewCell::setValue(const Val& value)
{
    if (m_val == value) {
        return;
    }

    m_val = value;
    emit valueChanged();
}

void TableViewCell::setRequestChangeFunction(const std::function<bool(int, int, const Val&)>& func)
{
    m_requestChangeFunction = func;
}

QVariant TableViewCell::value_property() const
{
    return m_val.toQVariant();
}

void TableViewCell::setValue_property(const QVariant& newValue)
{
    Val newVal = Val::fromQVariant(newValue);
    if (m_val == newVal) {
        return;
    }

    if (m_requestChangeFunction && !m_requestChangeFunction(m_row, m_column, newVal)) {
        return;
    }

    m_val = newVal;
    emit valueChanged();
}

int TableViewCell::row() const
{
    return m_row;
}

void TableViewCell::setRow(int row)
{
    if (m_row == row) {
        return;
    }

    m_row = row;
    emit rowChanged();
}

int TableViewCell::column() const
{
    return m_column;
}

void TableViewCell::setColumn(int column)
{
    if (m_column == column) {
        return;
    }

    m_column = column;
    emit columnChanged();
}

bool TableViewCell::hovered() const
{
    return m_hovered;
}

void TableViewCell::setHovered(bool hovered)
{
    if (m_hovered == hovered) {
        return;
    }

    m_hovered = hovered;
    emit hoveredChanged();
}

bool TableViewCell::pressed() const
{
    return m_pressed;
}

void TableViewCell::setPressed(bool pressed)
{
    if (m_pressed == pressed) {
        return;
    }

    m_pressed = pressed;
    emit pressedChanged();
}
