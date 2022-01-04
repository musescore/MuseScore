/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __MSCORE_H__
#define __MSCORE_H__

#include <QObject>

#include "config.h"

#include "infrastructure/draw/color.h"

namespace Ms {
#define MSC_VERSION     "4.00"
static constexpr int MSCVERSION = 400;

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
//    3.01  -
//    3.02  Engraving improvements for 3.6

//    4.00 (Version 4.0)
//       - The style is stored in a separate file (inside mscz)
//       - The ChordList is stored in a separate file (inside mscz)

enum class HairpinType : signed char;

static constexpr int VOICES = 4;

static constexpr int INVALID_INDEX = -1;

inline constexpr int staff2track(int staffIdx, int voiceIdx = 0)
{
    return staffIdx >= 0 ? staffIdx * VOICES + voiceIdx : INVALID_INDEX;
}

inline constexpr int track2staff(int track)
{
    return track >= 0 ? track / VOICES : INVALID_INDEX;
}

inline constexpr int track2voice(int track)
{
    return track >= 0 ? track % VOICES : INVALID_INDEX;
}

inline constexpr int trackZeroVoice(int track)
{
    return track >= 0 ? (track / VOICES) * VOICES : INVALID_INDEX;
}

static constexpr int MAX_TAGS = 32;

static constexpr int MAX_HEADERS = 3;
static constexpr int MAX_FOOTERS = 3;

static constexpr qreal INCH      = 25.4;
static constexpr qreal PPI       = 72.0; // printer points per inch
static constexpr qreal DPI_F     = 5;
static constexpr qreal DPI       = 72.0 * DPI_F;
static constexpr qreal SPATIUM20 = 5.0 * (DPI / 72.0);
static constexpr qreal DPMM      = DPI / INCH;

static constexpr int MAX_STAVES = 4;

static constexpr int SHADOW_NOTE_LIGHT = 135;

static constexpr char mimeSymbolFormat[]     = "application/musescore/symbol";
static constexpr char mimeSymbolListFormat[] = "application/musescore/symbollist";
static constexpr char mimeStaffListFormat[]  = "application/musescore/stafflist";

static constexpr int INVALID_STRING_INDEX = -1; // no ordinal for a physical string (0 = topmost in instrument)
static constexpr int INVALID_FRET_INDEX   = -1; // no ordinal for a fret

// no ordinal for the visual repres. of string
// (topmost in TAB varies according to visual order and presence of bass strings)
static constexpr int VISUAL_INVALID_STRING_INDEX = -100;

using ID = uint64_t;
static constexpr ID INVALID_ID = 0;

//---------------------------------------------------------
//   BracketType
//    System Brackets
//---------------------------------------------------------

enum class BracketType : signed char {
    NORMAL, BRACE, SQUARE, LINE, NO_BRACKET = -1
};

//---------------------------------------------------------
//   TransposeDirection
//---------------------------------------------------------

enum class TransposeDirection : char {
    UP, DOWN, CLOSEST, UNKNOWN
};

//---------------------------------------------------------
//   TransposeMode
//---------------------------------------------------------

enum class TransposeMode : char {
    TO_KEY, BY_INTERVAL, DIATONICALLY, UNKNOWN
};

//---------------------------------------------------------
//   SelectType
//---------------------------------------------------------

enum class SelectType : char {
    SINGLE, RANGE, ADD, REPLACE
};

//---------------------------------------------------------
//    AccidentalVal
//---------------------------------------------------------

enum class AccidentalVal : signed char {
    SHARP3  = 3,
    SHARP2  = 2,
    SHARP   = 1,
    NATURAL = 0,
    FLAT    = -1,
    FLAT2   = -2,
    FLAT3   = -3,
    MIN     = FLAT3,
    MAX     = SHARP3
};

//---------------------------------------------------------
//    KeySigNaturals (positions of naturals in key sig. changes)
//---------------------------------------------------------

enum class KeySigNatural : char {
    NONE   = 0,               // no naturals, except for change to CMaj/Amin
    BEFORE = 1,               // naturals before accidentals
    AFTER  = 2                // naturals after accidentals (but always before if going sharps <=> flats)
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
constexpr int STAFF_GROUP_MAX = int(StaffGroup::TAB) + 1; // out of enum to avoid compiler complains about not handled switch cases

//---------------------------------------------------------
//   MScoreError
//---------------------------------------------------------

enum class MsError {
    MS_NO_ERROR,
    NO_NOTE_SELECTED,
    NO_CHORD_REST_SELECTED,
    NO_LYRICS_SELECTED,
    NO_NOTE_REST_SELECTED,
    NO_FLIPPABLE_SELECTED,
    NO_STAFF_SELECTED,
    NO_NOTE_FIGUREDBASS_SELECTED,
    CANNOT_INSERT_TUPLET,
    CANNOT_SPLIT_TUPLET,
    CANNOT_SPLIT_MEASURE_FIRST_BEAT,
    CANNOT_SPLIT_MEASURE_TUPLET,
    INSUFFICIENT_MEASURES,
    CANNOT_SPLIT_MEASURE_REPEAT,
    CANNOT_SPLIT_MEASURE_TOO_SHORT,
    CANNOT_REMOVE_TIME_TUPLET,
    CANNOT_REMOVE_TIME_MEASURE_REPEAT,
    NO_DEST,
    DEST_TUPLET,
    TUPLET_CROSSES_BAR,
    DEST_LOCAL_TIME_SIGNATURE,
    DEST_TREMOLO,
    NO_MIME,
    DEST_NO_CR,
    CANNOT_CHANGE_LOCAL_TIMESIG,
    CORRUPTED_MEASURE,
};

/// \cond PLUGIN_API \private \endcond
struct MScoreError {
    MsError no;
    const char* group;
    const char* txt;
};

//---------------------------------------------------------
//   MScore
//    MuseScore application object
//---------------------------------------------------------

class MScore
{
    Q_GADGET

