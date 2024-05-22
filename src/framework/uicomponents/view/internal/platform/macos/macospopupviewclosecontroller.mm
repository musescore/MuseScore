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

#include "macospopupviewclosecontroller.h"

#include <AppKit/NSEvent.h>
#include <QQuickWindow>

using namespace muse::uicomponents;

id<NSObject> minimizeObserverToken = nil;

MacOSPopupViewCloseController::MacOSPopupViewCloseController(const modularity::ContextPtr& iocCtx, QObject* parent)
    : PopupViewCloseController(iocCtx, parent)
{
}

void MacOSPopupViewCloseController::doUpdateEventFilters()
{
    if (active()) {
        qApp->installNativeEventFilter(this);
        if (!minimizeObserverToken) {
            initWindowMinimizedObserver();
        }
    } else {
        qApp->removeNativeEventFilter(this);
        if (minimizeObserverToken) {
            [[NSNotificationCenter defaultCenter] removeObserver:minimizeObserverToken];
            minimizeObserverToken = nil;
        }
    }

    PopupViewCloseController::doUpdateEventFilters();
}

bool MacOSPopupViewCloseController::nativeEventFilter(const QByteArray& eventType, void* message, qintptr*)
{
    if (eventType != "mac_generic_NSEvent") {
        return false;
    }

    NSEvent* event = static_cast<NSEvent*>(message);
    if ([event type] == NSEventTypeRightMouseDown || [event type] == NSEventTypeLeftMouseDown) {
        if (!popupHasFocus()) {
            doFocusOut();
        }
    }

    return false;
}

void MacOSPopupViewCloseController::initWindowMinimizedObserver()
{
    WId wid = parentItem()->window()->winId();
    NSView* nsView = (__bridge NSView*)reinterpret_cast<void*>(wid);
    NSWindow* nsWindow = [nsView window];

    minimizeObserverToken = [[NSNotificationCenter defaultCenter]
                             addObserverForName:@"NSWindowWillMiniaturizeNotification"
                             object:nsWindow
                             queue:nil
                             usingBlock:^(NSNotification*) {
                                 notifyAboutClose();
                             }];
}
