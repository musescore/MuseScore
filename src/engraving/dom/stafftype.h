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

#ifndef MU_ENGRAVING_STAFFTYPE_H
#define MU_ENGRAVING_STAFFTYPE_H

#include "draw/types/font.h"

#include "engravingitem.h"
#include "mscore.h"

#include "../types/types.h"

#include "modularity/ioc.h"
#include "../iengravingconfiguration.h"

namespace mu::engraving {
class Chord;
class ChordRest;
class Staff;

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
    TablatureFretFont();

    String family;                                            // the family of the physical font to use
    String displayName;                                       // the name to display to the user
    double defSize = 9.0;                                     // the default size of the font
    double defYOffset = 0.0;                                  // the default Y displacement
    Char xChar = u'X';                                        // the char to use for 'x'
    std::array<String, NUM_OF_BASSSTRING_SLASHES> slashChar;  // the char used to draw one or more '/' symbols
    std::array<String, NUM_OF_DIGITFRETS> displayDigit;       // the string to draw for digit frets
    Char displayLetter[NUM_OF_LETTERFRETS];                   // the char to use for letter frets

    bool read(XmlReader&, int mscVersion);
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
    double defSize;                  // the default size of the font
    double defYOffset;               // the default Y displacement
    double gridBeamWidth  = GRID_BEAM_DEF_WIDTH;       // the width of the 'grid'-style beam (in sp)
    double gridStemHeight = GRID_STEM_DEF_HEIGHT;      // the height of the 'grid'-style stem (in sp)
    double gridStemWidth  = GRID_STEM_DEF_WIDTH;       // the width of the 'grid'-style stem (in sp)
    // the note value with no beaming in 'grid'-style beaming
    DurationType zeroBeamLevel = DurationType::V_QUARTER;
    Char displayDot;                 // the char to use to draw a dot
    Char displayValue[int(TabVal::NUM_OF)];           // the char to use to draw a duration value

    bool read(XmlReader&, int mscVersion);
};

// ready-made staff types
// keep in sync with the _presets initialization in StaffType::initStaffTypes() and _defaultPreset

enum class StaffTypes : signed char {
    STANDARD,
    PERC_1LINE, PERC_2LINE, PERC_3LINE, PERC_5LINE,
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

enum class ShowTiedFret : unsigned char {
    TIE_AND_FRET,
    TIE,
    NONE,
};

enum class ParenthesizeTiedFret : unsigned char {
    START_OF_SYSTEM,
    START_OF_MEASURE,
    NEVER,
};

//---------------------------------------------------------
//   StaffType
//---------------------------------------------------------

class StaffType
{
    static inline muse::GlobalInject<IEngravingConfiguration> configuration;
public:
    StaffType();

    StaffType(StaffGroup sg, const String& xml, const String& name, int lines, int stpOff, double lineDist, bool genClef, bool showBarLines,
              bool stemless, bool genTimeSig, bool genKeySig, bool showLedgerLiness, bool invisible, const Color& color);

    StaffType(StaffGroup sg, const String& xml, const String& name, int lines, int stpOff, double lineDist, bool genClef, bool showBarLines,
              bool stemless, bool genTimesig, bool invisible, const Color& color, const String& durFontName, double durFontSize,
              double durFontUserY, double genDur, bool fretFontUseTextStyle, const String& fretFontName, double fretFontSize,
              double fretFontUserY, TablatureSymbolRepeat symRepeat, bool linesThrough, TablatureMinimStyle minimStyle, bool onLines,
              bool showRests, bool stemsDown, bool stemThrough, bool upsideDown, bool showTabFingering, bool useNumbers, bool showBackTied);

    virtual ~StaffType() = default;

    bool operator==(const StaffType&) const;

    const MStyle& style() const;
    const Score* score() const { return m_score; }

    StaffGroup group() const { return m_group; }
    void setGroup(StaffGroup g) { m_group = g; }
    StaffTypes type() const;
    const String& name() const { return m_name; }
    const String& xmlName() const { return m_xmlName; }
    void setName(const String& val) { m_name = val; }
    void setXmlName(const String& val) { m_xmlName = val; }
    String translatedGroupName() const;

