//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "mscore/album.h"

#define DIR QString("libmscore/album/")

using namespace Ms;

//---------------------------------------------------------
//   TestAlbum
//---------------------------------------------------------

class TestAlbum : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void album01();
      void album_78521();
      void album_76101();
      void album_105716();
      void album_105621();
      void album_105641();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestAlbum::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
///  album01
//--------------------------------------------------------

void TestAlbum::album01()
      {
      Album album;
      album.setName("test");
      album.append(new AlbumItem(root + "/" + DIR + "album01-01.mscx"));
      album.append(new AlbumItem(root + "/" + DIR + "album01-02.mscx"));
      album.createScore("album01.mscx");
      QVERIFY(compareFiles("album01.mscx", DIR + "album01-ref.mscx"));
      }

//---------------------------------------------------------
///  album_78521
///   test cases for input scores that contain no actual measures:
///   -album_78521-vbox contains only a VBox (but no measures)
///   -album_78521-empty contains no MeasureBase objects
//--------------------------------------------------------

void TestAlbum::album_78521()
      {
      Album album;
      album.setName("test");
      album.append(new AlbumItem(root + "/" + DIR + "album_78521-vbox.mscx"));
      album.append(new AlbumItem(root + "/" + DIR + "album_78521-vbox.mscx"));
      album.append(new AlbumItem(root + "/" + DIR + "album_78521-empty.mscx"));
      album.createScore("album_78521-vbox-vbox-empty.mscx");
      QVERIFY(compareFiles("album_78521-vbox-vbox-empty.mscx", DIR + "album_78521-vbox-vbox-empty-ref.mscx"));
      }

//---------------------------------------------------------
//   album_76101
//    appends two scores that do not have manually-inserted initial clefs
//          album_76101-01.mscx is two measures of treble clef
//          album_76101-02.mscx is two measures of bass clef
//    desired behavior is for initial clef of second score to be copied even if initial clef element is "generated" (which occurs if intial clef was not manually inserted)
//    this test verifies that there is a non-generated clef added to final tick of final measure of first section
//--------------------------------------------------------

void TestAlbum::album_76101()
      {
      Album album;
      album.setName("test");
      album.append(new AlbumItem(root + "/" + DIR + "album_76101-01.mscx"));
      album.append(new AlbumItem(root + "/" + DIR + "album_76101-02.mscx"));
      album.createScore("album_76101.mscx");
      QVERIFY(compareFiles("album_76101.mscx", DIR + "album_76101-ref.mscx"));
      }

//---------------------------------------------------------
//   album_105716
//    tests functionality of options to add SectionBreak or add PageBreak between each score, for all combinations.
//    input scores may end without any breaks, with only line break, with only page break, or line/page break and sectionbreak.
//--------------------------------------------------------

void TestAlbum::album_105716()
      {
      Album album;
      album.setName("test");
      album.append(new AlbumItem(root + "/" + DIR + "album_105716-measure-nobreak.mscx"));
      album.append(new AlbumItem(root + "/" + DIR + "album_105716-measure-linebreak.mscx"));
      album.append(new AlbumItem(root + "/" + DIR + "album_105716-measure-pagebreak.mscx"));
      album.append(new AlbumItem(root + "/" + DIR + "album_105716-measure-linebreak-sectionbreak.mscx"));
      album.append(new AlbumItem(root + "/" + DIR + "album_105716-measure-pagebreak-sectionbreak.mscx"));
      album.append(new AlbumItem(root + "/" + DIR + "album_105716-measure-nobreak.mscx"));

      album.createScore("album_105716-joined-addSectionBreak-addPageBreak.mscx", true, true);
      QVERIFY(compareFiles("album_105716-joined-addSectionBreak-addPageBreak.mscx", DIR + "album_105716-joined-addSectionBreak-addPageBreak-ref.mscx"));

      album.createScore("album_105716-joined-addPageBreak.mscx", true, false);
      QVERIFY(compareFiles("album_105716-joined-addPageBreak.mscx", DIR + "album_105716-joined-addPageBreak-ref.mscx"));

      album.createScore("album_105716-joined-addSectionBreak.mscx", false, true);
      QVERIFY(compareFiles("album_105716-joined-addSectionBreak.mscx", DIR + "album_105716-joined-addSectionBreak-ref.mscx"));

      album.createScore("album_105716-joined-noaddBreak.mscx", false, false);
      QVERIFY(compareFiles("album_105716-joined-noaddBreak.mscx", DIR + "album_105716-joined-noaddBreak-ref.mscx"));
      }

//---------------------------------------------------------
//   album_105621
//    crash when removing second score from AlbumManager after previously joining the album.
//    crash occured because TBox::clone() would not allocate memory for a new Text* object.
//--------------------------------------------------------

void TestAlbum::album_105621()
      {
      Album album;
      album.setName("test");
      album.append(new AlbumItem(root + "/" + DIR + "album_105621-measure.mscx"));
      album.append(new AlbumItem(root + "/" + DIR + "album_105621-textbox.mscx"));
      album.createScore("album_105621.mscx");
      album.remove(1); // crash would occur here in ~TBox
      }

//---------------------------------------------------------
//   album_105641
//    appends two scores of 3 staves: 1-staff piano & 2-staff piano.
//    second score "album_105641-no-initial-clef-02.mscx" doesn't have initial clef on 2nd staff,
//                  but instead has a clef before first chordrest of 1st measure.
//    desired behavior is no crash.  Should not try to add initial clef if initial clef doesn't exist.
//--------------------------------------------------------

void TestAlbum::album_105641()
      {
      Album album;
      album.setName("test");
      album.append(new AlbumItem(root + "/" + DIR + "album_105641-no-initial-clef-01.mscx"));
      album.append(new AlbumItem(root + "/" + DIR + "album_105641-no-initial-clef-02.mscx"));
      album.createScore("album_105641-no-initial-clef.mscx");
      QVERIFY(compareFiles("album_105641-no-initial-clef.mscx", DIR + "album_105641-no-initial-clef-ref.mscx"));
      }

QTEST_MAIN(TestAlbum)
#include "tst_album.moc"

