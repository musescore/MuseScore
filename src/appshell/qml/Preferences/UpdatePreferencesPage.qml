/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

PreferencesPage {
    id: root

    UpdatePreferencesModel {
        id: updateModel
    }

    Column {
        anchors.fill: parent

        spacing: 18

        StyledTextLabel {
            text: qsTrc("appshell", "Automatic Update Check")
            font: ui.theme.bodyBoldFont
        }

        CheckBox {
            text: qsTrc("appshell", "Check for new version of MuseScore")

            visible: updateModel.isAppUpdatable()
            checked: updateModel.needCheckForNewAppVersion

            onClicked: {
                updateModel.needCheckForNewAppVersion = !checked
            }
        }

        CheckBox {
            width: 200

            text: qsTrc("appshell", "Check for new version of MuseScore extensions")

            checked: updateModel.needCheckForNewExtensionsVersion

            onClicked: {
                updateModel.needCheckForNewExtensionsVersion = !checked
            }
        }
    }
}
