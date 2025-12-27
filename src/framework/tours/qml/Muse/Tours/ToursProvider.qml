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

pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents
import Muse.Tours

Item {
    id: root

    anchors.fill: parent

    ToursProviderModel {
        id: providerModel

        onOpenTourStep: function(parent, title, description, previewImageOrGifUrl, videoExplanationUrl, index, total) {
            tourStepLoader.open(parent, title, description, previewImageOrGifUrl, videoExplanationUrl, index, total)
        }

        onCloseCurrentTourStep: {
            tourStepLoader.close()
        }
    }

    Loader {
        id: tourStepLoader

        anchors.fill: parent

        active: false

        sourceComponent: TourStepPopup {
            closePolicies: providerModel.canControlTourPopupClosing ? PopupView.NoAutoClose : PopupView.CloseOnPressOutsideParent

            onHideRequested: {
                Qt.callLater(tourStepLoader.unloadTourStep)
            }

            onNextRequested: {
                Qt.callLater(providerModel.showNext)
            }

            onClosed: {
                Qt.callLater(tourStepLoader.unloadTourStep)
            }
        }

        function loadTourStepPopup() {
            tourStepLoader.active = true
        }

        function unloadTourStep() {
            providerModel.onTourStepClosed(root.parent)

            tourStepLoader.active = false
        }

        function open(parent, title, description, previewImageOrGifUrl, videoExplanationUrl, index, total) {
            loadTourStepPopup()

            update(parent, title, description, previewImageOrGifUrl, videoExplanationUrl, index, total)

            var tourStepPopup = tourStepLoader.item as TourStepPopup
            tourStepPopup.open()
        }

        function close() {
            var tourStepPopup = tourStepLoader.item as TourStepPopup
            if (!Boolean(tourStepPopup)) {
                return
            }

            tourStepPopup.close()
        }

        function update(parent, title, description, previewImageOrGifUrl, videoExplanationUrl, index, total) {
            var tourStepPopup = tourStepLoader.item
            if (!Boolean(tourStepPopup)) {
                return
            }

            root.parent = parent
            tourStepPopup.title = title
            tourStepPopup.description = description
            tourStepPopup.previewImageOrGifUrl = previewImageOrGifUrl
            tourStepPopup.videoExplanationUrl = videoExplanationUrl
            tourStepPopup.index = index
            tourStepPopup.total = total
        }
    }
}
