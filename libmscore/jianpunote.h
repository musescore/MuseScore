//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __JIANPUNOTE_H__
#define __JIANPUNOTE_H__

/**
 \file
 Definition of Jianpu (numbered notation) Note class.
*/

#include "element.h"
#include "symbol.h"
#include "pitchspelling.h"
#include "shape.h"
#include "note.h"
#include "durationtype.h"

class QPainter;

namespace Ms {

class Note;
class Chord;
class Score;
class Sym;
class StaffType;
class TDuration;
enum class SymId;


//---------------------------------------------------------------------------------------
//   @@ JianpuNote
///    Graphical representation of a Jianpu (numbered notation) note.
//
//---------------------------------------------------------------------------------------

class JianpuNote : public Note {
   public:
      // Baseline in Jianpu Staff (barline span: -4 to +4) to draw top of note/rest number on.
      static const int NOTE_BASELINE = -1;

      // Ratios used to reduce width and height of font bounding-box returned by QFontMetricsF.
      static const float FONT_BBOX_WIDTH_RATIO;
      static const float FONT_BBOX_HEIGHT_RATIO;

      static const int MAX_OCTAVE_DOTS = 4;
      static const int MAX_OCTAVE_COLS = 2;
      static const int MAX_OCTAVE_ROWS = 2;
      static const int OCTAVE_DOT_WIDTH = 10;
      static const int OCTAVE_DOT_HEIGHT = 10;
      static const int OCTAVE_DOT_X_SPACE = 3;  // Horizontal Space between octave dots
      static const int OCTAVE_DOT_Y_SPACE = 3;  // Vertical Space between octave dots
      static const int OCTAVE_DOT_COL_WIDTH = OCTAVE_DOT_WIDTH + OCTAVE_DOT_X_SPACE;
      static const int OCTAVE_DOT_ROW_HEIGHT = OCTAVE_DOT_HEIGHT + OCTAVE_DOT_Y_SPACE;
      static const int OCTAVE_DOTBOX_WIDTH = MAX_OCTAVE_COLS * OCTAVE_DOT_COL_WIDTH; // For 2x2 dots
      static const int OCTAVE_DOTBOX_HEIGHT = MAX_OCTAVE_ROWS * OCTAVE_DOT_ROW_HEIGHT; // For 2x2 dots
      static const int OCTAVE_DOTBOX_Y_OFFSET = 10;  // Y-offset between octave dot and note number boxes
      static const int DURATION_DASH_X_SPACE = 30;   // Horizontal Space between duration dashes
      static const int DURATION_DASH_WIDTH = 30;
      static const int DURATION_DASH_HEIGHT = 8;
      static const int DURATION_DOT_WIDTH = 10;
      static const int DURATION_DOT_HEIGHT = 10;
      static const int DURATION_DOT_X_SPACE = 15;    // Horizontal Space between duration dots
      static const int DURATION_DOT_X_OFFSET = -20;  // Adjustment of duration dot's x-position

      static const int BEAM_HEIGHT = 4;
      static const int BEAM_Y_SPACE = 8;       // Vertical Space between beams
      //static const int BEAM_Y_OFFSET = 10;     // Y-offset between note(octave-dots)/rest and beam

      JianpuNote(Score* score = 0);
      JianpuNote(const Note& note, bool link = false);
      JianpuNote(const JianpuNote& note, bool link = false);
      virtual ~JianpuNote();

      JianpuNote& operator=(const JianpuNote& note) = delete;
      virtual JianpuNote* clone() const override  { return new JianpuNote(*this, false); }

      virtual void setTpc(int v) override;
      virtual void setPitch(int val) override;

      virtual void read(XmlReader& xml) override;
      virtual bool readProperties(XmlReader& xml) override;
      virtual void write(XmlWriter& xml) const override;

      virtual void setDotY(Direction) override;
      virtual void layout() override;
      virtual void layout2() override;
      virtual void draw(QPainter*) const override;

      void setNoteByNumber(int number, int octave, TDuration::DurationType duration);
      void setNoteByPitch(int pitch, int tpc, TDuration::DurationType duration);

      int noteNumber() const { return _noteNumber; }
      int noteOctave() const { return _noteOctave; }
      int durationDashCount() const { return _durationDashCount; }
      const QRectF& noteNumberBox() const { return _noteNumberBox; }
      const QRectF& octaveDotBox() const { return _octaveDotBox; }
      const QRectF& durationDashBox() const { return _durationDashBox; }

   protected:
      void initialize();
      int pitch2octaveByKey(int pitch, Key key);
      int tpc2numberNoteByKey(int tpc, Key key);
      int calcDashCount(TDuration::DurationType duration);

   private:
      int _noteNumber;            ///< Jianpu note number (1 to 7).
      int _noteOctave;            ///< Octave number of note: 0 (middle octave, octave #4),
                                  ///<     negative (lower octaves), positive (upper octaves)
      int _durationDashCount;     ///< Number of duration dashes ("-")
      QRectF _noteNumberBox;      ///< Bounding box for note number
      QRectF _octaveDotBox;       ///< Bounding box for lower or uppper octave dots
      QRectF _durationDashBox;    ///< Bounding box for duration dashes
      };

}     // namespace Ms

#endif

