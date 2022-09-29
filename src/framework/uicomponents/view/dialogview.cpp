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

#include <QScreen>
#include <QWindow>

#include "log.h"

using namespace mu::uicomponents;

DialogView::DialogView(QQuickItem* parent)
    : PopupView(parent)
{
    setObjectName("DialogView");
    setClosePolicy(NoAutoClose);
}

bool DialogView::isDialog() const
{
    return true;
}

void DialogView::beforeShow()
{
    QWindow* qMainWindow = mainWindow()->qWindow();
    IF_ASSERT_FAILED(qMainWindow) {
        return;
    }

    QRect referenceRect = qMainWindow->geometry();
    if (referenceRect.isEmpty() && qMainWindow->screen()) {
        referenceRect = qMainWindow->screen()->availableGeometry();
    }

    const QRect& dlgRect = geometry();

    m_globalPos = referenceRect.center() - QPointF(dlgRect.width() / 2, dlgRect.height() / 2) + m_localPos;

    //! NOTE ok will be if they call accept
    setErrCode(Ret::Code::Cancel);
}

void DialogView::onHidden()
{
    PopupView::onHidden();

    if (m_loop.isRunning()) {
        m_loop.exit();
    }
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
