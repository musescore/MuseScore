//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __MSCORE_H__
#define __MSCORE_H__

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


class MStyle;
class Sequencer;

static const int VOICES = 4;
static const int MAX_TAGS = 32;

static const qreal INCH = 25.4;
static const qreal PPI  = 72.0;           // printer points per inch
static const qreal SPATIUM20 = 5.0 / PPI; // size of Spatium for 20pt font in inch
static const int MAX_STAVES = 4;

static const char mimeSymbolFormat[]      = "application/mscore/symbol";
static const char mimeSymbolListFormat[]  = "application/mscore/symbollist";
static const char mimeStaffListFormat[]   = "application/mscore/stafflist";

//---------------------------------------------------------
//   ElementType
//    The value of this enum determines the "stacking order"
//    of elements on the canvas.
//---------------------------------------------------------

enum ElementType {
      INVALID = 0,
      SYMBOL  = 1,
      TEXT,
      INSTRUMENT_NAME,
      SLUR_SEGMENT,
      BAR_LINE,
      STEM_SLASH,
      LINE,
      BRACKET,
      ARPEGGIO,
      ACCIDENTAL,
      NOTE,
      STEM,
      CLEF,
      KEYSIG,
      TIMESIG,
      REST,
      BREATH,
      GLISSANDO,
      REPEAT_MEASURE,
      IMAGE,
/*19*/TIE,
      ARTICULATION,
      CHORDLINE,
      DYNAMIC,
      BEAM,
      HOOK,
      LYRICS,
      FIGURED_BASS,
      MARKER,
      JUMP,
      FINGERING,
      TUPLET,
/*30*/TEMPO_TEXT,
      STAFF_TEXT,
      REHEARSAL_MARK,
      INSTRUMENT_CHANGE,
      HARMONY,
      FRET_DIAGRAM,
      BEND,
      TREMOLOBAR,
      VOLTA,
      HAIRPIN_SEGMENT,
      OTTAVA_SEGMENT,
      TRILL_SEGMENT,
      TEXTLINE_SEGMENT,
      VOLTA_SEGMENT,
      LAYOUT_BREAK,
      SPACER,
      STAFF_STATE,
      LEDGER_LINE,
      NOTEHEAD,
      NOTEDOT,
      TREMOLO,
      MEASURE,
      STAFF_LINES,
      SELECTION,
      LASSO,
      SHADOW_NOTE,
      RUBBERBAND,
      TAB_DURATION_SYMBOL,
      FSYMBOL,
      PAGE,

      // not drawable elements:
      HAIRPIN,
      OTTAVA,
      PEDAL,
      TRILL,
      TEXTLINE,
      SEGMENT,
      SYSTEM,
      COMPOUND,
      CHORD,
      SLUR,

      // special types for drag& drop:
      ELEMENT,
      ELEMENT_LIST,
      STAFF_LIST,
      MEASURE_LIST,
      LAYOUT,

      HBOX,
      VBOX,
      TBOX,
      FBOX,
      ACCIDENTAL_BRACKET,
      ICON,
      OSSIA,

      MAXTYPE
      };

//---------------------------------------------------------
//   ValueType
//    used for Note->velocity
//---------------------------------------------------------

enum ValueType {
      OFFSET_VAL, USER_VAL
      };

//---------------------------------------------------------
//   Direction
//    used for stem and slur
//---------------------------------------------------------

enum Direction  {
      AUTO, UP, DOWN
      };

//---------------------------------------------------------
//   DirectionH
//    used for note head mirror
//---------------------------------------------------------

enum DirectionH {
      DH_AUTO, DH_LEFT, DH_RIGHT
      };

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
//   NoteHeadGroup
//---------------------------------------------------------

enum NoteHeadGroup {
      HEAD_NORMAL, HEAD_CROSS, HEAD_DIAMOND, HEAD_TRIANGLE, HEAD_MI,
      HEAD_SLASH, HEAD_XCIRCLE, HEAD_DO, HEAD_RE, HEAD_FA, HEAD_LA, HEAD_TI,
      HEAD_SOL,
      HEAD_BREVIS_ALT,
      HEAD_GROUPS,
      HEAD_INVALID = -1
      };

static const int HEAD_TYPES = 4;

//---------------------------------------------------------
//   NoteHeadType
//---------------------------------------------------------

enum NoteHeadType {
      HEAD_AUTO, HEAD_WHOLE, HEAD_HALF, HEAD_QUARTER, HEAD_BREVIS
      };

//---------------------------------------------------------
//   BracketType
//    System Brackets
//---------------------------------------------------------

enum BracketType {
      BRACKET_NORMAL, BRACKET_AKKOLADE, NO_BRACKET = -1
      };

//---------------------------------------------------------
//   Anchor
//    used for Spanner elements
//---------------------------------------------------------

enum Anchor {
      ANCHOR_SEGMENT, ANCHOR_MEASURE, ANCHOR_CHORD, ANCHOR_NOTE
      };

