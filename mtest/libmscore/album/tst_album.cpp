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

QTEST_MAIN(TestAlbum)
#include "tst_album.moc"

