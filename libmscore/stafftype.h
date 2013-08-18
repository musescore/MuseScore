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

class QPainter;

namespace Ms {

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

class Chord;
class ChordRest;
class Staff;
class Xml;

//---------------------------------------------------------
//   StaffType
//---------------------------------------------------------

class StaffType {
      bool _builtin;          // used for memory management: do not delete if true

   protected:
      QString _name;
      int _lines;
      int _stepOffset;
      Spatium _lineDistance;

      bool _genClef;          // create clef at beginning of system
      bool _showBarlines;
      bool _slashStyle;       // do not show stems
      bool _genTimesig;       // whether time signature is shown or not

   public:
      StaffType();
      StaffType(const QString& name, int lines, qreal lineDist, bool genClef,
            bool showBarLines, bool stemless, bool genTimeSig) :
            _name(name), _lineDistance(Spatium(lineDist)), _genClef(genClef),_showBarlines(showBarLines),
            _slashStyle(stemless), _genTimesig(genTimeSig)
            {
            _builtin = false;
            setLines(lines);
            }
      virtual ~StaffType() {}

      QString name() const                     { return _name;            }
      void setName(const QString& val)         { _name = val;             }
      virtual StaffGroup group() const = 0;
      virtual StaffType* clone() const = 0;
      virtual const char* groupName() const = 0;
      virtual bool isEqual(const StaffType&) const;
      virtual bool isSameStructure(const StaffType&) const;
      virtual void setLines(int val);
      int lines() const                        { return _lines;           }
      void setStepOffset(int v)                { _stepOffset = v;         }
      int stepOffset() const                   { return _stepOffset;      }
      void setLineDistance(const Spatium& val) { _lineDistance = val;     }
      Spatium lineDistance() const             { return _lineDistance;    }
      void setGenClef(bool val)                { _genClef = val;          }
      bool genClef() const                     { return _genClef;         }
      void setShowBarlines(bool val)           { _showBarlines = val;     }
      bool showBarlines() const                { return _showBarlines;    }
      virtual void write(Xml& xml, int) const;
      void writeProperties(Xml& xml) const;
      virtual void read(XmlReader&);
      bool readProperties(XmlReader& e);
      void setSlashStyle(bool val)             { _slashStyle = val;       }
      bool slashStyle() const                  { return _slashStyle;      }
      bool genTimesig() const                  { return _genTimesig;      }
      void setGenTimesig(bool val)             { _genTimesig = val;       }
      qreal doty1() const;
      qreal doty2() const;
      bool builtin()            { return _builtin; }
      void setBuiltin(bool val) { _builtin = val; }

      // static function to deal with presets
      static const StaffType* getDefaultPreset(StaffGroup grp, int* idx);
      static size_t numOfPresets();
      static const StaffType* preset(int idx);
      static const StaffType* presetFromName(QString& name, int* idx);
      static const StaffType* presetFromXmlName(QString& xmlName, int* idx);
      static const QString& presetXmlName(int idx);
      static const QString& presetName(int idx);
      };

// ready-made staff types:

enum {
      STANDARD_STAFF_TYPE,
      PERC_1LINE_STAFF_TYPE, PERC_3LINE_STAFF_TYPE, PERC_5LINE_STAFF_TYPE,
      TAB_6SIMPLE_STAFF_TYPE, TAB_6COMMON_STAFF_TYPE, TAB_6FULL_STAFF_TYPE,
            TAB_4SIMPLE_STAFF_TYPE, TAB_4COMMON_STAFF_TYPE, TAB_4FULL_STAFF_TYPE,
            TAB_UKULELE_STAFF_TYPE, TAB_BALALAJKA_STAFF_TYPE, TAB_ITALIAN_STAFF_TYPE, TAB_FRENCH_STAFF_TYPE,
      STAFF_TYPES,
      // some usefull shorthands:
            PERC_DEFAULT_STAFF_TYPE = PERC_5LINE_STAFF_TYPE,
            TAB_DEFAULT_STAFF_TYPE = TAB_6COMMON_STAFF_TYPE
      };

//---------------------------------------------------------
//   StaffTypePitched
//---------------------------------------------------------

class StaffTypePitched : public StaffType {
      bool _genKeysig;        // create key signature at beginning of system
      bool _showLedgerLines;

