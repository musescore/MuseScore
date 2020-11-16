//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2015 Werner Schweer & others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __STAFFTYPE_H__
#define __STAFFTYPE_H__

#include "element.h"
#include "spatium.h"
#include "mscore.h"
#include "durationtype.h"
#include "note.h"

namespace Ms {

class Chord;
class ChordRest;
class Staff;
class XmlWriter;

// all in spatium units
#define STAFFTYPE_TAB_DEFAULTSTEMLEN_UP   3.0
#define STAFFTYPE_TAB_DEFAULTSTEMDIST_UP  1.0
#define STAFFTYPE_TAB_DEFAULTSTEMPOSY_UP  -STAFFTYPE_TAB_DEFAULTSTEMDIST_UP
#define STAFFTYPE_TAB_DEFAULTSTEMLEN_DN   3.0
#define STAFFTYPE_TAB_DEFAULTSTEMDIST_DN  1.0
#define STAFFTYPE_TAB_DEFAULTSTEMPOSY_DN  STAFFTYPE_TAB_DEFAULTSTEMDIST_DN
#define STAFFTYPE_TAB_DEFAULTSTEMLEN_THRU 3.5
#define STAFFTYPE_TAB_DEFAULTSTEMPOSX     0.75
#define STAFFTYPE_TAB_DEFAULTDOTDIST_X    0.75

// TAB STEM NOTATION
// the ratio between the length of a full stem and the length of a short stem
// (used for half note stems, in some TAB styles)
#define STAFFTYPE_TAB_SHORTSTEMRATIO      0.5
// metrics of slashes through half note stems
#define STAFFTYPE_TAB_SLASH_WIDTH         1.2   /* X width of half note slash */
#define STAFFTYPE_TAB_SLASH_SLANTY        0.8   /* the Y coord of the slash slant */
#define STAFFTYPE_TAB_SLASH_THICK         0.4   /* slash thickness */
#define STAFFTYPE_TAB_SLASH_DISPL         0.8   /* the total displacement between one slash and the next:
                                                      includes slash thickness and empty space between slashes*/
// the total height of a double slash
#define STAFFTYPE_TAB_SLASH_2TOTHEIGHT     (STAFFTYPE_TAB_SLASH_THICK+STAFFTYPE_TAB_SLASH_DISPL+STAFFTYPE_TAB_SLASH_SLANTY)
// the initial Y coord for a double shash on an UP stem = topmost corner of topmost slash
#define STAFFTYPE_TAB_SLASH_2STARTY_UP     ((STAFFTYPE_TAB_DEFAULTSTEMLEN_UP-STAFFTYPE_TAB_SLASH_2TOTHEIGHT)*0.5)
// the initial Y coord for a double shash on an DN stem = topmost corner of topmost slash
#define STAFFTYPE_TAB_SLASH_2STARTY_DN     ((STAFFTYPE_TAB_DEFAULTSTEMLEN_UP+STAFFTYPE_TAB_SLASH_2TOTHEIGHT)*0.5)
// same for a 4-ple slash
#define STAFFTYPE_TAB_SLASH_4TOTHEIGHT     (STAFFTYPE_TAB_SLASH_THICK+STAFFTYPE_TAB_SLASH_DISPL*3+STAFFTYPE_TAB_SLASH_SLANTY)
// the initial Y coord for a double shash on an UP stem = topmost corner of topmost slash
#define STAFFTYPE_TAB_SLASH_4STARTY_UP     ((STAFFTYPE_TAB_DEFAULTSTEMLEN_UP-STAFFTYPE_TAB_SLASH_4TOTHEIGHT)*0.5)
// the initial Y coord for a double shash on an DN stem = topmost corner of topmost slash
#define STAFFTYPE_TAB_SLASH_4STARTY_DN     ((STAFFTYPE_TAB_DEFAULTSTEMLEN_UP+STAFFTYPE_TAB_SLASH_4TOTHEIGHT)*0.5)

// HISTORIC TAB BASS STRING NOTATION
// The following constants refer to the specifics of bass string notation in historic
//    (Renaiss./Baroque French and Italian) tablatures.

// how much to lower a bass string note with slashes with respect to line distance (in fraction of line distance)
#define STAFFTYPE_TAB_BASSSLASH_YOFFSET   0.33
// The following constants could ideially be customizeable values;
//    they are currently constants to simplify implementation;
// Note that these constants do not constrain which strings of an instrument are
//    physically frettable (which is defined in the instrument itself) but fix the
//    number of bass strings for which the notation is able to express a fret number
//    rather than simply a string ordinal.
#define NUM_OF_BASSSTRINGS_WITH_LETTER    4     // the max number of bass strings frettable with letter notation (French)
#define NUM_OF_BASSSTRINGS_WITH_NUMBER    2     // the max number of bass strings frettable with number notation (Italian)

//---------------------------------------------------------
//   TablatureFont
//---------------------------------------------------------

#define NUM_OF_DIGITFRETS                 100   // the max fret number which can be rendered with numbers
#define NUM_OF_LETTERFRETS                17    // the max fret number which can be rendered with letters
#define NUM_OF_BASSSTRING_SLASHES         5     // the max number of slashes supported for French bass strings notation
                                                // (currently, only 3 slashes are used at most; another two are
                                                // foreseen for future customizability)

// default values for 'grid'-like beaming to use with value symbols in stemless TAB
static const qreal GRID_BEAM_DEF_WIDTH  = 0.25; // all values in sp
static const qreal GRID_STEM_DEF_HEIGHT = 1.75;
static const qreal GRID_STEM_DEF_WIDTH  = 0.125;

struct TablatureFretFont {
      QString family;                           // the family of the physical font to use
      QString displayName;                      // the name to display to the user
      qreal   defPitch;                         // the default size of the font
      qreal   defYOffset;                       // the default Y displacement
      QChar   xChar;                            // the char to use for 'x'
      QChar   ghostChar;                        // the char to use for ghost notes
      QString slashChar[NUM_OF_BASSSTRING_SLASHES];// the char used to draw one or more '/' symbols
      QString displayDigit[NUM_OF_DIGITFRETS];  // the string to draw for digit frets
      QChar   displayLetter[NUM_OF_LETTERFRETS];// the char to use for letter frets

