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

muse_create_qml_module(muse_uicomponents_qml ALIAS muse::uicomponents_qml FOR muse_uicomponents)

qt_add_qml_module(muse_uicomponents_qml
    URI Muse.UiComponents
    VERSION 1.0
    SOURCES
        abstractmenumodel.cpp
        abstractmenumodel.h
        abstracttoolbarmodel.cpp
        abstracttoolbarmodel.h
        buttonboxmodel.cpp
        buttonboxmodel.h
        colorpickermodel.cpp
        colorpickermodel.h
        dialogview.cpp
        dialogview.h
        dropdownview.cpp
        dropdownview.h
        filepickermodel.cpp
        filepickermodel.h
        filtervalue.cpp
        filtervalue.h
        iconview.cpp
        iconview.h
        internal/popupviewclosecontroller.cpp
        internal/popupviewclosecontroller.h
        itemmultiselectionmodel.cpp
        itemmultiselectionmodel.h
        menuitem.cpp
        menuitem.h
        menuview.cpp
        menuview.h
        popupview.cpp
        popupview.h
        qmllistproperty.cpp
        qmllistproperty.h
        selectableitemlistmodel.cpp
        selectableitemlistmodel.h
        selectmultipledirectoriesmodel.cpp
        selectmultipledirectoriesmodel.h
        sortervalue.cpp
        sortervalue.h
        sortfilterproxymodel.cpp
        sortfilterproxymodel.h
        textinputmodel.cpp
        textinputmodel.h
        toolbaritem.cpp
        toolbaritem.h
        validators/doubleinputvalidator.cpp
        validators/doubleinputvalidator.h
        validators/intinputvalidator.cpp
        validators/intinputvalidator.h
        windowview.cpp
        windowview.h


    QML_FILES
        Border.qml
        ButtonBox.qml
        CheckBox.qml
        ColorPicker.qml
        ContextMenuLoader.qml
        ExpandableBlank.qml
        ExpandableBlankSection.qml
        FilePicker.qml
        FlatButton.qml
        FlatButtonMenuIndicatorTriangle.qml
        FlatRadioButton.qml
        FlatRadioButtonList.qml
        FlatToggleButton.qml
        FocusableControl.qml
        FocusableItem.qml
        GradientRectangle.qml
        GridViewSectional.qml
        IncrementalPropertyControl.qml
        InfoPanel.qml
        ListItemBlank.qml
        MenuButton.qml
        NavigationFocusBorder.qml
        NumberInputField.qml
        PageIndicator.qml
        PageTabButton.qml
        PopupPanel.qml
        ProgressBar.qml
        ProgressButton.qml
        RadioButtonGroup.qml
        RealInputField.qml
        RoundedRadioButton.qml
        RoundedRectangle.qml
        SearchField.qml
        SelectMultipleDirectoriesDialog.qml
        SeparatorLine.qml
        StyledBusyIndicator.qml
        StyledDialogView.qml
        StyledDropdown.qml
        StyledFlickable.qml
        StyledGridView.qml
        StyledGroupBox.qml
        StyledIconLabel.qml
        StyledListView.qml
        StyledMenuLoader.qml
        StyledPopupView.qml
        StyledScrollBar.qml
        StyledSlider.qml
        StyledTabBar.qml
        StyledTabButton.qml
        StyledTextLabel.qml
        StyledToolBarItem.qml
        StyledToolBarView.qml
        StyledToolTip.qml
        StyledTreeView.qml
        StyledViewScrollAndZoomArea.qml
        TextInputArea.qml
        TextInputField.qml
        TimeInputField.qml
        ToggleButton.qml
        Utils.mjs
        ValueAdjustControl.qml
        ValueList.qml
        VisibilityBox.qml
        VisibilityControls.qml
        
        internal/ArrowScrollButton.qml
        internal/DirectoriesTopPanel.qml
        internal/DirectoriesView.qml
        internal/GridViewDelegate.qml
        internal/GridViewSection.qml
        internal/MenuMetrics.qml
        internal/PopupContent.qml
        internal/StyledDropdownLoader.qml
        internal/StyledDropdownView.qml
        internal/StyledMenu.qml
        internal/StyledMenuItem.qml
        internal/ValueListHeaderItem.qml
        internal/ValueListItem.qml
        internal/ViewScrollBar.qml

        treeview/BasicTableView.qml
        treeview/BasicTableViewStyle.qml
        treeview/LegacyTreeView.qml
        treeview/LegacyTreeViewItemDelegateLoader.qml
        treeview/LegacyTreeViewStyle.qml
        treeview/Style.qml
        treeview/TableViewColumn.qml
        treeview/TableViewItemDelegateLoader.qml
    
    RESOURCES
        treeview/images/header.png

    IMPORTS
        QtQuick
)

if (OS_IS_MAC)
    target_sources(muse_uicomponents_qml PRIVATE
        internal/platform/macos/macospopupviewclosecontroller.mm
        internal/platform/macos/macospopupviewclosecontroller.h
        )

    set_source_files_properties(
        internal/platform/macos/macospopupviewclosecontroller.mm
        PROPERTIES
        SKIP_UNITY_BUILD_INCLUSION ON
        SKIP_PRECOMPILE_HEADERS ON
        )
endif(OS_IS_MAC)

if (OS_IS_WIN)
    target_sources(muse_uicomponents_qml PRIVATE
        internal/platform/win/winpopupviewclosecontroller.cpp
        internal/platform/win/winpopupviewclosecontroller.h
    )
endif(OS_IS_WIN)

if (${Qt6_VERSION} VERSION_GREATER_EQUAL 6.9)
    qt_target_qml_sources(muse_uicomponents_qml
        QML_FILES
            StyledRectangularShadow.qml
    )
else()
    qt_target_qml_sources(muse_uicomponents_qml
        QML_FILES
            StubStyledRectangularShadow.qml
    )
    set_source_files_properties(StubStyledRectangularShadow.qml PROPERTIES
        QT_QML_SOURCE_TYPENAME StyledRectangularShadow.qml
    )
endif()

if (MUSE_MODULE_UI_DISABLE_MODALITY)
    target_compile_definitions(muse_uicomponents_qml PRIVATE MUSE_MODULE_UI_DISABLE_MODALITY)
endif()

# Necessary for the auto-generated sources
target_include_directories(muse_uicomponents_qml PRIVATE validators)

if (MUSE_MODULE_UI_TESTS)
    add_subdirectory(tests)
endif()
