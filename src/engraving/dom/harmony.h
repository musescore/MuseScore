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

#include <vector>

#include "chordlist.h"
#include "parenthesis.h"
#include "draw/types/font.h"

#include "textbase.h"
#include "score.h"

#include "pitchspelling.h"
#include "realizedharmony.h"

namespace mu::engraving {
struct ChordDescription;
class ParsedChord;
class Score;

enum class HarmonyType : unsigned char {
    STANDARD,
    ROMAN,
    NASHVILLE
};

enum class HarmonyRenderItemType : unsigned char {
    TEXT,
    PAREN
};

struct HarmonyRenderItem {
    virtual ~HarmonyRenderItem() = default;
    HarmonyRenderItem(bool align, double _x, double _y)
        : m_hAlign(align), m_pos(_x, _y) {}

    PointF pos() const { return m_pos + m_offset; }
    double x() const { return m_pos.x(); }
    double y() const { return m_pos.y(); }
    void movex(double v) { m_pos.rx() += v; }
    void movey(double v) { m_pos.ry() += v; }
    void setx(double v) { m_pos.rx() = v; }
    void sety(double v) { m_pos.ry() = v; }
    void setOffset(const PointF& p) { m_offset = p; }

    virtual double height() const = 0;

    bool align() const { return m_hAlign; }

    virtual RectF boundingRect() const = 0;
    virtual RectF tightBoundingRect() const = 0;

    virtual double leftPadding() const = 0;
    virtual double rightPadding() const = 0;
    virtual double bboxBaseLine() const = 0;

    virtual HarmonyRenderItemType type() const = 0;

private:
    bool m_hAlign = true;
    PointF m_offset;       // Offset for placing within the TextBase.
    PointF m_pos;          // Position of segments relative to Harmony position
};

struct ChordSymbolParen : HarmonyRenderItem {
    ChordSymbolParen(Parenthesis* p, bool hAlign, double _x, double _y)
        : HarmonyRenderItem(hAlign, _x, _y), parenItem(p) {}
    ~ChordSymbolParen() { delete parenItem; }

    RectF boundingRect() const override { return parenItem->shape().bbox(); }
    RectF tightBoundingRect() const override { return parenItem->shape().bbox(); }

    double top = DBL_MAX;
    double bottom = -DBL_MAX;
    double closingParenPos = -DBL_MAX;

    Parenthesis* parenItem = nullptr;

    double height() const override { return parenItem->height(); }

    double leftPadding() const override { return parenItem->direction() == DirectionH::LEFT ? OUTER_PADDING : INNER_PADDING; }
    double rightPadding() const override { return parenItem->direction() == DirectionH::LEFT ? INNER_PADDING : OUTER_PADDING; }

    double bboxBaseLine() const override { return bottom; }

    HarmonyRenderItemType type() const override { return HarmonyRenderItemType::PAREN; }

private:
    static constexpr double INNER_PADDING = 0.05;
    static constexpr double OUTER_PADDING = 0.1;
};

//---------------------------------------------------------
//   TextSegment
//---------------------------------------------------------

struct TextSegment : HarmonyRenderItem {
    double width() const;
    double capHeight() const;
    double bboxBaseLine() const override;
    RectF boundingRect() const override;
    RectF tightBoundingRect() const override;

    void setText(const String& t) { m_text = t; }
    String text() const { return m_text; }

    muse::draw::Font font() const { return m_font; }
    void setFont(const muse::draw::Font& f);

    double leftPadding() const override { return 0.0; }
    double rightPadding() const override { return 0.0; }

    double height() const override { return tightBoundingRect().height(); }

    HarmonyRenderItemType type() const override { return HarmonyRenderItemType::TEXT; }

