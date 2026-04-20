/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "macoswindowlevelcontroller.h"

#include <Cocoa/Cocoa.h>
#include <QWindow>

using namespace muse::uicomponents;

static NSWindow* nsWindowForQWindow(QWindow* window)
{
    if (!window) {
        return nil;
    }

    NSView* nsView = (__bridge NSView*)reinterpret_cast<void*>(window->winId());
    return [nsView window];
}

void MacOSWindowLevelController::setAlwaysAboveApp(QWindow* window)
{
    NSWindow* nsWindow = nsWindowForQWindow(window);
    if (!nsWindow) {
        return;
    }
    [nsWindow setLevel:NSFloatingWindowLevel];
    [nsWindow setHidesOnDeactivate:YES];
}
