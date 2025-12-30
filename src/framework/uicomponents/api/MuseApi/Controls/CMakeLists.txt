# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_qml_module(muse_uicomponents_qmlapi ALIAS muse::uicomponents_qmlapi FOR muse_uicomponents)

set(UICOMPONENTS_QML_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../qml/Muse/UiComponents)

# NOTE We provide these components for public use in extensions.
# Please add them here very carefully; once added, they cannot be easily removed.
# If we need to make breaking changes to our internal component,
# we need to consider adding the old version to this module for backward compatibility.

set(UICOMPONENTS_FILES
    ${UICOMPONENTS_QML_DIR}/Border.qml
    ${UICOMPONENTS_QML_DIR}/ButtonBox.qml
    ${UICOMPONENTS_QML_DIR}/CheckBox.qml
    ${UICOMPONENTS_QML_DIR}/ColorPicker.qml
    ${UICOMPONENTS_QML_DIR}/ContextMenuLoader.qml
    ${UICOMPONENTS_QML_DIR}/ExpandableBlank.qml
    ${UICOMPONENTS_QML_DIR}/ExpandableBlankSection.qml
    ${UICOMPONENTS_QML_DIR}/FilePicker.qml
    ${UICOMPONENTS_QML_DIR}/FlatButton.qml
    ${UICOMPONENTS_QML_DIR}/FlatButtonMenuIndicatorTriangle.qml
    ${UICOMPONENTS_QML_DIR}/FlatRadioButton.qml
    ${UICOMPONENTS_QML_DIR}/FlatRadioButtonList.qml
    ${UICOMPONENTS_QML_DIR}/FlatToggleButton.qml
    ${UICOMPONENTS_QML_DIR}/FocusableControl.qml
    ${UICOMPONENTS_QML_DIR}/FocusableItem.qml
    ${UICOMPONENTS_QML_DIR}/GradientRectangle.qml
    ${UICOMPONENTS_QML_DIR}/GridViewSectional.qml
    ${UICOMPONENTS_QML_DIR}/IncrementalPropertyControl.qml
    ${UICOMPONENTS_QML_DIR}/InfoPanel.qml
    ${UICOMPONENTS_QML_DIR}/ListItemBlank.qml
    ${UICOMPONENTS_QML_DIR}/MenuButton.qml
    ${UICOMPONENTS_QML_DIR}/NumberInputField.qml
    ${UICOMPONENTS_QML_DIR}/PageIndicator.qml
    ${UICOMPONENTS_QML_DIR}/PageTabButton.qml
    ${UICOMPONENTS_QML_DIR}/PopupPanel.qml
    ${UICOMPONENTS_QML_DIR}/ProgressBar.qml
    ${UICOMPONENTS_QML_DIR}/ProgressButton.qml
    ${UICOMPONENTS_QML_DIR}/RadioButtonGroup.qml
    ${UICOMPONENTS_QML_DIR}/RealInputField.qml
    ${UICOMPONENTS_QML_DIR}/RoundedRadioButton.qml
    ${UICOMPONENTS_QML_DIR}/RoundedRectangle.qml
    ${UICOMPONENTS_QML_DIR}/SearchField.qml
    ${UICOMPONENTS_QML_DIR}/SelectMultipleDirectoriesDialog.qml
    ${UICOMPONENTS_QML_DIR}/SeparatorLine.qml
    ${UICOMPONENTS_QML_DIR}/StyledBusyIndicator.qml
    ${UICOMPONENTS_QML_DIR}/StyledDialogView.qml
    ${UICOMPONENTS_QML_DIR}/StyledDropdown.qml
    ${UICOMPONENTS_QML_DIR}/StyledFlickable.qml
    ${UICOMPONENTS_QML_DIR}/StyledGridView.qml
    ${UICOMPONENTS_QML_DIR}/StyledGroupBox.qml
    ${UICOMPONENTS_QML_DIR}/StyledIconLabel.qml
    ${UICOMPONENTS_QML_DIR}/StyledListView.qml
    ${UICOMPONENTS_QML_DIR}/StyledMenuLoader.qml
    ${UICOMPONENTS_QML_DIR}/StyledPopupView.qml
    ${UICOMPONENTS_QML_DIR}/StyledScrollBar.qml
    ${UICOMPONENTS_QML_DIR}/StyledSlider.qml
    ${UICOMPONENTS_QML_DIR}/StyledTabBar.qml
    ${UICOMPONENTS_QML_DIR}/StyledTabButton.qml
    ${UICOMPONENTS_QML_DIR}/StyledTextLabel.qml
    ${UICOMPONENTS_QML_DIR}/StyledToolBarItem.qml
    ${UICOMPONENTS_QML_DIR}/StyledToolBarView.qml
    ${UICOMPONENTS_QML_DIR}/StyledToolTip.qml
    ${UICOMPONENTS_QML_DIR}/StyledTreeView.qml
    ${UICOMPONENTS_QML_DIR}/StyledViewScrollAndZoomArea.qml
    ${UICOMPONENTS_QML_DIR}/TextInputArea.qml
    ${UICOMPONENTS_QML_DIR}/TextInputField.qml
    ${UICOMPONENTS_QML_DIR}/TimeInputField.qml
    ${UICOMPONENTS_QML_DIR}/ToggleButton.qml
    ${UICOMPONENTS_QML_DIR}/ValueAdjustControl.qml
    ${UICOMPONENTS_QML_DIR}/ValueList.qml
    ${UICOMPONENTS_QML_DIR}/VisibilityBox.qml
    ${UICOMPONENTS_QML_DIR}/VisibilityControls.qml
)

foreach(qml_file ${UICOMPONENTS_FILES})
    get_filename_component(filename ${qml_file} NAME)
    set_source_files_properties(${qml_file}
        PROPERTIES
        QT_RESOURCE_ALIAS "MuseApi/Controls/${filename}"
    )
endforeach()

qt_add_qml_module(muse_uicomponents_qmlapi
    URI MuseApi.Controls
    VERSION 1.0
    QML_FILES
        ${UICOMPONENTS_FILES}

        ExtensionBlank.qml
    IMPORTS
        QtQuick
)

fixup_qml_module_dependencies(muse_uicomponents_qmlapi)
