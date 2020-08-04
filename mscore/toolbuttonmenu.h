//=============================================================================
//  toolbuttonmenu.h
//
//  Copyright (C) 2016 Peter Jonas <shoogle@users.noreply.github.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef TOOLBUTTONMENU_H
#define TOOLBUTTONMENU_H

#include "accessibletoolbutton.h"

namespace Ms {
//---------------------------------------------------------
//   ToolButtonMenu
//   ==============
//   This creates a button with an arrow next to it. Clicking the button triggers the default
//   action, while pressing the arrow brings up a menu of alternative actions. Selecting an
//   alternative action triggers it and also has some effect on the default action.
//
//   The menu may contain:
//      - Any number of alternative actions in addition to the default action.
//      - Other actions that do not affect the default action.
//         - These are not considered "alternative actions" for our purposes.
//
//   The effect of selecting an alternative action is determined by the swapAction boolean.
//      TRUE: The selected action replaces the default action (it becomes the new default).
//      FALSE: The default action remains unchanged, but its icon is updated to match the
//         icon of the selected alternative action.
//---------------------------------------------------------

class ToolButtonMenu : public AccessibleToolButton   // : public QToolButton {
{
    Q_OBJECT

private:
    QActionGroup* _alternativeActions;
    bool _swapAction;

public:
    ToolButtonMenu(QString str,QAction* defaultAction,QActionGroup* alternativeActions,QWidget* parent = nullptr,bool swapAction = true);
    void addAction(QAction* a) { menu()->addAction(a); }
    void addSeparator() { menu()->addSeparator(); }
    void addActions(QList<QAction*> actions)
    {
        for (QAction* a : actions) {
            addAction(a);
        }
    }

private:
    void switchIcon(QAction* a)
    {
        Q_ASSERT(!_swapAction);
        Q_ASSERT(_alternativeActions->actions().contains(a));
        defaultAction()->setIcon(a->icon());
    }

private slots:
    void handleAlternativeAction(QAction* a);

protected:
    void keyPressEvent(QKeyEvent* event) override;
};
} // namespace Ms

#endif // TOOLBUTTONMENU_H
