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

#include "config.h"

namespace Ms {

#define MSC_VERSION     "2.07"
static constexpr int MSCVERSION = 207;

// History:
//    1.3   added staff->_barLineSpan
//    1.4   (Version 0.9)
//    1.5   save xoff/yoff in mm instead of pixel
//    1.6   save harmony base/root as tpc value
//    1.7   invert semantic of page fill limit
//    1.8   slur id, slur anchor in in Note
//    1.9   image size stored in mm instead of pixel (Versions 0.9.2 -0.9.3)
//    1.10  TextLine properties changed (Version 0.9.4)
//    1.11  Instrument name in part saved as TextC (Version 0.9.5)
//    1.12  use durationType, remove tickLen
//    1.13  Clefs: userOffset is not (mis)used for vertical layout position
//    1.14  save user modified beam position as spatium value (Versions 0.9.6 - 1.3)
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
//    2.00  (Version 2.0)
//    2.01  save SlurSegment position relative to staff
//    2.02  save instrumentId, note slashes
//    2.03  save Box topGap, bottomGap in spatium units
//    2.04  added hideSystemBarLine flag to Staff
//    2.05  breath segment changed to use tick of following chord rather than preceding chord
//    2.06  Glissando moved from final chord to start note (Version 2.0.x)
//    2.07  irregular, breakMMrest, more style options, system divider, bass string for tab (2.1)


class MStyle;
class Sequencer;

static constexpr int VOICES = 4;
inline int staff2track(int staffIdx) { return staffIdx << 2; }
inline int track2staff(int voice)    { return voice >> 2;    }
inline int track2voice(int track)    { return track & 3;     }
inline int trackZeroVoice(int track) { return track & ~3;    }

static const int MAX_TAGS = 32;

static constexpr qreal INCH      = 25.4;
static constexpr qreal PPI       = 72.0;           // printer points per inch
static constexpr qreal SPATIUM20 = 5.0;
static constexpr qreal DPI       = 72.0;
static constexpr qreal DPMM      = DPI / INCH;

static constexpr int MAX_STAVES  = 4;

static const int  SHADOW_NOTE_LIGHT       = 120;

static const char mimeSymbolFormat[]      = "application/musescore/symbol";
static const char mimeSymbolListFormat[]  = "application/musescore/symbollist";
static const char mimeStaffListFormat[]   = "application/musescore/stafflist";

static const int  VISUAL_STRING_NONE      = -100;     // no ordinal for the visual repres. of string (topmost in TAB
                                                      // varies according to visual order and presence of bass strings)
static const int  STRING_NONE             = -1;       // no ordinal for a physical string (0 = topmost in instrument)
static const int  FRET_NONE               = -1;       // no ordinal for a fret

//---------------------------------------------------------
//   Enumeration wrapper macro allowing exposure of
//   enum class to QML without manual code reproduction
//
//   In addition to using this Macro, you need to do one/all of the following:
//   - In mscore.cpp - QQmlEngine* MScore::qml()
//       qmlRegisterUncreatableType<MSQE_name>("MuseScore", 1, 0, "name", tr("You can't create an enumeration"))
//     Allows usage of name.VALUE from within a plugin as value
//   - At the bottom of the file where you use this Macro
//       Q_DECLARE_METATYPE(Ms::MSQE_name::E);
//     Allows declaring Q_PROPERTY of the type Ms::MSQE_name::E
//      Only useful if the next point is also done
//   - In mscore.cpp - void MScore::init()
//       qRegisterMetaType<MSQE_name::E>("name");
//     Allows using Ms::MSQE_name::E as return type for the READ function
//      and parameter type for the WRITE function on a Q_PROPERTY of that type
//      on the condition that you also declare Q_ENUMS(Ms::MSQE_name::E)
//      for that same Q_OBJECT
//
//   Q_INVOKABLES can't use the QML-wrapped enum type, nor the enum class type as
//    parameters. Those should be from the correct storageType and static_casts should
//    be applied when necessary.
//---------------------------------------------------------
#define MS_QML_ENUM(name, storageType, ...)\
      enum class name : storageType {\
            __VA_ARGS__\
      };\
      class MSQE_##name {\
            Q_GADGET\
            Q_ENUMS(E)\
      public:\
            enum class E : storageType {\
                  __VA_ARGS__\
            };\
      };


//---------------------------------------------------------
//   ArticulationType
//---------------------------------------------------------