   public:
      StaffTypePitched();
      StaffTypePitched(const QString& name, int lines, qreal lineDist, bool genClef,
            bool showBarLines, bool stemless, bool genTimeSig, bool genKeySig, bool showLedgerLines) :
            StaffType(name, lines, lineDist, genClef, showBarLines, stemless, genTimeSig),
            _genKeysig(genKeySig), _showLedgerLines(showLedgerLines)
            {
            }
      virtual StaffGroup group() const        { return STANDARD_STAFF_GROUP; }
      virtual StaffTypePitched* clone() const { return new StaffTypePitched(*this); }
      virtual const char* groupName() const   { return "pitched"; }
      virtual bool isEqual(const StaffType&) const;
      virtual bool isSameStructure(const StaffType& st) const;

      virtual void read(XmlReader&);
      virtual void write(Xml& xml, int) const;

      void setGenKeysig(bool val)              { _genKeysig = val;        }
      bool genKeysig() const                   { return _genKeysig;       }
      void setShowLedgerLines(bool val)        { _showLedgerLines = val;  }
      bool showLedgerLines() const             { return _showLedgerLines; }
      };

//---------------------------------------------------------
//   StaffTypePercussion
//---------------------------------------------------------

class StaffTypePercussion : public StaffType {
      bool _genKeysig;        // create key signature at beginning of system
      bool _showLedgerLines;

   public:
      StaffTypePercussion();
      StaffTypePercussion(const QString& name, int lines, qreal lineDist, bool genClef,
            bool showBarLines, bool stemless, bool genTimeSig, bool genKeySig, bool showLedgerLines) :
            StaffType(name, lines, lineDist, genClef, showBarLines, stemless, genTimeSig),
            _genKeysig(genKeySig), _showLedgerLines(showLedgerLines)
            {
            }
      virtual StaffGroup group() const           { return PERCUSSION_STAFF_GROUP; }
      virtual StaffTypePercussion* clone() const { return new StaffTypePercussion(*this); }
      virtual const char* groupName() const      { return "percussion"; }
      virtual bool isEqual(const StaffType&) const;
      virtual bool isSameStructure(const StaffType& st) const;

      virtual void read(XmlReader&);
      virtual void write(Xml& xml, int) const;

      void setGenKeysig(bool val)                { _genKeysig = val;        }
      bool genKeysig() const                     { return _genKeysig;       }
      void setShowLedgerLines(bool val)          { _showLedgerLines = val;  }
      bool showLedgerLines() const               { return _showLedgerLines; }
      };

//---------------------------------------------------------
//   TablatureFont
//---------------------------------------------------------

#define NUM_OF_DIGITFRETS     25
#define NUM_OF_LETTERFRETS    17

struct TablatureFretFont {
      QString           family;                 // the family of the physical font to use
      QString           displayName;            // the name to display to the user
      qreal             defPitch;               // the default size of the font
      qreal             defYOffset;             // the default Y displacement
      QChar             xChar;                  // the char to use for 'x'
      QChar             ghostChar;              // the char to use for ghost notes
      QString           displayDigit[NUM_OF_DIGITFRETS];    // the string to draw for digit frets
      QChar             displayLetter[NUM_OF_LETTERFRETS];  // the char to use for letter frets

      bool read(XmlReader&);
};

enum {
      TAB_VAL_LONGA = 0,
      TAB_VAL_BREVIS,
      TAB_VAL_SEMIBREVIS,
      TAB_VAL_MINIMA,
      TAB_VAL_SEMIMINIMA,
      TAB_VAL_FUSA,
      TAB_VAL_SEMIFUSA,
      TAB_VAL_32,
      TAB_VAL_64,
      TAB_VAL_128,
      TAB_VAL_256,
            NUM_OF_TAB_VALS
};

enum TablatureMinimStyle {
      TAB_MINIM_NONE = 0,                       // do not draw half notes at all
      TAB_MINIM_SHORTER,                        // draw half notes with a shorter stem
      TAB_MINIM_SLASHED                         // draw half notes with stem with two slashes
};

struct TablatureDurationFont {
      QString           family;                 // the family of the physical font to use
      QString           displayName;            // the name to display to the user
      qreal             defPitch;               // the default size of the font
      qreal             defYOffset;             // the default Y displacement
      QChar             displayDot;             // the char to use to draw a dot
      QChar             displayValue[NUM_OF_TAB_VALS];       // the char to use to draw a duration value

