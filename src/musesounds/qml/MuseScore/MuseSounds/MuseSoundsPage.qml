/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.MuseSounds

import "internal"

FocusScope {
    id: root

    property alias color: background.color
    property string section: ""

    MuseSoundsListModel {
        id: museSoundsModel
    }

    Component.onCompleted: {
        museSoundsModel.load()
    }

    QtObject {
        id: prv

        readonly property int sideMargin: 46
    }

    NavigationSection {
        id: navSec
        name: "MuseSounds"
        enabled: root.enabled && root.visible
        order: 3
        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }
    }

    NavigationPanel {
        id: navTopPanel
        name: "MuseSoundsTopPanel"
        section: navSec
        order: 1
        accessible.name: qsTrc("appshell", "MuseSounds")
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: ui.theme.backgroundSecondaryColor
    }

    RowLayout {
        id: topLayout

        anchors.top: parent.top
        anchors.topMargin: prv.sideMargin
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: prv.sideMargin

        spacing: 12

        StyledTextLabel {
            id: pageTitle
            Layout.fillWidth: true

            text: qsTrc("appshell", "MuseSounds")
            font: ui.theme.titleBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        SearchField {
            id: searchField

            Layout.preferredWidth: 220

            navigation.name: "MuseSoundsSearch"
            navigation.panel: navTopPanel
            navigation.order: 1
            accessible.name: qsTrc("musesounds", "Search sounds")

            onSearchTextChanged: {
                museSoundsModel.searchText = searchField.searchText
            }
        }
    }

    GradientRectangle {
        id: topGradient

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: flickable.top

        startColor: root.color
        endColor: "transparent"
    }

    StyledFlickable {
        id: flickable

        anchors.top: topLayout.bottom
        anchors.topMargin: 24
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        contentWidth: width
        contentHeight: column.implicitHeight

        topMargin: topGradient.height
        bottomMargin: 24

        Column {
            id: column

            anchors.fill: parent
            anchors.leftMargin: prv.sideMargin
            anchors.rightMargin: prv.sideMargin

            spacing: 22

            Repeater {
                model: museSoundsModel

                delegate: SoundCatalogListView {
                    required property string catalogTitle
                    required property var catalogSoundsLibraries
                    required property int index

                    width: parent.width

                    title: catalogTitle

                    model: catalogSoundsLibraries

                    visible: count > 0

                    flickableItem: column

                    navigationPanel.section: navSec
                    navigationPanel.name: title + "Sounds"
                    navigationPanel.order: index

                    onNavigationActivated: function(itemRect) {
                        Utils.ensureContentVisible(flickable, itemRect, headerHeight + 16)
                    }
                }
            }
        }
    }

    Column {
        id: errorMessageColumn
        visible: museSoundsModel.isEmpty

        anchors.top: parent.top
        //! NOTE: should be synchronized with the error message from Learn page
        anchors.topMargin: 127 + topGradient.height + Math.max(parent.height / 3 - height / 2, 0)
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: 16

        StyledTextLabel {
            width: parent.width
            font: ui.theme.tabBoldFont
            text: qsTrc("musesounds", "Sorry, we are unable to load these sounds right now")
        }

        StyledTextLabel {
            width: parent.width
            text: qsTrc("global", "Please check your internet connection or try again later.")
        }
    }

    StyledTextLabel {
        id: noResultsLabel

        anchors.top: parent.top
        anchors.topMargin: 127 + topGradient.height + Math.max(parent.height / 3 - height / 2, 0)
        anchors.left: parent.left
        anchors.right: parent.right

        font: ui.theme.tabBoldFont
        text: qsTrc("global", "No results found")

        visible: museSoundsModel.noResultsFound && !errorMessageColumn.visible
    }

    GradientRectangle {
        id: bottomGradient
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        startColor: "transparent"
        endColor: root.color
    }
}
