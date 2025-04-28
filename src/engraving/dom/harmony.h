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

#ifndef MU_ENGRAVING_HARMONY_H
#define MU_ENGRAVING_HARMONY_H

#include <vector>

#include "draw/types/font.h"

#include "textbase.h"

#include "pitchspelling.h"
#include "realizedharmony.h"

namespace mu::engraving {
struct ChordDescription;
class ParsedChord;

//---------------------------------------------------------
//   TextSegment
//---------------------------------------------------------

struct TextSegment {
    muse::draw::Font m_font;
    String text;
    double x, y = 0;         // Position of segments relative to each other.
    PointF offset;       // Offset for placing within the TextBase.
    bool select = false;

    double width() const;
    RectF boundingRect() const;
    RectF tightBoundingRect() const;
    PointF pos() const { return PointF(x, y) + offset; }

    TextSegment() { select = false; x = y = 0.0; }
    TextSegment(const muse::draw::Font& f, double _x, double _y)
        : m_font(f), x(_x), y(_y), select(false) {}
    TextSegment(const String&, const muse::draw::Font&, double x, double y);
    void set(const String&, const muse::draw::Font&, double x, double y, PointF offset);
    void setText(const String& t) { text = t; }
};

//---------------------------------------------------------
//   @@ Harmony
///    root note and bass note are notated as "tonal pitch class":
///   <table>
///         <tr><td>&nbsp;</td><td>bb</td><td> b</td><td> -</td><td> #</td><td>##</td></tr>
///         <tr><td>C</td>     <td> 0</td><td> 7</td><td>14</td><td>21</td><td>28</td></tr>
///         <tr><td>D</td>     <td> 2</td><td> 9</td><td>16</td><td>23</td><td>30</td></tr>
///         <tr><td>E</td>     <td> 4</td><td>11</td><td>18</td><td>25</td><td>32</td></tr>
///         <tr><td>F</td>     <td>-1</td><td> 6</td><td>13</td><td>20</td><td>27</td></tr>
///         <tr><td>G</td>     <td> 1</td><td> 8</td><td>15</td><td>22</td><td>29</td></tr>
///         <tr><td>A</td>     <td> 3</td><td>10</td><td>17</td><td>24</td><td>31</td></tr>
///         <tr><td>B</td>     <td> 5</td><td>12</td><td>19</td><td>26</td><td>33</td></tr></table>
//
//   @P bassTpc   int   bass note as "tonal pitch class"
//   @P id        int   harmony identifier
//   @P rootTpc   int   root note as "tonal pitch class"
//---------------------------------------------------------

struct RenderAction;
class HDegree;

class Harmony final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, Harmony)
    DECLARE_CLASSOF(ElementType::HARMONY)

