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
#include "libmscore/album.h"
#include "libmscore/excerpt.h"
#include "mscore/preferences.h"

#define DIR QString("libmscore/albumsIO/")

using namespace Ms;

//---------------------------------------------------------
//   TestAlbums
//---------------------------------------------------------

class TestAlbumsIO : public QObject, public MTest
{
    Q_OBJECT

    void saveAlbumUnloadedTest(const char* file, bool legacy = false);
    void saveAlbumLoadedTest(const char* file, bool legacy = false);
    void exportCompressedAlbumTest(const char* file);
    void stringsTest(const char* file);
    void addRemoveTest(const char* file);
    void partsTest(const char* file);

    void loadScores(Album* album);

private slots:
    void initTestCase();

    void albumsUnloadedSimple() { saveAlbumUnloadedTest("smallPianoAlbum"); }
    void albumsUnloadedParts() { saveAlbumUnloadedTest("albumWithParts"); }
    void albumsUnloadedLegacy() { saveAlbumUnloadedTest("muse2", true); }

    void albumsLoadedSimple() { saveAlbumLoadedTest("smallPianoAlbum"); }
    void albumsLoadedParts() { saveAlbumLoadedTest("albumWithParts"); }
    void albumsLoadedLegacy() { saveAlbumLoadedTest("muse2", true); }

    void exportCompressedAlbum() { exportCompressedAlbumTest("smallPianoAlbum"); }

    void albumsStrings() { stringsTest("smallPianoAlbum"); }

    void albumsAddRemove() { addRemoveTest("smallPianoAlbum"); }
    void albumsAddRemoveParts() { addRemoveTest("albumWithParts"); }

