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

#ifndef __HARMONY_H__
#define __HARMONY_H__

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
    mu::draw::Font m_font;
    String text;
    double x, y;         // Position of segments relative to each other.
    mu::PointF offset;     // Offset for placing within the TextBase.
    bool select;

    double width() const;
    mu::RectF boundingRect() const;
    mu::RectF tightBoundingRect() const;
    mu::PointF pos() const { return mu::PointF(x, y) + offset; }

    TextSegment() { select = false; x = y = 0.0; }
    TextSegment(const mu::draw::Font& f, double _x, double _y)
        : m_font(f), x(_x), y(_y), select(false) {}
    TextSegment(const String&, const mu::draw::Font&, double x, double y);
    void set(const String&, const mu::draw::Font&, double x, double y, mu::PointF offset);
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
//   @P baseTpc   int   bass note as "tonal pitch class"
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

    void setBaseCase(NoteCaseType c) { m_baseCase = c; }
    NoteCaseType baseCase() const { return m_baseCase; }
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

    void determineRootBaseSpelling(NoteSpellingType& rootSpelling, NoteCaseType& rootCase, NoteSpellingType& baseSpelling,
                                   NoteCaseType& baseCase);

    void textChanged();

    bool isEditable() const override { return true; }
    void startEdit(EditData&) override;
    bool isEditAllowed(EditData&) const override;
    bool edit(EditData&) override;
    void endEdit(EditData&) override;

    bool isRealizable() const;

    String hFunction() const { return m_function; }
    String hUserName() const { return m_userName; }
    String hTextName() const { return m_textName; }
    int baseTpc() const { return m_baseTpc; }
    void setBaseTpc(int val) { m_baseTpc = val; }
    int rootTpc() const { return m_rootTpc; }
    void setRootTpc(int val) { m_rootTpc = val; }
    void setTextName(const String& s) { m_textName = s; }
    void setFunction(const String& s) { m_function = s; }
    String rootName();
    String baseName();
    void addDegree(const HDegree& d);
    size_t numberOfDegrees() const;
    HDegree degree(int i) const;
    void clearDegrees();
    const std::vector<HDegree>& degreeList() const;
    const ParsedChord* parsedForm() const;
    HarmonyType harmonyType() const { return m_harmonyType; }
    void setHarmonyType(HarmonyType val);

    const std::vector<TextSegment*>& textList() const { return m_textList; }

    double harmonyHeight() const { return m_harmonyHeight; }
    void setHarmonyHeight(double h) { m_harmonyHeight = h; }

    void afterRead();
    String harmonyName() const;
    void render();

    const ChordDescription* parseHarmony(const String& s, int* root, int* base, bool syntaxOnly = false);

    const String& extensionName() const;

    String xmlKind() const;
    String musicXmlText() const;
    String xmlSymbols() const;
    String xmlParens() const;
    StringList xmlDegrees() const;

    void resolveDegreeList();

    double baseLine() const override;

    const ChordDescription* fromXml(const String&, const String&, const String&, const String&, const std::list<HDegree>&);
    const ChordDescription* fromXml(const String& s, const std::list<HDegree>&);
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

private:

    void determineRootBaseSpelling();

    void draw(mu::draw::Painter*) const override;
    void drawEditMode(mu::draw::Painter* p, EditData& ed, double currentViewScaling) override;
    void render(const String&, double&, double&);
    void render(const std::list<RenderAction>& renderList, double&, double&, int tpc,
                NoteSpellingType noteSpelling = NoteSpellingType::STANDARD, NoteCaseType noteCase = NoteCaseType::AUTO);
    Sid getPropertyStyle(Pid) const override;

    Harmony* findInSeg(Segment* seg) const;

    int m_rootTpc = Tpc::TPC_INVALID;               // root note for chord
    int m_baseTpc = Tpc::TPC_INVALID;               // bass note or chord base; used for "slash" chords
    // or notation of base note in chord
    int m_id = -1;                    // >0 = id of matched chord from chord list, if applicable
    // -1 = invalid chord
    // <-10000 = private id of generated chord or matched chord with no id
    String m_function;          // numeric representation of root for RNA or Nashville
    String m_userName;          // name as typed by user if applicable
    String m_textName;          // name recognized from chord list, read from score file, or constructed from imported source
    mutable ParsedChord* m_parsedForm = nullptr;   // parsed form of chord
    bool m_isMisspelled = false; // show spell check warning
    HarmonyType m_harmonyType = HarmonyType::STANDARD;   // used to control rendering, transposition, export, etc.
    double m_harmonyHeight = 0.0;       // used for calculating the height is frame while editing.

    mutable RealizedHarmony m_realizedHarmony; // the realized harmony used for playback

    std::vector<HDegree> m_degreeList;
    std::vector<mu::draw::Font> m_fontList; // temp values used in render()
    std::vector<TextSegment*> m_textList;   // rendered chord

    bool m_leftParen = false;
    bool m_rightParen = false;   // include opening and/or closing parenthesis
    bool m_play = true;                     // whether or not to play back the harmony

    NoteSpellingType m_rootSpelling = NoteSpellingType::STANDARD;
    NoteSpellingType m_baseSpelling = NoteSpellingType::STANDARD;
    NoteCaseType m_rootCase = NoteCaseType::AUTO;
    NoteCaseType m_baseCase = NoteCaseType::AUTO;                // case as typed
    NoteCaseType m_rootRenderCase = NoteCaseType::AUTO;
    NoteCaseType m_baseRenderCase = NoteCaseType::AUTO;           // case to render
};
} // namespace mu::engraving
#endif