      bool read(XmlReader&);
};

//---------------------------------------------------------
//   StaffTypeTablature
//---------------------------------------------------------

class StaffTypeTablature : public StaffType {

   protected:
      // configurable properties
      qreal       _durationFontSize;      // the size (in points) for the duration symbol font
      qreal       _durationFontUserY;     // the vertical offset (spatium units) for the duration symb. font
                                          // user configurable
      qreal       _fretFontSize;          // the size (in points) for the fret marks font
      qreal       _fretFontUserY;         // additional vert. offset of fret marks with respect to
                                          // the string line (spatium unit); user configurable
      bool        _genDurations;          // whether duration symbols are drawn or not
      bool        _linesThrough;          // whether lines for strings and stems may pass through fret marks or not
      TablatureMinimStyle _minimStyle;    // how to draw minim stems (stem-and-beam durations only)
      bool        _onLines;               // whether fret marks are drawn on the string lines or between them
      bool        _showRests;             // whether to draw rests or not
      bool        _stemsDown;             // stems are drawn downward (stem-and-beam durations only)
      bool        _stemsThrough;          // stems are drawn through the staff rather than beside it (stem-and-beam durations only)
      bool        _upsideDown;            // whether lines are drwan with highest string at top (false) or at bottom (true)
      bool        _useNumbers;            // true: use numbers ('0' - ...) for frets | false: use letters ('a' - ...)

      // internally managed variables
      qreal       _durationBoxH, _durationBoxY; // the height and the y rect.coord. (relative to staff top line)
                                          // of a box bounding all duration symbols (raster units) internally computed:
                                          // depends upon _onString and the metrics of the duration font
      QFont       _durationFont;          // font used to draw dur. symbols; cached for efficiency
      int         _durationFontIdx;       // the index of current dur. font in dur. font array
      qreal       _durationYOffset;       // the vertical offset to draw duration symbols with respect to the
                                          // string lines (raster units); internally computed: depends upon _onString
      bool        _durationMetricsValid;  // whether duration font metrics are valid or not
      qreal       _fretBoxH, _fretBoxY;   // the height and the y rect.coord. (relative to staff line)
                                          // of a box bounding all fret characters (raster units) internally computed:
                                          // depends upon _onString, _useNumbers and the metrics of the fret font
      QFont       _fretFont;              // font used to draw fret marks; cached for efficiency
      int         _fretFontIdx;           // the index of current fret font in fret font array
      qreal       _fretYOffset;           // the vertical offset to draw fret marks with respect to the string lines;
                                          // (raster units); internally computed: depends upon _onString, _useNumbers
                                          // and the metrics of the fret font
      bool        _fretMetricsValid;      // whether fret font metrics are valid or not
      qreal       _refDPI;                // reference value used to last compute metrics and to see if they are still valid

      // the array of configured fonts
      static QList<TablatureFretFont>     _fretFonts;
      static QList<TablatureDurationFont> _durationFonts;

      void init();                        // init to reasonable defaults

   public:
      StaffTypeTablature() : StaffType() { init(); }
      StaffTypeTablature(const QString& name, int lines, qreal lineDist, bool genClef,
            bool showBarLines, bool stemless, bool genTimesig,
            const QString& durFontName, qreal durFontSize, qreal durFontUserY, qreal genDur,
            const QString& fretFontName, qreal fretFontSize, qreal fretFontUserY,
            bool linesThrough, TablatureMinimStyle minimStyle, bool onLines, bool showRests,
            bool stemsDown, bool stemThrough, bool upsideDown, bool useNumbers)
            {
            setName(name);
            setLines(lines);
            setLineDistance(Spatium(lineDist));
            setGenClef(genClef);
            setShowBarlines(showBarLines);
            setSlashStyle(stemless);
            setGenTimesig(genTimesig);
            setDurationFontName(durFontName);
            setDurationFontSize(durFontSize);
            setDurationFontUserY(durFontUserY);
            setGenDurations(genDur);
            setFretFontName(fretFontName);
            setFretFontSize(fretFontSize);
            setFretFontUserY(fretFontUserY);
            setLinesThrough(linesThrough);
            setMinimStyle(minimStyle);
            setOnLines(onLines);
            setShowRests(showRests);
            setStemsDown(stemsDown);
            setStemsThrough(stemThrough);
            setUpsideDown(upsideDown);
            setUseNumbers(useNumbers);
            }

