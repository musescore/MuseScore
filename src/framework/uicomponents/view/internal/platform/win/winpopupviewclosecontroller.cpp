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

#include "winpopupviewclosecontroller.h"

#include <Windows.h>

using namespace muse::uicomponents;

WinPopupViewCloseController::WinPopupViewCloseController(const muse::modularity::ContextPtr& iocCtx, QObject* parent)
    : PopupViewCloseController(iocCtx, parent)
{
}

void WinPopupViewCloseController::doUpdateEventFilters()
{
    if (active()) {
        qApp->installNativeEventFilter(this);
    } else {
        qApp->removeNativeEventFilter(this);
    }

    PopupViewCloseController::doUpdateEventFilters();
}

bool WinPopupViewCloseController::nativeEventFilter(const QByteArray& eventType, void* message, qintptr*)
{
    if (eventType != "windows_generic_MSG") {
        return false;
    }

    MSG* msg = static_cast<MSG*>(message);
    if (msg == nullptr) {
        return false;
    }

    if (msg->message == WM_NCLBUTTONDOWN || msg->message == WM_NCRBUTTONDOWN) {
        if (!popupHasFocus()) {
            doFocusOut(QCursor::pos());
        }
    }

    return false;
}
