/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

import Muse.Ui
import Muse.UiComponents

BaseSection {
    id: root

    title: qsTrc("preferences", "Automatic updates")

    property bool isAppUpdatable: true
    property alias needCheckForNewAppVersion: needCheckEnable.checked
    property alias autoUpdateNewAppVersion: autoUpdateToggle.checked
    property string museScorePrivacyPolicyUrl

    signal needCheckForNewAppVersionChangeRequested(bool check)
    signal autoUpdateNewAppVersionChangeRequested(bool enabled)

    ToggleButton {
        id: needCheckEnable
        width: parent.width

        text: qsTrc("preferences", "Check automatically for updates to MuseScore Studio")

        visible: root.isAppUpdatable

        navigation.name: "NeedCheckEnableButton"
        navigation.panel: root.navigation
        navigation.row: 0

        onToggled: {
            root.needCheckForNewAppVersionChangeRequested(!checked)
        }
    }

    ToggleButton {
        id: autoUpdateToggle
        width: parent.width

        text: qsTrc("preferences", "Download and install updates automatically")

        visible: root.isAppUpdatable
        //! NOTE: Turned off together with "Check automatically..." and turned on with it again in model
        enabled: needCheckEnable.checked

        navigation.name: "AutoUpdateToggle"
        navigation.panel: root.navigation
        navigation.row: 1

        onToggled: {
            root.autoUpdateNewAppVersionChangeRequested(!checked)
        }
    }

    StyledTextLabel {
        width: parent.width

        text: qsTrc("preferences", "Checking for updates requires network access. In order to protect your privacy, MuseScore Studio does not store any personal information. See our <a href=\"%1\">privacy policy</a> for more info.").arg(root.museScorePrivacyPolicyUrl).replace("\n", "<br>")

        horizontalAlignment: Qt.AlignLeft
        wrapMode: Text.WordWrap
        maximumLineCount: 3
    }
}
