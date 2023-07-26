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

#ifndef __STAFFTYPE_H__
#define __STAFFTYPE_H__

#include "draw/types/color.h"
#include "draw/types/font.h"

#include "engravingitem.h"
#include "mscore.h"

#include "types/types.h"

#include "modularity/ioc.h"
#include "iengravingconfiguration.h"

namespace mu::engraving {
class Chord;
class ChordRest;
class Staff;

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
#define STAFFTYPE_TAB_SLASH_2TOTHEIGHT     (STAFFTYPE_TAB_SLASH_THICK + STAFFTYPE_TAB_SLASH_DISPL + STAFFTYPE_TAB_SLASH_SLANTY)
// the initial Y coord for a double shash on an UP stem = topmost corner of topmost slash
#define STAFFTYPE_TAB_SLASH_2STARTY_UP     ((STAFFTYPE_TAB_DEFAULTSTEMLEN_UP - STAFFTYPE_TAB_SLASH_2TOTHEIGHT) * 0.5)
// the initial Y coord for a double shash on an DN stem = topmost corner of topmost slash
#define STAFFTYPE_TAB_SLASH_2STARTY_DN     ((STAFFTYPE_TAB_DEFAULTSTEMLEN_UP + STAFFTYPE_TAB_SLASH_2TOTHEIGHT) * 0.5)
// same for a 4-ple slash
#define STAFFTYPE_TAB_SLASH_4TOTHEIGHT     (STAFFTYPE_TAB_SLASH_THICK + STAFFTYPE_TAB_SLASH_DISPL * 3 + STAFFTYPE_TAB_SLASH_SLANTY)
// the initial Y coord for a double shash on an UP stem = topmost corner of topmost slash
#define STAFFTYPE_TAB_SLASH_4STARTY_UP     ((STAFFTYPE_TAB_DEFAULTSTEMLEN_UP - STAFFTYPE_TAB_SLASH_4TOTHEIGHT) * 0.5)
// the initial Y coord for a double shash on an DN stem = topmost corner of topmost slash
#define STAFFTYPE_TAB_SLASH_4STARTY_DN     ((STAFFTYPE_TAB_DEFAULTSTEMLEN_UP + STAFFTYPE_TAB_SLASH_4TOTHEIGHT) * 0.5)

// HISTORIC TAB BASS STRING NOTATION
// The following constants refer to the specifics of bass string notation in historic
//    (Renaiss./Baroque French and Italian) tablatures.

// how much to lower a bass string note with slashes with respect to line distance (in fraction of line distance)
#define STAFFTYPE_TAB_BASSSLASH_YOFFSET   0.33
// The following constants could ideally be customizable values;
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
static const double GRID_BEAM_DEF_WIDTH  = 0.25; // all values in sp
static const double GRID_STEM_DEF_HEIGHT = 1.75;
static const double GRID_STEM_DEF_WIDTH  = 0.125;

struct TablatureFretFont {
    String family;                             // the family of the physical font to use
    String displayName;                        // the name to display to the user
    double defPitch;                             // the default size of the font
    double defYOffset;                           // the default Y displacement
    Char xChar;                                // the char to use for 'x'
    Char deadNoteChar;                            // the char to use for dead notes
    String slashChar[NUM_OF_BASSSTRING_SLASHES];  // the char used to draw one or more '/' symbols
    String displayDigit[NUM_OF_DIGITFRETS];    // the string to draw for digit frets
    Char displayLetter[NUM_OF_LETTERFRETS];    // the char to use for letter frets

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
    NONE = 0,                         // do not draw half notes at all
    SHORTER,                          // draw half notes with a shorter stem
    SLASHED                           // draw half notes with stem with two slashes
};

enum class TablatureSymbolRepeat : char {
    NEVER = 0,                      // never repeat the same duration symbol
    SYSTEM,                         // repeat at the beginning of a new system
    MEASURE,                        // repeat at the beginning of a new measure
    ALWAYS                          // always repeat
};

struct TablatureDurationFont {
    String family;                   // the family of the physical font to use
    String displayName;              // the name to display to the user
    double defPitch;                   // the default size of the font
    double defYOffset;                 // the default Y displacement
    double gridBeamWidth  = GRID_BEAM_DEF_WIDTH;       // the width of the 'grid'-style beam (in sp)
    double gridStemHeight = GRID_STEM_DEF_HEIGHT;      // the height of the 'grid'-style stem (in sp)
    double gridStemWidth  = GRID_STEM_DEF_WIDTH;       // the width of the 'grid'-style stem (in sp)
    // the note value with no beaming in 'grid'-style beaming
    DurationType zeroBeamLevel = DurationType::V_QUARTER;
    Char displayDot;                 // the char to use to draw a dot
    Char displayValue[int(TabVal::NUM_OF)];           // the char to use to draw a duration value

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
    TAB_7COMMON, TAB_8COMMON, TAB_9COMMON, TAB_10COMMON,
    TAB_7SIMPLE, TAB_8SIMPLE, TAB_9SIMPLE, TAB_10SIMPLE,
    STAFF_TYPES,
    // some useful shorthands:
    PERC_DEFAULT = StaffTypes::PERC_5LINE,
    TAB_DEFAULT = StaffTypes::TAB_6COMMON,
};

//---------------------------------------------------------
//   StaffType
//---------------------------------------------------------

class StaffType
{
    INJECT_STATIC(IEngravingConfiguration, engravingConfiguration)

