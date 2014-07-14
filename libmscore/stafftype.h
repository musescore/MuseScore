//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
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

namespace Ms {

class Chord;
class ChordRest;
class Staff;
class Xml;

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

// the ratio between the length of a full stem and the lenght of a short stem
// (used for half note stems, in some TAB styles)
#define STAFFTYPE_TAB_SHORTSTEMRATIO      0.5

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

//---------------------------------------------------------
//   TablatureFont
//---------------------------------------------------------

#define NUM_OF_DIGITFRETS     25
#define NUM_OF_LETTERFRETS    17

struct TablatureFretFont {
      QString family;                 // the family of the physical font to use
      QString displayName;            // the name to display to the user
      qreal   defPitch;               // the default size of the font
      qreal   defYOffset;             // the default Y displacement
      QChar   xChar;                  // the char to use for 'x'
      QChar   ghostChar;              // the char to use for ghost notes
      QString displayDigit[NUM_OF_DIGITFRETS];    // the string to draw for digit frets
      QChar   displayLetter[NUM_OF_LETTERFRETS];  // the char to use for letter frets

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
      NUM_OF
      };

enum class TablatureMinimStyle : char {
      NONE = 0,                       // do not draw half notes at all
      SHORTER,                        // draw half notes with a shorter stem
      SLASHED                         // draw half notes with stem with two slashes
      };

struct TablatureDurationFont {
      QString family;                 // the family of the physical font to use
      QString displayName;            // the name to display to the user
      qreal   defPitch;               // the default size of the font
      qreal   defYOffset;             // the default Y displacement
      QChar   displayDot;             // the char to use to draw a dot
      QChar   displayValue[int(TabVal::NUM_OF)];       // the char to use to draw a duration value

      bool read(XmlReader&);
      };

// ready-made staff types:

enum class StaffTypes : char {
      STANDARD,
      PERC_1LINE, PERC_3LINE, PERC_5LINE,
      TAB_6SIMPLE, TAB_6COMMON, TAB_6FULL,
            TAB_4SIMPLE, TAB_4COMMON, TAB_4FULL,
            TAB_UKULELE, TAB_BALALAJKA, TAB_ITALIAN, TAB_FRENCH,
      STAFF_TYPES,
      // some usefull shorthands:
            PERC_DEFAULT = StaffTypes::PERC_5LINE,
            TAB_DEFAULT = StaffTypes::TAB_6COMMON
      };

static const int  STAFF_GROUP_NAME_MAX_LENGTH   = 32;

//---------------------------------------------------------
//   StaffType
//---------------------------------------------------------

class StaffType {
      StaffGroup _group = StaffGroup::STANDARD;

      QString _xmlName;                   // the name used to reference this preset in intruments.xml
      QString _name;                      // user visible name

      int _lines            = 5;
      int _stepOffset       = 0;
      Spatium _lineDistance = Spatium(1);

      bool _genClef         = true;       // create clef at beginning of system
      bool _showBarlines    = true;
      bool _slashStyle      = false;      // do not show stems
      bool _genTimesig      = true;       // whether time signature is shown or not
      bool _genKeysig       = true;       // create key signature at beginning of system
      bool _showLedgerLines = true;

      // configurable properties
      qreal _durationFontSize = 15.0;     // the size (in points) for the duration symbol font
      qreal _durationFontUserY = 0.0;     // the vertical offset (spatium units) for the duration symb. font
                                          // user configurable
      qreal _fretFontSize  = 10.0;        // the size (in points) for the fret marks font
      qreal _fretFontUserY = 0.0;         // additional vert. offset of fret marks with respect to
                                          // the string line (spatium unit); user configurable
      bool  _genDurations = false;        // whether duration symbols are drawn or not
      bool  _linesThrough = false;        // whether lines for strings and stems may pass through fret marks or not
      TablatureMinimStyle _minimStyle = TablatureMinimStyle::NONE;    // how to draw minim stems (stem-and-beam durations only)
      bool  _onLines      = true;         // whether fret marks are drawn on the string lines or between them
      bool  _showRests    = false;        // whether to draw rests or not
      bool  _stemsDown    = true;         // stems are drawn downward (stem-and-beam durations only)
      bool  _stemsThrough = true;         // stems are drawn through the staff rather than beside it (stem-and-beam durations only)
      bool  _upsideDown   = false;        // whether lines are drwan with highest string at top (false) or at bottom (true)
      bool  _useNumbers   = true;         // true: use numbers ('0' - ...) for frets | false: use letters ('a' - ...)