    void setLines(int val) { m_lines = val; }
    int lines() const { return m_lines; }
    int middleLine() const;
    int bottomLine() const;
    void setStepOffset(int v) { m_stepOffset = v; }
    int stepOffset() const { return m_stepOffset; }
    void setLineDistance(const Spatium& val) { m_lineDistance = val; }
    Spatium lineDistance() const { return m_lineDistance; }
    void setGenClef(bool val) { m_genClef = val; }
    bool genClef() const { return m_genClef; }
    void setShowBarlines(bool val) { m_showBarlines = val; }
    bool showBarlines() const { return m_showBarlines; }
    double userMag() const { return m_userMag; }
    bool isSmall() const { return m_small; }
    bool invisible() const { return m_invisible; }
    const Color& color() const { return m_color; }
    void setUserMag(double val) { m_userMag = val; }
    void setSmall(bool val) { m_small = val; }
    void setInvisible(bool val) { m_invisible = val; }
    void setColor(const Color& val) { m_color = val; }
    Spatium yoffset() const { return m_yoffset; }
    void setYoffset(Spatium val) { m_yoffset = val; }
    double spatium() const;

    void setStemless(bool val) { m_stemless = val; }
    bool stemless() const { return m_stemless; }
    bool genTimesig() const { return m_genTimesig; }
    void setGenTimesig(bool val) { m_genTimesig = val; }
    double doty1() const;
    double doty2() const;

    // static function to deal with presets
    static const StaffType* getDefaultPreset(StaffGroup grp);
    static const StaffType* preset(StaffTypes idx);
    static const StaffType* presetFromXmlName(const String& xmlName);

    void setGenKeysig(bool val) { m_genKeysig = val; }
    bool genKeysig() const { return m_genKeysig; }
    void setShowLedgerLines(bool val) { m_showLedgerLines = val; }
    bool showLedgerLines() const { return m_showLedgerLines; }
    void setNoteHeadScheme(NoteHeadScheme s) { m_noteHeadScheme = s; }
    NoteHeadScheme noteHeadScheme() const { return m_noteHeadScheme; }

    String fretString(int fret, int string, bool deadNote) const;     // returns a string with the text for fret
    String durationString(DurationType type, int dots) const;

    // functions to cope with historic TAB's peculiarities, like upside-down, bass string notations
    int     physStringToVisual(int strg) const;                   // return the string in visual order from physical string
    int     visualStringToPhys(int line) const;                   // return the string in physical order from visual string
    double   physStringToYOffset(int strg) const;                  // return the string Y offset (in sp, chord-relative)
    String tabBassStringPrefix(int strg, bool* hasFret) const;   // return a string with the prefix, if any, identifying a bass string
    void    drawInputStringMarks(muse::draw::Painter* p, int string, const Color& selectionColor, const RectF& rect) const;
    int     numOfTabLedgerLines(int string) const;

    // properties getters (some getters require updated metrics)
    double durationBoxH() const;
    double durationBoxY() const;

    const muse::draw::Font& durationFont() const { return m_durationFont; }
    const TablatureDurationFont& tabDurationFont() const { return m_durationFonts[m_durationFontIdx]; }
    const String& durationFontName() const { return m_durationFonts[m_durationFontIdx].displayName; }
    double durationFontSize() const { return m_durationFontSize; }
    double durationFontUserY() const { return m_durationFontUserY; }
    double durationFontYOffset() const { return m_durationYOffset + m_durationFontUserY * SPATIUM20; }
    double durationGridYOffset() const { return m_durationGridYOffset; }
    double fretBoxH() const { return m_fretBoxH; }
    double deadFretBoxH() const { return m_deadFretBoxH; }
    double fretBoxY() const { return m_fretBoxY + m_fretFontUserY * SPATIUM20; }
    double deadFretBoxY() const { return m_deadFretBoxY + m_fretFontUserY * SPATIUM20; }

    // 2 methods to return the size of a box masking lines under a fret mark
    double fretMaskH() const { return m_lineDistance.val() * SPATIUM20; }
    double fretMaskY() const { return (m_onLines ? -0.5 : -1.0) * m_lineDistance.val() * SPATIUM20; }

    const muse::draw::Font& fretFont() const { return m_fretFont; }
    const String fretFontName() const { return m_fretFontInfo.displayName; }
    double fretFontSize() const { return m_fretFontSize; }
    double fretFontUserY() const { return m_fretFontUserY; }
    double fretFontYOffset() const { return m_fretYOffset + m_fretFontUserY * SPATIUM20; }
    bool  genDurations() const { return m_genDurations; }
    bool  linesThrough() const { return m_linesThrough; }
    TablatureMinimStyle minimStyle() const { return m_minimStyle; }
    TablatureSymbolRepeat symRepeat() const { return m_symRepeat; }
    bool  onLines() const { return m_onLines; }
    bool  showRests() const { return m_showRests; }
    bool  stemsDown() const { return m_stemsDown; }
    bool  stemThrough() const { return m_stemsThrough; }
    bool  upsideDown() const { return m_upsideDown; }
    bool  showTabFingering() const { return m_showTabFingering; }
    bool  useNumbers() const { return m_useNumbers; }
    bool  showBackTied() const { return m_showBackTied; }
    bool  fretUseTextStyle() const { return m_fretUseTextStyle; }
    TextStyleType fretTextStyle() const { return m_fretTextStyle; }
    size_t fretPresetIdx() const { return m_fretPresetIdx; }

