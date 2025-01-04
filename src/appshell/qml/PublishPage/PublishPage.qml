/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.AppShell 1.0
import Muse.Dock 1.0

import MuseScore.NotationScene 1.0
import MuseScore.Playback 1.0

import "../NotationPage"

DockPage {
    id: root

    objectName: "Publish"
    uri: "musescore://publish"

    required property NavigationSection topToolbarKeyNavSec

    property NavigationSection publishToolBarKeyNavSec: NavigationSection {
        id: keynavSec
        name: "PublishToolBarSection"
        order: 2
    }

    property NotationPageModel pageModel: NotationPageModel {}

    mainToolBars: [
        DockToolBar {
            id: notationToolBar

            objectName: root.objectName + "_notationToolBar"
            title: qsTrc("appshell", "Notation toolbar")

            floatable: false
            closable: false
            resizable: false
            separatorsVisible: false

            alignment: DockToolBarAlignment.Center
            contentBottomPadding: 2

            navigationSection: root.topToolbarKeyNavSec

            NotationToolBar {
                navigationPanel.section: notationToolBar.navigationSection
                navigationPanel.order: 2
            }
        },

        DockToolBar {
            id: playbackToolBar

            objectName: root.objectName + "_playbackToolBar"
            title: qsTrc("appshell", "Playback controls")

            separatorsVisible: false
            alignment: DockToolBarAlignment.Right
            contentBottomPadding: 2

            navigationSection: root.topToolbarKeyNavSec

            PlaybackToolBar {
                navigationPanelSection: playbackToolBar.navigationSection
                navigationPanelOrder: 3

                floating: playbackToolBar.floating
            }
        },

        DockToolBar {
            id: undoRedoToolBar

            objectName: root.objectName + "_undoRedoToolBar"
            title: qsTrc("appshell", "Undo/redo toolbar")

            floatable: false
            closable: false
            resizable: false
            separatorsVisible: false

            alignment: DockToolBarAlignment.Right
            contentBottomPadding: 2

            navigationSection: root.topToolbarKeyNavSec

            UndoRedoToolBar {
                navigationPanel.section: undoRedoToolBar.navigationSection
                navigationPanel.order: 4
            }
        }
    ]

    toolBars: [
        DockToolBar {
            id: publishToolBar

            objectName: "publishToolBar"
            title: qsTrc("appshell", "Publish toolbar")

            floatable: false

            navigationSection: root.publishToolBarKeyNavSec

            PublishToolBar {
                navigationPanel.section: publishToolBar.navigationSection
                navigationPanel.order: 1
            }
        }
    ]

    central: NotationView {
        name: "PublishNotationView"
        publishMode: true
    }

    statusBar: DockStatusBar {
        objectName: "publishStatusBar"

        navigationSection: content.navigationSection

        NotationStatusBar {
            id: content
        }
    }
}