      // internally managed variables
      qreal _durationBoxH = 0.0;
      qreal _durationBoxY = 0.0;          // the height and the y rect.coord. (relative to staff top line)
                                          // of a box bounding all duration symbols (raster units) internally computed:
                                          // depends upon _onString and the metrics of the duration font
      QFont _durationFont;                // font used to draw dur. symbols; cached for efficiency
      int   _durationFontIdx = 0;         // the index of current dur. font in dur. font array
      qreal _durationYOffset = 0.0;       // the vertical offset to draw duration symbols with respect to the
                                          // string lines (raster units); internally computed: depends upon _onString
      bool  _durationMetricsValid = false;  // whether duration font metrics are valid or not
      qreal _fretBoxH = 0.0;
      qreal _fretBoxY = 0.0;              // the height and the y rect.coord. (relative to staff line)
                                          // of a box bounding all fret characters (raster units) internally computed:
                                          // depends upon _onString, _useNumbers and the metrics of the fret font
      QFont _fretFont;                    // font used to draw fret marks; cached for efficiency
      int   _fretFontIdx = 0;             // the index of current fret font in fret font array
      qreal _fretYOffset = 0.0;           // the vertical offset to draw fret marks with respect to the string lines;
                                          // (raster units); internally computed: depends upon _onString, _useNumbers
                                          // and the metrics of the fret font
      bool  _fretMetricsValid = false;    // whether fret font metrics are valid or not
      qreal _refDPI = 0.0;                // reference value used to last compute metrics and to see if they are still valid

      // the array of configured fonts
      static QList<TablatureFretFont> _fretFonts;
      static QList<TablatureDurationFont> _durationFonts;
      static std::vector<StaffType> _presets;

      void  setDurationMetrics();
      void  setFretMetrics();

      static bool readConfigFile(const QString& fileName);
      static const char    groupNames[STAFF_GROUP_MAX][STAFF_GROUP_NAME_MAX_LENGTH];      // used in UI
      static const QString fileGroupNames[STAFF_GROUP_MAX];                               // used in .msc? files

   public:
      StaffType();
      StaffType(StaffGroup sg, const QString& xml, const QString& name, int lines, qreal lineDist, bool genClef,
            bool showBarLines, bool stemless, bool genTimeSig, bool genKeySig, bool showLedgerLines);

      StaffType(StaffGroup sg, const QString& xml, const QString& name, int lines, qreal lineDist, bool genClef,
                  bool showBarLines, bool stemless, bool genTimesig,
                  const QString& durFontName, qreal durFontSize, qreal durFontUserY, qreal genDur,
                  const QString& fretFontName, qreal fretFontSize, qreal fretFontUserY,
                  bool linesThrough, TablatureMinimStyle minimStyle, bool onLines, bool showRests,
                  bool stemsDown, bool stemThrough, bool upsideDown, bool useNumbers);

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

      void setLines(int val);
      int lines() const                        { return _lines;           }
      void setStepOffset(int v)                { _stepOffset = v;         }
      int stepOffset() const                   { return _stepOffset;      }
      void setLineDistance(const Spatium& val) { _lineDistance = val;     }
      Spatium lineDistance() const             { return _lineDistance;    }
      void setGenClef(bool val)                { _genClef = val;          }
      bool genClef() const                     { return _genClef;         }
      void setShowBarlines(bool val)           { _showBarlines = val;     }
      bool showBarlines() const                { return _showBarlines;    }

      void write(Xml& xml) const;
      void read(XmlReader&);

      void setSlashStyle(bool val)             { _slashStyle = val;       }
      bool slashStyle() const                  { return _slashStyle;      }
      bool genTimesig() const                  { return _genTimesig;      }
      void setGenTimesig(bool val)             { _genTimesig = val;       }
      qreal doty1() const;
      qreal doty2() const;

      // static function to deal with presets
      static const StaffType* getDefaultPreset(StaffGroup grp);
      static const StaffType* preset(StaffTypes idx);
      static const StaffType* presetFromXmlName(QString& xmlName);

      void setGenKeysig(bool val)              { _genKeysig = val;        }
      bool genKeysig() const                   { return _genKeysig;       }
      void setShowLedgerLines(bool val)        { _showLedgerLines = val;  }
      bool showLedgerLines() const             { return _showLedgerLines; }

      QString fretString(int fret, bool ghost) const;   // returns a string with the text for fret
      QString durationString(TDuration::DurationType type, int dots) const;

