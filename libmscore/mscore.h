//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __MSCORE_H__
#define __MSCORE_H__

namespace Ms {

#define MSC_VERSION     "1.24"
static const int MSCVERSION = 124;

// History:
//      1.3   added staff->_barLineSpan
//      1.5   save xoff/yoff in mm instead of pixel
//      1.6   save harmony base/root as tpc value
//      1.7   invert semantic of page fill limit
//      1.8   slur id, slur anchor in in Note
//      1.9   image size stored in mm instead of pixel
//      1.10  TextLine properties changed
//      1.11  Instrument name in part saved as TextC
//      1.12  use durationType, remove tickLen
//      1.13  Clefs: userOffset is not (mis)used for vertical layout position
// ==========1.0, 1.1 1.2
//    1.14  save user modified beam position as spatium value
//    1.15  save timesig inline; Lyrics "endTick" replaced by "ticks"
//    1.16  spanners (hairpin, trill etc.) are now inline and have no ticks anymore
//    1.17  new <Score> toplevel structure to support linked parts (excerpts)
//    1.18  save lyrics as subtype to chord/rest to allow them associated with
//          grace notes
//    1.19  replace text style numbers by text style names; box margins are now
//          used
//    1.20  instrument names are saved as html again
//    1.21  no cleflist anymore
//    1.22  timesig changed
//    1.23  measure property for actual length
//    1.24  default image size is spatium dependent
//      -   symbol numbers in TextLine() replaced by symbol names
//          TextStyle: frameWidth, paddingWidth are now in Spatium units (instead of mm)


class MStyle;
class Sequencer;

static const int VOICES = 4;
inline int staff2track(int staffIdx) { return staffIdx << 2; }
inline int track2staff(int voice)    { return voice >> 2;    }
inline int track2voice(int track)    { return track & 3;     }
inline int trackZeroVoice(int track) { return track & ~3;    }

static const int MAX_TAGS = 32;

static const qreal INCH = 25.4;
static const qreal PPI  = 72.0;           // printer points per inch
static const qreal SPATIUM20 = 5.0 / PPI; // size of Spatium for 20pt font in inch
static const int MAX_STAVES = 4;
#define MMSP(x)  Spatium((x) * .1)

static const char mimeSymbolFormat[]      = "application/mscore/symbol";
static const char mimeSymbolListFormat[]  = "application/mscore/symbollist";
static const char mimeStaffListFormat[]   = "application/mscore/stafflist";

static const int  VISUAL_STRING_NONE      = -2;       // no ordinal for the visual repres. of string (0 = topmost in TAB)
static const int  STRING_NONE             = -1;       // no ordinal for a physical string (0 = topmost in instrument)
static const int  FRET_NONE               = -1;       // no ordinal for a fret

//---------------------------------------------------------
//   ArticulationType
//---------------------------------------------------------

enum ArticulationType {
      Articulation_Fermata,
      Articulation_Shortfermata,
      Articulation_Longfermata,
      Articulation_Verylongfermata,
      Articulation_Thumb,
      Articulation_Sforzatoaccent,
      Articulation_Espressivo,
      Articulation_Staccato,
      Articulation_Staccatissimo,
      Articulation_Tenuto,
      Articulation_Portato,
      Articulation_Marcato,
      Articulation_Ouvert,
      Articulation_Plusstop,
      Articulation_Upbow,
      Articulation_Downbow,
      Articulation_Reverseturn,
      Articulation_Turn,
      Articulation_Trill,
      Articulation_Prall,
      Articulation_Mordent,
      Articulation_PrallPrall,
      Articulation_PrallMordent,
      Articulation_UpPrall,
      Articulation_DownPrall,
      Articulation_UpMordent,
      Articulation_DownMordent,
      Articulation_PrallDown,
      Articulation_PrallUp,
      Articulation_LinePrall,
      Articulation_Schleifer,
      Articulation_Snappizzicato,
      Articulation_Tapping,
      Articulation_Slapping,
      Articulation_Popping,
      ARTICULATIONS
      };


//---------------------------------------------------------
//   BracketType
//    System Brackets
//---------------------------------------------------------

enum BracketType {
      BRACKET_NORMAL, BRACKET_BRACE, BRACKET_SQUARE, BRACKET_LINE, NO_BRACKET = -1
      };

//---------------------------------------------------------
//   PlaceText
//---------------------------------------------------------

enum PlaceText {
      PLACE_AUTO, PLACE_ABOVE, PLACE_BELOW, PLACE_LEFT
      };

//---------------------------------------------------------
//   AlignmentFlags
//---------------------------------------------------------

enum AlignmentFlags {
      ALIGN_LEFT     = 0,
      ALIGN_RIGHT    = 1,
      ALIGN_HCENTER  = 2,
      ALIGN_TOP      = 0,
      ALIGN_BOTTOM   = 4,
      ALIGN_VCENTER  = 8,
      ALIGN_BASELINE = 16,
      ALIGN_CENTER = ALIGN_HCENTER | ALIGN_VCENTER,
      ALIGN_HMASK = ALIGN_LEFT | ALIGN_RIGHT | ALIGN_HCENTER,
      ALIGN_VMASK = ALIGN_TOP | ALIGN_BOTTOM | ALIGN_VCENTER | ALIGN_BASELINE
      };

//---------------------------------------------------------
//   OffsetType
//---------------------------------------------------------

enum OffsetType {
      OFFSET_ABS,       ///< offset in point units
      OFFSET_SPATIUM    ///< offset in space units
      };

//---------------------------------------------------------
//   BeamMode
//---------------------------------------------------------

enum class BeamMode {
      AUTO, BEGIN, MID, END, NONE, BEGIN32, BEGIN64, INVALID = -1
      };

#define beamModeMid(a) (a == BeamMode::MID || a == BeamMode::BEGIN32 || a == BeamMode::BEGIN64)

//---------------------------------------------------------
//   TransposeDirection
//---------------------------------------------------------

enum TransposeDirection {
      TRANSPOSE_UP, TRANSPOSE_DOWN, TRANSPOSE_CLOSEST
      };

//---------------------------------------------------------
//   TransposeMode
//---------------------------------------------------------

enum TransposeMode {
      TRANSPOSE_BY_KEY, TRANSPOSE_BY_INTERVAL, TRANSPOSE_DIATONICALLY
      };

//---------------------------------------------------------
//   SelectType
//---------------------------------------------------------

enum SelectType {
      SELECT_SINGLE, SELECT_RANGE, SELECT_ADD
      };

//---------------------------------------------------------
//   NoteType
//---------------------------------------------------------

enum NoteType {
      NOTE_NORMAL,
      NOTE_ACCIACCATURA,
      NOTE_APPOGGIATURA,       // grace notes
      NOTE_GRACE4,
      NOTE_GRACE16,
      NOTE_GRACE32,
      NOTE_INVALID
      };

//---------------------------------------------------------
//    AccidentalVal
//---------------------------------------------------------

enum AccidentalVal {
      SHARP2  = 2,
      SHARP   = 1,
      NATURAL = 0,
      FLAT    = -1,
      FLAT2   = -2
      };

//---------------------------------------------------------
//    KeySigNaturals (positions of naturals in key sig. changes)
//---------------------------------------------------------

enum KeySigNatural {
      NAT_NONE   = 0,             // no naturals, except for change to CMaj/Amin
      NAT_BEFORE = 1,             // naturals before accidentals
      NAT_AFTER  = 2              // naturals after accidentals (but always before if going sharps <=> flats)
      };

//---------------------------------------------------------
//   UpDownMode
//---------------------------------------------------------

enum UpDownMode {
      UP_DOWN_CHROMATIC, UP_DOWN_OCTAVE, UP_DOWN_DIATONIC
      };

//---------------------------------------------------------
//   StaffGroup
//---------------------------------------------------------

enum StaffGroup {
      STANDARD_STAFF_GROUP, PERCUSSION_STAFF_GROUP, TAB_STAFF_GROUP
      };
const int STAFF_GROUP_MAX = TAB_STAFF_GROUP + 1;      // out of enum to avoid compiler complains about not handled switch cases

//---------------------------------------------------------
//   Text Style Type
//    Enumerate the list of build in text styles.
//    Must be in sync with list in setDefaultStyle().
//---------------------------------------------------------

enum {
      TEXT_STYLE_UNSTYLED = -1,
      TEXT_STYLE_UNKNOWN = -2,

