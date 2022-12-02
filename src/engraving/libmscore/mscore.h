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

#include "global/containers.h"

#include "engraving/types/types.h"

namespace mu::engraving {
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

static constexpr size_t VOICES = 4;

inline constexpr track_idx_t staff2track(staff_idx_t staffIdx, voice_idx_t voiceIdx = 0)
{
    return staffIdx != mu::nidx ? staffIdx * VOICES + voiceIdx : mu::nidx;
}

inline constexpr staff_idx_t track2staff(track_idx_t track)
{
    return track != mu::nidx ? track / VOICES : mu::nidx;
}

inline constexpr voice_idx_t track2voice(track_idx_t track)
{
    return track != mu::nidx ? track % VOICES : mu::nidx;
}

inline constexpr track_idx_t trackZeroVoice(track_idx_t track)
{
    return track != mu::nidx ? (track / VOICES) * VOICES : mu::nidx;
}

inline constexpr bool isUpVoice(voice_idx_t voiceIdx)
{
    return !(voiceIdx & 1);
}

inline constexpr bool isDownVoice(voice_idx_t voiceIdx)
{
    return voiceIdx & 1;
}

static constexpr int MAX_TAGS = 32;

static constexpr int MAX_HEADERS = 3;
static constexpr int MAX_FOOTERS = 3;

static constexpr double INCH      = 25.4;
static constexpr double PPI       = 72.0; // printer points per inch
static constexpr double DPI_F     = 5;
static constexpr double DPI       = 72.0 * DPI_F;
static constexpr double SPATIUM20 = 5.0 * (DPI / 72.0);
static constexpr double DPMM      = DPI / INCH;

static constexpr int MAX_STAVES = 4;

static constexpr char mimeSymbolFormat[]     = "application/musescore/symbol";
static constexpr char mimeSymbolListFormat[] = "application/musescore/symbollist";
static constexpr char mimeStaffListFormat[]  = "application/musescore/stafflist";

static constexpr int INVALID_STRING_INDEX = -1; // no ordinal for a physical string (0 = topmost in instrument)
static constexpr int INVALID_FRET_INDEX   = -1; // no ordinal for a fret

static constexpr ID INVALID_ID = 0;

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
    static int _hRaster, _vRaster;
    static bool _verticalOrientation;

public:

    static MsError _error;

    static void init();
    static void registerUiTypes();

    static double hRaster() { return _hRaster; }
    static double vRaster() { return _vRaster; }
    static void setHRaster(int val) { _hRaster = val; }
    static void setVRaster(int val) { _vRaster = val; }
    static void setNudgeStep(double val) { nudgeStep = val; }
    static void setNudgeStep10(double val) { nudgeStep10 = val; }
    static void setNudgeStep50(double val) { nudgeStep50 = val; }

    static bool verticalOrientation() { return _verticalOrientation; }
    static void setVerticalOrientation(bool val) { _verticalOrientation = val; }

    static bool warnPitchRange;
    static int pedalEventsMinTicks;

    static bool playRepeats;
    static int playbackSpeedIncrement;
    static double nudgeStep;
    static double nudgeStep10;
    static double nudgeStep50;
    static int defaultPlayDuration;

// #ifndef NDEBUG
    static bool noHorizontalStretch;
    static bool noVerticalStretch;
    static bool useFallbackFont;
// #endif
    static bool debugMode;
    static bool testMode;
    static bool testWriteStyleToScore;

    static int sampleRate;
    static int mtcType;

    static bool saveTemplateMode;
    static bool noGui;

    static bool noExcerpts;
    static bool noImages;

    static bool pdfPrinting;
    static bool svgPrinting;
    static double pixelRatio;

    static double verticalPageGap;
    static double horizontalPageGapEven;
    static double horizontalPageGapOdd;

    static void setError(MsError e) { _error = e; }

    static std::string errorToString(MsError err);
};
} // namespace mu::engraving

#endif