      bool read(XmlReader&);
      };

enum class TabVal : char {
      VAL_LONGA = 0,
      VAL_BREVIS,
      VAL_SEMIBREVIS,
      VAL_MINIMA,
      VAL_SEMIMINIMA,
      VAL_FUSA,
      VAL_SEMIFUSA,
      VAL_32,
      VAL_64,
      VAL_128,
      VAL_256,
      VAL_512,
      VAL_1024,
      NUM_OF
      };

enum class TablatureMinimStyle : char {
      NONE = 0,                       // do not draw half notes at all
      SHORTER,                        // draw half notes with a shorter stem
      SLASHED                         // draw half notes with stem with two slashes
      };

enum class TablatureSymbolRepeat : char {
      NEVER = 0,                    // never repeat the same duration symbol
      SYSTEM,                       // repeat at the beginning of a new system
      MEASURE,                      // repeat at the beginning of a new measure
      ALWAYS                        // always repeat
      };

struct TablatureDurationFont {
      QString family;                 // the family of the physical font to use
      QString displayName;            // the name to display to the user
      qreal   defPitch;               // the default size of the font
      qreal   defYOffset;             // the default Y displacement
      qreal   gridBeamWidth  = GRID_BEAM_DEF_WIDTH;   // the width of the 'grid'-style beam (in sp)
      qreal   gridStemHeight = GRID_STEM_DEF_HEIGHT;  // the height of the 'grid'-style stem (in sp)
      qreal   gridStemWidth  = GRID_STEM_DEF_WIDTH;   // the width of the 'grid'-style stem (in sp)
      // the note value with no beaming in 'grid'-style beaming
      TDuration::DurationType zeroBeamLevel = TDuration::DurationType::V_QUARTER;
      QChar   displayDot;             // the char to use to draw a dot
      QChar   displayValue[int(TabVal::NUM_OF)];       // the char to use to draw a duration value

