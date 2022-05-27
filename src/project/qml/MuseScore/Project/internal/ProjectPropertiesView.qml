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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

StyledListView {
    id: root

    Layout.fillHeight: true
    Layout.fillWidth: true

    spacing: 4

    property int propertyNameWidth: 110

    property NavigationPanel navigationPanel: null
    property int navigationColumnStart: 0

    scrollBarPolicy: ScrollBar.AlwaysOn

    model: projectPropertiesModel

    delegate: RowLayout{
        anchors.left: parent ? parent.left : undefined
        anchors.right: parent ? parent.right : undefined
        spacing: 0
        PropertyItem {
            width: root.width - 10

            index: root.navigationColumnStart + model.index
            propertyName: model.propertyName
            propertyValue: model.propertyValue
            isStandardProperty: model.isStandardProperty
            isFileInfoPanelProperty: false
            propertyNameWidth: root.propertyNameWidth

            navigationPanel: root.navigationPanel

            onPropertyNameChanged: function() {
                model.propertyName = propertyName
            }

            onPropertyValueChanged: function() {
                model.propertyValue = propertyValue
            }

            onChangePositionOfListIndex: function() {
                root.positionViewAtIndex(model.index, ListView.Contain)
            }

            onDeleteProperty: function() {
                projectPropertiesModel.deleteProperty(model.index)
            }
        }

        Item { Layout.preferredWidth: 16 }
    }
}
