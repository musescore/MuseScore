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
#include "style.h"

namespace Ms {

#define MSC_VERSION     "3.00"
static constexpr int MSCVERSION = 300;

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
//
//    2.07  irregular, breakMMrest, more style options, system divider, bass string for tab (3.0)

//    3.00  (Version 3.0 alpha)


class MStyle;
class Sequencer;

enum class HairpinType : char;

#ifndef VOICES
#define VOICES 4
#endif

inline int staff2track(int staffIdx) { return staffIdx << 2; }
inline int track2staff(int voice)    { return voice >> 2;    }
inline int track2voice(int track)    { return track & 3;     }
inline int trackZeroVoice(int track) { return track & ~3;    }

static const int MAX_TAGS = 32;

static constexpr qreal INCH      = 25.4;
static constexpr qreal PPI       = 72.0;           // printer points per inch
static constexpr qreal DPI_F     = 5;
static constexpr qreal DPI       = 72.0 * DPI_F;
static constexpr qreal SPATIUM20 = 5.0 * (DPI / 72.0);
static constexpr qreal DPMM      = DPI / INCH;

static constexpr int MAX_STAVES  = 4;

static const int  SHADOW_NOTE_LIGHT       = 135;

static const char mimeSymbolFormat[]      = "application/musescore/symbol";
static const char mimeSymbolListFormat[]  = "application/musescore/symbollist";
static const char mimeStaffListFormat[]   = "application/musescore/stafflist";

static const int  VISUAL_STRING_NONE      = -100;     // no ordinal for the visual repres. of string (topmost in TAB
                                                      // varies according to visual order and presence of bass strings)
static const int  STRING_NONE             = -1;       // no ordinal for a physical string (0 = topmost in instrument)
static const int  FRET_NONE               = -1;       // no ordinal for a fret

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

enum class NoteType : unsigned char {
      NORMAL        = 0,
      ACCIACCATURA  = 0x1,
      APPOGGIATURA  = 0x2,       // grace notes
      GRACE4        = 0x4,
      GRACE16       = 0x8,
      GRACE32       = 0x10,
      GRACE8_AFTER  = 0x20,
      GRACE16_AFTER = 0x40,
      GRACE32_AFTER = 0x80,
      INVALID       = 0xFF
      };
// Q_ENUM_NS(NoteType);

constexpr NoteType operator| (NoteType t1, NoteType t2) {
      return static_cast<NoteType>(static_cast<int>(t1) | static_cast<int>(t2));
      }
constexpr bool operator& (NoteType t1, NoteType t2) {
      return static_cast<int>(t1) & static_cast<int>(t2);
      }

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

enum class NoteHeadScheme : char {
      HEAD_NORMAL = 0,
      HEAD_PITCHNAME,
      HEAD_PITCHNAME_GERMAN,
      HEAD_SOLFEGE,
      HEAD_SOLFEGE_FIXED,
      HEAD_SHAPE_NOTE_4,
      HEAD_SHAPE_NOTE_7_AIKIN,
      HEAD_SHAPE_NOTE_7_FUNK,
      HEAD_SHAPE_NOTE_7_WALKER,
      HEAD_SCHEMES
      };

//---------------------------------------------------------
//   BarLineType
//---------------------------------------------------------

enum class BarLineType {
      NORMAL           = 1,
      DOUBLE           = 2,
      START_REPEAT     = 4,
      END_REPEAT       = 8,
      BROKEN           = 0x10,
      END              = 0x20,
      DOTTED           = 0x80
      };

constexpr BarLineType operator| (BarLineType t1, BarLineType t2) {
      return static_cast<BarLineType>(static_cast<int>(t1) | static_cast<int>(t2));
      }
constexpr bool operator& (BarLineType t1, BarLineType t2) {
      return static_cast<int>(t1) & static_cast<int>(t2);
      }


// Icon() subtypes
enum class IconType : signed char {
      NONE = -1,
      ACCIACCATURA, APPOGGIATURA, GRACE4, GRACE16, GRACE32,
      GRACE8_AFTER, GRACE16_AFTER, GRACE32_AFTER,
      SBEAM, MBEAM, NBEAM, BEAM32, BEAM64, AUTOBEAM,
      FBEAM1, FBEAM2,
      VFRAME, HFRAME, TFRAME, FFRAME, MEASURE,
      BRACKETS, PARENTHESES
      };

//---------------------------------------------------------
//   MScoreError
//---------------------------------------------------------

enum MsError {
      MS_NO_ERROR,
      NO_NOTE_SELECTED,
      NO_CHORD_REST_SELECTED,
      NO_LYRICS_SELECTED,
      NO_NOTE_REST_SELECTED,
      NO_NOTE_SLUR_SELECTED,
      NO_STAFF_SELECTED,
      NO_NOTE_FIGUREDBASS_SELECTED,
      CANNOT_INSERT_TUPLET,
      CANNOT_SPLIT_TUPLET,
      CANNOT_SPLIT_MEASURE_FIRST_BEAT,
      CANNOT_SPLIT_MEASURE_TUPLET,
      NO_DEST,
      DEST_TUPLET,
      TUPLET_CROSSES_BAR,
      DEST_LOCAL_TIME_SIGNATURE,
      DEST_TREMOLO,
      NO_MIME,
      DEST_NO_CR,
      CANNOT_CHANGE_LOCAL_TIMESIG,
      };

struct MScoreError {
      MsError no;
      const char* group;
      const char* txt;
      };

//---------------------------------------------------------
//   MPaintDevice
//---------------------------------------------------------

class MPaintDevice : public QPaintDevice {