    // properties setters (setting some props invalidates metrics)
    void  setDurationFontName(const String&);
    void  setDurationFontSize(double);
    void  setDurationFontUserY(double val) { m_durationFontUserY = val; }
    void  setFretFontName(const String&);
    void  setFretFontSize(double);
    void  setFretFontUserY(double val) { m_fretFontUserY = val; }
    void  setGenDurations(bool val) { m_genDurations = val; }
    void  setLinesThrough(bool val) { m_linesThrough = val; }
    void  setMinimStyle(TablatureMinimStyle val) { m_minimStyle = val; }
    void  setSymbolRepeat(TablatureSymbolRepeat val) { m_symRepeat  = val; }
    void  setOnLines(bool);
    void  setShowRests(bool val) { m_showRests = val; }
    void  setStemsDown(bool val) { m_stemsDown = val; }
    void  setStemsThrough(bool val) { m_stemsThrough = val; }
    void  setUpsideDown(bool val) { m_upsideDown = val; }
    void  setShowTabFingering(bool val) { m_showTabFingering = val; }
    void  setUseNumbers(bool val);
    void  setShowBackTied(bool val) { m_showBackTied = val; }
    void  setScore(Score* score) { m_score = score; }
    void  setFretUseTextStyle(bool val) { m_fretUseTextStyle = val; }
    void  setFretTextStyle(const TextStyleType& val);
    void  setFretPresetIdx(size_t idx);
    void  setFretPreset(const String& str);

    bool isTabStaff() const { return m_group == StaffGroup::TAB; }
    bool isDrumStaff() const { return m_group == StaffGroup::PERCUSSION; }

    bool isSimpleTabStaff() const;
    bool isCommonTabStaff() const;
    bool isHiddenElementOnTab(Sid commonTabStyle, Sid simpleTabStyle) const;

    void styleChanged();

    // static functions for font config files
    static std::vector<String> tabFontNames(bool bDuration);
    static bool tabFontData(bool bDuration, size_t nIdx, double& pSize, double& pYOff);

    static void initStaffTypes(const Color& defaultColor);
    static const std::vector<StaffType>& presets() { return m_presets; }

private:

    friend class TabDurationSymbol;

    Score* m_score = nullptr;

    void  setDurationMetrics();
    void  setFretMetrics();

    static bool readTabConfigFile(const String& fileName);

    StaffGroup m_group = StaffGroup::STANDARD;

    String m_xmlName;         // the name used to reference this preset in instruments.xml
    String m_name;            // user visible name

    double m_userMag = 1.0;           // allowed 0.1 - 10.0
    Spatium m_yoffset;
    bool m_small = false;
    bool m_invisible = false;
    Color m_color;

    int m_lines = 5;
    int m_stepOffset = 0;
    Spatium m_lineDistance = Spatium(1);

    bool m_showBarlines = true;
    bool m_showLedgerLines = true;
    bool m_stemless = false;       // do not show stems

    bool m_genClef = true;         // create clef at beginning of system
    bool m_genTimesig = true;      // whether time signature is shown or not
    bool m_genKeysig = true;       // create key signature at beginning of system

    // Standard: configurable properties
    NoteHeadScheme m_noteHeadScheme = NoteHeadScheme::HEAD_NORMAL;

