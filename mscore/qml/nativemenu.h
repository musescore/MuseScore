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

#ifndef __QML_NATIVEMENU_H__
#define __QML_NATIVEMENU_H__

namespace Ms {
//---------------------------------------------------------
//   QmlNativeMenu
//---------------------------------------------------------

class QmlNativeMenu : public QQuickItem
{
    Q_OBJECT

    QList<QObject*> _contentData;
    QPoint pos;

    bool _visible = false;

    Q_PROPERTY(QQmlListProperty<QObject> contentData READ contentData CONSTANT)
    Q_CLASSINFO("DefaultProperty", "contentData")

    Q_PROPERTY(int x READ x WRITE setX)
    Q_PROPERTY(int y READ y WRITE setY)

    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)) //???
    QQmlListProperty<QObject> contentData() { return QQmlListProperty<QObject>(this, &_contentData); }   // TODO: use different QQmlListProperty constructor?
#else
    QQmlListProperty<QObject> contentData() { return QQmlListProperty<QObject>(this, _contentData); }   // TODO: use different QQmlListProperty constructor?
#endif

    QMenu* createMenu() const;
    void showMenu(QPoint p);

signals:
    void visibleChanged();

public:
    QmlNativeMenu(QQuickItem* parent = nullptr);

    int x() const { return pos.x(); }
    int y() const { return pos.y(); }
    void setX(int val) { pos.setX(val); }
    void setY(int val) { pos.setY(val); }

    bool visible() const { return _visible; }
    void setVisible(bool val);

    Q_INVOKABLE void open();
    Q_INVOKABLE void popup();
};

//---------------------------------------------------------
//   QmlMenuSeparator
//---------------------------------------------------------

class QmlMenuSeparator : public QObject
{
    Q_OBJECT
public:
    QmlMenuSeparator(QObject* parent = nullptr)
        : QObject(parent) {}
};

//---------------------------------------------------------
//   QmlMenuItem
//---------------------------------------------------------

class QmlMenuItem : public QObject
{
    Q_OBJECT

    QString _text;
    bool _checkable = false;
    bool _checked = false;
    bool _enabled = true;

    Q_PROPERTY(QString text MEMBER _text)
    Q_PROPERTY(bool checkable MEMBER _checkable)
    Q_PROPERTY(bool checked MEMBER _checked)
    Q_PROPERTY(bool enabled MEMBER _enabled)

signals:
    void triggered(bool checked);

public:
    QmlMenuItem(QObject* parent = nullptr)
        : QObject(parent) {}

    const QString& text() const { return _text; }
    bool checkable() const { return _checkable; }
    bool checked() const { return _checked; }
    bool enabled() const { return _enabled; }
};
} // namespace Ms
#endif
