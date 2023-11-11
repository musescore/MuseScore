//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __IMPORTMXMLNOTEDURATION_H__
#define __IMPORTMXMLNOTEDURATION_H__

#include "libmscore/durationtype.h"
#include "libmscore/fraction.h"
#include "importmxmlpass1.h"

namespace Ms {

class MxmlLogger;

//---------------------------------------------------------
//   mxmlNoteDuration
//---------------------------------------------------------

/**
 Parse the note time related part of the /score-partwise/part/measure/note node.
 */

class mxmlNoteDuration
      {
public:
      mxmlNoteDuration(int divs, MxmlLogger* logger, MusicXMLParserPass1* pass1) :
          _divs(divs), _logger(logger), _pass1(pass1) { /* nothing so far */ }
      QString checkTiming(const QString& type, const bool rest, const bool grace);
      Fraction duration() const { return _dura; } // duration to use
      Fraction calculatedDuration() const { return _calcDura; }   // value calculated from note type etcetera
      Fraction specifiedDuration() const { return _specDura; }    // value read from the duration element
      int dots() const { return _dots; }
      TDuration normalType() const { return _normalType; }
      bool readProperties(QXmlStreamReader& e);
      Fraction timeMod() const { return _timeMod; }

private:
      void duration(QXmlStreamReader& e);
      void timeModification(QXmlStreamReader& e);
      const int _divs;                                // the current divisions value
      int _dots = 0;
      Fraction _calcDura;
      Fraction _specDura;
      Fraction _dura;
      TDuration _normalType;
      Fraction _timeMod { 1, 1 };                     // default to no time modification
      MxmlLogger* _logger;                            ///< Error logger
      MusicXMLParserPass1* _pass1;
      };

} // namespace Ms

#endif
