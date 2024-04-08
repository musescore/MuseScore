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
#ifndef MUSE_TESTING_QTESTSUITE_H
#define MUSE_TESTING_QTESTSUITE_H

#include <QtTest/QtTest>
#include <QList>

#include "log.h"

namespace muse::testing {
class QTestSuite
{
public:
    QTestSuite() = default;

    ~QTestSuite()
    {
        TestList& list = testList();
        for (QObject* test : list) {
            delete test;
        }
        list.clear();
    }

    using TestList = QList<QObject*>;

    inline static TestList& testList()
    {
        static TestList list;
        return list;
    }

    inline static bool findObject(const QString& name)
    {
        TestList& list = testList();
        for (QObject* test : list) {
            if (test->objectName() == name) {
                return true;
            }
        }
        return false;
    }

    inline static void addTest(QObject* object)
    {
        TestList& list = testList();
        list.append(object);
    }

    inline static bool shouldExecute(QObject* qObj, int argc, char* argv[])
    {
        if (argc <= 1 || argv[1][0] == '-') {
            return true;
        }
        const QMetaObject* moTest = qObj->metaObject();
        for (int methodIdx = moTest->methodOffset(); methodIdx < moTest->methodCount(); ++methodIdx) {
            QMetaMethod mmTest = moTest->method(methodIdx);
            if (mmTest.methodType() == QMetaMethod::Slot) {
                if (QString(mmTest.methodSignature()).contains(argv[1])) {
                    return true;
                }
            }
        }
        return false;
    }

    inline static int run(int argc, char* argv[])
    {
        LOGD() << "========================== argc: " << argc;
        for (int i = 0; i < argc; ++i) {
            LOGD() << "========================== argv " << i << ": " << argv[i];
        }
        int ret = 0;
        for (QObject* test : testList()) {
            if (argc > 0 && std::find(argv + 1, argv + argc, test->objectName()) != argv + argc) {
                ret += QTest::qExec(test, 1, argv);
            } else if (shouldExecute(test, argc, argv)) {
                ret += QTest::qExec(test, argc, argv);
            }
        }
        return ret;
    }
};

template<class T>
class QTestHolder
{
public:
    QTestHolder(const QString& name)
    {
        if (!QTestSuite::findObject(name)) {
            T* t = new T();
            t->setObjectName(name);
            QTestSuite::addTest(t);
        }
    }
};

#ifdef QTEST_MAIN
#undef QTEST_MAIN
#endif

#define DECLARE_TEST(className) static muse::testing::QTestHolder<className> t_(#className);
#define QTEST_MAIN(className) DECLARE_TEST(className)
}

#endif // MUSE_TESTING_QTESTSUITE_H