enum class ArticulationType : char {
      Fermata,
      Shortfermata,
      Longfermata,
      Verylongfermata,
      Sforzatoaccent,
//      Espressivo,
      Staccato,
      Staccatissimo,
      Tenuto,
      Portato,
      Marcato,
      FadeIn,
      FadeOut,
      VolumeSwell,
      WiggleSawtooth,
      WiggleSawtoothWide,
      WiggleVibratoLargeFaster,
      WiggleVibratoLargeSlowest,
      Ouvert,
      Plusstop,
      Upbow,
      Downbow,
      Reverseturn,
      Turn,
      Trill,
      Prall,
      Mordent,
      PrallPrall,
      PrallMordent,
      UpPrall,
      DownPrall,
      UpMordent,
      DownMordent,
      PrallDown,
      PrallUp,
      LinePrall,
      Schleifer,
      Snappizzicato,
      ARTICULATIONS_PROPER,
//      Tapping,
//      Slapping,
//      Popping,
      // Fingerings
      ThumbPosition = ARTICULATIONS_PROPER,
      LuteFingThumb,
      LuteFingFirst,
      LuteFingSecond,
      LuteFingThird,
      ARTICULATIONS
      };


//---------------------------------------------------------
//   BracketType
//    System Brackets
//---------------------------------------------------------

enum class BracketType : signed char {
      NORMAL, BRACE, SQUARE, LINE, NO_BRACKET = -1
      };

//---------------------------------------------------------
//   PlaceText
//---------------------------------------------------------

enum class PlaceText : char {
      AUTO, ABOVE, BELOW, LEFT
      };

//---------------------------------------------------------
//   AlignmentFlags
//---------------------------------------------------------

enum class AlignmentFlags : char {
      LEFT     = 0,
      RIGHT    = 1,
      HCENTER  = 2,
      TOP      = 0,
      BOTTOM   = 4,
      VCENTER  = 8,
      BASELINE = 16,
      CENTER = AlignmentFlags::HCENTER | AlignmentFlags::VCENTER,
      HMASK = AlignmentFlags::LEFT | AlignmentFlags::RIGHT | AlignmentFlags::HCENTER,
      VMASK = AlignmentFlags::TOP | AlignmentFlags::BOTTOM | AlignmentFlags::VCENTER | AlignmentFlags::BASELINE
      };

//---------------------------------------------------------
//   OffsetType
//---------------------------------------------------------

enum class OffsetType : char {
      ABS,       ///< offset in point units
      SPATIUM    ///< offset in staff space units
      };

//---------------------------------------------------------
//   TransposeDirection
//---------------------------------------------------------

enum class TransposeDirection : char {
      UP, DOWN, CLOSEST
      };

//---------------------------------------------------------
//   TransposeMode
//---------------------------------------------------------

enum class TransposeMode : char {
      BY_KEY, BY_INTERVAL, DIATONICALLY
      };

//---------------------------------------------------------
//   SelectType
//---------------------------------------------------------

enum class SelectType : char {
      SINGLE, RANGE, ADD
      };

//---------------------------------------------------------
//   NoteType
//---------------------------------------------------------

enum class NoteType : char {
      NORMAL,
      ACCIACCATURA,
      APPOGGIATURA,       // grace notes
      GRACE4,
      GRACE16,
      GRACE32,
      GRACE8_AFTER,
      GRACE16_AFTER,
      GRACE32_AFTER,
      INVALID
      };

//---------------------------------------------------------
//    AccidentalVal
//---------------------------------------------------------

enum class AccidentalVal : signed char {
      SHARP2  = 2,
      SHARP   = 1,
      NATURAL = 0,
      FLAT    = -1,
      FLAT2   = -2
      };

//---------------------------------------------------------
//    KeySigNaturals (positions of naturals in key sig. changes)
//---------------------------------------------------------

enum class KeySigNatural : char {
      NONE   = 0,             // no naturals, except for change to CMaj/Amin
      BEFORE = 1,             // naturals before accidentals
      AFTER  = 2              // naturals after accidentals (but always before if going sharps <=> flats)
      };

//---------------------------------------------------------
//   UpDownMode
//---------------------------------------------------------

enum class UpDownMode : char {
      CHROMATIC, OCTAVE, DIATONIC
      };

//---------------------------------------------------------
//   StaffGroup
//---------------------------------------------------------

enum class StaffGroup : char {
      STANDARD, PERCUSSION, TAB
      };
const int STAFF_GROUP_MAX = int(StaffGroup::TAB) + 1;      // out of enum to avoid compiler complains about not handled switch cases

//---------------------------------------------------------
//   Text Style Type
//    Enumerate the list of build in text styles.
//    Must be in sync with list in setDefaultStyle().
//---------------------------------------------------------
MS_QML_ENUM(TextStyleType, signed char,\
      DEFAULT = 0,\
      TITLE,\
      SUBTITLE,\
      COMPOSER,\
      POET,\
      LYRIC1,\
      LYRIC2,\
      FINGERING,\
      LH_GUITAR_FINGERING,\
      RH_GUITAR_FINGERING,\
      \
      STRING_NUMBER,\
      INSTRUMENT_LONG,\
      INSTRUMENT_SHORT,\
      INSTRUMENT_EXCERPT,\
      DYNAMICS,\
      TECHNIQUE,\
      TEMPO,\
      METRONOME,\
      MEASURE_NUMBER,\
      TRANSLATOR,\
      \
      TUPLET,\
      SYSTEM,\
      STAFF,\
      HARMONY,\
      REHEARSAL_MARK,\
      REPEAT_LEFT,       /* align to start of measure */\
      REPEAT_RIGHT,      /* align to end of measure */\
      REPEAT,            /* obsolete */\
      VOLTA,\
      FRAME,\
      \
      TEXTLINE,\
      GLISSANDO,\
      OTTAVA,\
      PEDAL,\
      HAIRPIN,\
      BENCH,\
      HEADER,\
      FOOTER,\
      INSTRUMENT_CHANGE,\
      FIGURED_BASS,\
      \
      TEXT_STYLES\
      )

