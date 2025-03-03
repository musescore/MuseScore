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

#include "dialogview.h"

#include <QStyle>
#include <QWindow>
#include <QScreen>
#include <QApplication>

#include "log.h"
#include "async/async.h"

using namespace muse::uicomponents;

static const int DIALOG_WINDOW_FRAME_HEIGHT(20);

DialogView::DialogView(QQuickItem* parent)
    : PopupView(parent)
{
    setObjectName("DialogView");
    setClosePolicies(ClosePolicy::NoAutoClose);
}

bool DialogView::isDialog() const
{
    return true;
}

bool DialogView::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() != QEvent::ShortcutOverride) {
        return PopupView::eventFilter(watched, event);
    }

    QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
    if (!keyEvent || !shortcutOverrideIsAllowed()) {
        return PopupView::eventFilter(watched, event);
    }

    const auto triggerControl = [this, event](QObject* navigation) {
        // Both spontaneous and synthetic events will be received
        // We intercept both, but only need to invoke once
        if (event->spontaneous()) {
            async::Async::call(this, [navigation]() {
                QMetaObject::invokeMethod(navigation, "triggered");
            });
        }
    };

    switch (keyEvent->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return: {
        // Rule: Enter/return should always trigger the accent button unless the currently highlighted control
        // is itself a button, in which case we should trigger that button...
        QObject* accentButtonNavigation = nullptr;
        for (const QObject* btn : findButtons()) {
            const QVariant navigationVar = btn->property("navigation");
            const QVariant visibleVar = btn->property("visible");
            if (!navigationVar.isValid() || !visibleVar.isValid() || !visibleVar.toBool()) {
                continue;
            }

            QObject* navigation = navigationVar.value<QObject*>();
            if (!navigation) {
                continue;
            }

            // Is this button highlighed? If so trigger it...
            const QVariant navigationHighlightVar = navigation->property("highlight");
            if (navigationHighlightVar.isValid() && navigationHighlightVar.toBool()) {
                triggerControl(navigation);
                event->accept();
                return true;
            }

            const QVariant accentButtonVar = btn->property("accentButton");
            if (!accentButtonVar.isValid() || !accentButtonVar.toBool()) {
                continue;
            }

            // If accentButtonNavigation has been set before, then there are multiple visible accent buttons. This
            // shouldn't be allowed per the above rule...
            IF_ASSERT_FAILED(!accentButtonNavigation) {
                continue;
            }

            accentButtonNavigation = navigation;
        }

        // Did we find an accent button? If so trigger it...
        if (accentButtonNavigation) {
            triggerControl(accentButtonNavigation);
            event->accept();
            return true;
        }
    }
    default: break;
    }

    return PopupView::eventFilter(watched, event);
}

bool DialogView::shortcutOverrideIsAllowed() const
{
    //! NOTE: We should only override shortcuts if the current window is active.
    //! There are objects that do not activate focus when opened, for example Dropdowns.
    //! Therefore, we cannot simply use the current active window inside DialogView::eventFilter.
    //! Instead, we should ask the navigation system which window has focus:
    if (ui::INavigationSection* section = navigationController()->activeSection()) {
        return section->window() == qWindow();
    }

    return false;
}

void DialogView::beforeOpen()
{
    //! NOTE Set default title
    if (m_title.isEmpty()) {
        setTitle(application()->title());
    }
}

void DialogView::onHidden()
{
    PopupView::onHidden();

    if (m_loop.isRunning()) {
        m_loop.exit();
    }
}

QScreen* DialogView::resolveScreen() const
{
    QWindow* qMainWindow = mainWindow()->qWindow();
    QScreen* mainWindowScreen = qMainWindow->screen();
    if (!mainWindowScreen) {
        mainWindowScreen = QGuiApplication::primaryScreen();
    }

    return mainWindowScreen;
}

void DialogView::updateGeometry()
{
    const QScreen* screen = resolveScreen();
    QRect anchorRect = screen->availableGeometry();

    const QWindow* qMainWindow = mainWindow()->qWindow();
    bool mainWindowVisible = qMainWindow->isVisible();
    QRect referenceRect = qMainWindow->geometry();

    if (referenceRect.isEmpty() || !mainWindowVisible) {
        referenceRect = anchorRect;
    }

    QRect dlgRect = viewGeometry();

    // position the dialog in the center of the main window
    dlgRect.moveLeft(referenceRect.x() + (referenceRect.width() - dlgRect.width()) / 2);
    dlgRect.moveTop(referenceRect.y() + (referenceRect.height() - dlgRect.height()) / 2 + DIALOG_WINDOW_FRAME_HEIGHT);

    dlgRect.moveLeft(dlgRect.x() + m_localPos.x());
    dlgRect.moveTop(dlgRect.y() + m_localPos.y());

    // try to move the dialog if it doesn't fit on the screen

    int titleBarHeight = QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);

    if (dlgRect.left() <= anchorRect.left()) {
        dlgRect.moveLeft(anchorRect.left() + DIALOG_WINDOW_FRAME_HEIGHT);
    }

    if (dlgRect.top() - titleBarHeight <= anchorRect.top()) {
        dlgRect.moveTop(anchorRect.top() + titleBarHeight + DIALOG_WINDOW_FRAME_HEIGHT);
    }

    if (dlgRect.right() >= anchorRect.right()) {
        dlgRect.moveRight(anchorRect.right() - DIALOG_WINDOW_FRAME_HEIGHT);
    }

    if (dlgRect.bottom() >= anchorRect.bottom()) {
        dlgRect.moveBottom(anchorRect.bottom() - DIALOG_WINDOW_FRAME_HEIGHT);
    }

    // if after moving the dialog does not fit on the screen, then adjust the size of the dialog
    if (!anchorRect.contains(dlgRect)) {
        anchorRect -= QMargins(DIALOG_WINDOW_FRAME_HEIGHT, DIALOG_WINDOW_FRAME_HEIGHT + titleBarHeight,
                               DIALOG_WINDOW_FRAME_HEIGHT, DIALOG_WINDOW_FRAME_HEIGHT);
        dlgRect = anchorRect.intersected(dlgRect);
    }

    m_globalPos = dlgRect.topLeft();

    setContentWidth(dlgRect.width());
    setContentHeight(dlgRect.height());

    //! NOTE ok will be if they call accept
    setErrCode(Ret::Code::Cancel);
}

QRect DialogView::viewGeometry() const
{
    return QRect(m_globalPos.toPoint(), QSize(contentWidth(), contentHeight()));
}

QSet<const QObject*> DialogView::findButtons() const
{
    QSet<const QObject*> buttons;
    for (const QObject* obj: findChildren<QObject*>()) {
        const QString className = obj->metaObject()->className();
        if (className.contains("FlatButton")) {
            buttons.insert(obj);
        }
    }
    return buttons;
}

void DialogView::exec()
{
    open();
    m_loop.exec();

    activateNavigationParentControl();
}

void DialogView::show()
{
    open();
}

void DialogView::hide()
{
    close();
}

void DialogView::raise()
{
    if (isOpened()) {
        m_window->raise();
    }
}

void DialogView::accept()
{
    setErrCode(Ret::Code::Ok);
    close();
}

void DialogView::reject(int code)
{
    if (code > 0) {
        setErrCode(static_cast<Ret::Code>(code));
    } else {
        setErrCode(Ret::Code::Cancel);
    }

    close();
}
