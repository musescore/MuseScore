//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include <QGuiApplication>
#include <gmock/gmock.h>

#include "framework/global/runtime.h"
#include "environment.h"

GTEST_API_ int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);

    qputenv("QML_DISABLE_DISK_CACHE", "true");

    mu::runtime::mainThreadId(); //! NOTE Needs only call
    mu::runtime::setThreadName("main");

    mu::testing::Environment::setup();

    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
