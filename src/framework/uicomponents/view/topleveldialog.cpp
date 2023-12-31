/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "topleveldialog.h"

#include <QApplication>
#include <QKeyEvent>
#include <QWindow>

using namespace mu::uicomponents;

TopLevelDialog::TopLevelDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    auto updateStayOnTopHint = [this]() {
        bool stay = qApp->applicationState() == Qt::ApplicationActive;
        // Setting window flags causes dialogs with no parent to be hidden. In
        // InteractiveProvider::openWidgetDialog we setup callbacks for dialog
        // completion events that delete the dialog when it's done. However, QDialog
        // interprets the hiding of dialogs as a completion event. In order to avoid
        // deleting the dialog in this specific case, we use this member variable
        // and then ignore the deletion event altogether below in
        // TopLevelDialog::event.
        m_shouldInhibitVisibilityEvents = true;

        setWindowFlag(Qt::WindowStaysOnTopHint, stay);
        if (stay) {
            show();
        }
    };
    updateStayOnTopHint();
    connect(qApp, &QApplication::applicationStateChanged, this, updateStayOnTopHint);
}

TopLevelDialog::TopLevelDialog(const TopLevelDialog& dialog)
    : QDialog(dialog.parentWidget())
{
}

bool TopLevelDialog::event(QEvent* e)
{
#ifndef Q_OS_MAC
    if (e->type() == QEvent::Show) {
        windowHandle()->setTransientParent(mainWindow()->qWindow());
    }
#endif

    if (e->type() == QEvent::ShortcutOverride) {
        if (QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(e)) {
            if (keyEvent->key() == Qt::Key_Escape && keyEvent->modifiers() == Qt::NoModifier) {
                close();
                return true;
            }
        }
    }
    // We need to reset the visibility event inhibition when a dialog is
    // (re)shown so that it can be properly handled. This check relies on the
    // functional owner of this dialog's open/hide/etc events being processed
    // before this event() function. See similar note in
    // InteractiveProvider::openWidgetDialog.
    else if (e->type() == QEvent::Show) {
        m_shouldInhibitVisibilityEvents = false;
    }

    return QDialog::event(e);
}
