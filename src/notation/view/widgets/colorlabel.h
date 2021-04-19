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

#ifndef __AWLCOLORLABEL_H__
#define __AWLCOLORLABEL_H__

#include <QPushButton>

namespace Awl {
//---------------------------------------------------------
//   ColorLabel
//---------------------------------------------------------

class ColorLabel : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)

    QColor _color = Qt::blue;
    QPixmap* _pixmap = nullptr;

    virtual void paintEvent(QPaintEvent*);

signals:
    void colorChanged(QColor);

public slots:
    void colorButtonClicked(bool);

public:
    ColorLabel(QWidget* parent = 0);
    ~ColorLabel();
    void setColor(const QColor& c);
    virtual QSize sizeHint() const;
    void setPixmap(QPixmap*);
    QColor color() const { return _color; }
    QPixmap* pixmap() const { return _pixmap; }
};
}  // namespace Awl
#endif
