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

#include <QDebug>

#include "testing/qtestsuite.h"

class Sample1 : public QObject
{
    Q_OBJECT

public:
    Sample1();
    ~Sample1();

private slots:
    void test_case1();
};

Sample1::Sample1()
{
}

Sample1::~Sample1()
{
}

void Sample1::test_case1()
{
    qDebug() << Q_FUNC_INFO;
}

QTEST_MAIN(Sample1)

#include "tst_sample.moc"
