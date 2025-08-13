/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

import Muse.Tours 1.0

import "internal"

Item {
    id: root

    anchors.fill: parent

    property var provider: providerModel.toursProvider

    ToursProviderModel {
        id: providerModel
    }

    Loader {
        id: tourStepLoader

        anchors.fill: parent

        active: false

        sourceComponent: TourStepPopup {
            closePolicies: root.provider.canControlTourPopupClosing ? PopupView.NoAutoClose : PopupView.CloseOnPressOutsideParent

            onHideRequested: {
                Qt.callLater(unloadTourStep)
            }

            onNextRequested: {
                Qt.callLater(root.provider.showNext)
            }

            onClosed: {
                Qt.callLater(unloadTourStep)
            }
        }

        function loadTourStepPopup() {
            tourStepLoader.active = true
        }

        function unloadTourStep() {
            root.provider.onTourStepClosed(root.parent)

            tourStepLoader.active = false
        }

        function open(parent, title, description, preferredPlacement, previewImageOrGifUrl, videoExplanationUrl, index, total) {
            loadTourStepPopup()

            update(parent, title, description, preferredPlacement, previewImageOrGifUrl, videoExplanationUrl, index, total)

            var tourStepPopup = tourStepLoader.item
            tourStepPopup.open()
        }

        function close() {
            var tourStepPopup = tourStepLoader.item
            if (!Boolean(tourStepPopup)) {
                return
            }

            tourStepPopup.close()
        }

        function resolvePlacementPolicies(preferredPlacement) {
            if (preferredPlacement === "Below") {
                return PopupView.PreferBelow
            } else if (preferredPlacement === "Above") {
                return PopupView.PreferAbove
            } else if (preferredPlacement === "Left") {
                return PopupView.PreferLeft
            } else if (preferredPlacement === "Right") {
                return PopupView.PreferRight
            }
            return PopupView.Default
        }

        function update(parent, title, description, preferredPlacement, previewImageOrGifUrl, videoExplanationUrl, index, total) {
            var tourStepPopup = tourStepLoader.item
            if (!Boolean(tourStepPopup)) {
                return
            }

            root.parent = parent
            tourStepPopup.title = title
            tourStepPopup.description = description
            tourStepPopup.placementPolicies = resolvePlacementPolicies(preferredPlacement)
            tourStepPopup.previewImageOrGifUrl = previewImageOrGifUrl
            tourStepPopup.videoExplanationUrl = videoExplanationUrl
            tourStepPopup.index = index
            tourStepPopup.total = total
        }
    }

    Connections {
        target: root.provider

        function onOpenTourStep(parent, title, description, preferredPlacement, previewImageOrGifUrl, videoExplanationUrl, index, total) {
            tourStepLoader.open(parent, title, description, preferredPlacement, previewImageOrGifUrl, videoExplanationUrl, index, total)
        }

        function onCloseCurrentTourStep() {
            tourStepLoader.close()
        }
    }
}
