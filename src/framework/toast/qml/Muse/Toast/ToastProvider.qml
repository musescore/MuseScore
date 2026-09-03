/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

StyledListView {
    id: root

    property int animationDuration: 600

    width: 360
    implicitHeight: contentHeight
    height: implicitHeight

    spacing: 20

    clip: false

    x: parent.width - width - 20
    y: parent.height - height - 50

    model: toastmodel

    visible: root.count > 0

    property int navigationOrder: 0

    property NavigationSection navigationSection: NavigationSection {
        id: toastNavSec
        name: "ToastNotifications"
        enabled: root.enabled && root.visible
        order: root.navigationOrder
    }

    QtObject {
        id: prv

        function restoreFocus() {
            var newestItem = root.count > 0 ? root.itemAtIndex(root.count - 1) : null
            if (newestItem) {
                newestItem.navigation.requestActive()
            }
        }
    }

    ToastListModel {
        id: toastmodel
    }

    Component.onCompleted: {
        toastmodel.init()
    }

    MouseArea {
        // Disable interaction with elements below toast
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
    }

    delegate: ToastItem {
        id: itemRect

        width: root.width
        x: root.width
        opacity: 0

        title: model.title
        accessibleTitle: model.accessibleTitle
        iconCode: model.iconCode
        message: model.message
        actions: model.actions
        dismissable: model.dismissable

        progress: model.progress
        showProgressInfo: model.showProgressInfo
        timeElapsed: model.timeElapsed

        navigationSection: toastNavSec
        navigationOrder: root.count - 1 - model.index
        totalCount: root.count

        onShouldPauseTimerChanged: {
            if (shouldPauseTimer) {
                toastmodel.pauseToast(model.id)
            } else {
                toastmodel.resumeToast(model.id)
            }
        }

        onActionTriggered: function (actionStr) {
            toastmodel.executeAction(model.id, actionStr)
        }

        Component.onCompleted: {
            x = 0
            opacity = 1

            toastNavSec.requestPriority()
        }

        Component.onDestruction: {
            if (itemRect.navigationFocused) {
                Qt.callLater(prv.restoreFocus)
            }
        }

        Behavior on x {
            NumberAnimation {
                duration: root.animationDuration
                easing.type: Easing.OutCubic
            }
        }

        Behavior on opacity {
            OpacityAnimator {
                duration: root.animationDuration
            }
        }

        onDismissed: {
            toastmodel.dismissToast(model.id)
        }
    }

    add: null
    addDisplaced: null
}