    // TAB: configurable propertiesm
    double m_durationFontSize = 15.0;       // the size (in points) for the duration symbol font
    double m_durationFontUserY = 0.0;       // the vertical offset (spatium units) for the duration symb. font
    // user configurable
    double m_fretFontSize  = 10.0;          // the size (in points) for the fret marks font
    double m_fretFontUserY = 0.0;           // additional vert. offset of fret marks with respect to
    // the string line (spatium unit); user configurable
    bool m_genDurations = false;            // whether duration symbols are drawn or not
    bool m_linesThrough = false;            // whether lines for strings and stems may pass through fret marks or not
    TablatureMinimStyle m_minimStyle = TablatureMinimStyle::NONE;      // how to draw minim stems (stem-and-beam durations only)
    TablatureSymbolRepeat m_symRepeat = TablatureSymbolRepeat::NEVER;  // if and when to repeat the same duration symbol
    bool m_onLines = true;                  // whether fret marks are drawn on the string lines or between them
    bool m_showRests = false;               // whether to draw rests or not
    bool m_stemsDown = true;                // stems are drawn downward (stem-and-beam durations only)
    bool m_stemsThrough = true;             // stems are drawn through the staff rather than beside it (stem-and-beam durations only)
    bool m_upsideDown = false;              // whether lines are drawn with highest string at top (false) or at bottom (true)
    bool m_showTabFingering = false;        // Allow fingering in tablature staff (true) or not (false)
    bool m_useNumbers = true;               // true: use numbers ('0' - ...) for frets | false: use letters ('a' - ...)
    bool m_showBackTied = true;             // whether back-tied notes are shown or not

    // TAB: internally managed variables
    // Note: values in RASTER UNITS are independent from score scaling and
    //    must be multiplied by magS() to be used in contexts using sp units
    double m_durationBoxH = 0.0;
    double m_durationBoxY = 0.0;            // the height and the y rect.coord. (relative to staff top line)
                                            // of a box bounding all duration symbols (raster units) internally computed:
                                            // depends upon _onString and the metrics of the duration font
    muse::draw::Font m_durationFont;        // font used to draw dur. symbols; cached for efficiency
    size_t m_durationFontIdx = 0;           // the index of current dur. font in dur. font array
    double m_durationYOffset = 0.0;         // the vertical offset to draw duration symbols with respect to the
    // string lines (raster units); internally computed: depends upon _onString and duration font
    double m_durationGridYOffset = 0.0;     // the vertical offset to draw the bottom of duration grid with respect to the
    // string lines (raster units); internally computed: depends upon _onstring and duration font
    double m_fretBoxH = 0.0;
    double m_fretBoxY = 0.0;                // the height and the y rect.coord. (relative to staff line)
    double m_deadFretBoxH = 0.0;
    double m_deadFretBoxY = 0.0;
    // of a box bounding all fret characters (raster units) internally computed:
    // depends upon _onString, _useNumbers and the metrics of the fret font
    bool m_fretUseTextStyle = false;
    TextStyleType m_fretTextStyle = TextStyleType::TAB_FRET_NUMBER;
    muse::draw::Font m_fretFont;                      // font used to draw fret marks; cached for efficiency
    TablatureFretFont m_fretFontInfo;
    size_t m_fretPresetIdx = 0;           // the index of current fret font in fret font array
    double m_fretYOffset = 0.0;             // the vertical offset to draw fret marks with respect to the string lines;
    double m_deadFretYOffset = 0.0;
    // (raster units); internally computed: depends upon _onString, _useNumbers
    // and the metrics of the fret font

    // the array of configured fonts
    static std::vector<TablatureFretFont> m_fretFonts;
    static std::vector<TablatureDurationFont> m_durationFonts;
    static std::vector<StaffType> m_presets;
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
    DECLARE_CLASSOF(ElementType::TAB_DURATION_SYMBOL)

public:
    TabDurationSymbol(ChordRest* parent);
    TabDurationSymbol(ChordRest* parent, const StaffType* tab, DurationType type, int dots);
    TabDurationSymbol(const TabDurationSymbol&);
    TabDurationSymbol* clone() const override { return new TabDurationSymbol(*this); }

    bool isEditable() const override { return false; }

    const StaffType* tab() const { return m_tab; }
    const String& text() const { return m_text; }
    void setDuration(DurationType type, int dots, const StaffType* tab)
    {
        m_tab = tab;
        m_text = tab->durationString(type, dots);
    }

    bool isRepeat() const { return m_repeat; }
    void setRepeat(bool val) { m_repeat = val; }

    struct LayoutData : public EngravingItem::LayoutData {
        TabBeamGrid beamGrid = TabBeamGrid::NONE;         // value for special 'English' grid display
        double beamLength = 0.0;                          // if _grid==MEDIALFINAL, length of the beam toward previous grid element
        int beamLevel = 0.0;                                // if _grid==MEDIALFINAL, the number of beams
    };
    DECLARE_LAYOUTDATA_METHODS(TabDurationSymbol)

private:

    const StaffType* m_tab = nullptr;
    String m_text;
    bool m_repeat = false;
};
} // namespace mu::engraving
#endif
