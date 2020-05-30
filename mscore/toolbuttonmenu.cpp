//=============================================================================
//  toolbuttonmenu.cpp
//
//  Copyright (C) 2016 Peter Jonas <shoogle@users.noreply.github.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "toolbuttonmenu.h"

namespace Ms {
ToolButtonMenu::ToolButtonMenu(QString name,
                               QAction* defaultAction,
                               QActionGroup* alternativeActions,
                               QWidget* parent,
                               bool swapAction)
    : AccessibleToolButton(parent, defaultAction)
{
    // does the default action count as one of the alternative actions?
    Q_ASSERT(swapAction == alternativeActions->actions().contains(defaultAction));

    _swapAction = swapAction;
    _alternativeActions = alternativeActions;

    setMenu(new QMenu(name, this));
    menu()->setToolTipsVisible(true);

    setPopupMode(QToolButton::MenuButtonPopup);

    if (!swapAction) {
        addAction(defaultAction);
        addSeparator();
    }

    for (QAction* a : _alternativeActions->actions()) {
        a->setCheckable(true);
        a->setIconVisibleInMenu(true);
    }

    _alternativeActions->setExclusive(true);
    addActions(_alternativeActions->actions());

    connect(_alternativeActions, SIGNAL(triggered(QAction*)), this, SLOT(handleAlternativeAction(QAction*)));

    if (!swapAction) {
        // select first alternative action and use its icon for the default action
        QAction* a = _alternativeActions->actions().first();
        a->setChecked(true);
        switchIcon(a);
    }
}

void ToolButtonMenu::handleAlternativeAction(QAction* a)
{
    Q_ASSERT(_alternativeActions->actions().contains(a));

    if (_swapAction) {
        setDefaultAction(a);
    } else {
        switchIcon(a);
    }

    QAction* def = defaultAction();

    if (!def->isChecked()) {
        def->trigger();
    }
}

void ToolButtonMenu::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Space:
        menu()->exec(this->mapToGlobal(QPoint(0, size().height())));
        menu()->setFocus();
        menu()->setActiveAction(defaultAction());
        break;
    default:
        AccessibleToolButton::keyPressEvent(event);
    }
}
} // namespace Ms
