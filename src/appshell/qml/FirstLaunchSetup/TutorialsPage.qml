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
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.GraphicalEffects 1.0
import MuseScore.AppShell 1.0

Page {
    title: qsTrc("appshell/gettingstarted", "Video tutorials")
    explanation: qsTrc("appshell/gettingstarted", "Behind this dialog is the ‘Learn’ section, where you’ll find tutorials to get you started\n(Video tutorials require an internet connection)")

    titleContentSpacing: 24

    TutorialsPageModel {
        id: tutorialsModel
    }

    ColumnLayout {
        id: content

        anchors.fill: parent
        spacing: 20

        Item {
            id: imageArea

            Layout.fillWidth: true
            Layout.fillHeight: true

            Image {
                id: image

                anchors.centerIn: parent

                // Approx 40% of the image height is empty space
                height: parent.height * 1.4
                width: implicitWidth

                fillMode: Image.PreserveAspectFit
                source: "resources/VideoTutorials.png"

                layer.enabled: ui.isEffectsAllowed
                layer.effect: EffectOpacityMask {
                    maskSource: Rectangle {
                        width: image.width
                        height: image.height
                        radius: 3
                    }
                }
            }
        }

        StyledTextLabel {
            id: privacyInfo

            Layout.fillWidth: true

            text: qsTrc("appshell/gettingstarted", "In order to protect your privacy, MuseScore Studio does not collect any personal information. See our <a href=\"%1\">Privacy Policy</a> for more info.")
                  .arg(tutorialsModel.museScorePrivacyPolicyUrl())

            wrapMode: Text.WordWrap
        }
    }
}