//---------------------------------------------------------
//   BarLineType
//---------------------------------------------------------

enum class BarLineType : char {
      NORMAL, DOUBLE, START_REPEAT, END_REPEAT,
      BROKEN, END, END_START_REPEAT, DOTTED
      };

// Icon() subtypes
enum class IconType : signed char {
      NONE = -1,
      ACCIACCATURA, APPOGGIATURA, GRACE4, GRACE16, GRACE32,
      GRACE8_AFTER, GRACE16_AFTER, GRACE32_AFTER,
      SBEAM, MBEAM, NBEAM, BEAM32, BEAM64, AUTOBEAM,
      FBEAM1, FBEAM2,
      VFRAME, HFRAME, TFRAME, FFRAME, MEASURE,
      BRACKETS
      };

//---------------------------------------------------------
//   MScore
//    MuseScore application object
//---------------------------------------------------------

class MScore : public QObject {
      Q_OBJECT

      static MStyle* _defaultStyle;       // buildin modified by preferences
      static MStyle* _defaultStyleForParts;

      static MStyle* _baseStyle;          // buildin initial style
      static QString _globalShare;
      static int _hRaster, _vRaster;
      static bool _verticalOrientation;

#ifdef SCRIPT_INTERFACE
      static QQmlEngine* _qml;
#endif

   public:
      enum class Direction  : char { AUTO, UP, DOWN };
      enum class DirectionH : char { AUTO, LEFT, RIGHT };
      enum class OrnamentStyle : char { DEFAULT, BAROQUE};
      enum class GlissandoStyle : char { CHROMATIC, WHITE_KEYS, BLACK_KEYS, DIATONIC };
      Q_ENUMS(Direction DirectionH OrnamentStyle GlissandoStyle)

      static void init();

      static MStyle* defaultStyle();
      static MStyle* defaultStyleForParts();
      static MStyle* baseStyle();
      static void setDefaultStyle(MStyle*);
      static void defaultStyleForPartsHasChanged();

      static const QString& globalShare()   { return _globalShare; }
      static qreal hRaster()                { return _hRaster;     }
      static qreal vRaster()                { return _vRaster;     }
      static void setHRaster(int val)       { _hRaster = val;      }
      static void setVRaster(int val)       { _vRaster = val;      }
      static void setNudgeStep(qreal val)   { nudgeStep = val;     }
      static void setNudgeStep10(qreal val) { nudgeStep10 = val;   }
      static void setNudgeStep50(qreal val) { nudgeStep50 = val;   }

      static bool verticalOrientation()            { return _verticalOrientation; }
      static void setVerticalOrientation(bool val) { _verticalOrientation = val;  }

      static QColor selectColor[4];
      static QColor defaultColor;
      static QColor dropColor;
      static QColor layoutBreakColor;
      static QColor frameMarginColor;
      static QColor bgColor;
      static bool warnPitchRange;

      static bool playRepeats;
      static bool panPlayback;
      static qreal nudgeStep;
      static qreal nudgeStep10;
      static qreal nudgeStep50;
      static int defaultPlayDuration;
      static QString lastError;
      static bool layoutDebug;

      static int division;
      static int sampleRate;
      static int mtcType;
      static Sequencer* seq;

      static bool debugMode;
      static bool testMode;
      static bool saveTemplateMode;
      static bool noGui;

      static bool noExcerpts;
      static bool noImages;

      static bool pdfPrinting;

      static qreal verticalPageGap;
      static qreal horizontalPageGapEven;
      static qreal horizontalPageGapOdd;

#ifdef SCRIPT_INTERFACE
      static QQmlEngine* qml();
#endif
      virtual void endCmd() { };
      };

//---------------------------------------------------------
//   center
//---------------------------------------------------------

inline static qreal center(qreal x1, qreal x2)
      {
      return (x1 + (x2 - x1) * .5);
      }

//---------------------------------------------------------
//   limit
//---------------------------------------------------------

inline static int limit(int val, int min, int max)
      {
      if (val > max)
            return max;
      if (val < min)
            return min;
      return val;
      }

Q_DECLARE_FLAGS(Align, AlignmentFlags);
Q_DECLARE_OPERATORS_FOR_FLAGS(Align);

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::MScore::Direction);
Q_DECLARE_METATYPE(Ms::MScore::DirectionH);

#endif