    friend class TabDurationSymbol;

    StaffGroup _group = StaffGroup::STANDARD;

    String _xmlName;         // the name used to reference this preset in instruments.xml
    String _name;            // user visible name

    double _userMag           { 1.0 };           // allowed 0.1 - 10.0
    Spatium _yoffset         { 0.0 };
    bool _small              { false };
    bool _invisible          { false };
    mu::draw::Color _color   { engravingConfiguration()->defaultColor() };

    int _lines            = 5;
    int _stepOffset       = 0;
    Spatium _lineDistance = Spatium(1);

    bool _showBarlines    = true;
    bool _showLedgerLines = true;
    bool _stemless        = false;        // do not show stems

    bool _genClef         = true;         // create clef at beginning of system
    bool _genTimesig      = true;         // whether time signature is shown or not
    bool _genKeysig       = true;         // create key signature at beginning of system

    // Standard: configurable properties
    NoteHeadScheme _noteHeadScheme = NoteHeadScheme::HEAD_NORMAL;

    // TAB: configurable properties
    double _durationFontSize = 15.0;       // the size (in points) for the duration symbol font
    double _durationFontUserY = 0.0;       // the vertical offset (spatium units) for the duration symb. font
                                           // user configurable
    double _fretFontSize  = 10.0;          // the size (in points) for the fret marks font
    double _fretFontUserY = 0.0;           // additional vert. offset of fret marks with respect to
                                           // the string line (spatium unit); user configurable
    bool _genDurations = false;           // whether duration symbols are drawn or not
    bool _linesThrough = false;           // whether lines for strings and stems may pass through fret marks or not
    TablatureMinimStyle _minimStyle = TablatureMinimStyle::NONE;      // how to draw minim stems (stem-and-beam durations only)
    TablatureSymbolRepeat _symRepeat = TablatureSymbolRepeat::NEVER;  // if and when to repeat the same duration symbol
    bool _onLines      = true;            // whether fret marks are drawn on the string lines or between them
    bool _showRests    = false;           // whether to draw rests or not
    bool _stemsDown    = true;            // stems are drawn downward (stem-and-beam durations only)
    bool _stemsThrough = true;            // stems are drawn through the staff rather than beside it (stem-and-beam durations only)
    bool _upsideDown   = false;           // whether lines are drawn with highest string at top (false) or at bottom (true)
    bool _showTabFingering   = false;           // Allow fingering in tablature staff (true) or not (false)
    bool _useNumbers   = true;            // true: use numbers ('0' - ...) for frets | false: use letters ('a' - ...)
    bool _showBackTied = true;            // whether back-tied notes are shown or not

