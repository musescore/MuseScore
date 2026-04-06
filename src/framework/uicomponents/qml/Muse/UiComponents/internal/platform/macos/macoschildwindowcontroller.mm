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

#include "macoschildwindowcontroller.h"

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

void MacOSChildWindowController::attachWindow(QWindow* childWindow, QWindow* parentWindow)
{
    NSWindow* child = nsWindowForQWindow(childWindow);
    NSWindow* parent = nsWindowForQWindow(parentWindow);
    if (!child || !parent || child == parent) {
        return;
    }

    NSWindow* currentParent = [child parentWindow];
    if (currentParent && currentParent != parent) {
        [currentParent removeChildWindow:child];
    }

    [child orderFront:nil];

    if ([child parentWindow] != parent) {
        [parent addChildWindow:child ordered:NSWindowAbove];
    }
}

void MacOSChildWindowController::detachWindow(QWindow* childWindow)
{
    NSWindow* child = nsWindowForQWindow(childWindow);
    if (!child) {
        return;
    }

    NSWindow* parent = [child parentWindow];
    if (parent) {
        [parent removeChildWindow:child];
    }
}
