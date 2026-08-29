/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

import QtQuick
import QtQuick.Layouts
import QtQuick.Window

import Muse.Ui
import Muse.UiComponents
import MuseScore.Playback

Item {
    id: root

    property NavigationSection navigationSection: null
    property int contentNavigationPanelOrderStart: 0

    // NOTE: whether the enclosing DockPanel is currently floating (undocked)
    // -- fed in from NotationPage.qml (`floating: videoPanel.floating` on this
    // component's own instantiation), since this file can't reference that
    // panel's id directly (separate document). Full screen only makes sense
    // once this panel is its own floating window -- when docked, there's no
    // separate window to fullscreen, so "Full screen" would end up
    // fullscreening the whole MuseScore window instead, which isn't what a
    // user asking to fullscreen just the video expects (e.g. floating this
    // panel onto a second monitor and fullscreening it there).
    property bool floating: false

    // NOTE: docking while "full screen" (simulated, see below) leaves no
    // sensible state to return to on a later undock -- start fresh instead of
    // carrying a stale saved geometry/flag across a dock/undock cycle.
    onFloatingChanged: {
        if (!floating) {
            isFullScreen = false
            savedGeometry = null
        }
    }

    // NOTE: tracked ourselves rather than reading Window.window.visibility ===
    // Window.FullScreen -- see the NOTE on savedGeometry below for why native
    // showFullScreen()/showNormal() aren't used here at all.
    property bool isFullScreen: false

    // NOTE: this model -- unlike VideoPanel.qml itself -- is NOT behind the
    // asynchronous Loader below, so it's already loaded by the time the
    // enclosing DockPanel's own content actually needs it (pushed up via
    // Component.onCompleted on this component's own instantiation in
    // NotationPage.qml, the same pattern Mixer/Piano keyboard already use --
    // a plain property binding at the DockPanel level doesn't work here since
    // this whole file is the DockPanel's lazily-loaded default-property
    // content, only instantiated once the panel is actually shown).
    property alias contextMenuModel: contextMenuModel

    readonly property bool shouldLoadPanel: width > 0 && height > 0

    // NOTE: this does NOT use QWindow.showFullScreen()/showNormal() at all --
    // tried that first, but it turned out unreliable specifically for this
    // window type (Qt::Tool, frameless -- see KDDockWidgets' FloatingWindow
    // setup): showNormal() correctly flipped `visibility` back to Windowed
    // but never restored x/y/width/height (confirmed via logging: stayed at
    // the full-screen geometry), and manually restoring geometry afterwards
    // -- even with a delay -- left the window unable to re-enter full screen
    // on a later toggle (some internal native full-screen state apparently
    // stays stuck; docking then re-undocking the panel, which recreates the
    // underlying window, was the only thing that reset it). Doing this as a
    // plain geometry resize instead -- filling the window's current screen,
    // no native full-screen transition involved at all -- sidesteps that
    // stuck state entirely, at the cost of not hiding the OS menu bar/dock on
    // the screen it's on the way real OS full screen would.
    property var savedGeometry: null

    VideoPanelContextMenuModel {
        id: contextMenuModel

        floating: root.floating
        isFullScreen: root.isFullScreen

        Component.onCompleted: contextMenuModel.load()

        onToggleFullScreenRequested: {
            var win = root.Window.window
            if (!win) {
                return
            }

            if (root.isFullScreen) {
                if (root.savedGeometry) {
                    win.x = root.savedGeometry.x
                    win.y = root.savedGeometry.y
                    win.width = root.savedGeometry.width
                    win.height = root.savedGeometry.height
                    root.savedGeometry = null
                }
                root.isFullScreen = false
            } else {
                root.savedGeometry = { x: win.x, y: win.y, width: win.width, height: win.height }

                // NOTE: fills the AVAILABLE area of whichever screen the
                // window is currently on (QScreen::availableGeometry(), via
                // contextMenuModel.screenAvailableGeometry() -- win itself is
                // a KDDockWidgets::QuickView wrapper around the real
                // QQuickWindow and doesn't forward a usable `.screen`, so C++
                // resolves the right QScreen from the window's own position
                // instead), not the screen's raw full geometry -- the
                // available area is exactly what's left over once the OS
                // reserves its own space (the menu bar on macOS, the taskbar
                // on Windows, panels on Linux), so this fills the screen
                // without ever landing underneath any of that, on any
                // platform.
                var availableGeometry = contextMenuModel.screenAvailableGeometry(win.x, win.y)
                win.x = availableGeometry.x
                win.y = availableGeometry.y
                win.width = availableGeometry.width
                win.height = availableGeometry.height
                root.isFullScreen = true
            }
        }
    }

    function updateLoadedItem() {
        if (!videoPanelLoader.item) {
            return
        }

        videoPanelLoader.item.navigationSection = root.navigationSection
        videoPanelLoader.item.contentNavigationPanelOrderStart = root.contentNavigationPanelOrderStart
    }

    onNavigationSectionChanged: updateLoadedItem()
    onContentNavigationPanelOrderStartChanged: updateLoadedItem()

    Loader {
        id: videoPanelLoader

        anchors.fill: parent
        active: root.shouldLoadPanel
        asynchronous: true
        source: root.shouldLoadPanel ? "VideoPanel.qml" : ""

        onLoaded: {
            root.updateLoadedItem()
        }
    }

    Rectangle {
        anchors.fill: parent

        visible: root.shouldLoadPanel && videoPanelLoader.status === Loader.Error
        color: ui.theme.backgroundPrimaryColor

        ColumnLayout {
            anchors.centerIn: parent
            width: Math.max(0, Math.min(parent.width - 48, 560))
            spacing: 12

            StyledTextLabel {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font: ui.theme.headerBoldFont
                text: qsTrc("playback", "Video playback is unavailable")
            }

            StyledTextLabel {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                opacity: 0.75
                text: qsTrc("playback", "MuseScore could not load the Qt Multimedia module required by the Video panel. This build may be missing QtMultimedia in its package.")
            }
        }
    }
}
