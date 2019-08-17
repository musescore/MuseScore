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

#ifndef __TESTUTILS_H__
#define __TESTUTILS_H__


#include "libmscore/element.h"

namespace Ms {
      class MScore;
      class MasterScore;
      class Score;

//---------------------------------------------------------
//   MTest
//---------------------------------------------------------

class MTest {
   protected:
      Ms::MScore* mscore;
      QString root;     // root path of test source
      Ms::Score* score;
      EditData ed;

      MTest();
      Ms::MasterScore* readScore(const QString& name);
      Ms::MasterScore* readCreatedScore(const QString& name);
      bool saveScore(Ms::Score*, const QString& name) const;
      bool savePdf(Ms::MasterScore*, const QString& name);
      bool saveMusicXml(Ms::MasterScore*, const QString& name);
      bool saveMimeData(QByteArray mimeData, const QString& saveName);
      bool compareFiles(const QString& saveName, const QString& compareWith) const;
      bool saveCompareScore(Ms::Score*, const QString& saveName, const QString& compareWith) const;
      bool saveCompareMusicXmlScore(Ms::MasterScore*, const QString& saveName, const QString& compareWith);
      bool saveCompareMimeData(QByteArray, const QString& saveName, const QString& compareWith);
      Ms::Element* writeReadElement(Ms::Element* element);
      void initMTest();
      };
}

#endif

