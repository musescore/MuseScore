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
import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.AppShell 1.0
import MuseScore.Dock 1.0

import MuseScore.NotationScene 1.0
import MuseScore.Playback 1.0

import "../dockwindow"
import "../NotationPage"

DockPage {
    id: root

    objectName: "Publish"
    uri: "musescore://publish"

    property var topToolKeyNavSec

    property NavigationSection publishToolBarKeyNavSec: NavigationSection {
        id: keynavSec
        name: "PublushToolBarSection"
        order: 2
    }

    property NotationPageModel pageModel: NotationPageModel {}

    mainToolBars: [
        DockToolBar {
            id: notationToolBar

            objectName: root.objectName + "_notationToolBar"
            title: qsTrc("appshell", "Notation Toolbar")

            minimumWidth: 198

            contentComponent: NotationToolBar {
                navigation.section: root.topToolKeyNavSec
                navigation.order: 2

                onActiveFocusRequested: {
                    if (navigation.active) {
                        notationToolBar.forceActiveFocus()
                    }
                }
            }
        },

        DockToolBar {
            id: playbackToolBar

            objectName: root.objectName + "_playbackToolBar"
            title: qsTrc("appshell", "Playback Controls")

            width: root.width / 3
            minimumWidth: floating ? 526 : 476
            minimumHeight: floating ? 56 : 48

            contentComponent: PlaybackToolBar {
                navigation.section: root.topToolKeyNavSec
                navigation.order: 3

                floating: playbackToolBar.floating
            }
        },

        DockToolBar {
            id: undoRedoToolBar

            objectName: root.objectName + "_undoRedoToolBar"
            title: qsTrc("appshell", "Undo/Redo Toolbar")

            minimumWidth: 74
            maximumWidth: 74

            movable: false

            contentComponent: UndoRedoToolBar {
                navigation.section: root.topToolKeyNavSec
                navigation.order: 4
            }
        }
    ]

    toolBars: [
        DockToolBar {
            objectName: "publishToolBar"

            contentComponent: PublishToolBar {
                navigation.section: root.publishToolBarKeyNavSec
                navigation.order: 1
            }
        }
    ]

    central: NotationView {}

    statusBar: DockStatusBar {
        objectName: "publishStatusBar"

        NotationStatusBar {}
    }
}
