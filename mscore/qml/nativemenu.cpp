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

#include "nativemenu.h"

namespace Ms {
//---------------------------------------------------------
//   QmlNativeMenu
//---------------------------------------------------------

QmlNativeMenu::QmlNativeMenu(QQuickItem* parent)
    : QQuickItem(parent)
{}

//---------------------------------------------------------
//   QmlNativeMenu::createMenu
//---------------------------------------------------------

QMenu* QmlNativeMenu::createMenu() const
{
    QMenu* menu = new QMenu();

    for (QObject* obj : _contentData) {
        if (qobject_cast<QmlMenuSeparator*>(obj)) {
            menu->addSeparator();
        } else if (QmlMenuItem* m = qobject_cast<QmlMenuItem*>(obj)) {
            QAction* a = menu->addAction(m->text());
            a->setCheckable(m->checkable());
            a->setChecked(m->checked());
            a->setEnabled(m->enabled());
            connect(a, &QAction::triggered, m, &QmlMenuItem::triggered, Qt::QueuedConnection);
        }
    }

    return menu;
}

//---------------------------------------------------------
//   QmlNativeMenu::showMenu
//---------------------------------------------------------

void QmlNativeMenu::showMenu(QPoint p)
{
    _visible = true;
    emit visibleChanged();

    QMenu* menu = createMenu();
    menu->exec(p);
    menu->deleteLater();

    _visible = false;
    emit visibleChanged();
}

//---------------------------------------------------------
//   QmlNativeMenu::open
//---------------------------------------------------------

void QmlNativeMenu::open()
{
    const QPoint globalPos = parentItem()->mapToGlobal(pos).toPoint();
    showMenu(globalPos);
}

//---------------------------------------------------------
//   QmlNativeMenu::popup
//---------------------------------------------------------

void QmlNativeMenu::popup()
{
    showMenu(QCursor::pos());
}

//---------------------------------------------------------
//   QmlNativeMenu::setVisible
//---------------------------------------------------------

void QmlNativeMenu::setVisible(bool val)
{
    if (val == _visible) {
        return;
    }

    if (val) {
        open();
    } else {
        Q_ASSERT(false);     // trying to hide visible menu: not implemented and (is supposed to be) unused
    }
}
} // namespace Ms