    TextSegment(const String& s, const muse::draw::Font& f, double _x, double _y, bool align)
        : HarmonyRenderItem(align, _x, _y) { setText(s); setFont(f); }

private:
    muse::draw::Font m_font;
    String m_text;
};

class HDegree;

//---------------------------------------------------------
//   @@ HarmonyInfo
//      Contains all identifying information for a chord. The Harmony DOM class can have many of these
//      as polychords
//
//   @P m_bassTpc       int             bass note as "tonal pitch class"
//   @P m_rootTpc       int             root note as "tonal pitch class"
//   @P m_id            int             harmony identifier
//   @P m_textName      String          plaintext representation of chord
//   @P m_parsedChord   ParsedChord*    parsed representation of the plaintext chord
//---------------------------------------------------------

class HarmonyInfo
{
public:
    HarmonyInfo(int id, int rootTpc, int bassTpc, String textName, ParsedChord* pc, Score* score)
        : m_id(id), m_rootTpc(rootTpc), m_bassTpc(bassTpc), m_textName(textName), m_parsedChord(pc), m_score(score) {}
    HarmonyInfo(Score* score)
        : m_score(score) {}
    HarmonyInfo(const HarmonyInfo& h);
    ~HarmonyInfo();

    int id() const { return m_id; }
    void setId(int v) { m_id = v; }

    int rootTpc() const { return m_rootTpc; }
    void setRootTpc(int v) { m_rootTpc = v; }

    int bassTpc() const { return m_bassTpc; }
    void setBassTpc(int v) { m_bassTpc = v; }

    String textName() const { return m_textName; }
    void setTextName(const String& v) { m_textName = v; }

    ChordList* chordList() const { return m_score ? m_score->chordList() : nullptr; }

    const ChordDescription* descr() const;
    const ChordDescription* descr(const String&, const ParsedChord* pc = 0) const;
    const ChordDescription* getDescription();
    const ChordDescription* getDescription(const String&, const ParsedChord* pc = 0);
    const ChordDescription* generateDescription();

    void setParsedChord(ParsedChord* v) { m_parsedChord = v; }
    ParsedChord* parsedChord() const { return m_parsedChord; }
    ParsedChord* getParsedChord();

    bool hasModifiers() const;

private:
    int m_id = -1;                          // >0 = id of matched chord from chord list, if applicable
                                            // -1 = invalid chord
                                            // <-10000 = private id of generated chord or matched chord with no id
    int m_rootTpc = Tpc::TPC_INVALID;       // root note for chord
    int m_bassTpc = Tpc::TPC_INVALID;       // bass note or chord bass; used for "slash" chords
                                            // or notation of bass note in chord

    String m_textName;                      // name recognized from chord list, read from score file, or constructed from imported source
                                            // Also stores the whole RNA string to be rendered
    ParsedChord* m_parsedChord = nullptr;   // parsed form of chord
    Score* m_score = nullptr;
};

//---------------------------------------------------------
//   @@ Harmony
//      This DOM class represents the chord symbol in the score. It contains the necessary information to
//      layout standard chords and polychords
//---------------------------------------------------------

class Harmony final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, Harmony)
    DECLARE_CLASSOF(ElementType::HARMONY)