      // re-implemented virtual functions
      virtual StaffGroup group() const          { return TAB_STAFF_GROUP; }
      virtual StaffTypeTablature* clone() const { return new StaffTypeTablature(*this); }
      virtual const char* groupName() const     { return "tablature"; }
      virtual void read(XmlReader& e);
      virtual void write(Xml& xml, int) const;
      virtual bool isEqual(const StaffType&) const;
      virtual bool isSameStructure(const StaffType& st) const;

      QString     fretString(int fret, bool ghost) const;   // returns a string with the text for fret
      QString     durationString(TDuration::DurationType type, int dots) const;
      // functions to cope with tabulature visual order (top down or upside down)
      int         physStringToVisual(int strg) const;       // return the string in visual order from physical string
      int         VisualStringToPhys(int strg) const;       // return the string in physical order from visual string

      // properties getters (some getters require updated metrics)
      qreal durationBoxH();
      qreal durationBoxY();

      const QFont&  durationFont()             { return _durationFont;     }
      const QString durationFontName() const   { return _durationFonts[_durationFontIdx].displayName; }
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
      void  setDurationFontName(QString name);
      void  setDurationFontSize(qreal val);
      void  setDurationFontUserY(qreal val)     { _durationFontUserY = val; }
      void  setFretFontName(QString name);
      void  setFretFontSize(qreal val);
      void  setFretFontUserY(qreal val)   { _fretFontUserY = val;       }
      void  setGenDurations(bool val)     { _genDurations = val;        }
      virtual void setLines(int val)      { _lines = val; _stepOffset = (val / 2 - 2) * 2; }
      void  setLinesThrough(bool val)     { _linesThrough = val;        }
      void  setMinimStyle(TablatureMinimStyle val)    { _minimStyle = val;    }
      void  setOnLines(bool val);
      void  setShowRests(bool val)        { _showRests = val;           }
      void  setStemsDown(bool val)        { _stemsDown = val;           }
      void  setStemsThrough(bool val)     { _stemsThrough = val;        }
      void  setUpsideDown(bool val)       { _upsideDown = val;          }
      void  setUseNumbers(bool val)       { _useNumbers = val; _fretMetricsValid = false; }

      // utility functions for tab specially managed elements
      QPointF     chordStemPos(const Chord * chord) const;
      qreal       chordRestStemPosY(const ChordRest * chordRest) const;
      qreal       chordStemPosX(const Chord * /*chord*/) const    { return STAFFTYPE_TAB_DEFAULTSTEMPOSX; }
      QPointF     chordStemPosBeam(const  Chord * chord) const;
      qreal       chordStemLength(const Chord *chord) const;

      // static functions for font config files
      static bool             readConfigFile(const QString& fileName);
      static QList<QString>   fontNames(bool bDuration);
      static bool             fontData(bool bDuration, int nIdx, QString *pFamily,
                                    QString *pDisplayName, qreal * pSize, qreal *pYOff);

   protected:
      void  setDurationMetrics();
      void  setFretMetrics();
      };


extern void initStaffTypes();
//extern QList<StaffType*> staffTypes;

//---------------------------------------------------------
//   TabDurationSymbol
//    Element used to draw duration symbols above tablatures
//---------------------------------------------------------

class TabDurationSymbol : public Element {
      StaffTypeTablature* _tab;
      QString             _text;

   public:
      TabDurationSymbol(Score* s);
      TabDurationSymbol(Score* s, StaffTypeTablature * tab, TDuration::DurationType type, int dots);
      TabDurationSymbol(const TabDurationSymbol&);
      virtual TabDurationSymbol* clone() const  { return new TabDurationSymbol(*this); }
      virtual void draw(QPainter*) const;
      virtual bool isEditable() const           { return false; }
      virtual void layout();
      virtual ElementType type() const          { return TAB_DURATION_SYMBOL; }

      void  setDuration(TDuration::DurationType type, int dots, StaffTypeTablature* tab)
                                                { _tab = tab;
                                                  _text = tab->durationString(type, dots);
                                                }
      };


}     // namespace Ms
#endif
