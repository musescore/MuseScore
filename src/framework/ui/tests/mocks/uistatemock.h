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
 #pragma once

#include <gmock/gmock.h>

#include "ui/iuistate.h"

namespace muse::ui {
class UiStateMock : public IUiState
{
public:

    MOCK_METHOD(ValNt<QByteArray>, pageState, (const QString&), (const, override));
    MOCK_METHOD(void, setPageState, (const QString&, const QByteArray&), (override));

    MOCK_METHOD(QByteArray, windowGeometry, (), (const, override));
    MOCK_METHOD(void, setWindowGeometry, (const QByteArray&), (override));
    MOCK_METHOD(async::Notification, windowGeometryChanged, (), (const, override));

    MOCK_METHOD(bool, isVisible, (const QString&, bool), (const, override));
    MOCK_METHOD(void, setIsVisible, (const QString&, bool), (override));
    MOCK_METHOD(async::Notification, isVisibleChanged, (const QString&), (const, override));

    MOCK_METHOD(QString, uiItemState, (const QString&), (const, override));
    MOCK_METHOD(void, setUiItemState, (const QString&, const QString&), (override));
    MOCK_METHOD(async::Notification, uiItemStateChanged, (const QString&), (const, override));

    MOCK_METHOD(ToolConfig, toolConfig, (const QString&, const ToolConfig&), (const, override));
    MOCK_METHOD(void, setToolConfig, (const QString&, const ToolConfig&), (override));
    MOCK_METHOD(async::Notification, toolConfigChanged, (const QString&), (const, override));
};
}