public:
    Harmony(Segment* parent = 0);
    Harmony(const Harmony&);
    ~Harmony();

    Harmony* clone() const override { return new Harmony(*this); }

    int id() const; // WILL BE DEPRECATED AFTER RelaizedHarmony IS UPDATED

    bool play() const { return m_play; }

    void setBassCase(NoteCaseType c) { m_bassCase = c; }
    NoteCaseType bassCase() const { return m_bassCase; }
    void setRootCase(NoteCaseType c) { m_rootCase = c; }
    NoteCaseType rootCase() const { return m_rootCase; }

    Segment* getParentSeg() const;
    FretDiagram* getParentFretDiagram() const;
    Harmony* findNext() const;
    Harmony* findPrev() const;
    Fraction ticksTillNext(int utick, bool stopAtMeasureEnd = false) const;

    RealizedHarmony& realizedHarmony();
    const RealizedHarmony& getRealizedHarmony() const;

    bool isEditable() const override { return !isInFretBox(); }
    void startEdit(EditData&) override;
    bool isEditAllowed(EditData&) const override;
    bool edit(EditData&) override;
    void endEdit(EditData&) override;

    bool isPlayable() const override;

    bool isRealizable() const;
    bool isInFretBox() const;

    int bassTpc() const;                           // WILL BE DEPRECATED AFTER RelaizedHarmony IS UPDATED
    int rootTpc() const;                           // WILL BE DEPRECATED AFTER RelaizedHarmony IS UPDATED
    void addDegree(const HDegree& d);
    const std::vector<HDegree>& degreeList() const;
    HarmonyType harmonyType() const { return m_harmonyType; }
    void setHarmonyType(HarmonyType val);

    const ParsedChord* parsedForm() const;                                             // WILL BE DEPRECATED AFTER RelaizedHarmony IS UPDATED

    void afterRead();
    void render();

    bool isPolychord() const { return m_chords.size() > 1; }
    const std::vector<HarmonyInfo*> chords() const { return m_chords; }
    void addChord(HarmonyInfo* info) { m_chords.push_back(info); }

    bool hasModifiers() const;

    String harmonyName() const;

    double baseLine() const override;
    void spatiumChanged(double oldValue, double newValue) override;
    void localSpatiumChanged(double oldValue, double newValue) override;
    void setHarmony(const String& s);

    TranslatableString typeUserName() const override;
    String accessibleInfo() const override;
    String generateScreenReaderInfo() const;
    String screenReaderInfo() const override;

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    void undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps) override;
    using EngravingObject::undoChangeProperty;
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid id) const override;

    double mag() const override;

    double bassScale() const { return m_bassScale; }
    void setBassScale(double v) { m_bassScale = v; }

    Color curColor(const rendering::PaintOptions& opt) const override;
    void setColor(const Color& color) override;

    bool doNotStackModifiers() const { return m_doNotStackModifiers; }

    NoteCaseType rootRenderCase(HarmonyInfo* info) const;
    NoteCaseType bassRenderCase() const;

    FontStyle fontStyle() const override { return m_fontStyle; }
    String family() const override { return m_fontFamily; }
    double size() const override { return m_fontSize; }

    void setFontStyle(const FontStyle& val) override { m_fontStyle = val; }
    void setFamily(const String& val) override { m_fontFamily = val; }
    void setSize(const double& val) override { m_fontSize = val; }

    struct LayoutData : public TextBase::LayoutData {
        ld_field<double> harmonyHeight = { "[Harmony] harmonyHeight", 0.0 };    // used for calculating the height is frame while editing.
        ld_field<std::vector<LineF> > polychordDividerLines = { "[Harmony] polychordDividerLine", std::vector<LineF>() };
        ld_field<double> polychordDividerOffset = { "[Harmony] polychordDividerOffset", 0.0 };
        ld_field<double> baseline = { "[Harmony] baseline", 0.0 };
        ld_field<std::vector<muse::draw::Font> > fontList = "[Harmony] fontList";          // temp values used in render()
        ld_field<std::vector<HarmonyRenderItem*> > renderItemList = "[Harmony] renderItemList";              // rendered chord
    };
    DECLARE_LAYOUTDATA_METHODS(Harmony)

private:
    std::vector<HarmonyInfo*> m_chords;

    const std::vector<const ChordDescription*> parseHarmony(const String& s, bool syntaxOnly = false);
    const ChordDescription* parseSingleHarmony(const String& s, HarmonyInfo* info, bool syntaxOnly = false);

    Sid getPropertyStyle(Pid) const override;

    Harmony* findInSeg(Segment* seg) const;

    bool m_isMisspelled = false;                         // show spell check warning
    HarmonyType m_harmonyType = HarmonyType::STANDARD;   // used to control rendering, transposition, export, etc.

    mutable RealizedHarmony m_realizedHarmony;           // the realized harmony used for playback

    std::vector<HDegree> m_degreeList;

    bool m_play = true;                                  // whether or not to play back the harmony
    bool m_doNotStackModifiers = false;

    NoteCaseType m_rootCase = NoteCaseType::AUTO;
    NoteCaseType m_bassCase = NoteCaseType::AUTO;        // case as typed

    double m_bassScale = 1.0;

    // Overridden textbase properties to apply to whole item
    double m_fontSize = 10.0;
    String m_fontFamily = u"";
    FontStyle m_fontStyle;
};
} // namespace mu::engraving
