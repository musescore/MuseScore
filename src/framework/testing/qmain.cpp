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

#include <QGuiApplication>

#include "qtestsuite.h"
#include "environment.h"
#include "framework/global/runtime.h"

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);

    qputenv("QML_DISABLE_DISK_CACHE", "true");

    mu::runtime::mainThreadId(); //! NOTE Needs only call
    mu::runtime::setThreadName("main");

    mu::testing::Environment::setup();

    return mu::testing::QTestSuite::run(argc, argv);
}
