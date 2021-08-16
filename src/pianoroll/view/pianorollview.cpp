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

#include "pianorollview.h"

#include "libmscore/element.h"

#include <QPainter>

using namespace mu::pianoroll;

PianoRollView::PianoRollView(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
}

void PianoRollView::onCurrentNotationChanged()
{
    m_notation = globalContext()->currentNotation();
    if (!m_notation) {
        return;
    }

//    auto hh = m_notation->elements();
    INotationElementsPtr elements = m_notation->elements();
//    auto list = elements->elements();
    std::vector<Element*> list = elements->elements();
    for (Element* ele: list) {
        bool sel = ele->selected();
        auto type = ele->type();
        auto name = ele->name();

        int j = 9;
    }
    int j = 9;
}

void PianoRollView::load()
{
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onCurrentNotationChanged();
    });
}

void PianoRollView::paint(QPainter* p)
{
//    if (m_icon.isNull()) {
    p->fillRect(0, 0, width(), height(), m_color);
//        return;
//    }

    p->setPen(Qt::blue);
    p->drawEllipse(0, 0, width(), height());
    //p->fill(0, 0, width(), height(), m_color);

//    const QIcon::Mode mode = m_selected ? QIcon::Selected : QIcon::Active;
//    const QIcon::State state = m_active ? QIcon::On : QIcon::Off;
//    m_icon.paint(p, QRect(0, 0, width(), height()), Qt::AlignCenter, mode, state);
}