    void parts() { partsTest("albumWithParts"); }
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestAlbumsIO::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   loadScores
//---------------------------------------------------------

void TestAlbumsIO::loadScores(Album* album)
{
    for (auto& item : album->albumItems()) {
        QString path = item->fileInfo().canonicalFilePath();
        MasterScore* score = readScoreAlbums(path);
        item->setScore(score);
    }
}

//---------------------------------------------------------
//   saveAlbumUnloadedTest
///     read a .msca (Album) file, write to a new file and verify both files are identical
///     in this test, the scores of the album have NOT been loaded
//---------------------------------------------------------

void TestAlbumsIO::saveAlbumUnloadedTest(const char* file, bool legacy)
{
    MScore::debugMode = true;
    QString extension = legacy ? ".album" : ".msca";

    std::unique_ptr<Album> album(readAlbum(DIR + QString(file) + extension, legacy));
    Album::activeAlbum = album.get();
    QVERIFY(album);
    album->setIncludeAbsolutePaths(false);

    QFileInfo fi(QString(file) + "_generated" + ".msca");
    QVERIFY(saveAlbum(album.get(), fi.absoluteFilePath()));   // wrong path, but not deleted for debugging
    if (!legacy) {
        QVERIFY(saveCompareAlbum(album.get(), DIR + QString(file) + "_generated" + ".msca", DIR + QString(file) + ".msca"));
    } else {
        QVERIFY(saveCompareAlbum(album.get(), DIR + QString(file) + "_generated" + ".msca", DIR + QString(file) + "_ref" + ".msca"));
    }
}

//---------------------------------------------------------
//   saveAlbumLoadedTest
///     read a .msca (Album) file, write to a new file and verify both files are identical
///     in this test, the scores of the album have been loaded
//---------------------------------------------------------

void TestAlbumsIO::saveAlbumLoadedTest(const char* file, bool legacy)
{
    MScore::debugMode = true;
    QString extension = legacy ? ".album" : ".msca";

    std::unique_ptr<Album> album(readAlbum(DIR + QString(file) + extension, legacy));
    Album::activeAlbum = album.get();
    QVERIFY(album);
    album->setIncludeAbsolutePaths(false);

    // load scores
    loadScores(album.get());

    QFileInfo fi(QString(file) + "_generated" + ".msca");
    QVERIFY(saveAlbum(album.get(), fi.absoluteFilePath()));   // wrong path, but not deleted for debugging
    if (!legacy) {
        QVERIFY(saveCompareAlbum(album.get(), DIR + QString(file) + "_generated" + ".msca", DIR + QString(file) + ".msca"));
    } else {
        QVERIFY(saveCompareAlbum(album.get(), DIR + QString(file) + "_generated" + ".msca", DIR + QString(file) + "_loaded_ref" + ".msca"));
    }

    album->createCombinedScore();

    QFileInfo fi2(QString(file) + "_generated2" + ".msca");
    QVERIFY(saveAlbum(album.get(), fi2.absoluteFilePath()));   // wrong path, but not deleted for debugging
    if (!legacy) {
        QVERIFY(saveCompareAlbum(album.get(), DIR + QString(file) + "_generated2" + ".msca", DIR + QString(file) + ".msca"));
    } else {
        QVERIFY(saveCompareAlbum(album.get(), DIR + QString(file) + "_generated2" + ".msca",
                                 DIR + QString(file) + "_loaded_ref" + ".msca"));
    }
}

//---------------------------------------------------------
//   exportCompressedAlbumTest
//---------------------------------------------------------

void TestAlbumsIO::exportCompressedAlbumTest(const char* file)
{
    MScore::debugMode = true;
    std::unique_ptr<Album> album(readAlbum(DIR + QString(file) + ".msca"));
    Album::activeAlbum = album.get();
    QVERIFY(album);

    // load scores
    loadScores(album.get());

    QFileInfo fi(QString(file) + "_generated" + ".mscaz");
    QFile fp(fi.filePath());
    QVERIFY(album->exportAlbum(&fp, fi));

    QDir d(fi.absolutePath());
    d.mkdir("imported");

    Album::importAlbum(fi.absoluteFilePath(), QDir(fi.absolutePath() + QDir::separator() + "imported"));

//    for (auto item : album->albumItems()) {
//        QVERIFY(compareFilesFromPaths(item->fileInfo.absoluteFilePath(),
//                                      fi.absolutePath() + QDir::separator() + "imported" + QDir::separator() + album->exportedScoreFolder() + QDir::separator() + item->fileInfo.absoluteFilePath().split(QDir::separator()).last()));
//    }
    QVERIFY(compareFilesFromPaths(root + "/" + DIR + QString(file) + "_comp_ref.msca",
                                  fi.absolutePath() + QDir::separator() + "imported" + QDir::separator() + QString(file) + "_generated"
                                  + ".msca"));
}

//---------------------------------------------------------
//   stringsTest
//---------------------------------------------------------

void TestAlbumsIO::stringsTest(const char* file)
{
    MScore::debugMode = true;
    std::unique_ptr<Album> album(readAlbum(DIR + QString(file) + ".msca"));
    Album::activeAlbum = album.get();
    QVERIFY(album);

    // load scores
    loadScores(album.get());

    auto x = album->composers();
    QVERIFY(x.size() == 2);
    QCOMPARE(x.at(0), QString("Sergios - Anestis Kefalidis"));
    QCOMPARE(x.at(1), QString("Oregano"));
    auto y = album->lyricists();
    QVERIFY(y.size() == 1);
    QCOMPARE(y.at(0), QString("Garlic"));
    auto z = album->scoreTitles();
    QVERIFY(z.size() == 3);
    QCOMPARE(z.at(0), QString("Piano1"));
    QCOMPARE(z.at(1), QString("Piano2"));
    QCOMPARE(z.at(2), QString("Piano3"));

    album->removeScore(0);

    auto x2 = album->composers();
    QVERIFY(x2.size() == 1);
    QCOMPARE(x2.at(0), QString("Oregano"));
    auto y2 = album->lyricists();
    QVERIFY(y2.size() == 1);
    QCOMPARE(y2.at(0), QString("Garlic"));
    auto z2 = album->scoreTitles();
    QVERIFY(z2.size() == 2);
    QCOMPARE(z2.at(0), QString("Piano2"));
    QCOMPARE(z2.at(1), QString("Piano3"));
}

//---------------------------------------------------------
//   addRemoveTest
//---------------------------------------------------------

void TestAlbumsIO::addRemoveTest(const char* file)
{
    MScore::debugMode = true;
    std::unique_ptr<Album> album(readAlbum(DIR + QString(file) + ".msca"));
    Album::activeAlbum = album.get();
    QVERIFY(album);

    // load scores
    loadScores(album.get());

    if (strcmp(file, "smallPianoAlbum") == 0) {
        QVERIFY(album->albumItems().size() == 3);
    } else if (strcmp(file, "albumWithParts") == 0) {
        QVERIFY(album->albumItems().size() == 2);
    }

    int albumSize = album->albumItems().size();
    int lastIndex = albumSize - 1;

    MasterScore* ms = album->albumItems().at(lastIndex)->score();
    ms->setRequiredByMuseScore(true); // prevents deleting when removing from the Album
    album->removeScore(ms);
    QCOMPARE(int(album->albumItems().size()), albumSize - 1);
    album->addScore(ms);
    QCOMPARE(int(album->albumItems().size()), albumSize);

    album->createCombinedScore();
    MasterScore* combinedScore = album->getCombinedScore();
    for (auto& e : combinedScore->excerpts()) {
        QCOMPARE(e->partScore()->isMultiMovementScore(), true);
    }
    auto excerptCheck = [&]() {
                            for (auto& e : combinedScore->excerpts()) {
                                QCOMPARE(int(e->partScore()->movements()->size()), int(combinedScore->movements()->size()));
                            }
                        };

    QCOMPARE(int(combinedScore->movements()->size()), albumSize + 1);
    excerptCheck();
    album->removeScore(ms);
    QCOMPARE(int(album->albumItems().size()), albumSize - 1);
    QCOMPARE(int(combinedScore->movements()->size()), albumSize);
    excerptCheck();
    album->addScore(ms);
    QCOMPARE(int(album->albumItems().size()), albumSize);
    QCOMPARE(int(combinedScore->movements()->size()), albumSize + 1);
    excerptCheck();
    ms->setRequiredByMuseScore(false);
}

//---------------------------------------------------------
//   partsTest
//---------------------------------------------------------

void TestAlbumsIO::partsTest(const char* file)
{
    MScore::debugMode = true;
    std::unique_ptr<Album> album(readAlbum(DIR + QString(file) + ".msca"));
    Album::activeAlbum = album.get();
    std::unique_ptr<MasterScore> aScore(readScore(DIR + "Piano1.mscx"));
    std::unique_ptr<MasterScore> bScore(readScore(DIR + "Movement_3.mscx"));

    QVERIFY(album);
    loadScores(album.get());
    album->createCombinedScore();

    QVERIFY(album->checkPartCompatibility());
    QVERIFY(!album->checkPartCompatibility(aScore.get()));
    QVERIFY(!album->checkPartCompatibility(bScore.get()));

    QVERIFY(album->getCombinedScore()->excerpts().size());
    album->removeAlbumExcerpts();
    QVERIFY(album->getCombinedScore()->excerpts().size() == 0);
}

QTEST_MAIN(TestAlbumsIO)
#include "tst_albumsIO.moc"