    static QString _globalShare;
    static int _hRaster, _vRaster;
    static bool _verticalOrientation;

public:

    static MsError _error;
    static std::vector<MScoreError> errorList;

    static void init();
    static void registerUiTypes();

    static const QString& globalShare() { return _globalShare; }
    static qreal hRaster() { return _hRaster; }
    static qreal vRaster() { return _vRaster; }
    static void setHRaster(int val) { _hRaster = val; }
    static void setVRaster(int val) { _vRaster = val; }
    static void setNudgeStep(qreal val) { nudgeStep = val; }
    static void setNudgeStep10(qreal val) { nudgeStep10 = val; }
    static void setNudgeStep50(qreal val) { nudgeStep50 = val; }

    static bool verticalOrientation() { return _verticalOrientation; }
    static void setVerticalOrientation(bool val) { _verticalOrientation = val; }

    static bool warnPitchRange;
    static int pedalEventsMinTicks;

    static bool harmonyPlayDisableCompatibility;
    static bool harmonyPlayDisableNew;
    static bool playRepeats;
    static int playbackSpeedIncrement;
    static qreal nudgeStep;
    static qreal nudgeStep10;
    static qreal nudgeStep50;
    static int defaultPlayDuration;
    static QString lastError;

// #ifndef NDEBUG
    static bool noHorizontalStretch;
    static bool noVerticalStretch;
    static bool showSegmentShapes;
    static bool showSkylines;
    static bool showMeasureShapes;
    static bool showBoundingRect;
    static bool showSystemBoundingRect;
    static bool showCorruptedMeasures;
    static bool useFallbackFont;
// #endif
    static bool debugMode;
    static bool testMode;

    static int sampleRate;
    static int mtcType;

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

    static void setError(MsError e) { _error = e; }
    static const char* errorMessage();
    static const char* errorGroup();
};

//---------------------------------------------------------
//   center
//---------------------------------------------------------

static constexpr qreal center(qreal x1, qreal x2)
{
    return x1 + (x2 - x1) * .5;
}

//---------------------------------------------------------
//   limit
//---------------------------------------------------------

static constexpr int limit(int val, int min, int max)
{
    if (val > max) {
        return max;
    }
    if (val < min) {
        return min;
    }
    return val;
}
} // namespace Ms

#endif
