/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "flickeringworkaround.h"

#include <QWindow>
#include <windows.h>

#include "log.h"

using namespace muse::vst;

// Original window procedure
WNDPROC g_pOriginalWndProc = nullptr;

/*!
 * Workaround for https://bugreports.qt.io/browse/QTBUG-132285
 * Inspired by comment from Grzegorz Plonka.
 * Pass the `hWnd` belongs to the `QWindow` hosting the QML view and VST window.
 * When being dragged, this will undo a change apparently introduced in Qt 6.5.8
 * and that is still present in 6.10.0: the wrong adding of the `SWP_NOCOPYBITS` flag
 * due to misinterpreted window size when the DPI scaling is not 100%.
 */
LRESULT CALLBACK RemoveNoCopybitsFlagWndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                             LPARAM lParam)
{
    const LRESULT ret = CallWindowProc(g_pOriginalWndProc, hWnd, uMsg, wParam, lParam);
    if (uMsg == WM_WINDOWPOSCHANGING) {
        WINDOWPOS* pWinPos = reinterpret_cast<WINDOWPOS*>(lParam);
        if (pWinPos && (pWinPos->flags & SWP_NOCOPYBITS)) {
            pWinPos->flags &= ~SWP_NOCOPYBITS;
        }
    }
    return ret;
}

void FlickeringWorkaround::preventFlickering(QWindow* w)
{
    WId winId = w->winId();
    const auto hwnd = reinterpret_cast<HWND>(winId);
    LOGDA() << "hwnd: " << hwnd;
    g_pOriginalWndProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtr(
            hwnd,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(RemoveNoCopybitsFlagWndProc)
            ));
}