//---------------------------------------------------------
//   Placement
//---------------------------------------------------------

enum Placement {
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

Q_DECLARE_FLAGS(Align, AlignmentFlags);
Q_DECLARE_OPERATORS_FOR_FLAGS(Align);

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

enum BeamMode {
      BEAM_AUTO    = 0,
      BEAM_BEGIN   = 0x01,
      BEAM_MID     = 0x02,
      BEAM_END     = 0x04,
      BEAM_NO      = 0x08,
      BEAM_BEGIN32 = 0x10,
      BEAM_BEGIN64 = 0x20,
      BEAM_INVALID = -1
      };

#define beamModeMid(a) (a & (BEAM_MID | BEAM_BEGIN32 | BEAM_BEGIN64))

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
      TRANSPOSE_BY_KEY, TRANSPOSE_BY_INTERVAL
      };

//---------------------------------------------------------
//   DynamicType
//---------------------------------------------------------

enum DynamicType {
      DYNAMIC_STAFF, DYNAMIC_PART, DYNAMIC_SYSTEM
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
//    Accidental Values
//---------------------------------------------------------

enum AccidentalType {
      ACC_NONE,
      ACC_SHARP,
      ACC_FLAT,
      ACC_SHARP2,
      ACC_FLAT2,
      ACC_NATURAL,

      ACC_FLAT_SLASH,
      ACC_FLAT_SLASH2,
      ACC_MIRRORED_FLAT2,
      ACC_MIRRORED_FLAT,
      ACC_MIRRIRED_FLAT_SLASH,
      ACC_FLAT_FLAT_SLASH,

      ACC_SHARP_SLASH,
      ACC_SHARP_SLASH2,
      ACC_SHARP_SLASH3,
      ACC_SHARP_SLASH4,

      ACC_SHARP_ARROW_UP,
      ACC_SHARP_ARROW_DOWN,
      ACC_SHARP_ARROW_BOTH,
      ACC_FLAT_ARROW_UP,
      ACC_FLAT_ARROW_DOWN,
      ACC_FLAT_ARROW_BOTH,
      ACC_NATURAL_ARROW_UP,
      ACC_NATURAL_ARROW_DOWN,
      ACC_NATURAL_ARROW_BOTH,
      ACC_SORI,
      ACC_KORON,
      ACC_END
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
      PITCHED_STAFF, PERCUSSION_STAFF, TAB_STAFF
      };

//---------------------------------------------------------
//   ClefType
//---------------------------------------------------------

enum ClefType {
      CLEF_INVALID = -1,
      CLEF_G = 0,
      CLEF_G1,
      CLEF_G2,
      CLEF_G3,
      CLEF_F,
      CLEF_F8,
      CLEF_F15,
      CLEF_F_B,
      CLEF_F_C,
      CLEF_C1,
      CLEF_C2,
      CLEF_C3,
      CLEF_C4,
      CLEF_TAB,
      CLEF_PERC,
      CLEF_C5,
      CLEF_G4,
      CLEF_F_8VA,
      CLEF_F_15MA,
      CLEF_PERC2,
      CLEF_TAB2,
      CLEF_MAX
      };

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
      TEXT_STYLE_DYNAMICS2,
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
//   SegmentType
//---------------------------------------------------------

enum SegmentType {
      SegClef                 = 0x1,
      SegKeySig               = 0x2,
      SegTimeSig              = 0x4,
      SegStartRepeatBarLine   = 0x8,
      SegBarLine              = 0x10,
      SegGrace                = 0x20,
      SegChordRest            = 0x40,
      SegBreath               = 0x80,
      SegEndBarLine           = 0x100,
      SegTimeSigAnnounce      = 0x200,
      SegKeySigAnnounce       = 0x400,
      SegAll                  = 0xfff
      };
typedef QFlags<SegmentType> SegmentTypes;
Q_DECLARE_OPERATORS_FOR_FLAGS(SegmentTypes)

//---------------------------------------------------------
//   BarLineType
//---------------------------------------------------------

enum BarLineType {
      NORMAL_BAR, DOUBLE_BAR, START_REPEAT, END_REPEAT,
      BROKEN_BAR, END_BAR, END_START_REPEAT
      };

// Icon() subtypes
enum {
      ICON_ACCIACCATURA, ICON_APPOGGIATURA, ICON_GRACE4, ICON_GRACE16, ICON_GRACE32,
      ICON_GRACE8B,
      ICON_SBEAM, ICON_MBEAM, ICON_NBEAM, ICON_BEAM32, ICON_BEAM64, ICON_AUTOBEAM,
      ICON_FBEAM1, ICON_FBEAM2,
      ICON_VFRAME, ICON_HFRAME, ICON_TFRAME, ICON_FFRAME, ICON_MEASURE
      };

//---------------------------------------------------------
//   NoteVal
//    helper structure
//---------------------------------------------------------

struct NoteVal {
      int pitch;
      int fret;
      int string;
      NoteHeadGroup headGroup;
      NoteVal() { pitch = -1; fret = -1; string = -1; headGroup = HEAD_NORMAL; }
      };

//---------------------------------------------------------
//   MScore
//    MuseScore application object
//---------------------------------------------------------

class MScore : public QObject {
      Q_OBJECT
      Q_ENUMS(ElementType)
      Q_ENUMS(ValueType)
      Q_ENUMS(Direction)
      Q_ENUMS(DirectionH)
      Q_ENUMS(SegmentType)