    // TAB: internally managed variables
    // Note: values in RASTER UNITS are independent from score scaling and
    //    must be multiplied by magS() to be used in contexts using sp units
    mutable double _durationBoxH = 0.0;
    double mutable _durationBoxY = 0.0;            // the height and the y rect.coord. (relative to staff top line)
    // of a box bounding all duration symbols (raster units) internally computed:
    // depends upon _onString and the metrics of the duration font
    mu::draw::Font _durationFont;                  // font used to draw dur. symbols; cached for efficiency
    size_t _durationFontIdx = 0;             // the index of current dur. font in dur. font array
    mutable double _durationYOffset = 0.0;         // the vertical offset to draw duration symbols with respect to the
    // string lines (raster units); internally computed: depends upon _onString and duration font
    mutable double _durationGridYOffset = 0.0;     // the vertical offset to draw the bottom of duration grid with respect to the
    // string lines (raster units); internally computed: depends upon _onstring and duration font
    mutable bool _durationMetricsValid = false;     // whether duration font metrics are valid or not
    mutable double _fretBoxH = 0.0;
    mutable double _fretBoxY = 0.0;                // the height and the y rect.coord. (relative to staff line)
    // of a box bounding all fret characters (raster units) internally computed:
    // depends upon _onString, _useNumbers and the metrics of the fret font
    mu::draw::Font _fretFont;                      // font used to draw fret marks; cached for efficiency
    size_t _fretFontIdx = 0;                 // the index of current fret font in fret font array
    mutable double _fretYOffset = 0.0;             // the vertical offset to draw fret marks with respect to the string lines;
    // (raster units); internally computed: depends upon _onString, _useNumbers
    // and the metrics of the fret font
    mutable bool _fretMetricsValid = false;       // whether fret font metrics are valid or not
    mutable double _refDPI = 0.0;                  // reference value used to last computed metrics and to see if they are still valid

    // the array of configured fonts
    static std::vector<TablatureFretFont> _fretFonts;
    static std::vector<TablatureDurationFont> _durationFonts;
    static std::vector<StaffType> _presets;

    void  setDurationMetrics() const;
    void  setFretMetrics() const;

    static bool readConfigFile(const String& fileName);

public:
    StaffType();

    StaffType(StaffGroup sg, const String& xml, const String& name, int lines, int stpOff, double lineDist, bool genClef, bool showBarLines,
              bool stemless, bool genTimeSig, bool genKeySig, bool showLedgerLiness, bool invisible, const mu::draw::Color& color);

    StaffType(StaffGroup sg, const String& xml, const String& name, int lines, int stpOff, double lineDist, bool genClef, bool showBarLines,
              bool stemless, bool genTimesig, bool invisible, const mu::draw::Color& color, const String& durFontName, double durFontSize,
              double durFontUserY, double genDur, const String& fretFontName, double fretFontSize, double fretFontUserY,
              TablatureSymbolRepeat symRepeat, bool linesThrough, TablatureMinimStyle minimStyle, bool onLines, bool showRests,
              bool stemsDown, bool stemThrough, bool upsideDown, bool showTabFingering, bool useNumbers, bool showBackTied);

    virtual ~StaffType() = default;

    bool operator==(const StaffType&) const;

    StaffGroup group() const { return _group; }
    void setGroup(StaffGroup g) { _group = g; }
    StaffTypes type() const;
    const String& name() const { return _name; }
    const String& xmlName() const { return _xmlName; }
    void setName(const String& val) { _name = val; }
    void setXmlName(const String& val) { _xmlName = val; }
    String translatedGroupName() const;

    void setLines(int val) { _lines = val; }
    int lines() const { return _lines; }
    int middleLine() const;
    int bottomLine() const;
    void setStepOffset(int v) { _stepOffset = v; }
    int stepOffset() const { return _stepOffset; }
    void setLineDistance(const Spatium& val) { _lineDistance = val; }
    Spatium lineDistance() const { return _lineDistance; }
    void setGenClef(bool val) { _genClef = val; }
    bool genClef() const { return _genClef; }
    void setShowBarlines(bool val) { _showBarlines = val; }
    bool showBarlines() const { return _showBarlines; }
    double userMag() const { return _userMag; }
    bool isSmall() const { return _small; }
    bool invisible() const { return _invisible; }
    const mu::draw::Color& color() const { return _color; }
    void setUserMag(double val) { _userMag = val; }
    void setSmall(bool val) { _small = val; }
    void setInvisible(bool val) { _invisible = val; }
    void setColor(const mu::draw::Color& val) { _color = val; }
    Spatium yoffset() const { return _yoffset; }
    void setYoffset(Spatium val) { _yoffset = val; }
    double spatium(const MStyle& style) const;

