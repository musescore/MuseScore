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

    friend class layout::v0::TLayout;

    int _rootTpc;               // root note for chord
    int _baseTpc;               // bass note or chord base; used for "slash" chords
                                // or notation of base note in chord
    int _id;                    // >0 = id of matched chord from chord list, if applicable
                                // -1 = invalid chord
                                // <-10000 = private id of generated chord or matched chord with no id
    String _function;          // numeric representation of root for RNA or Nashville
    String _userName;          // name as typed by user if applicable
    String _textName;          // name recognized from chord list, read from score file, or constructed from imported source
    mutable ParsedChord* _parsedForm;   // parsed form of chord
    bool _isMisspelled = false; // show spell check warning
    HarmonyType _harmonyType;   // used to control rendering, transposition, export, etc.
    double _harmonyHeight;       // used for calculating the height is frame while editing.

    mutable RealizedHarmony _realizedHarmony; // the realized harmony used for playback

    std::vector<HDegree> _degreeList;
    std::vector<mu::draw::Font> fontList; // temp values used in render()
    std::list<TextSegment*> textList;   // rendered chord

    bool _leftParen, _rightParen;   // include opening and/or closing parenthesis
    bool _play;                     // whether or not to play back the harmony

    mutable mu::RectF _tbbox;

    NoteSpellingType _rootSpelling, _baseSpelling;
    NoteCaseType _rootCase, _baseCase;                // case as typed
    NoteCaseType _rootRenderCase, _baseRenderCase;    // case to render

    void determineRootBaseSpelling();

    void draw(mu::draw::Painter*) const override;
    void drawEditMode(mu::draw::Painter* p, EditData& ed, double currentViewScaling) override;
    void render(const String&, double&, double&);
    void render(const std::list<RenderAction>& renderList, double&, double&, int tpc,
                NoteSpellingType noteSpelling = NoteSpellingType::STANDARD, NoteCaseType noteCase = NoteCaseType::AUTO);
    Sid getPropertyStyle(Pid) const override;

    Harmony* findInSeg(Segment* seg) const;

public:
    Harmony(Segment* parent = 0);
    Harmony(const Harmony&);
    ~Harmony();

    Harmony* clone() const override { return new Harmony(*this); }

    void setId(int d) { _id = d; }
    int id() const { return _id; }

    bool play() const { return _play; }

    void setBaseCase(NoteCaseType c) { _baseCase = c; }
    NoteCaseType baseCase() const { return _baseCase; }
    void setRootCase(NoteCaseType c) { _rootCase = c; }
    NoteCaseType rootCase() const { return _rootCase; }

    bool leftParen() const { return _leftParen; }
    bool rightParen() const { return _rightParen; }
    void setLeftParen(bool leftParen) { _leftParen = leftParen; }
    void setRightParen(bool rightParen) { _rightParen = rightParen; }

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

    String hFunction() const { return _function; }
    String hUserName() const { return _userName; }
    String hTextName() const { return _textName; }
    int baseTpc() const { return _baseTpc; }
    void setBaseTpc(int val) { _baseTpc = val; }
    int rootTpc() const { return _rootTpc; }
    void setRootTpc(int val) { _rootTpc = val; }
    void setTextName(const String& s) { _textName = s; }
    void setFunction(const String& s) { _function = s; }
    String rootName();
    String baseName();
    void addDegree(const HDegree& d);
    size_t numberOfDegrees() const;
    HDegree degree(int i) const;
    void clearDegrees();
    const std::vector<HDegree>& degreeList() const;
    const ParsedChord* parsedForm() const;
    HarmonyType harmonyType() const { return _harmonyType; }
    void setHarmonyType(HarmonyType val);

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
};
} // namespace mu::engraving
#endif
