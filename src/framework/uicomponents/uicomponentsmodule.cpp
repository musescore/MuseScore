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

#include <QtQml>
#include "view/sampleobject.h"
#include "view/validators/doubleinputvalidator.h"
#include "view/validators/intinputvalidator.h"
#include "view/iconview.h"
#include "view/sortfilterproxymodel.h"
#include "view/popupview.h"
#include "view/dialogview.h"
#include "view/filepickermodel.h"
#include "view/itemmultiselectionmodel.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

using namespace mu::uicomponents;
using namespace mu::framework;

static void uicomponents_init_qrc()
{
    Q_INIT_RESOURCE(uicomponents);
}

std::string UiComponentsModule::moduleName() const
{
    return "uicomponents";
}

void UiComponentsModule::registerExports()
{
}

void UiComponentsModule::registerResources()
{
    uicomponents_init_qrc();
}

void UiComponentsModule::registerUiTypes()
{
    qmlRegisterType<SampleObject>("MuseScore.UiComponents", 1, 0, "SampleObject");

    qmlRegisterUncreatableType<QAbstractItemModel>("MuseScore.UiComponents", 1, 0, "AbstractItemModel", "Cannot ...");

    qmlRegisterType<DoubleInputValidator>("MuseScore.UiComponents", 1, 0, "DoubleInputValidator");
    qmlRegisterType<IntInputValidator>("MuseScore.UiComponents", 1, 0, "IntInputValidator");
    qmlRegisterType<IntInputValidator>("MuseScore.UiComponents", 1, 0, "IntInputValidator");

    qmlRegisterType<IconView>("MuseScore.UiComponents", 1, 0, "IconView");

    qmlRegisterType<SortFilterProxyModel>("MuseScore.UiComponents", 1, 0, "SortFilterProxyModel");
    qmlRegisterType<FilterValue>("MuseScore.UiComponents", 1, 0, "FilterValue");
    qmlRegisterType<SorterValue>("MuseScore.UiComponents", 1, 0, "SorterValue");
    qmlRegisterUncreatableType<CompareType>("MuseScore.UiComponents", 1, 0, "CompareType", "Cannot create a CompareType");

    qmlRegisterType<PopupView>("MuseScore.UiComponents", 1, 0, "PopupView");
    qmlRegisterType<DialogView>("MuseScore.UiComponents", 1, 0, "DialogView");
    qmlRegisterType<FilePickerModel>("MuseScore.UiComponents", 1, 0, "FilePickerModel");
    qmlRegisterType<ItemMultiSelectionModel>("MuseScore.UiComponents", 1, 0, "ItemMultiSelectionModel");

    auto ui = modularity::ioc()->resolve<ui::IUiEngine>(moduleName());
    if (ui) {
        ui->addSourceImportPath(uicomponents_QML_IMPORT);
    }
}
