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

#include <gtest/gtest.h>

#include "notation/internal/scorepropertiesitem.h"
#include <QLabel>
#include <QApplication>
//#include <QTest>

using namespace mu;
using namespace mu::notation;

class ScorePropertiesItemTests : public ::testing::Test
{
public:

};

TEST_F(ScorePropertiesItemTests, StandardProperty)
{
    //! CASE When assembling a property item with a non editable label
    QLabel *label = new QLabel("MyLabel");

    ScorePropertiesItem item(label, nullptr);

    //! GIVEN I get the label name
    QString labelName = item.Label();

    //! CHECK It is what I expect it to be
    EXPECT_TRUE(labelName == "MyLabel");
    delete label;
}

TEST_F(ScorePropertiesItemTests, NonStandardProperty)
{
    //! CASE When assembling a property item with a editable label
    QLineEdit *label = new QLineEdit("MyLabel");

    ScorePropertiesItem item(label, nullptr);
 
    //! GIVEN I get the label name
    QString labelName = item.Label();


    //! CHECK It is what I expect it to be
    EXPECT_TRUE(labelName == "MyLabel");
    delete label;
}