public:
    Harmony(Segment* parent = 0);
    Harmony(const Harmony&);
    ~Harmony();

    Harmony* clone() const override { return new Harmony(*this); }

    void setId(int d) { m_id = d; }
    int id() const { return m_id; }

    bool play() const { return m_play; }

    void setBassCase(NoteCaseType c) { m_bassCase = c; }
    NoteCaseType bassCase() const { return m_bassCase; }
    void setRootCase(NoteCaseType c) { m_rootCase = c; }
    NoteCaseType rootCase() const { return m_rootCase; }

    bool leftParen() const { return m_leftParen; }
    bool rightParen() const { return m_rightParen; }
    void setLeftParen(bool leftParen) { m_leftParen = leftParen; }
    void setRightParen(bool rightParen) { m_rightParen = rightParen; }

    Harmony* findNext() const;
    Harmony* findPrev() const;
    Fraction ticksTillNext(int utick, bool stopAtMeasureEnd = false) const;
    Segment* getParentSeg() const;

    const ChordDescription* descr() const;
    const ChordDescription* descr(const String&, const ParsedChord* pc = 0) const;
    const ChordDescription* getDescription();
    const ChordDescription* getDescription(const String&, const ParsedChord* pc = 0);
    const ChordDescription* generateDescription();

    RealizedHarmony& realizedHarmony();
    const RealizedHarmony& getRealizedHarmony() const;

    void determineRootBassSpelling(NoteSpellingType& rootSpelling, NoteCaseType& rootCase, NoteSpellingType& bassSpelling,
                                   NoteCaseType& bassCase);

    bool isEditable() const override { return !isInFretBox(); }
    void startEditTextual(EditData&) override;
    bool isTextualEditAllowed(EditData&) const override;
    bool editTextual(EditData&) override;
    void endEditTextual(EditData&) override;

    bool isRealizable() const;
    bool isInFretBox() const;

    String hFunction() const { return m_function; }
    String hTextName() const { return m_textName; }
    int bassTpc() const { return m_bassTpc; }
    void setBassTpc(int val) { m_bassTpc = val; }
    int rootTpc() const { return m_rootTpc; }
    void setRootTpc(int val) { m_rootTpc = val; }
    void setTextName(const String& s) { m_textName = s; }
    void setFunction(const String& s) { m_function = s; }
    void addDegree(const HDegree& d);
    const std::vector<HDegree>& degreeList() const;
    const ParsedChord* parsedForm() const;
    HarmonyType harmonyType() const { return m_harmonyType; }
    void setHarmonyType(HarmonyType val);

    const std::vector<TextSegment*>& textList() const { return m_textList; }

    void afterRead();
    String harmonyName() const;
    void render();

    const ChordDescription* parseHarmony(const String& s, int& root, int& bass, bool syntaxOnly = false);

    const String& extensionName() const;

    String xmlKind() const;
    String musicXmlText() const;
    String xmlSymbols() const;
    String xmlParens() const;
    StringList xmlDegrees() const;

    double baseLine() const override;

    const ChordDescription* fromXml(const String&, const String&, const String&, const String&, const std::list<HDegree>&);
    const ChordDescription* fromXml(const String& s);
    void spatiumChanged(double oldValue, double newValue) override;
    void localSpatiumChanged(double oldValue, double newValue) override;
    void setHarmony(const String& s);

    TranslatableString typeUserName() const override;
    String accessibleInfo() const override;
    String generateScreenReaderInfo() const;
    String screenReaderInfo() const override;

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid id) const override;

    double mag() const override;
    void setUserMag(double m) { m_userMag = m; }

    void undoMoveSegment(Segment* newSeg, Fraction tickDiff) override;

    Color curColor() const override;

    struct LayoutData : public TextBase::LayoutData {
        ld_field<double> harmonyHeight = { "[Harmony] harmonyHeight", 0.0 };           // used for calculating the height is frame while editing.
    };
    DECLARE_LAYOUTDATA_METHODS(Harmony)

private:

    void determineRootBassSpelling();

    void render(const String&, double&, double&);
    void render(const std::list<RenderAction>& renderList, double&, double&, int tpc,
                NoteSpellingType noteSpelling = NoteSpellingType::STANDARD, NoteCaseType noteCase = NoteCaseType::AUTO);
    Sid getPropertyStyle(Pid) const override;

    Harmony* findInSeg(Segment* seg) const;

    int m_rootTpc = Tpc::TPC_INVALID;               // root note for chord
    int m_bassTpc = Tpc::TPC_INVALID;               // bass note or chord bass; used for "slash" chords
    // or notation of bass note in chord
    int m_id = -1;                    // >0 = id of matched chord from chord list, if applicable
    // -1 = invalid chord
    // <-10000 = private id of generated chord or matched chord with no id
    String m_function;          // numeric representation of root for RNA or Nashville
    String m_userName;          // name as typed by user if applicable
    String m_textName;          // name recognized from chord list, read from score file, or constructed from imported source
    mutable ParsedChord* m_parsedForm = nullptr;   // parsed form of chord
    bool m_isMisspelled = false; // show spell check warning
    HarmonyType m_harmonyType = HarmonyType::STANDARD;   // used to control rendering, transposition, export, etc.

    mutable RealizedHarmony m_realizedHarmony; // the realized harmony used for playback

    std::vector<HDegree> m_degreeList;
    std::vector<muse::draw::Font> m_fontList; // temp values used in render()
    std::vector<TextSegment*> m_textList;   // rendered chord

    bool m_leftParen = false;
    bool m_rightParen = false;   // include opening and/or closing parenthesis
    bool m_play = true;                     // whether or not to play back the harmony

    NoteSpellingType m_rootSpelling = NoteSpellingType::STANDARD;
    NoteSpellingType m_bassSpelling = NoteSpellingType::STANDARD;
    NoteCaseType m_rootCase = NoteCaseType::AUTO;
    NoteCaseType m_bassCase = NoteCaseType::AUTO;                // case as typed
    NoteCaseType m_rootRenderCase = NoteCaseType::AUTO;
    NoteCaseType m_bassRenderCase = NoteCaseType::AUTO;           // case to render

    std::optional<double> m_userMag;
};
} // namespace mu::engraving
#endif
