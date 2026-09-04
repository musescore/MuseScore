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
    property alias autoDownloadNewAppVersion: autoDownloadCheckBox.checked
    property string museScorePrivacyPolicyUrl

    signal needCheckForNewAppVersionChangeRequested(bool check)
    signal autoDownloadNewAppVersionChangeRequested(bool download)

    ToggleButton {
        id: needCheckEnable
        width: parent.width

        text: qsTrc("preferences", "Check automatically for updates to MuseScore Studio is available")

        visible: root.isAppUpdatable

        navigation.name: "NeedCheckEnableButton"
        navigation.panel: root.navigation
        navigation.row: 0

        onToggled: {
            root.needCheckForNewAppVersionChangeRequested(!checked)
        }
    }

    CheckBox {
        id: autoDownloadCheckBox
        width: parent.width

        text: qsTrc("preferences", "Download updates in the background")

        visible: root.isAppUpdatable
        enabled: needCheckEnable.checked

        navigation.name: "AutoDownloadCheckBox"
        navigation.panel: root.navigation
        navigation.row: 1

        onClicked: {
            root.autoDownloadNewAppVersionChangeRequested(!checked)
        }
    }

    StyledTextLabel {
        width: parent.width

        text: qsTrc("preferences", "Checking for updates requires network access. In order to protect your privacy, MuseScore Studio does not store any personal information. See our <a href=\"%1\">privacy policy</a> for more info.")
              .arg(root.museScorePrivacyPolicyUrl)
              .replace("\n", "<br>")

        horizontalAlignment: Qt.AlignLeft
        wrapMode: Text.WordWrap
        maximumLineCount: 3
    }
}
