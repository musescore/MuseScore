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

#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QGuiApplication>

#include "global/stringutils.h"
#include "global/runtime.h"

#include "environment.h"

#include "log.h"

GTEST_API_ int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);

    qputenv("QML_DISABLE_DISK_CACHE", "true");

    //! NOTE Fixed filter value
    //! When the test is run in QtCreator with debuger,
    //! the filter is passed as --gtest_filter="SuiteName.TestName"
    //! but expected --gtest_filter=SuiteName.TestName (without quotes)
    //! (maybe QtCreator bug)
    std::vector<std::string> args;
    args.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        std::string arg = argv[i];
        if (muse::strings::startsWith(arg, "--gtest_filter=")) {
            std::vector<std::string> filter;
            muse::strings::split(arg, filter, "=");
            if (filter.size() > 1) {
                std::string val = filter.at(1);
                if (val.at(0) == '"') {
                    val = val.substr(1, val.size() - 2);
                }
                arg = filter.at(0) + "=" + val;
            }
        }
        args.push_back(arg);
    }

    std::vector<char*> argsc;
    argsc.reserve(argc + 1);
    for (size_t i = 0; i < args.size(); ++i) {
        argsc.push_back(const_cast<char*>(args.at(i).c_str()));
    }
    argsc.push_back(NULL);
    argv = argsc.data();

    muse::runtime::mainThreadId(); //! NOTE Needs only call
    muse::runtime::setThreadName("main");

    muse::testing::Environment::setup();

    testing::InitGoogleMock(&argc, argv);

    GTEST_FLAG_SET(death_test_style, "threadsafe");

    PROFILER_CLEAR;

    int code = RUN_ALL_TESTS();

    muse::testing::Environment::deinit();

    PROFILER_PRINT;

    return code;
}
