/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "iconview.h"

#include <QPainter>

using namespace mu::uicomponents;

IconView::IconView(QQuickItem* parent)
    : QuickPaintedView(parent)
{
}

void IconView::paint(QPainter* p)
{
    if (m_icon.isNull()) {
        p->fillRect(0, 0, width(), height(), m_color);
        return;
    }

    const QIcon::Mode mode = m_selected ? QIcon::Selected : QIcon::Active;
    const QIcon::State state = m_active ? QIcon::On : QIcon::Off;
    m_icon.paint(p, QRect(0, 0, width(), height()), Qt::AlignCenter, mode, state);
}

void IconView::setIcon(QVariant v)
{
    if (v.canConvert<QIcon>()) {
        m_icon = v.value<QIcon>();
    } else if (v.canConvert<QColor>()) {
        m_color = v.value<QColor>();
        m_icon = QIcon();
    } else if (v.canConvert<QPixmap>()) {
        m_icon = QIcon(v.value<QPixmap>());
    } else {
        m_icon = QIcon();
        m_color = QColor(Qt::white);
    }

    update();
}

QVariant IconView::icon() const
{
    return QVariant::fromValue(m_icon);
}

bool IconView::selected() const
{
    return m_selected;
}

void IconView::setSelected(bool val)
{
    m_selected = val;
    update();
}

bool IconView::active() const
{
    return m_active;
}

void IconView::setActive(bool val)
{
    m_active = val;
    update();
}
