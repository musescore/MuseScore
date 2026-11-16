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

pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Project

import "internal/ConvertFileToScore"

Item {
    id: root

    property alias navigationSection: navigationPanel.section
    property int navigationOrderStart: 0

    signal closeRequested()

    implicitWidth: loader.item ? loader.item.implicitWidth : 0
    implicitHeight: loader.item ? loader.item.implicitHeight : 0

    StyledRectangularShadow {
        anchors.fill: background
        radius: background.radius
    }

    Rectangle {
        id: background

        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor
        border.color: ui.theme.strokeColor
        border.width: Math.max(ui.theme.borderWidth, 1)
        radius: 3
    }

    NavigationPanel {
        id: navigationPanel

        name: "ScoreReviewPanel"
        enabled: root.visible
        direction: NavigationPanel.Both
        order: root.navigationOrderStart

        accessible.name: loader.item ? loader.item.accessibleName : ""
        accessible.description: loader.item ? loader.item.accessibleDescription : ""
    }

    ReviewScoreQualityModel {
        id: reviewModel

        onCloseRequested: root.closeRequested()
    }

    Loader {
        id: loader

        anchors.fill: parent

        sourceComponent: reviewModel.step === ReviewScoreQualityModel.Rating ? ratingPanelComp : feedbackPanelComp
    }

    Component {
        id: ratingPanelComp

        ScoreQualityRatingPanel {
            navigationPanel: navigationPanel

            onGoodRequested: reviewModel.submitGood()
            onBadRequested: reviewModel.submitBad()
        }
    }

    Component {
        id: feedbackPanelComp

        ScoreQualityFeedbackPanel {
            navigationPanel: navigationPanel

            onSubmitRequested: function(comment) { reviewModel.submitFeedback(comment) }
            onCloseRequested: reviewModel.skipFeedback()
        }
    }
}
