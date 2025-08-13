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
#include "uicomponentsmodule.h"

#include <QQmlEngine>
#include "view/sampleobject.h"
#include "view/validators/doubleinputvalidator.h"
#include "view/validators/intinputvalidator.h"
#include "view/iconview.h"
#include "view/sortfilterproxymodel.h"
#include "view/popupview.h"
#include "view/dialogview.h"
#include "view/dropdownview.h"
#include "view/menuview.h"
#include "view/filepickermodel.h"
#include "view/colorpickermodel.h"
#include "view/itemmultiselectionmodel.h"
#include "view/textinputmodel.h"
#include "view/selectmultipledirectoriesmodel.h"
#include "view/buttonboxmodel.h"
#include "view/abstracttoolbarmodel.h"

#include "view/treeview/qquicktreemodeladaptor_p.h"
#include "view/treeview/qquickrangemodel_p.h"
#include "view/treeview/qquickwheelarea_p.h"
#include "view/treeview/qquickabstractstyle_p.h"
#include "view/treeview/qquickselectionmode_p.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "ui/uitypes.h"
#include "ui/iinteractiveuriregister.h"

using namespace muse::uicomponents;
using namespace muse::ui;
using namespace muse::modularity;

static void uicomponents_init_qrc()
{
    Q_INIT_RESOURCE(uicomponents);
    Q_INIT_RESOURCE(graphicaleffects);
}

std::string UiComponentsModule::moduleName() const
{
    return "uicomponents";
}

void UiComponentsModule::registerExports()
{
}

void UiComponentsModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("muse://interactive/selectmultipledirectories"),
                        ContainerMeta(ContainerType::QmlDialog, "Muse/UiComponents/SelectMultipleDirectoriesDialog.qml"));
    }
}

void UiComponentsModule::registerResources()
{
    uicomponents_init_qrc();
}

void UiComponentsModule::registerUiTypes()
{
    qmlRegisterType<SampleObject>("Muse.UiComponents", 1, 0, "SampleObject");

    qmlRegisterUncreatableType<QAbstractItemModel>("Muse.UiComponents", 1, 0, "AbstractItemModel", "Cannot …");

    qmlRegisterType<DoubleInputValidator>("Muse.UiComponents", 1, 0, "DoubleInputValidator");
    qmlRegisterType<IntInputValidator>("Muse.UiComponents", 1, 0, "IntInputValidator");
    qmlRegisterType<IntInputValidator>("Muse.UiComponents", 1, 0, "IntInputValidator");

    qmlRegisterType<IconView>("Muse.UiComponents", 1, 0, "IconView");

    qmlRegisterType<SortFilterProxyModel>("Muse.UiComponents", 1, 0, "SortFilterProxyModel");
    qmlRegisterType<FilterValue>("Muse.UiComponents", 1, 0, "FilterValue");
    qmlRegisterType<SorterValue>("Muse.UiComponents", 1, 0, "SorterValue");
    qmlRegisterUncreatableType<CompareType>("Muse.UiComponents", 1, 0, "CompareType", "Cannot create a CompareType");

    qmlRegisterType<PopupView>("Muse.UiComponents", 1, 0, "PopupView");
    qmlRegisterType<DialogView>("Muse.UiComponents", 1, 0, "DialogView");
    qmlRegisterType<DropdownView>("Muse.UiComponents", 1, 0, "DropdownView");
    qmlRegisterType<MenuView>("Muse.UiComponents", 1, 0, "MenuView");

    qmlRegisterType<FilePickerModel>("Muse.UiComponents", 1, 0, "FilePickerModel");
    qmlRegisterType<ColorPickerModel>("Muse.UiComponents", 1, 0, "ColorPickerModel");
    qmlRegisterType<ItemMultiSelectionModel>("Muse.UiComponents", 1, 0, "ItemMultiSelectionModel");

    qmlRegisterType<TextInputModel>("Muse.UiComponents", 1, 0, "TextInputModel");
    qmlRegisterType<SelectMultipleDirectoriesModel>("Muse.UiComponents", 1, 0, "SelectMultipleDirectoriesModel");
    qmlRegisterType<ButtonBoxModel>("Muse.UiComponents", 1, 0, "ButtonBoxModel");

    qmlRegisterType<QQuickTreeModelAdaptor1>("Muse.UiComponents.Private", 1, 0, "TreeModelAdaptor");
    qmlRegisterType<QQuickRangeModel1>("Muse.UiComponents.Private", 1, 0, "RangeModel");
    qmlRegisterType<QQuickWheelArea1>("Muse.UiComponents.Private", 1, 0, "WheelArea");
    qmlRegisterType<QQuickAbstractStyle1>("Muse.UiComponents.Private", 1, 0, "AbstractStyle");
    qmlRegisterType<QQuickPadding1>("Muse.UiComponents.Private", 1, 0, "Padding");
    qmlRegisterUncreatableType<QQuickSelectionMode1>("Muse.UiComponents.Private", 1, 0, "SelectionMode",
                                                     QLatin1String("Do not create objects of type SelectionMode"));

    qmlRegisterUncreatableType<ToolBarItemType>("Muse.UiComponents", 1, 0, "ToolBarItemType", "Cannot create a ToolBarItemType");
    qmlRegisterUncreatableType<PopupPosition>("Muse.UiComponents", 1, 0, "PopupPosition",
                                              QLatin1String("Do not create objects of type PopupPosition"));

    auto ui = ioc()->resolve<ui::IUiEngine>(moduleName());
    if (ui) {
        ui->addSourceImportPath(muse_uicomponents_QML_IMPORT);
    }
}
