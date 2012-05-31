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

class MStyle;
class Sequencer;

#ifndef MSCORE_GLOBAL
//---------------------------------------------------------
//   MScore
//    MuseScore application object
//---------------------------------------------------------

class MScore : public QObject {
      Q_OBJECT
      Q_ENUMS(ElementType)

   private:
      static MStyle* _defaultStyle;       // default modified by preferences
      static MStyle* _baseStyle;          // buildin initial style
      static QString _globalShare;
      static int _hRaster, _vRaster;

   public:
#endif

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

#ifndef MSCORE_GLOBAL
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
#endif

