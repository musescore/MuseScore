/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
#ifndef MU_PLUGINS_UIENGINEMOCK_H
#define MU_PLUGINS_UIENGINEMOCK_H

#include <gmock/gmock.h>

#include <QObject>

#include "ui/iuiengine.h"

namespace mu::plugins {
class UiEngineMock : public QObject, public ui::IUiEngine
{
public:
    MOCK_METHOD(void, updateTheme, (), (override));
    MOCK_METHOD(QQmlEngine*, qmlEngine, (), (const, override));
    MOCK_METHOD(void, clearComponentCache, (), (override));

    MOCK_METHOD(void, addSourceImportPath, (const QString& path), (override));
};
}

#endif // MU_PLUGINS_UIENGINEMOCK_H
