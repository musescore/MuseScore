//=============================================================================
//  BWW to MusicXML converter
//  Part of MusE Score
//  Linux Music Score Editor
//  $Id: parser.h 4873 2011-10-19 19:33:04Z lvinken $
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef BWW2MXML_PARSER_H
#define BWW2MXML_PARSER_H

/**
 \file
 Definition of class Parser
 */

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVector>

#include "writer.h"

namespace Bww {

  class Lexer;

  struct NoteDescription
  {
    QString pitch;
    QString beam;
    QString type;
    int dots;
    bool tieStart;
    bool tieStop;
    StartStop triplet;
    bool grace;
    StartStop beamState;
    QVector<BeamType> beamList;
    NoteDescription(const QString _pitch, const QString _beam,
                    const QString _type, const int _dots,
                    bool _tieStart = false, bool _tieStop = false,
                    StartStop _triplet = ST_NONE,
                    bool _grace = false)
                      : pitch(_pitch), beam(_beam),
                      type(_type), dots(_dots),
                      tieStart(_tieStart), tieStop(_tieStop),
                      triplet(_triplet),
                      grace(_grace),
                      beamState(ST_NONE),
                      beamList(maxBeamLevel, BM_NONE)
    {}
  };

  struct MeasureDescription
  {
    MeasureBeginFlags mbf;
    QList<NoteDescription> notes;
    MeasureEndFlags mef;
    int duration;
    MeasureDescription()
      : duration(0)
    {}
  };

  /**
   The bww parser.
   */

  class Parser
  {
  public:
    Parser(Lexer& l, Writer& w);
    void parse();
  private:
    void errorHandler(QString s);
    void parseBar(Bww::MeasureEndFlags& mef);
    void parseNote();
    void parseGraces();
    void parsePart(Bww::MeasureBeginFlags& mbf, Bww::MeasureEndFlags& mef);
    void parseSeqNonNotes();
    void parseSeqNotes();
    void parseString();
    void parseTempo();
    void parseTSig();
    Lexer& lex;                         ///< The lexer
    Writer& wrt;                        ///< The writer
    QString title;                      ///< Title read from the header
    QString type;                       ///< Type read from the header
    QString composer;                   ///< Composer read from the header
    QString footer;                     ///< Footer read from the header
    int tempo;                          ///< Tune tempo read from the header
    int beat;                           ///< Beat type, read from the clef line
    int beats;                          ///< Number of beats, read from the clef line
    //bool inMeasure;                     ///< State: writing the notes in a measure
    //int measureNr;                      ///< Current measure number
    bool tieStart;                      ///< Tie start pending
    bool inTie;                         ///< In a tie
    bool tripletStart;                  ///< Triplet start pending
    bool inTriplet;                     ///< In a triplet
    QList<MeasureDescription> measures; ///< Parsed measures
    bool tsigFound;                     ///< A valid time signature was found
  };

} // namespace Bww

#endif // BWW2MXML_PARSER_H