      // functions to cope with tabulature visual order (top down or upside down)
      int physStringToVisual(int strg) const;       // return the string in visual order from physical string
      int visualStringToPhys(int strg) const;       // return the string in physical order from visual string

      // properties getters (some getters require updated metrics)
      qreal durationBoxH();
      qreal durationBoxY();

      const QFont&  durationFont()           { return _durationFont;     }
      const QString durationFontName() const { return _durationFonts[_durationFontIdx].displayName; }
      qreal durationFontSize() const      { return _durationFontSize;   }
      qreal durationFontUserY() const     { return _durationFontUserY;  }
      qreal durationFontYOffset()         { setDurationMetrics(); return _durationYOffset + _durationFontUserY * MScore::DPI*SPATIUM20; }
      qreal fretBoxH()                    { setFretMetrics(); return _fretBoxH; }
      qreal fretBoxY()                    { setFretMetrics(); return _fretBoxY + _fretFontUserY * MScore::DPI*SPATIUM20; }

      // 2 methods to return the size of a box masking lines under a fret mark
      qreal fretMaskH()                   { return _lineDistance.val() * MScore::DPI*SPATIUM20; }
      qreal fretMaskY()                   { return (_onLines ? -0.5 : -1.0) * _lineDistance.val() * MScore::DPI*SPATIUM20; }

      const QFont&  fretFont()            { return _fretFont;           }
      const QString fretFontName() const  { return _fretFonts[_fretFontIdx].displayName; }
      qreal fretFontSize() const          { return _fretFontSize;       }
      qreal fretFontUserY() const         { return _fretFontUserY;      }
      qreal fretFontYOffset()             { setFretMetrics(); return _fretYOffset + _fretFontUserY * MScore::DPI*SPATIUM20; }
      bool  genDurations() const          { return _genDurations;       }
      bool  linesThrough() const          { return _linesThrough;       }
      TablatureMinimStyle minimStyle () const   { return _minimStyle;   }
      bool  onLines() const               { return _onLines;            }
      bool  showRests() const             { return _showRests;          }
      bool  stemsDown() const             { return _stemsDown;          }
      bool  stemThrough() const           { return _stemsThrough;       }
      bool  upsideDown() const            { return _upsideDown;         }
      bool  useNumbers() const            { return _useNumbers;         }

      // properties setters (setting some props invalidates metrics)
      void  setDurationFontName(const QString&);
      void  setDurationFontSize(qreal);
      void  setDurationFontUserY(qreal val)     { _durationFontUserY = val; }
      void  setFretFontName(const QString&);
      void  setFretFontSize(qreal);
      void  setFretFontUserY(qreal val)   { _fretFontUserY = val;       }
      void  setGenDurations(bool val)     { _genDurations = val;        }
      void  setLinesThrough(bool val)     { _linesThrough = val;        }
      void  setMinimStyle(TablatureMinimStyle val)    { _minimStyle = val;    }
      void  setOnLines(bool);
      void  setShowRests(bool val)        { _showRests = val;           }
      void  setStemsDown(bool val)        { _stemsDown = val;           }
      void  setStemsThrough(bool val)     { _stemsThrough = val;        }
      void  setUpsideDown(bool val)       { _upsideDown = val;          }
      void  setUseNumbers(bool val)       { _useNumbers = val; _fretMetricsValid = false; }

      // utility functions for tab specially managed elements
      QPointF chordStemPos(const Chord*) const;
      qreal   chordRestStemPosY(const ChordRest*) const;
      qreal   chordStemPosX(const Chord*) const       { return STAFFTYPE_TAB_DEFAULTSTEMPOSX; }
      QPointF chordStemPosBeam(const  Chord*) const;
      qreal   chordStemLength(const Chord*) const;

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

class TabDurationSymbol : public Element {
      StaffType* _tab;
      QString    _text;

   public:
      TabDurationSymbol(Score* s);
      TabDurationSymbol(Score* s, StaffType* tab, TDuration::DurationType type, int dots);
      TabDurationSymbol(const TabDurationSymbol&);
      virtual TabDurationSymbol* clone() const  { return new TabDurationSymbol(*this); }
      virtual void draw(QPainter*) const;
      virtual bool isEditable() const           { return false; }
      virtual void layout();
      virtual Element::Type type() const        { return Element::Type::TAB_DURATION_SYMBOL; }

      void setDuration(TDuration::DurationType type, int dots, StaffType* tab) {
            _tab = tab;
            _text = tab->durationString(type, dots);
            }
      };

}     // namespace Ms
#endif