    void setStemless(bool val) { _stemless = val; }
    bool stemless() const { return _stemless; }
    bool genTimesig() const { return _genTimesig; }
    void setGenTimesig(bool val) { _genTimesig = val; }
    double doty1() const;
    double doty2() const;

    // static function to deal with presets
    static const StaffType* getDefaultPreset(StaffGroup grp);
    static const StaffType* preset(StaffTypes idx);
    static const StaffType* presetFromXmlName(const String& xmlName);

    void setGenKeysig(bool val) { _genKeysig = val; }
    bool genKeysig() const { return _genKeysig; }
    void setShowLedgerLines(bool val) { _showLedgerLines = val; }
    bool showLedgerLines() const { return _showLedgerLines; }
    void setNoteHeadScheme(NoteHeadScheme s) { _noteHeadScheme = s; }
    NoteHeadScheme noteHeadScheme() const { return _noteHeadScheme; }

    String fretString(int fret, int string, bool deadNote) const;     // returns a string with the text for fret
    String durationString(DurationType type, int dots) const;

    // functions to cope with historic TAB's peculiarities, like upside-down, bass string notations
    int     physStringToVisual(int strg) const;                   // return the string in visual order from physical string
    int     visualStringToPhys(int line) const;                   // return the string in physical order from visual string
    double   physStringToYOffset(int strg) const;                  // return the string Y offset (in sp, chord-relative)
    String tabBassStringPrefix(int strg, bool* hasFret) const;   // return a string with the prefix, if any, identifying a bass string
    void    drawInputStringMarks(mu::draw::Painter* p, int string, voice_idx_t voice, const RectF& rect) const;
    int     numOfTabLedgerLines(int string) const;

    // properties getters (some getters require updated metrics)
    double durationBoxH() const;
    double durationBoxY() const;

    const mu::draw::Font& durationFont() const { return _durationFont; }
    const String durationFontName() const { return _durationFonts[_durationFontIdx].displayName; }
    double durationFontSize() const { return _durationFontSize; }
    double durationFontUserY() const { return _durationFontUserY; }
    double durationFontYOffset() const { setDurationMetrics(); return _durationYOffset + _durationFontUserY * SPATIUM20; }
    double durationGridYOffset() const { setDurationMetrics(); return _durationGridYOffset; }
    double fretBoxH() const { setFretMetrics(); return _fretBoxH; }
    double fretBoxY() const { setFretMetrics(); return _fretBoxY + _fretFontUserY * SPATIUM20; }

    // 2 methods to return the size of a box masking lines under a fret mark
    double fretMaskH() const { return _lineDistance.val() * SPATIUM20; }
    double fretMaskY() const { return (_onLines ? -0.5 : -1.0) * _lineDistance.val() * SPATIUM20; }

    const mu::draw::Font& fretFont() const { return _fretFont; }
    const String fretFontName() const { return _fretFonts[_fretFontIdx].displayName; }
    double fretFontSize() const { return _fretFontSize; }
    double fretFontUserY() const { return _fretFontUserY; }
    double fretFontYOffset() const { setFretMetrics(); return _fretYOffset + _fretFontUserY * SPATIUM20; }
    bool  genDurations() const { return _genDurations; }
    bool  linesThrough() const { return _linesThrough; }
    TablatureMinimStyle minimStyle() const { return _minimStyle; }
    TablatureSymbolRepeat symRepeat() const { return _symRepeat; }
    bool  onLines() const { return _onLines; }
    bool  showRests() const { return _showRests; }
    bool  stemsDown() const { return _stemsDown; }
    bool  stemThrough() const { return _stemsThrough; }
    bool  upsideDown() const { return _upsideDown; }
    bool  showTabFingering() const { return _showTabFingering; }
    bool  useNumbers() const { return _useNumbers; }
    bool  showBackTied() const { return _showBackTied; }