      bool read(XmlReader&);
      };

// ready-made staff types
// keep in sync with the _presets initialization in StaffType::initStaffTypes()

enum class StaffTypes : signed char {
      STANDARD,
      PERC_1LINE, PERC_3LINE, PERC_5LINE,
      TAB_6SIMPLE, TAB_6COMMON, TAB_6FULL,
      TAB_4SIMPLE, TAB_4COMMON, TAB_4FULL,
      TAB_5SIMPLE, TAB_5COMMON, TAB_5FULL,
      TAB_UKULELE, TAB_BALALAJKA, TAB_DULCIMER,
      TAB_ITALIAN, TAB_FRENCH,
      TAB_7COMMON, TAB_8COMMON,
      STAFF_TYPES,
      // some useful shorthands:
      PERC_DEFAULT = StaffTypes::PERC_5LINE,
      TAB_DEFAULT = StaffTypes::TAB_6COMMON,
      };

static const int  STAFF_GROUP_NAME_MAX_LENGTH   = 32;

//---------------------------------------------------------
//   StaffType
//---------------------------------------------------------

class StaffType {
      friend class TabDurationSymbol;

      StaffGroup _group = StaffGroup::STANDARD;

      QString _xmlName;                   // the name used to reference this preset in instruments.xml
      QString _name;                      // user visible name

      qreal _userMag           { 1.0   };       // allowed 0.1 - 10.0
      Spatium _yoffset         { 0.0   };
      bool _small              { false };
      bool _invisible          { false };
      QColor _color            { QColor(Qt::black) };

      int _lines            = 5;
      int _stepOffset       = 0;
      Spatium _lineDistance = Spatium(1);

      bool _showBarlines    = true;
      bool _showLedgerLines = true;
      bool _stemless        = false;      // do not show stems

      bool _genClef         = true;       // create clef at beginning of system
      bool _genTimesig      = true;       // whether time signature is shown or not
      bool _genKeysig       = true;       // create key signature at beginning of system

      // Standard: configurable properties
      NoteHead::Scheme _noteHeadScheme = NoteHead::Scheme::HEAD_NORMAL;

      // TAB: configurable properties
      qreal _durationFontSize = 15.0;     // the size (in points) for the duration symbol font
      qreal _durationFontUserY = 0.0;     // the vertical offset (spatium units) for the duration symb. font
                                          // user configurable
      qreal _fretFontSize  = 10.0;        // the size (in points) for the fret marks font
      qreal _fretFontUserY = 0.0;         // additional vert. offset of fret marks with respect to
                                          // the string line (spatium unit); user configurable
      bool  _genDurations = false;        // whether duration symbols are drawn or not
      bool  _linesThrough = false;        // whether lines for strings and stems may pass through fret marks or not
      TablatureMinimStyle _minimStyle = TablatureMinimStyle::NONE;    // how to draw minim stems (stem-and-beam durations only)
      TablatureSymbolRepeat _symRepeat = TablatureSymbolRepeat::NEVER;// if and when to repeat the same duration symbol
      bool  _onLines      = true;         // whether fret marks are drawn on the string lines or between them
      bool  _showRests    = false;        // whether to draw rests or not
      bool  _stemsDown    = true;         // stems are drawn downward (stem-and-beam durations only)
      bool  _stemsThrough = true;         // stems are drawn through the staff rather than beside it (stem-and-beam durations only)
      bool  _upsideDown   = false;        // whether lines are drawn with highest string at top (false) or at bottom (true)
      bool  _showTabFingering   = false;        // Allow fingering in tablature staff (true) or not (false)
      bool  _useNumbers   = true;         // true: use numbers ('0' - ...) for frets | false: use letters ('a' - ...)
      bool  _showBackTied = true;         // whether back-tied notes are shown or not

