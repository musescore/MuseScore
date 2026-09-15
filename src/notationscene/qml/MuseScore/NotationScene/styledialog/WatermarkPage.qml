/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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
import QtQuick.Layouts
import QtQuick.Controls

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

StyledFlickable {
    id: root

    contentWidth: Math.max(column.implicitWidth, root.width)
    contentHeight: column.implicitHeight

    WatermarkPageModel {
        id: watermarkModel
    }

    ColumnLayout {
        id: column
        width: parent.width
        spacing: 12

        StyledGroupBox {
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/voltas", "Watermark")

            ColumnLayout {
                width: parent.width
                spacing: 12

                StyleToggle {
                    styleItem: watermarkModel.watermarkEnabled
                    text: qsTrc("notation/editstyle/voltas", "Show watermark on all pages")
                }
            }
        }

        StyledGroupBox {
            enabled: watermarkModel.watermarkEnabled.value === true
            Layout.fillWidth: true
            Layout.minimumWidth: 500
            title: qsTrc("notation/editstyle/voltas", "Watermark Settings")

            ColumnLayout {
                width: parent.width
                spacing: 12

                StyleControlRowWithReset {
                    styleItem: watermarkModel.watermarkType
                    label: qsTrc("notation", "Type:")
                    labelAreaWidth: 160
                    controlAreaWidth: 150

                    ComboBoxDropdown {
                        id: typeDropdown
                        width: 150
                        model: [
                            { text: qsTrc("notation", "Text"), value: 0 },
                            { text: qsTrc("notation", "Image"), value: 1 }
                        ]
                        styleItem: watermarkModel.watermarkType
                        onHandleItem: function(val) {
                            watermarkModel.watermarkType.value = val
                            if (val === 1 && watermarkModel.watermarkAngle.value === -45.0) {
                                watermarkModel.watermarkAngle.value = 0.0
                            } else if (val === 0 && watermarkModel.watermarkAngle.value === 0.0) {
                                watermarkModel.watermarkAngle.value = -45.0
                            }
                        }
                    }
                }

                TextFieldWithReset {
                    visible: watermarkModel.watermarkType.value === 0
                    styleItem: watermarkModel.watermarkText
                    label: qsTrc("notation", "Text:")
                    labelAreaWidth: 160
                    controlAreaWidth: 138
                }

                StyleControlRowWithReset {
                    visible: watermarkModel.watermarkType.value === 1
                    styleItem: watermarkModel.watermarkImagePath
                    label: qsTrc("notation", "Image:")
                    labelAreaWidth: 160
                    controlAreaWidth: 326

                    FilePicker {
                        id: imagePicker
                        width: 326
                        pickerType: FilePicker.PickerType.File
                        dialogTitle: qsTrc("notation", "Select Watermark Image")
                        filter: qsTrc("notation", "Image files (*.png *.jpg *.jpeg *.bmp)")
                        path: watermarkModel.watermarkImagePath.value.toString()
                        onPathEdited: function(newPath) {
                            watermarkModel.watermarkImagePath.value = newPath
                        }
                    }
                }

                StyleSpinboxWithReset {
                    visible: watermarkModel.watermarkType.value === 1
                    styleItem: watermarkModel.watermarkImageScale
                    label: qsTrc("notation", "Image scale:")
                    inPercentage: true
                    min: 10
                    max: 500
                    labelAreaWidth: 160
                    controlAreaWidth: 100
                    spinBoxWidth: 100
                }

                StyleSpinboxWithReset {
                    styleItem: watermarkModel.watermarkOpacity
                    label: qsTrc("notation", "Opacity:")
                    inPercentage: true
                    min: 0
                    max: 100
                    labelAreaWidth: 160
                    controlAreaWidth: 100
                    spinBoxWidth: 100
                }

                StyleSpinboxWithReset {
                    styleItem: watermarkModel.watermarkAngle
                    label: qsTrc("notation", "Rotation angle:")
                    min: -180
                    max: 180
                    step: 5
                    decimals: 1
                    suffix: '°'
                    labelAreaWidth: 160
                    controlAreaWidth: 100
                    spinBoxWidth: 100
                    resetOnClicked: function() {
                        watermarkModel.watermarkAngle.value = (watermarkModel.watermarkType.value === 1 ? 0.0 : -45.0)
                    }
                    resetEnabled: watermarkModel.watermarkType.value === 1 
                                  ? (watermarkModel.watermarkAngle.value !== 0.0) 
                                  : (watermarkModel.watermarkAngle.value !== -45.0)
                }
            }
        }
    }
}
