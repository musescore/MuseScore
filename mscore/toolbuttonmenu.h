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
//   action, while pressing the arrow brings up a menu of alternative actions and/or other
//   actions. Selecting an alternative action has some effect on the default action's icon
//   and/or its behavior. Other actions have no effect on the default action.
//---------------------------------------------------------

class ToolButtonMenu : public AccessibleToolButton { // : public QToolButton {
      Q_OBJECT

   public:
      enum class TYPES {
            // What happens to the default action if an alternative (non-default) action is triggered?
            FIXED, // default action is also triggered but is otherwise unchanged.
            ICON_CHANGED, // default action is triggered and its icon is modified and/or set to that of the triggering action.
            ACTION_SWAPPED // default action is not triggered. Triggering action (and its icon) become the new default.
            };

   private:
      TYPES _type;
      QActionGroup* _alternativeActions;

   public:
      ToolButtonMenu(QString str,
                     TYPES type,
                     QAction* defaultAction,
                     QActionGroup* alternativeActions,
                     QWidget* parent);
      void addAction(QAction* a) { menu()->addAction(a); }
      void addSeparator() { menu()->addSeparator(); }
      void addActions(QList<QAction*> actions) { for (QAction* a : actions) addAction(a); }

   private:
      void switchIcon(QAction* a) {
            Q_ASSERT(_type == TYPES::ICON_CHANGED && _alternativeActions->actions().contains(a));
            defaultAction()->setIcon(a->icon());
            }

   protected:
      virtual void changeIcon(QAction* a) { switchIcon(a); }

   private slots:
      void handleAlternativeAction(QAction* a);

      };

} // namespace Ms

#endif // TOOLBUTTONMENU_H
