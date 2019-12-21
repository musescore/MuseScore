//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA and others
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

#include "actioneventobserver.h"

#include "google_analytics/ganalytics.h"

#include <QEvent>
#include <QShortcutEvent>
#include <QMenu>
#include <QToolButton>
#include "shortcut.h"

//---------------------------------------------------------
//   ActionEventObserver
//---------------------------------------------------------

ActionEventObserver::ActionEventObserver(QObject *parent) : QObject(parent)
      {
      }

//---------------------------------------------------------
//   extractActionData
//---------------------------------------------------------

QPair<QString, QString> ActionEventObserver::extractActionData(QObject* watched)
      {
      QPair<QString, QString> result;

      QString actionCategory;
      QString actionKey;

      if (qobject_cast<QMenu*>(watched)) {
            QMenu* watchedMenu = qobject_cast<QMenu*>(watched);

            QAction* activeAction = watchedMenu->activeAction();

            if (activeAction) {
                  actionKey = activeAction->data().toString();
                  actionCategory = QStringLiteral("menu item click");
                  }
            }
      else if (qobject_cast<QToolButton*>(watched)) {
            QToolButton* watchedButton = qobject_cast<QToolButton*>(watched);

            QAction* activeAction = watchedButton->defaultAction();

            if (activeAction) {
                  actionKey = activeAction->data().toString();
                  actionCategory = QStringLiteral("button clicked");
                  }
            }

      result.first = actionCategory;
      result.second = actionKey;

      return result;
      }

//---------------------------------------------------------
//   eventFilter
//---------------------------------------------------------

bool ActionEventObserver::eventFilter(QObject *watched, QEvent *event)
      {
      if (event->type() == QEvent::MouseButtonRelease) {

            QPair<QString, QString> actionData = extractActionData(watched);

            telemetryService()->sendEvent(actionData.first, actionData.second);

            }
      else if (event->type() == QEvent::Shortcut) {
            QShortcutEvent* shortCutEvent = static_cast<QShortcutEvent*>(event);

            Ms::Shortcut* shortcut = Ms::Shortcut::getShortcutByKeySequence(shortCutEvent->key());

            telemetryService()->sendEvent("shortcut", shortcut->key());
            }

      return false;
      }
