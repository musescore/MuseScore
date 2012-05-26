//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: score.cpp 5149 2011-12-29 08:38:43Z wschweer $
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

class Element;

class MScore;
class Score;

//---------------------------------------------------------
//   MTest
//---------------------------------------------------------

class MTest {
   protected:
      MScore* mscore;
      QString root;     // root path of test source
      Score* score;

      MTest();
      Score* readScore(const QString& name);
      Score* readCreatedScore(const QString& name);
      bool saveScore(Score*, const QString& name);
      bool savePdf(Score*, const QString& name);
      bool saveCompareScore(Score*, const QString& saveName, const QString& compareWith);
      Element* writeReadElement(Element* element);
      void initMTest();
      };

#endif