   private:
      static MStyle* _defaultStyle;       // default modified by preferences
      static MStyle* _baseStyle;          // buildin initial style
      static QString _globalShare;
      static int _hRaster, _vRaster;

   public:
       enum ElementType {
            INVALID = 0,
            SYMBOL  = 1,
            TEXT,
            INSTRUMENT_NAME,
            SLUR_SEGMENT,
            BAR_LINE,
            STEM_SLASH,
            LINE,
            BRACKET,
            ARPEGGIO,
            ACCIDENTAL,
            NOTE,
            STEM,
            CLEF,
            KEYSIG,
            TIMESIG,
            REST,
            BREATH,
            GLISSANDO,
            REPEAT_MEASURE,
            IMAGE,
            TIE,
            ARTICULATION,
            CHORDLINE,
            DYNAMIC,
            BEAM,
            HOOK,
            LYRICS,
            FIGURED_BASS,
            MARKER,
            JUMP,
            FINGERING,
            TUPLET,
            TEMPO_TEXT,
            STAFF_TEXT,
            REHEARSAL_MARK,
            INSTRUMENT_CHANGE,
            HARMONY,
            FRET_DIAGRAM,
            BEND,
            TREMOLOBAR,
            VOLTA,
            HAIRPIN_SEGMENT,
            OTTAVA_SEGMENT,
            TRILL_SEGMENT,
            TEXTLINE_SEGMENT,
            VOLTA_SEGMENT,
            LAYOUT_BREAK,
            SPACER,
            STAFF_STATE,
            LEDGER_LINE,
            NOTEHEAD,
            NOTEDOT,
            TREMOLO,
            MEASURE,
            STAFF_LINES,
            SELECTION,
            LASSO,
            SHADOW_NOTE,
            RUBBERBAND,
            TAB_DURATION_SYMBOL,
            FSYMBOL,
            PAGE,
            HAIRPIN,
            OTTAVA,
            PEDAL,
            TRILL,
            TEXTLINE,
            SEGMENT,
            SYSTEM,
            COMPOUND,
            CHORD,
            SLUR,
            ELEMENT,
            ELEMENT_LIST,
            STAFF_LIST,
            MEASURE_LIST,
            LAYOUT,
            HBOX,
            VBOX,
            TBOX,
            FBOX,
            ACCIDENTAL_BRACKET,
            ICON,
            OSSIA,

            MAXTYPE
            };

      enum ValueType  { OFFSET_VAL, USER_VAL };
      enum Direction  { AUTO, UP, DOWN };
      enum DirectionH { DH_AUTO, DH_LEFT, DH_RIGHT };
      enum SegmentType {
            SegClef                 = 0x1,
            SegKeySig               = 0x2,
            SegTimeSig              = 0x4,
            SegStartRepeatBarLine   = 0x8,
            SegBarLine              = 0x10,
            SegGrace                = 0x20,
            SegChordRest            = 0x40,
            SegBreath               = 0x80,
            SegEndBarLine           = 0x100,
            SegTimeSigAnnounce      = 0x200,
            SegKeySigAnnounce       = 0x400,
            SegAll                  = 0xfff
            };

      static void init();
      static MStyle* defaultStyle();
      static MStyle* baseStyle();
      static void setDefaultStyle(MStyle*);
      static const QString& globalShare() { return _globalShare; }
      static qreal hRaster()              { return _hRaster;     }
      static qreal vRaster()              { return _vRaster;     }
      static void setHRaster(int val)     { _hRaster = val;      }
      static void setVRaster(int val)     { _vRaster = val;      }

      static QColor selectColor[4];
      static QColor defaultColor;
      static QColor dropColor;
      static QColor layoutBreakColor;
      static QColor bgColor;
      static bool warnPitchRange;

      static bool replaceFractions;
      static bool playRepeats;
      static bool panPlayback;
      static qreal nudgeStep;
      static int defaultPlayDuration;
      static QString partStyle;
      static QString soundFont;
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
      };

Q_DECLARE_METATYPE(MScore::ElementType)
Q_DECLARE_METATYPE(MScore::ValueType)
Q_DECLARE_METATYPE(MScore::Direction)
Q_DECLARE_METATYPE(MScore::DirectionH)
Q_DECLARE_METATYPE(MScore::SegmentType)

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

#endif

