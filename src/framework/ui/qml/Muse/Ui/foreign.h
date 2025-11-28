/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <qqmlintegration.h>

#include "ui/internal/uiengine.h"
#include "ui/api/themeapi.h"
#include "ui/view/qmltooltip.h"
#include "ui/view/qmldataformatter.h"
#include "ui/view/interactiveprovider.h"

#ifdef Q_OS_MACOS
#include "ui/view/platform/macos/macosmainwindowbridge.h"
#else
#include "ui/view/mainwindowbridge.h"
#endif

#include "ui/view/iconcodes.h"
#include "ui/view/musicalsymbolcodes.h"

#include "ui/uitypes.h"

// TODO: refactor the core `ui` module to avoid the need for this file
// The core `ui` module should be sufficiently decoupled from these types,
// so that they can be moved into `ui_qml` and register themselves directly.

namespace muse::ui {
struct UiEngineForeign {
    Q_GADGET
    QML_FOREIGN(muse::ui::UiEngine)
    QML_NAMED_ELEMENT(UiEngine)
    QML_UNCREATABLE("Must be created in C++ only")
};

struct ThemeApiForeign {
    Q_GADGET
    QML_FOREIGN(muse::api::ThemeApi)
    QML_NAMED_ELEMENT(QmlTheme)
    QML_UNCREATABLE("Must be created in C++ only")
};

struct QmlToolTipForeign {
    Q_GADGET
    QML_FOREIGN(muse::ui::QmlToolTip)
    QML_NAMED_ELEMENT(QmlToolTip)
    QML_UNCREATABLE("Must be created in C++ only")
};

struct QmlDataFormatterForeign {
    Q_GADGET
    QML_FOREIGN(muse::ui::QmlDataFormatter)
    QML_NAMED_ELEMENT(DataFormatter)
    QML_UNCREATABLE("Must be created in C++ only")
};

struct InteractiveProviderForeign {
    Q_GADGET
    QML_FOREIGN(muse::ui::InteractiveProvider)
    QML_NAMED_ELEMENT(CppInteractiveProvider)
    QML_UNCREATABLE("Must be created in C++ only")
};

struct MainWindowBridgeForeign {
    Q_GADGET
#ifdef Q_OS_MACOS
    QML_FOREIGN(muse::ui::MacOSMainWindowBridge)
#else
    QML_FOREIGN(muse::ui::MainWindowBridge)
#endif
    QML_NAMED_ELEMENT(MainWindowBridge)
};

namespace IconCodeForeign {
Q_NAMESPACE;
QML_FOREIGN_NAMESPACE(muse::ui::IconCode);
QML_NAMED_ELEMENT(IconCode);
}

namespace MusicalSymbolCodesForeign {
Q_NAMESPACE;
QML_FOREIGN_NAMESPACE(muse::ui::MusicalSymbolCodes);
QML_NAMED_ELEMENT(MusicalSymbolCodes);
}

namespace ContainerTypeForeign {
Q_NAMESPACE;
QML_FOREIGN_NAMESPACE(muse::ui::ContainerType);
QML_NAMED_ELEMENT(ContainerType);
}
}