      TEXT_STYLE_DEFAULT = 0,
      TEXT_STYLE_TITLE,
      TEXT_STYLE_SUBTITLE,
      TEXT_STYLE_COMPOSER,
      TEXT_STYLE_POET,
      TEXT_STYLE_LYRIC1,
      TEXT_STYLE_LYRIC2,
      TEXT_STYLE_FINGERING,
      TEXT_STYLE_INSTRUMENT_LONG,
      TEXT_STYLE_INSTRUMENT_SHORT,

      TEXT_STYLE_INSTRUMENT_EXCERPT,
      TEXT_STYLE_DYNAMICS,
      TEXT_STYLE_TECHNIK,
      TEXT_STYLE_TEMPO,
      TEXT_STYLE_METRONOME,
      TEXT_STYLE_MEASURE_NUMBER,
      TEXT_STYLE_TRANSLATOR,
      TEXT_STYLE_TUPLET,
      TEXT_STYLE_SYSTEM,

      TEXT_STYLE_STAFF,
      TEXT_STYLE_HARMONY,
      TEXT_STYLE_REHEARSAL_MARK,
      TEXT_STYLE_REPEAT_LEFT,       // align to start of measure
      TEXT_STYLE_REPEAT_RIGHT,      // align to end of measure
      TEXT_STYLE_REPEAT,            // obsolete
      TEXT_STYLE_VOLTA,
      TEXT_STYLE_FRAME,
      TEXT_STYLE_TEXTLINE,
      TEXT_STYLE_GLISSANDO,

