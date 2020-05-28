//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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

#include "qmliconview.h"

namespace Ms {
//---------------------------------------------------------
//   QmlIconView::paint
//---------------------------------------------------------

void QmlIconView::paint(QPainter* p)
{
    if (_icon.isNull()) {
        p->fillRect(0, 0, width(), height(), _color);
        return;
    }

    const QIcon::Mode mode = _selected ? QIcon::Selected : QIcon::Active;
    const QIcon::State state = _active ? QIcon::On : QIcon::Off;
    _icon.paint(p, QRect(0, 0, width(), height()), Qt::AlignCenter, mode, state);
}

//---------------------------------------------------------
//   QmlIconView::setIcon
//---------------------------------------------------------

void QmlIconView::setIcon(QVariant v)
{
    if (v.canConvert<QIcon>()) {
        _icon = v.value<QIcon>();
    } else if (v.canConvert<QColor>()) {
        _color = v.value<QColor>();
        _icon = QIcon();
    } else if (v.canConvert<QPixmap>()) {
        _icon = QIcon(v.value<QPixmap>());
    } else {
        _icon = QIcon();
        _color = QColor(Qt::white);
    }

    update();
}
}
