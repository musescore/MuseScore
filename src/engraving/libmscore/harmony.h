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

#include "draw/font.h"

#include "text.h"
#include "pitchspelling.h"
#include "realizedharmony.h"

namespace Ms {
struct ChordDescription;
class ParsedChord;

//---------------------------------------------------------
//   TextSegment
//---------------------------------------------------------

struct TextSegment {
    mu::draw::Font m_font;
    QString text;
    qreal x, y;         // Position of segments relative to each other.
    mu::PointF offset;     // Offset for placing within the TextBase.
    bool select;

    qreal width() const;
    mu::RectF boundingRect() const;
    mu::RectF tightBoundingRect() const;
    mu::PointF pos() const { return mu::PointF(x, y) + offset; }

    TextSegment() { select = false; x = y = 0.0; }
    TextSegment(const mu::draw::Font& f, qreal _x, qreal _y)
        : m_font(f), x(_x), y(_y), select(false) {}
    TextSegment(const QString&, const mu::draw::Font&, qreal x, qreal y);
    void set(const QString&, const mu::draw::Font&, qreal x, qreal y, mu::PointF offset);
    void setText(const QString& t) { text = t; }
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
    int _rootTpc;                         // root note for chord
    int _baseTpc;                         // bass note or chord base; used for "slash" chords
                                          // or notation of base note in chord
    int _id;                              // >0 = id of matched chord from chord list, if applicable
                                          // -1 = invalid chord
                                          // <-10000 = private id of generated chord or matched chord with no id
    QString _function;                    // numeric representation of root for RNA or Nashville
    QString _userName;                    // name as typed by user if applicable
    QString _textName;                    // name recognized from chord list, read from score file, or constructed from imported source
    ParsedChord* _parsedForm;             // parsed form of chord
    bool showSpell = false;               // show spell check warning
    HarmonyType _harmonyType;             // used to control rendering, transposition, export, etc.
    qreal _harmonyHeight;                 // used for calculating the the height is frame while editing.

    mutable RealizedHarmony _realizedHarmony;      //the realized harmony used for playback

    QList<HDegree> _degreeList;
    QList<mu::draw::Font> fontList;                // temp values used in render()
    QList<TextSegment*> textList;         // rendered chord

    bool _leftParen, _rightParen;         // include opening and/or closing parenthesis
    bool _play;                           // whether or not to play back the harmony

    mutable mu::RectF _tbbox;

    NoteSpellingType _rootSpelling, _baseSpelling;
    NoteCaseType _rootCase, _baseCase;                // case as typed
    NoteCaseType _rootRenderCase, _baseRenderCase;    // case to render

    void determineRootBaseSpelling();
    void draw(mu::draw::Painter*) const override;
    void drawEditMode(mu::draw::Painter* p, EditData& ed) override;
    void render(const QString&, qreal&, qreal&);
    void render(const QList<RenderAction>& renderList, qreal&, qreal&, int tpc, NoteSpellingType noteSpelling = NoteSpellingType::STANDARD,
                NoteCaseType noteCase = NoteCaseType::AUTO);
    Sid getPropertyStyle(Pid) const override;

    Harmony* findInSeg(Segment* seg) const;

public:
    Harmony(Score* = 0);
    Harmony(const Harmony&);
    ~Harmony();

    Harmony* clone() const override { return new Harmony(*this); }
    ElementType type() const override { return ElementType::HARMONY; }

    void setId(int d) { _id = d; }
    int id() const { return _id; }

    void setPlay(bool p) { _play = p; }
    bool play() const { return _play; }

    void setBaseCase(NoteCaseType c) { _baseCase = c; }
    void setRootCase(NoteCaseType c) { _rootCase = c; }

    bool leftParen() const { return _leftParen; }
    bool rightParen() const { return _rightParen; }
    void setLeftParen(bool leftParen) { _leftParen = leftParen; }
    void setRightParen(bool rightParen) { _rightParen = rightParen; }

    void scanElements(void* data, void (* func)(void*, Element*), bool all=true) override;

    Harmony* findNext() const;
    Harmony* findPrev() const;
    Fraction ticksTillNext(int utick, bool stopAtMeasureEnd = false) const;
    Segment* getParentSeg() const;

    const ChordDescription* descr() const;
    const ChordDescription* descr(const QString&, const ParsedChord* pc = 0) const;
    const ChordDescription* getDescription();
    const ChordDescription* getDescription(const QString&, const ParsedChord* pc = 0);
    const ChordDescription* generateDescription();

    RealizedHarmony& realizedHarmony();
    const RealizedHarmony& getRealizedHarmony() const;

    void determineRootBaseSpelling(NoteSpellingType& rootSpelling, NoteCaseType& rootCase, NoteSpellingType& baseSpelling,
                                   NoteCaseType& baseCase);

    void textChanged();
    void layout() override;
    void layout1() override;

    bool isEditable() const override { return true; }
    void startEdit(EditData&) override;
    bool edit(EditData&) override;
    void endEdit(EditData&) override;

    bool isRealizable() const;

    QString hFunction() const { return _function; }
    QString hUserName() const { return _userName; }
    QString hTextName() const { return _textName; }
    int baseTpc() const { return _baseTpc; }
    void setBaseTpc(int val) { _baseTpc = val; }
    int rootTpc() const { return _rootTpc; }
    void setRootTpc(int val) { _rootTpc = val; }
    void setTextName(const QString& s) { _textName = s; }
    void setFunction(const QString& s) { _function = s; }
    QString rootName();
    QString baseName();
    void addDegree(const HDegree& d);
    int numberOfDegrees() const;
    HDegree degree(int i) const;
    void clearDegrees();
    const QList<HDegree>& degreeList() const;
    const ParsedChord* parsedForm();
    HarmonyType harmonyType() const { return _harmonyType; }
    void setHarmonyType(HarmonyType val);

    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;
    QString harmonyName() const;
    void render();

    const ChordDescription* parseHarmony(const QString& s, int* root, int* base, bool syntaxOnly = false);

    const QString& extensionName() const;

    QString xmlKind() const;
    QString musicXmlText() const;
    QString xmlSymbols() const;
    QString xmlParens() const;
    QStringList xmlDegrees() const;

    void resolveDegreeList();

    qreal baseLine() const override;

    const ChordDescription* fromXml(const QString&, const QString&, const QString&, const QString&, const QList<HDegree>&);
    const ChordDescription* fromXml(const QString& s, const QList<HDegree>&);
    const ChordDescription* fromXml(const QString& s);
    void spatiumChanged(qreal oldValue, qreal newValue) override;
    void localSpatiumChanged(qreal oldValue, qreal newValue) override;
    void setHarmony(const QString& s);
    mu::PointF calculateBoundingRect();
    qreal xShapeOffset() const;

    QString userName() const override;
    QString accessibleInfo() const override;
    QString generateScreenReaderInfo() const;
    QString screenReaderInfo() const override;

    bool acceptDrop(EditData&) const override;
    Element* drop(EditData&) override;

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant& v) override;
    QVariant propertyDefault(Pid id) const override;
};
}     // namespace Ms
#endif
