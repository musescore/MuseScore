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
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Dock 1.0

import MuseScore.Project 1.0
import MuseScore.Cloud 1.0
import MuseScore.Learn 1.0

import "../dockwindow"

DockPage {
    id: root

    property string section: "scores"
    property string subSection: ""

    objectName: "Home"
    uri: "musescore://home"

    onSectionChanged: {
        Qt.callLater(root.setCurrentCentral, section)
    }

    function setCurrentCentral(name) {
        if (section === name || !Boolean(name)) {
            return
        }

        section = name

        switch (name) {
        case "scores": root.central = scoresComp; break
        case "add-ons": root.central = addonsComp; break
        case "audio": root.central = audioComp; break
        case "feautured": root.central = feauturedComp; break
        case "learn": root.central = learnComp; break
        case "support": root.central = supportComp; break
        case "account": root.central = accountComp; break
        }
    }

    panels: [
        DockPanel {
            objectName: "homeMenu"

            minimumWidth: 76
            maximumWidth: 292

            allowedAreas: Qt.NoDockWidgetArea

            HomeMenu {
                currentPageName: root.section

                onSelected: {
                    root.setCurrentCentral(name)
                }
            }
        }
    ]

    central: scoresComp

    Component {
        id: accountComp

        AccountPage {}
    }

    Component {
        id: scoresComp

        ScoresPage {}
    }

    Component {
        id: addonsComp

        AddonsContent {
            section: root.subSection
        }
    }

    Component {
        id: audioComp

        StyledTextLabel {
            anchors.centerIn: parent
            text: "Audio & VST"
        }
    }

    Component {
        id: feauturedComp

        StyledTextLabel {
            anchors.centerIn: parent
            text: "Feautured"
        }
    }

    Component {
        id: learnComp

        LearnPage {
            section: root.subSection
        }
    }

    Component {
        id: supportComp

        StyledTextLabel {
            anchors.centerIn: parent
            text: "Support"
        }
    }
}