      // TAB: internally managed variables
      // Note: values in RASTER UNITS are independent from score scaling and
      //    must be multiplied by magS() to be used in contexts using sp units
      mutable qreal _durationBoxH = 0.0;
      qreal mutable _durationBoxY = 0.0;          // the height and the y rect.coord. (relative to staff top line)
                                          // of a box bounding all duration symbols (raster units) internally computed:
                                          // depends upon _onString and the metrics of the duration font
      QFont _durationFont;                // font used to draw dur. symbols; cached for efficiency
      int   _durationFontIdx = 0;         // the index of current dur. font in dur. font array
      mutable qreal _durationYOffset = 0.0;       // the vertical offset to draw duration symbols with respect to the
                                          // string lines (raster units); internally computed: depends upon _onString and duration font
      mutable qreal _durationGridYOffset = 0.0;   // the vertical offset to draw the bottom of duration grid with respect to the
                                          // string lines (raster units); internally computed: depends upon _onstring and duration font
      mutable bool  _durationMetricsValid = false;  // whether duration font metrics are valid or not
      mutable qreal _fretBoxH = 0.0;
      mutable qreal _fretBoxY = 0.0;              // the height and the y rect.coord. (relative to staff line)
                                          // of a box bounding all fret characters (raster units) internally computed:
                                          // depends upon _onString, _useNumbers and the metrics of the fret font
      QFont _fretFont;                    // font used to draw fret marks; cached for efficiency
      int   _fretFontIdx = 0;             // the index of current fret font in fret font array
      mutable qreal _fretYOffset = 0.0;           // the vertical offset to draw fret marks with respect to the string lines;
                                          // (raster units); internally computed: depends upon _onString, _useNumbers
                                          // and the metrics of the fret font
      mutable  bool _fretMetricsValid = false;    // whether fret font metrics are valid or not
      mutable qreal _refDPI = 0.0;                // reference value used to last computed metrics and to see if they are still valid

      // the array of configured fonts
      static QList<TablatureFretFont> _fretFonts;
      static QList<TablatureDurationFont> _durationFonts;
      static std::vector<StaffType> _presets;

      void  setDurationMetrics() const;
      void  setFretMetrics() const;

      static bool readConfigFile(const QString& fileName);
      static const char    groupNames[STAFF_GROUP_MAX][STAFF_GROUP_NAME_MAX_LENGTH];      // used in UI
      static const QString fileGroupNames[STAFF_GROUP_MAX];                               // used in .msc? files

   public:
      StaffType();
      StaffType(StaffGroup sg, const QString& xml, const QString& name, int lines, int stpOff, qreal lineDist,
            bool genClef, bool showBarLines, bool stemless, bool genTimeSig,
            bool genKeySig, bool showLedgerLines, bool invisible, const QColor& color);

      StaffType(StaffGroup sg, const QString& xml, const QString& name, int lines, int stpOff, qreal lineDist,
            bool genClef, bool showBarLines, bool stemless, bool genTimesig, bool invisible, const QColor& color,
            const QString& durFontName, qreal durFontSize, qreal durFontUserY, qreal genDur,
            const QString& fretFontName, qreal fretFontSize, qreal fretFontUserY, TablatureSymbolRepeat symRepeat,
            bool linesThrough, TablatureMinimStyle minimStyle, bool onLines, bool showRests,
            bool stemsDown, bool stemThrough, bool upsideDown, bool showTabFingering, bool useNumbers, bool showBackTied);

      virtual ~StaffType() {}
      bool operator==(const StaffType&) const;
      bool isSameStructure(const StaffType&) const;

      StaffGroup group() const                 { return _group;           }
      const QString& name() const              { return _name;            }
      const QString& xmlName() const           { return _xmlName;         }
      void setName(const QString& val)         { _name = val;             }
      void setXmlName(const QString& val)      { _xmlName = val;          }
      const char* groupName() const;
      static const char* groupName(StaffGroup);

      void setLines(int val)                   { _lines = val;            }
      int lines() const                        { return _lines;           }
      void setStepOffset(int v)                { _stepOffset = v;         }
      int stepOffset() const                   { return _stepOffset;      }
      void setLineDistance(const Spatium& val) { _lineDistance = val;     }
      Spatium lineDistance() const             { return _lineDistance;    }
      void setGenClef(bool val)                { _genClef = val;          }
      bool genClef() const                     { return _genClef;         }
      void setShowBarlines(bool val)           { _showBarlines = val;     }
      bool showBarlines() const                { return _showBarlines;    }
      qreal userMag() const                    { return _userMag;         }
      bool small() const                       { return _small;           }
      bool invisible() const                   { return _invisible;       }
      const QColor& color() const              { return _color;           }
      void setUserMag(qreal val)               { _userMag = val;          }
      void setSmall(bool val)                  { _small = val;            }
      void setInvisible(bool val)              { _invisible = val;        }
      void setColor(const QColor& val)         { _color = val;            }
      Spatium yoffset() const                  { return _yoffset;         }
      void setYoffset(Spatium val)             { _yoffset = val;          }
      qreal spatium(Score*) const;