   protected:
      virtual int metric(PaintDeviceMetric m) const;

   public:
      MPaintDevice() : QPaintDevice() {}
      virtual QPaintEngine* paintEngine() const;
      virtual ~MPaintDevice() {}
      };

//---------------------------------------------------------
//   MScore
//    MuseScore application object
//---------------------------------------------------------

class MScore : public QObject {
      static MStyle _baseStyle;          // buildin initial style
      static MStyle _defaultStyle;       // buildin modified by preferences
      static MStyle* _defaultStyleForParts;

      static QString _globalShare;
      static int _hRaster, _vRaster;
      static bool _verticalOrientation;

#ifdef SCRIPT_INTERFACE
      static QQmlEngine* _qml;
#endif

      static MPaintDevice* _paintDevice;

   public:
      enum class DirectionH : char { AUTO, LEFT, RIGHT };
      enum class OrnamentStyle : char { DEFAULT, BAROQUE};

      static MsError _error;
      static std::vector<MScoreError> errorList;

      static void init();

      static const MStyle& baseStyle()             { return _baseStyle;            }
      static MStyle& defaultStyle()                { return _defaultStyle;         }
      static const MStyle* defaultStyleForParts()  { return _defaultStyleForParts; }

      static bool readDefaultStyle(QString file);
      static void setDefaultStyle(const MStyle& s) { _defaultStyle = s; }
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

// #ifndef NDEBUG
      static bool noHorizontalStretch;
      static bool noVerticalStretch;
      static bool showSegmentShapes;
      static bool showMeasureShapes;
      static bool showBoundingRect;
      static bool showCorruptedMeasures;
      static bool useFallbackFont;
      static bool autoplaceSlurs;
// #endif
      static bool debugMode;
      static bool testMode;

      static int division;
      static int sampleRate;
      static int mtcType;
      static Sequencer* seq;

      static bool saveTemplateMode;
      static bool noGui;

      static bool noExcerpts;
      static bool noImages;

      static bool pdfPrinting;
      static bool svgPrinting;
      static double pixelRatio;

      static qreal verticalPageGap;
      static qreal horizontalPageGapEven;
      static qreal horizontalPageGapOdd;

#ifdef SCRIPT_INTERFACE
      static QQmlEngine* qml();
#endif
      static MPaintDevice* paintDevice();
      virtual void endCmd() { };

      static void setError(MsError e) { _error = e; }
      static const char* errorMessage();
      static const char* errorGroup();
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

//---------------------------------------------------------
//   qml access to containers
//
//   QmlListAccess provides a convenience interface for
//   QQmlListProperty providing read-only access to plugins
//   for std::vector, QVector and QList items
//---------------------------------------------------------

template <typename T> class QmlListAccess : public QQmlListProperty<T> {
public:
      QmlListAccess<T>(QObject* obj, std::vector<T*>& container)
            : QQmlListProperty<T>(obj, &container, &stdVectorCount, &stdVectorAt) {};

      QmlListAccess<T>(QObject* obj, QVector<T*>& container)
            : QQmlListProperty<T>(obj, &container, &qVectorCount, &qVectorAt) {};

      QmlListAccess<T>(QObject* obj, QList<T*>& container)
            : QQmlListProperty<T>(obj, &container, &qListCount, &qListAt) {};

      static int stdVectorCount(QQmlListProperty<T>* l)     { return static_cast<std::vector<T*>*>(l->data)->size(); }
      static T* stdVectorAt(QQmlListProperty<T>* l, int i)  { return static_cast<std::vector<T*>*>(l->data)->at(i); }
      static int qVectorCount(QQmlListProperty<T>* l)       { return static_cast<QVector<T*>*>(l->data)->size(); }
      static T* qVectorAt(QQmlListProperty<T>* l, int i)    { return static_cast<QVector<T*>*>(l->data)->at(i); }
      static int qListCount(QQmlListProperty<T>* l)         { return static_cast<QList<T*>*>(l->data)->size(); }
      static T* qListAt(QQmlListProperty<T>* l, int i)      { return static_cast<QList<T*>*>(l->data)->at(i); }
      };

}     // namespace Ms

// Q_DECLARE_METATYPE(Ms::Direction);
// Q_DECLARE_METATYPE(Ms::MSQE_Direction::E);
// Q_DECLARE_METATYPE(Ms::Direction::E);

Q_DECLARE_METATYPE(Ms::MScore::DirectionH);
Q_DECLARE_METATYPE(Ms::BarLineType);

#endif
