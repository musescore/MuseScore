//=============================================================================
//  Awl
//  Audio Widget Library
//
//  Copyright (C) 2002-2006 by Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "colorlabel.h"

namespace Awl {
//---------------------------------------------------------
//   ColorLabel
//---------------------------------------------------------

ColorLabel::ColorLabel(QWidget* parent)
    : QPushButton(parent)
{
    connect(this, &QPushButton::clicked, this, &ColorLabel::colorButtonClicked);
    setFlat(true);
}

ColorLabel::~ColorLabel()
{
    delete _pixmap;
}

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void ColorLabel::setColor(const QColor& c)
{
    const bool changed = _color != c;
    _color = c;
    update();
    if (changed) {
        emit this->colorChanged(_color);
    }
}

//---------------------------------------------------------
//   setPixmap
//---------------------------------------------------------

void ColorLabel::setPixmap(QPixmap* pm)
{
    delete _pixmap;
    _pixmap = pm;
    update();
}

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize ColorLabel::sizeHint() const
{
    return QSize(30, 20);
}

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void ColorLabel::paintEvent(QPaintEvent* ev)
{
    QPainter p(this);
    if (_pixmap) {
        p.drawTiledPixmap(rect(), *_pixmap);
    } else {
        p.fillRect(rect(), _color);
    }

    QPushButton::paintEvent(ev);
}

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void ColorLabel::colorButtonClicked(bool)
{
    if (_pixmap) {
        return;
    }
    QColor c = QColorDialog::getColor(_color, this,
                                      tr("Select Color"),
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
// namespace Awl
