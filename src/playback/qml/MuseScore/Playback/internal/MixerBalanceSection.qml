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
import MuseScore.Audio 1.0

MixerPanelSection {
    id: root

    headerTitle: qsTrc("playback", "Pan")

    Item {
        id: contentWrapper

        height: contentRow.implicitHeight
        width: root.delegateDefaultWidth

        Row {
            id: contentRow

            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 8

            KnobControl {
                id: balanceKnob

                value: item.balance

                onMoved: {
                    item.balance = value
                }
            }

            TextInputField {
                id: balanceTextInputField

                anchors.verticalCenter: balanceKnob.verticalCenter

                textHorizontalAlignment: Qt.AlignHCenter

                height: 24
                width: 36

                validator: IntInputValidator {
                    id: intInputValidator
                    top: 100
                    bottom: -100
                }

                currentText: item.balance

                onCurrentTextEdited: {
                    if (item.balance !== Number(newTextValue)) {
                        item.balance = Number(newTextValue)
                    }
                }
            }
        }
    }
}
