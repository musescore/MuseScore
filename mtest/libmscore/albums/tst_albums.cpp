//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA
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

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "mscore/musescore.h"
#include "mscore/preferences.h"
#include "libmscore/measure.h"

#define DIR QString("libmscore/albums/")

using namespace Ms;

//---------------------------------------------------------
//   TestAlbums
//---------------------------------------------------------

class TestAlbums : public QObject, public MTest
{
    Q_OBJECT

    void albumAddScore();
    void albumItemDuration();
    void albumItemEnable();
    void albumItemBreaks();
    void albumBreaks();

private slots:
    void initTestCase();

    void albumAddScoreTest() { albumAddScore(); }
    void albumItemDurationTest() { albumItemDuration(); }
    void albumItemEnableTest() { albumItemEnable(); }
    void albumItemBreaksTest() { albumItemBreaks(); }
    void albumBreaksTest() { albumBreaks(); }
};

#define private public
#include "libmscore/album.h"

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestAlbums::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   albumItemTest
//---------------------------------------------------------

void TestAlbums::albumAddScore()
{
    Album myAlbum;
    Album::activeAlbum = &myAlbum;
    MasterScore* aScore = readScore(DIR + "AlbumItemTest.mscx"); // deleted when aItem is deleted
    MasterScore* bScore = new MasterScore();
    AlbumItem* aItem = myAlbum.addScore(aScore, true); // deleted when myAlbum is deleted

    QCOMPARE(&aItem->album, &myAlbum);
    QVERIFY(myAlbum.albumItems().size() == 1);

    QVERIFY(Album::scoreInActiveAlbum(aScore));
    QVERIFY(!Album::scoreInActiveAlbum(bScore));

    QCOMPARE(aItem->setScore(bScore), -1);

    delete bScore;
}

//---------------------------------------------------------
//   albumItemDuration
//---------------------------------------------------------

void TestAlbums::albumItemDuration()
{
    Album myAlbum;
    Album::activeAlbum = &myAlbum;
    MasterScore* aScore = readScore(DIR + "AlbumItemTest.mscx");
    AlbumItem* aItem = myAlbum.addScore(aScore, true);

    QCOMPARE(aItem->duration(), aScore->duration());
    int scoreDuration1 = aScore->duration();
    aScore->appendMeasures(2);
    int scoreDuration2 = aScore->duration();
    QVERIFY(scoreDuration1 < scoreDuration2);
    QCOMPARE(aItem->duration(), aScore->duration());
}

//---------------------------------------------------------
//   albumItemEnable
//---------------------------------------------------------

void TestAlbums::albumItemEnable()
{
    Album myAlbum;
    Album::activeAlbum = &myAlbum;
    MasterScore* aScore = readScore(DIR + "AlbumItemTest.mscx");
    AlbumItem* aItem = myAlbum.addScore(aScore, true);

    QVERIFY(aItem->enabled());
    QCOMPARE(aItem->enabled(), aScore->enabled()); // true
    aItem->setEnabled(false);
    QVERIFY(!aItem->enabled());
    QCOMPARE(aItem->enabled(), aScore->enabled()); // false
}

//---------------------------------------------------------
//   albumItemBreaks
//---------------------------------------------------------

void TestAlbums::albumItemBreaks()
{
    Album myAlbum;
    Album::activeAlbum = &myAlbum;
    MasterScore* aScore = readScore(DIR + "AlbumItemTest.mscx");

    // addSectionBreak
    QCOMPARE(aScore->lastMeasure()->sectionBreak(), false);
    AlbumItem* aItem = myAlbum.addScore(aScore, true); // AlbumItem::addAlbumSectionBreak() gets called by Album::addScore
    QCOMPARE(aScore->lastMeasure()->sectionBreak(), true);

    // getSectionBreak
    QCOMPARE(aItem->getSectionBreak(), aScore->lastMeasure()->el().back());

    // m_pauseDuration part 1
    QCOMPARE(aItem->getSectionBreak()->pause(), aItem->m_pauseDuration);
    qreal newPauseDuration = aItem->m_pauseDuration * 2;
    aItem->getSectionBreak()->setPause(newPauseDuration);
    QCOMPARE(aItem->getSectionBreak()->pause(), newPauseDuration);

    // removeSectionBreak
    aItem->removeAlbumSectionBreak();
    QCOMPARE(aScore->lastMeasure()->sectionBreak(), false);

    // m_pauseDuration part 2
    aItem->addAlbumSectionBreak();
    QCOMPARE(aItem->getSectionBreak()->pause(), aItem->m_pauseDuration);

    // addPageBreak
    QCOMPARE(aScore->lastMeasure()->pageBreak(), false);
    aItem->addAlbumPageBreak();
    QCOMPARE(aScore->lastMeasure()->pageBreak(), true);

    // removePageBreak
    aItem->removeAlbumPageBreak();
    QCOMPARE(aScore->lastMeasure()->pageBreak(), false);
}

//---------------------------------------------------------
//   albumBreaks
//---------------------------------------------------------

void TestAlbums::albumBreaks()
{
    Album myAlbum;
    Album::activeAlbum = &myAlbum;
    MasterScore* aScore = readScore(DIR + "AlbumItemTest.mscx");
    MasterScore* bScore = readScore(DIR + "AlbumItemTest.mscx");
    myAlbum.addScore(aScore);
    myAlbum.addScore(bScore);
    myAlbum.addAlbumPageBreaks();
    QVERIFY(aScore->lastMeasure()->pageBreak());
    QVERIFY(bScore->lastMeasure()->pageBreak());
    QVERIFY(aScore->lastMeasure()->sectionBreak());
    QVERIFY(bScore->lastMeasure()->sectionBreak());
    myAlbum.removeAlbumPageBreaks();
    QVERIFY(!aScore->lastMeasure()->pageBreak());
    QVERIFY(!bScore->lastMeasure()->pageBreak());
    QVERIFY(aScore->lastMeasure()->sectionBreak());
    QVERIFY(bScore->lastMeasure()->sectionBreak());
    myAlbum.removeAlbumSectionBreaks();
    QVERIFY(!aScore->lastMeasure()->pageBreak());
    QVERIFY(!bScore->lastMeasure()->pageBreak());
    QVERIFY(!aScore->lastMeasure()->sectionBreak());
    QVERIFY(!bScore->lastMeasure()->sectionBreak());
}

QTEST_MAIN(TestAlbums)
#include "tst_albums.moc"
