//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA
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

#include "nativetooltip.h"

#include <QToolTip>

namespace Ms {
//---------------------------------------------------------
//   QmlNativeToolTip
//---------------------------------------------------------

QmlNativeToolTip::QmlNativeToolTip(QWidget* w, QObject* parent)
    : QObject(parent), _widget(w)
{
    _timer.setSingleShot(true);
    connect(&_timer, &QTimer::timeout, this, &QmlNativeToolTip::showToolTip);
}

//---------------------------------------------------------
//   QmlNativeToolTip::setItem
//---------------------------------------------------------

void QmlNativeToolTip::setItem(QQuickItem* i)
{
    if (i != _item) {
        const int interval = i && _lastShownText.isEmpty() ? qApp->styleHints()->mousePressAndHoldInterval() : 100;
        _timer.start(interval);
        _item = i;
    }
}

//---------------------------------------------------------
//   QmlNativeToolTip::showToolTip
//---------------------------------------------------------

void QmlNativeToolTip::showToolTip()
{
    if (QToolTip::text() == _lastShownText) {
        QToolTip::hideText();
    }
    _lastShownText.clear();

    if (!_item) {
        return;
    }

    const QPointF topLeft = _item->mapToGlobal(QPointF(0, 0));
    const QRect rect(topLeft.x(), topLeft.y(), _item->width(), _item->height());
    const QPoint pos(QCursor::pos());

    if (rect.contains(pos)) {
        QToolTip::showText(pos, _text, _widget, rect);     // TODO: it doesn't look like setting rect has an effect here...
        _lastShownText = _text;
    }
}
} // namespace Ms
