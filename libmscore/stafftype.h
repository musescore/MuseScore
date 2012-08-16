//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: stafftype.h 5568 2012-04-22 10:08:43Z wschweer $
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

#define STAFFTYPE_TAB_DEFAULTSTEMLEN      3.0
#define STAFFTYPE_TAB_DEFAULTSTEMDIST     1.0
#define STAFFTYPE_TAB_DEFAULTSTEMPOSX     0.75
#define STAFFTYPE_TAB_DEFAULTSTEMPOSY     -STAFFTYPE_TAB_DEFAULTSTEMDIST

class Staff;
class Xml;
class QPainter;

//---------------------------------------------------------
//   StaffType
//---------------------------------------------------------

class StaffType {

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
      virtual ~StaffType() {}
      StaffType(const QString& s);
      QString name() const                     { return _name;            }
      void setName(const QString& val)         { _name = val;             }
      virtual StaffGroup group() const = 0;
      virtual StaffType* clone() const = 0;
      virtual const char* groupName() const = 0;
      virtual bool isEqual(const StaffType&) const;
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
      virtual void write(Xml& xml, int) const;
      void writeProperties(Xml& xml) const;
      virtual void read(const QDomElement&);
      bool readProperties(const QDomElement& e);
      void setSlashStyle(bool val)             { _slashStyle = val;       }
      bool slashStyle() const                  { return _slashStyle;      }
      bool genTimesig() const                  { return _genTimesig;      }
      void setGenTimesig(bool val)             { _genTimesig = val;       }
      qreal doty1() const;
      qreal doty2() const;
      };

// first three staff types in staffTypes[] are build in:

enum {
      PITCHED_STAFF_TYPE, TAB_STAFF_TYPE, PERCUSSION_STAFF_TYPE,
      STAFF_TYPES
      };

//---------------------------------------------------------
//   StaffTypePitched
//---------------------------------------------------------

class StaffTypePitched : public StaffType {
      bool _genKeysig;        // create key signature at beginning of system
      bool _showLedgerLines;

   public:
      StaffTypePitched();
      StaffTypePitched(const QString& s) : StaffType(s) {}
      virtual StaffGroup group() const        { return PITCHED_STAFF; }
      virtual StaffTypePitched* clone() const { return new StaffTypePitched(*this); }
      virtual const char* groupName() const   { return "pitched"; }
      virtual bool isEqual(const StaffType&) const;

      virtual void read(const QDomElement&);
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
      virtual bool isEqual(const StaffType&) const;

   public:
      StaffTypePercussion();
      StaffTypePercussion(const QString& s) : StaffType(s) {}
      virtual StaffGroup group() const           { return PERCUSSION_STAFF; }
      virtual StaffTypePercussion* clone() const { return new StaffTypePercussion(*this); }
      virtual const char* groupName() const      { return "percussion"; }

      virtual void read(const QDomElement&);
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
#define NUM_OF_LETTERFRETS    18

struct TablatureFretFont {
      QString           family;                 // the family of the physical font to use
      QString           displayName;            // the name to display to the user
      qreal             defPitch;               // the default size of the font
      QChar             xChar;                  // the char to use for 'x'
      QChar             ghostChar;              // the char to use for ghost notes
      QString           displayDigit[NUM_OF_DIGITFRETS];    // the string to draw for digit frets
      QChar             displayLetter[NUM_OF_LETTERFRETS];  // the char to use for letter frets

      bool read(const QDomElement&);
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

struct TablatureDurationFont {
      QString           family;                 // the family of the physical font to use
      QString           displayName;            // the name to display to the user
      qreal             defPitch;               // the default size of the font
      QChar             displayDot;             // the char to use to draw a dot
      QChar             displayValue[NUM_OF_TAB_VALS];       // the char to use to draw a duration value

      bool read(const QDomElement&);
};

//---------------------------------------------------------
//   StaffTypeTablature
//---------------------------------------------------------

class StaffTypeTablature : public StaffType {

   protected:
      // configurable properties
//      QString     _durationFontName;      // the name of the font used for duration symbols
      qreal       _durationFontSize;      // the size (in points) for the duration symbol font
      qreal       _durationFontUserY;     // the vertical offset (spatium units) for the duration symb. font
                                          // user configurable
//      QString     _fretFontName;          // the name of the font used for fret marks
      qreal       _fretFontSize;          // the size (in points) for the fret marks font
      qreal       _fretFontUserY;         // additional vert. offset of fret marks with respect to
                                          // the string line (spatium unit); user configurable
      bool        _genDurations;          // whether duration symbols are drawn or not
      bool        _linesThrough;          // whether lines for strings and stems may pass through fret marks or not
      bool        _onLines;               // whether fret marks are drawn on the string lines or between them
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
      StaffTypeTablature(const QString& s) : StaffType(s) { init(); }

