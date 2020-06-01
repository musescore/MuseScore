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

#ifndef __QMLICONVIEW_H__
#define __QMLICONVIEW_H__

namespace Ms {
//---------------------------------------------------------
//   QmlIconView
//---------------------------------------------------------

class QmlIconView : public QQuickPaintedItem
{
    Q_OBJECT

    QColor _color;
    QIcon _icon;
    bool _selected { false };
    bool _active   { false };

    Q_PROPERTY(QVariant icon READ icon WRITE setIcon)
    Q_PROPERTY(bool selected READ selected WRITE setSelected)
    Q_PROPERTY(bool active READ active WRITE setActive)

public:
    QmlIconView(QQuickItem* parent = nullptr)
        : QQuickPaintedItem(parent) {}

    QVariant icon() const { return QVariant::fromValue(_icon); }
    void setIcon(QVariant val);

    bool selected() const { return _selected; }
    void setSelected(bool val) { _selected = val; update(); }

    bool active() const { return _active; }
    void setActive(bool val) { _active = val; update(); }

    void paint(QPainter*) override;
};
} // namespace Ms
#endif