    // properties setters (setting some props invalidates metrics)
    void  setDurationFontName(const String&);
    void  setDurationFontSize(double);
    void  setDurationFontUserY(double val) { _durationFontUserY = val; }
    void  setFretFontName(const String&);
    void  setFretFontSize(double);
    void  setFretFontUserY(double val) { _fretFontUserY = val; }
    void  setGenDurations(bool val) { _genDurations = val; }
    void  setLinesThrough(bool val) { _linesThrough = val; }
    void  setMinimStyle(TablatureMinimStyle val) { _minimStyle = val; }
    void  setSymbolRepeat(TablatureSymbolRepeat val) { _symRepeat  = val; }
    void  setOnLines(bool);
    void  setShowRests(bool val) { _showRests = val; }
    void  setStemsDown(bool val) { _stemsDown = val; }
    void  setStemsThrough(bool val) { _stemsThrough = val; }
    void  setUpsideDown(bool val) { _upsideDown = val; }
    void  setShowTabFingering(bool val) { _showTabFingering = val; }
    void  setUseNumbers(bool val) { _useNumbers = val; _fretMetricsValid = false; }
    void  setShowBackTied(bool val) { _showBackTied = val; }

    // utility functions for tab specially managed elements
    mu::PointF chordStemPos(const Chord*) const;
    double   chordRestStemPosY(const ChordRest*) const;
    double   chordStemPosX(const Chord*) const { return STAFFTYPE_TAB_DEFAULTSTEMPOSX; }
    mu::PointF chordStemPosBeam(const Chord*) const;
    double   chordStemLength(const Chord*) const;

    bool isTabStaff() const { return _group == StaffGroup::TAB; }
    bool isDrumStaff() const { return _group == StaffGroup::PERCUSSION; }

    bool isSimpleTabStaff() const;
    bool isCommonTabStaff() const;
    bool isHiddenElementOnTab(const MStyle& style, Sid commonTabStyle, Sid simpleTabStyle) const;

    // static functions for font config files
    static std::vector<String> fontNames(bool bDuration);
    static bool fontData(bool bDuration, size_t nIdx, String* pFamily, String* pDisplayName, double* pSize, double* pYOff);

    static void initStaffTypes();
    static const std::vector<StaffType>& presets() { return _presets; }
};

//---------------------------------------------------------
//   TabDurationSymbol
//    EngravingItem used to draw duration symbols above tablatures
//---------------------------------------------------------

enum class TabBeamGrid : char {
    NONE = 0,
    INITIAL,
    MEDIALFINAL,
    NUM_OF
};

class TabDurationSymbol final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, TabDurationSymbol)

public:
    TabDurationSymbol(ChordRest* parent);
    TabDurationSymbol(ChordRest* parent, const StaffType* tab, DurationType type, int dots);
    TabDurationSymbol(const TabDurationSymbol&);
    TabDurationSymbol* clone() const override { return new TabDurationSymbol(*this); }
    void draw(mu::draw::Painter*) const override;
    bool isEditable() const override { return false; }

    TabBeamGrid beamGrid() { return m_beamGrid; }
    void setBeamGrid(TabBeamGrid g) { m_beamGrid = g; }

    double beamLength() const { return m_beamLength; }
    void setBeamLength(double l) { m_beamLength = l; }
    int beamLevel() const { return m_beamLevel; }
    void setBeamLevel(int l) { m_beamLevel = l; }

    void layout2();                 // second step of layout: after horiz. pos. are defined, compute width of 'grid beams'

    const StaffType* tab() const { return m_tab; }
    const String& text() const { return m_text; }
    void setDuration(DurationType type, int dots, const StaffType* tab)
    {
        m_tab = tab;
        m_text = tab->durationString(type, dots);
    }

    bool isRepeat() const { return m_repeat; }
    void setRepeat(bool val) { m_repeat = val; }

private:

    double m_beamLength = 0.0;                      // if _grid==MEDIALFINAL, length of the beam toward previous grid element
    int m_beamLevel = 0;                            // if _grid==MEDIALFINAL, the number of beams
    TabBeamGrid m_beamGrid = TabBeamGrid::NONE;     // value for special 'English' grid display
    const StaffType* m_tab = nullptr;
    String m_text;
    bool m_repeat = false;
};
} // namespace mu::engraving
#endif