      void write(XmlWriter& xml) const;
      void read(XmlReader&);

      void setStemless(bool val)               { _stemless = val;       }
      bool stemless() const                    { return _stemless;      }
      bool genTimesig() const                  { return _genTimesig;      }
      void setGenTimesig(bool val)             { _genTimesig = val;       }
      qreal doty1() const;
      qreal doty2() const;

      // static function to deal with presets
      static const StaffType* getDefaultPreset(StaffGroup grp);
      static const StaffType* preset(StaffTypes idx);
      static const StaffType* presetFromXmlName(QString& xmlName);

      void setGenKeysig(bool val)              { _genKeysig = val;          }
      bool genKeysig() const                   { return _genKeysig;         }
      void setShowLedgerLines(bool val)        { _showLedgerLines = val;    }
      bool showLedgerLines() const             { return _showLedgerLines;   }
      void setNoteHeadScheme(NoteHead::Scheme s) { _noteHeadScheme = s;     }
      NoteHead::Scheme noteHeadScheme() const    { return _noteHeadScheme;  }

      QString fretString(int fret, int string, bool ghost) const;   // returns a string with the text for fret
      QString durationString(TDuration::DurationType type, int dots) const;

      // functions to cope with historic TAB's peculiarities, like upside-down, bass string notations
      int     physStringToVisual(int strg) const;                 // return the string in visual order from physical string
      int     visualStringToPhys(int line) const;                 // return the string in physical order from visual string
      qreal   physStringToYOffset(int strg) const;                // return the string Y offset (in sp, chord-relative)
      QString tabBassStringPrefix(int strg, bool* hasFret) const; // return a string with the prefix, if any, identifying a bass string
      void    drawInputStringMarks(QPainter* p, int string, int voice, QRectF rect) const;
      int     numOfTabLedgerLines(int string) const;

      // properties getters (some getters require updated metrics)
      qreal durationBoxH() const;
      qreal durationBoxY() const;

      const QFont&  durationFont() const     { return _durationFont;     }
      const QString durationFontName() const { return _durationFonts[_durationFontIdx].displayName; }
      qreal durationFontSize() const      { return _durationFontSize;   }
      qreal durationFontUserY() const     { return _durationFontUserY;  }
      qreal durationFontYOffset() const        { setDurationMetrics(); return _durationYOffset + _durationFontUserY * SPATIUM20; }
      qreal durationGridYOffset() const        { setDurationMetrics(); return _durationGridYOffset;}
      qreal fretBoxH() const              { setFretMetrics(); return _fretBoxH; }
      qreal fretBoxY() const              { setFretMetrics(); return _fretBoxY + _fretFontUserY * SPATIUM20; }

      // 2 methods to return the size of a box masking lines under a fret mark
      qreal fretMaskH() const             { return _lineDistance.val() * SPATIUM20; }
      qreal fretMaskY() const             { return (_onLines ? -0.5 : -1.0) * _lineDistance.val() * SPATIUM20; }

      const QFont&  fretFont() const      { return _fretFont;           }
      const QString fretFontName() const  { return _fretFonts[_fretFontIdx].displayName; }
      qreal fretFontSize() const          { return _fretFontSize;       }
      qreal fretFontUserY() const         { return _fretFontUserY;      }
      qreal fretFontYOffset() const       { setFretMetrics(); return _fretYOffset + _fretFontUserY * SPATIUM20; }
      bool  genDurations() const          { return _genDurations;       }
      bool  linesThrough() const          { return _linesThrough;       }
      TablatureMinimStyle minimStyle() const    { return _minimStyle;   }
      TablatureSymbolRepeat symRepeat() const   { return _symRepeat;    }
      bool  onLines() const               { return _onLines;            }
      bool  showRests() const             { return _showRests;          }
      bool  stemsDown() const             { return _stemsDown;          }
      bool  stemThrough() const           { return _stemsThrough;       }
      bool  upsideDown() const            { return _upsideDown;         }
      bool  showTabFingering() const            { return _showTabFingering;         }
      bool  useNumbers() const            { return _useNumbers;         }
      bool  showBackTied() const          { return _showBackTied;       }