      // re-implemented virtual functions
      virtual StaffGroup group() const          { return TAB_STAFF; }
      virtual StaffTypeTablature* clone() const { return new StaffTypeTablature(*this); }
      virtual const char* groupName() const     { return "tablature"; }
      virtual void read(const QDomElement& e);
      virtual void write(Xml& xml, int) const;
      virtual bool isEqual(const StaffType&) const;

      QString     fretString(int fret, bool ghost);   // returns a string with the text for fret
      QString     durationString(TDuration::DurationType type, int dots);

      // properties getters (some getters require updated metrics)
      qreal durationBoxH();
      qreal durationBoxY();

      const QFont&  durationFont()             { return _durationFont;     }
      const QString durationFontName() const   { return _durationFonts[_durationFontIdx].displayName; }
      qreal durationFontSize() const      { return _durationFontSize; }
      qreal durationFontUserY() const     { return _durationFontUserY;}
      qreal durationFontYOffset()         { setDurationMetrics(); return _durationYOffset + _durationFontUserY * MScore::DPI*SPATIUM20; }
      qreal fretBoxH()                    { setFretMetrics(); return _fretBoxH; }
      qreal fretBoxY()                    { setFretMetrics(); return _fretBoxY + _fretFontUserY * MScore::DPI*SPATIUM20; }
      const QFont&  fretFont()            { return _fretFont;         }
      const QString fretFontName() const  { return _fretFonts[_fretFontIdx].displayName; }
      qreal fretFontSize() const          { return _fretFontSize;     }
      qreal fretFontUserY() const         { return _fretFontUserY;    }
      qreal fretFontYOffset()             { setFretMetrics(); return _fretYOffset + _fretFontUserY * MScore::DPI*SPATIUM20; }
      bool  genDurations() const          { return _genDurations;     }
      bool  linesThrough() const          { return _linesThrough;     }
      bool  onLines() const               { return _onLines;          }
      bool  upsideDown() const            { return _upsideDown;       }
      bool  useNumbers() const            { return _useNumbers;       }
      // properties setters (setting some props invalidates metrics)
      void  setDurationFontName(QString name);
      void  setDurationFontSize(qreal val);
      void  setDurationFontUserY(qreal val)     { _durationFontUserY = val; }
      void  setFretFontName(QString name);
      void  setFretFontSize(qreal val);
      void  setFretFontUserY(qreal val)   { _fretFontUserY = val;     }
      void  setGenDurations(bool val)     { _genDurations = val;      }
      void  setLinesThrough(bool val)     { _linesThrough = val;      }
      void  setOnLines(bool val);
      void  setUpsideDown(bool val)       { _upsideDown = val;        }
      void  setUseNumbers(bool val)       { _useNumbers = val; _fretMetricsValid = false; }

      // static functions for font config files
      static bool             readConfigFile(const QString& fileName);
      static QList<QString>   fontNames(bool bDuration);
      static bool             fontData(bool bDuration, int nIdx, QString *pFamily,
                                    QString *pDisplayName, qreal * pSize);

   protected:
      void  setDurationMetrics();
      void  setFretMetrics();
      };


extern void initStaffTypes();
extern QList<StaffType*> staffTypes;

//---------------------------------------------------------
//   TabDurationSymbol
//    Element used to draw duration symbols above tablatures
//---------------------------------------------------------

class TabDurationSymbol : public Element {
      StaffTypeTablature* _tab;
      QString             _text;

      void buildText(TDuration::DurationType type, int dots)
                                                { _text = _tab->durationString(type, dots); }

   public:
      TabDurationSymbol(Score* s);
      TabDurationSymbol(Score* s, StaffTypeTablature * tab, TDuration::DurationType type, int dots);
      TabDurationSymbol(const TabDurationSymbol&);
      virtual TabDurationSymbol* clone() const  { return new TabDurationSymbol(*this); }
      virtual void draw(QPainter*) const;
      virtual bool isEditable() const           { return false; }
      virtual ElementType type() const          { return TAB_DURATION_SYMBOL; }

      void  setDuration(TDuration::DurationType type, int dots) { buildText(type, dots); }
      void  setTablature(StaffTypeTablature * tab)              { _tab = tab; }
      };

#endif
