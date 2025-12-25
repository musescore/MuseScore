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

#include "colorlabel.h"

#include <QColorDialog>
#include <QPainter>

#include "translation.h"

using namespace muse;

namespace Awl {
ColorLabel::ColorLabel(QWidget* parent)
    : QPushButton(parent)
{
    connect(this, &QPushButton::clicked, this, &ColorLabel::colorButtonClicked);
    setFlat(true);
}

void ColorLabel::setColor(const QColor& c)
{
    const bool changed = _color != c;
    _color = c;
    update();
    if (changed) {
        emit this->colorChanged(_color);
    }
}

QSize ColorLabel::sizeHint() const
{
    return QSize(30, 30);
}

void ColorLabel::paintEvent(QPaintEvent* ev)
{
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(_color);
    p.drawRoundedRect(rect(), 3, 3);

    QPushButton::paintEvent(ev);
}

void ColorLabel::colorButtonClicked(bool)
{
    QColor c = QColorDialog::getColor(_color, this,
                                      muse::qtrc("notation", "Select color"),
                                      QColorDialog::ShowAlphaChannel
                                      );
    if (c.isValid()) {
        if (_color != c) {
            _color = c;
            emit colorChanged(_color);
            update();
        }
    }
}
}