      // properties setters (setting some props invalidates metrics)
      void  setDurationFontName(const QString&);
      void  setDurationFontSize(qreal);
      void  setDurationFontUserY(qreal val)     { _durationFontUserY = val; }
      void  setFretFontName(const QString&);
      void  setFretFontSize(qreal);
      void  setFretFontUserY(qreal val)   { _fretFontUserY = val;       }
      void  setGenDurations(bool val)     { _genDurations = val;        }
      void  setLinesThrough(bool val)     { _linesThrough = val;        }
      void  setMinimStyle(TablatureMinimStyle val)          { _minimStyle = val;    }
      void  setSymbolRepeat(TablatureSymbolRepeat val)      { _symRepeat  = val;    }
      void  setOnLines(bool);
      void  setShowRests(bool val)        { _showRests = val;           }
      void  setStemsDown(bool val)        { _stemsDown = val;           }
      void  setStemsThrough(bool val)     { _stemsThrough = val;        }
      void  setUpsideDown(bool val)       { _upsideDown = val;          }
      void  setShowTabFingering (bool val) { _showTabFingering = val;         }
      void  setUseNumbers(bool val)       { _useNumbers = val; _fretMetricsValid = false; }
      void  setShowBackTied(bool val)     { _showBackTied = val;        }

      // utility functions for tab specially managed elements
      QPointF chordStemPos(const Chord*) const;
      qreal   chordRestStemPosY(const ChordRest*) const;
      qreal   chordStemPosX(const Chord*) const       { return STAFFTYPE_TAB_DEFAULTSTEMPOSX; }
      QPointF chordStemPosBeam(const  Chord*) const;
      qreal   chordStemLength(const Chord*) const;

      bool isTabStaff() const  { return _group == StaffGroup::TAB; }
      bool isDrumStaff() const { return _group == StaffGroup::PERCUSSION; }
      // static functions for font config files
      static QList<QString> fontNames(bool bDuration);
      static bool fontData(bool bDuration, int nIdx, QString *pFamily, QString *pDisplayName, qreal * pSize, qreal *pYOff);

      static void initStaffTypes();
      static const std::vector<StaffType>& presets() { return _presets; }
      };

//---------------------------------------------------------
//   TabDurationSymbol
//    Element used to draw duration symbols above tablatures
//---------------------------------------------------------

enum class TabBeamGrid : char {
      NONE = 0,
      INITIAL,
      MEDIALFINAL,
      NUM_OF
      };

class TabDurationSymbol final : public Element {
      qreal       _beamLength { 0.0 };      // if _grid==MEDIALFINAL, length of the beam toward previous grid element
      int         _beamLevel  { 0 };       // if _grid==MEDIALFINAL, the number of beams
      TabBeamGrid _beamGrid   { TabBeamGrid::NONE };        // value for special 'English' grid display
      const StaffType*  _tab  { nullptr};
      QString     _text;
      bool        _repeat     { false };

   public:
      TabDurationSymbol(Score* s);
      TabDurationSymbol(Score* s, const StaffType* tab, TDuration::DurationType type, int dots);
      TabDurationSymbol(const TabDurationSymbol&);
      TabDurationSymbol* clone() const override  { return new TabDurationSymbol(*this); }
      void draw(QPainter*) const override;
      bool isEditable() const override           { return false; }
      void layout() override;
      ElementType type() const override          { return ElementType::TAB_DURATION_SYMBOL; }

      TabBeamGrid beamGrid()                     { return _beamGrid; }
      void layout2();               // second step of layout: after horiz. pos. are defined, compute width of 'grid beams'
      void setDuration(TDuration::DurationType type, int dots, const StaffType* tab) {
            _tab = tab;
            _text = tab->durationString(type, dots);
            }
      bool isRepeat() const                     { return _repeat; }
      void setRepeat(bool val)                  { _repeat = val;  }
      };

}     // namespace Ms
#endif
