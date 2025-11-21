/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#pragma once

#include "global/containers.h"

#include "engraving/types/types.h"

namespace mu::engraving {
static constexpr size_t VOICES = 4;

inline constexpr track_idx_t staff2track(staff_idx_t staffIdx, voice_idx_t voiceIdx = 0)
{
    return staffIdx != muse::nidx ? staffIdx * VOICES + voiceIdx : muse::nidx;
}

inline constexpr staff_idx_t track2staff(track_idx_t track)
{
    return track != muse::nidx ? track / VOICES : muse::nidx;
}

inline constexpr voice_idx_t track2voice(track_idx_t track)
{
    return track != muse::nidx ? track % VOICES : muse::nidx;
}

inline constexpr track_idx_t trackZeroVoice(track_idx_t track)
{
    return track != muse::nidx ? (track / VOICES) * VOICES : muse::nidx;
}

inline constexpr bool isUpVoice(voice_idx_t voiceIdx)
{
    return !(voiceIdx & 1);
}

inline constexpr bool isDownVoice(voice_idx_t voiceIdx)
{
    return voiceIdx & 1;
}

static constexpr int MAX_HEADERS = 3;
static constexpr int MAX_FOOTERS = 3;

// UNIVERSAL UNITS
static constexpr double INCH      = 25.4; // millimitres per inch
static constexpr double PPI       = 72.0; // typographical points per inch

// INTERNAL DRAWING UNITS
static constexpr double DPI = 1200;
static constexpr double DPMM = DPI / INCH;

// NOTE: the SMuFL default is actually 20pt. We use 10 for historical reasons
// and back-compatibility, but this will be multiplied x2 during layout.
static constexpr double MUSICAL_SYMBOLS_DEFAULT_FONT_SIZE = 10.0;

static constexpr double UI_ICONS_DEFAULT_FONT_SIZE = 12.0;

static constexpr int MAX_STAVES = 4;

static constexpr char mimeSymbolFormat[]     = "application/musescore/symbol";
static constexpr char mimeSymbolListFormat[] = "application/musescore/symbollist";
static constexpr char mimeStaffListFormat[]  = "application/musescore/stafflist";

static constexpr int INVALID_STRING_INDEX = -1; // no ordinal for a physical string (0 = topmost in instrument)
static constexpr int INVALID_FRET_INDEX   = -1; // no ordinal for a fret

static constexpr muse::ID INVALID_ID = 0;

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
//   UpDownMode
//---------------------------------------------------------

enum class UpDownMode : char {
    CHROMATIC, OCTAVE, DIATONIC
};

//---------------------------------------------------------
//   OffsetType
//---------------------------------------------------------

enum class OffsetType : unsigned char {
    ABS,         ///< offset in point units
    SPATIUM      ///< offset in staff space units
};

//---------------------------------------------------------
//   MScoreError
//---------------------------------------------------------

enum class MsError : unsigned char {
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
    SOURCE_PARTIAL_TUPLET,
    DEST_TUPLET,
    TUPLET_CROSSES_BAR,
    DEST_LOCAL_TIME_SIGNATURE,
    SOURCE_PARTIAL_TREMOLO,
    DEST_TREMOLO,
    NO_MIME,
    DEST_NO_CR,
    CANNOT_CHANGE_LOCAL_TIMESIG_MEASURE_NOT_EMPTY,
    CANNOT_CHANGE_LOCAL_TIMESIG_HAS_EXCERPTS,
    CORRUPTED_MEASURE,
    CANNOT_REMOVE_KEY_SIG,
    CANNOT_JOIN_MEASURE_STAFFTYPE_CHANGE,
    CANNOT_REPEAT_SELECTION,
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
    static bool warnGuitarBends;
    static int pedalEventsMinTicks;

    static double nudgeStep;
    static double nudgeStep10;
    static double nudgeStep50;

// #ifndef NDEBUG
    static bool noHorizontalStretch;
    static bool noVerticalStretch;
    static bool useFallbackFont;
// #endif
    static bool debugMode;
    static bool testMode;
    static bool testWriteStyleToScore;

    static bool saveTemplateMode;
    static bool noGui;

    static bool noImages;

    static bool pdfPrinting;
    static bool svgPrinting;

    static double verticalPageGap;
    static double horizontalPageGapEven;
    static double horizontalPageGapOdd;

    static void setError(MsError e) { _error = e; }

    static std::string errorToString(MsError err);
};
} // namespace mu::engraving
