/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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
#include "uicomponentsmodule.h"

#include <QQmlEngine>

#include "view/treeview/qquicktreemodeladaptor_p.h"
#include "view/treeview/qquickabstractstyle_p.h"
#include "view/treeview/qquickselectionmode_p.h"

#include "modularity/ioc.h"

#include "ui/uitypes.h"
#include "ui/iinteractiveuriregister.h"

using namespace muse::uicomponents;
using namespace muse::ui;
using namespace muse::modularity;

std::string UiComponentsModule::moduleName() const
{
    return "uicomponents";
}

void UiComponentsModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("muse://interactive/selectmultipledirectories"),
                        ContainerMeta(ContainerType::QmlDialog, "Muse.UiComponents", "SelectMultipleDirectoriesDialog"));
    }
}

void UiComponentsModule::registerUiTypes()
{
    qmlRegisterType<QQuickTreeModelAdaptor1>("Muse.UiComponents.Private", 1, 0, "TreeModelAdaptor");
    qmlRegisterType<QQuickAbstractStyle1>("Muse.UiComponents.Private", 1, 0, "AbstractStyle");
    qmlRegisterType<QQuickPadding1>("Muse.UiComponents.Private", 1, 0, "Padding");
    qmlRegisterUncreatableType<QQuickSelectionMode1>("Muse.UiComponents.Private", 1, 0, "SelectionMode",
                                                     QLatin1String("Do not create objects of type SelectionMode"));
}
