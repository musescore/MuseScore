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

#ifndef __QML_NATIVETOOLTIP_H__
#define __QML_NATIVETOOLTIP_H__

namespace Ms {
//---------------------------------------------------------
//   QmlNativeToolTip
//---------------------------------------------------------

class QmlNativeToolTip : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem * item READ item WRITE setItem)
    Q_PROPERTY(QString text READ text WRITE setText)

    QWidget* _widget;
    QPointer<QQuickItem> _item = nullptr;
    QString _text;
    QString _lastShownText;
    QTimer _timer;

private slots:
    void showToolTip();

public:
    QmlNativeToolTip(QWidget* w, QObject* parent = nullptr);

    QQuickItem* item() const { return _item; }
    void setItem(QQuickItem*);

    const QString& text() const { return _text; }
    void setText(const QString& t) { _text = t; }
};
} // namespace Ms
#endif
