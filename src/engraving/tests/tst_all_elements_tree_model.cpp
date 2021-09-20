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

#include "testing/qtestsuite.h"
#include "testbase.h"
#include "libmscore/masterscore.h"

static const QString ALL_ELEMENTS_DATA_DIR("all_elements_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestTreeModel
///   Ensures that the tree model is consistent. Starting
///   from Score each element in the tree should have the
///   correct parent element, the one whose children list
///   it appears in.
//---------------------------------------------------------

class TestTreeModel : public QObject, public MTest
{
    Q_OBJECT

    void tstTree(QString file);
    void traverseTree(EngravingObject* element);

private slots:
    void initTestCase();
    void tstTreeElements() { tstTree("layout_elements.mscx"); }
    void tstTreeTablature() { tstTree("layout_elements_tab.mscx"); }
    void tstTreeMoonlight() { tstTree("moonlight.mscx"); }
    void tstTreeGoldberg() { tstTree("goldberg.mscx"); }
};

QString elementToText(EngravingObject* element);

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestTreeModel::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   tstTree
//---------------------------------------------------------

void TestTreeModel::tstTree(QString file)
{
    MasterScore* score = readScore(ALL_ELEMENTS_DATA_DIR + file);
    traverseTree(score);
}

//---------------------------------------------------------
//   traverseTree
///   Checks whether parent element of current element is
///   correct, then recursively checks all children.
//---------------------------------------------------------

void TestTreeModel::traverseTree(EngravingObject* element)
{
    for (int i = 0; i < element->scanChildCount(); ++i) {
        EngravingObject* child = element->scanChild(i);
        // child should never be nullptr
        if (!child) {
            qDebug() << "EngravingItem returned nullptr in treeChild()!";
            qDebug() << "EngravingItem: " << elementToText(element);
            qDebug() << "Number of children: " << element->scanChildCount();
            qDebug() << "Children: ";
            for (int i = 0; i < element->scanChildCount(); i++) {
                qDebug() << element->scanChild(i);
            }
        }
        QVERIFY(child);
        // if parent is not correct print some logging info and exit
        if (child->scanParent() != element) {
            qDebug() << "EngravingItem does not have correct parent!";
            qDebug() << "EngravingItem name: " << elementToText(child);
            qDebug() << "Parent in tree model: " << elementToText(child->scanParent());
            qDebug() << "Expected parent: " << elementToText(element);
        }
        QCOMPARE(child->scanParent(), element);

        // recursively apply to the rest of the tree
        traverseTree(child);
    }
}

//---------------------------------------------------------
//   elementToText
///   for printing debug info about any element
//---------------------------------------------------------

QString elementToText(EngravingObject* element)
{
    if (element == nullptr) {
        return "nullptr";
    }
    if (element->isEngravingItem()) {
        return toEngravingItem(element)->accessibleInfo();
    }
    return element->userName();
}

QTEST_MAIN(TestTreeModel)
#include "tst_all_elements_tree_model.moc"
