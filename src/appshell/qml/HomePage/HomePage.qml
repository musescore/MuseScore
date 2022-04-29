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

    onSetParamsRequested: function(params) {
        if (Boolean(params["section"])) {
            setCurrentCentral(params["section"])

            if (Boolean(params["subSection"])) {
                subSection = params["subSection"]
            }
        }
    }

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
        case "plugins": root.central = pluginsComp; break
        case "audio": root.central = audioComp; break
        case "learn": root.central = learnComp; break
        case "account": root.central = accountComp; break
        }
    }

    panels: [
        DockPanel {
            objectName: "homeMenu"

            width: maximumWidth
            minimumWidth: 76
            maximumWidth: 280

            floatable: false
            closable: false

            HomeMenu {
                currentPageName: root.section

                onSelected: function(name) {
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
        id: pluginsComp

        PluginsPage {
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
        id: learnComp

        LearnPage {
            section: root.subSection
        }
    }
}
