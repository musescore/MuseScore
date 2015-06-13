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
      Album* album = new Album();
      album->setName("test");
      
      AlbumItem* item = new AlbumItem;
      item->path = root + "/" + DIR + "album01-01.mscx";
      album->append(item);
      
      item = new AlbumItem;
      item->path = root + "/" + DIR + "album01-02.mscx";
      album->append(item);
      
      album->createScore("album01.mscx");
      
      QVERIFY(compareFiles("album01.mscx", DIR + "album01-ref.mscx"));
      }

QTEST_MAIN(TestAlbum)
#include "tst_album.moc"