      TEXT_STYLE_STRING_NUMBER,
      TEXT_STYLE_OTTAVA,
      TEXT_STYLE_BENCH,
      TEXT_STYLE_HEADER,
      TEXT_STYLE_FOOTER,
      TEXT_STYLE_INSTRUMENT_CHANGE,
      TEXT_STYLE_LYRICS_VERSE_NUMBER,
      TEXT_STYLE_FIGURED_BASS,
      TEXT_STYLES
      };

//---------------------------------------------------------
//   BarLineType
//---------------------------------------------------------

enum BarLineType {
      NORMAL_BAR, DOUBLE_BAR, START_REPEAT, END_REPEAT,
      BROKEN_BAR, END_BAR, END_START_REPEAT, DOTTED_BAR
      };

// Icon() subtypes
enum {
      ICON_ACCIACCATURA, ICON_APPOGGIATURA, ICON_GRACE4, ICON_GRACE16, ICON_GRACE32,
      ICON_GRACE8B,
      ICON_SBEAM, ICON_MBEAM, ICON_NBEAM, ICON_BEAM32, ICON_BEAM64, ICON_AUTOBEAM,
      ICON_FBEAM1, ICON_FBEAM2,
      ICON_VFRAME, ICON_HFRAME, ICON_TFRAME, ICON_FFRAME, ICON_MEASURE,
      ICON_BRACKETS
      };

//---------------------------------------------------------
//   MScore
//    MuseScore application object
//---------------------------------------------------------

class MScore : public QObject {
      Q_OBJECT
      Q_ENUMS(ValueType)
      Q_ENUMS(Direction)
      Q_ENUMS(DirectionH)

   private:
      static MStyle* _defaultStyle;       // default modified by preferences
      static MStyle* _baseStyle;          // buildin initial style
      static QString _globalShare;
      static int _hRaster, _vRaster;

   public:
      enum ValueType  { OFFSET_VAL, USER_VAL };
      enum Direction  { AUTO, UP, DOWN };
      enum DirectionH { DH_AUTO, DH_LEFT, DH_RIGHT };

      static void init();
      static MStyle* defaultStyle();
      static MStyle* baseStyle();
      static void setDefaultStyle(MStyle*);
      static const QString& globalShare() { return _globalShare; }
      static qreal hRaster()              { return _hRaster;     }
      static qreal vRaster()              { return _vRaster;     }
      static void setHRaster(int val)     { _hRaster = val;      }
      static void setVRaster(int val)     { _vRaster = val;      }
      static void setNudgeStep10(qreal val)     { nudgeStep10 = val;      }
      static void setNudgeStep50(qreal val)     { nudgeStep50 = val;      }

      static QColor selectColor[4];
      static QColor defaultColor;
      static QColor dropColor;
      static QColor layoutBreakColor;
      static QColor frameMarginColor;
      static QColor bgColor;
      static bool warnPitchRange;

      static bool replaceFractions;
      static bool playRepeats;
      static bool panPlayback;
      static qreal nudgeStep;
      static qreal nudgeStep10;
      static qreal nudgeStep50;
      static int defaultPlayDuration;
      static QString partStyle;
      static QString lastError;
      static bool layoutDebug;

      static int division;
      static int sampleRate;
      static int mtcType;
      static Sequencer* seq;

      static qreal PDPI;
      static qreal DPI;
      static qreal DPMM;
      static bool debugMode;
      static bool testMode;
      };

static const int HEAD_TYPES = 4;

//---------------------------------------------------------
//   center
//---------------------------------------------------------

inline static qreal center(qreal x1, qreal x2) {
      return (x1 + (x2 - x1) * .5);
      }

//---------------------------------------------------------
//   restrict
//---------------------------------------------------------

inline static int restrict(int val, int min, int max) {
      if (val > max)
            return max;
      if (val < min)
            return min;
      return val;
      }

Q_DECLARE_FLAGS(Align, AlignmentFlags);
Q_DECLARE_OPERATORS_FOR_FLAGS(Align);
Q_ENUMS(DirectionH)

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::BeamMode)
Q_DECLARE_METATYPE(Ms::MScore::ValueType)
Q_DECLARE_METATYPE(Ms::MScore::Direction)
Q_DECLARE_METATYPE(Ms::MScore::DirectionH)


#endif

