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
#ifndef MU_TESTING_QTESTSUITE_H
#define MU_TESTING_QTESTSUITE_H

#include <QtTest/QtTest>
#include <QList>

namespace mu::testing {
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

    inline static int run(int argc, char* argv[])
    {
        //! TODO Add filter by the test function
        int ret = 0;
        for (QObject* test : testList()) {
            ret += QTest::qExec(test, argc, argv);
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

#define DECLARE_TEST(className) static mu::testing::QTestHolder<className> _t(#className);
#define QTEST_MAIN(className) DECLARE_TEST(className)
}

#endif // MU_TESTING_QTESTSUITE_H
