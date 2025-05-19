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
#include "draw/types/font.h"

#include "textbase.h"
#include "score.h"

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

    bool hAlign = true;

    double width() const;
    RectF boundingRect() const;
    RectF tightBoundingRect() const;
    PointF pos() const { return PointF(x, y) + offset; }

    TextSegment(const String&, const muse::draw::Font&, double x, double y, bool align);
    TextSegment(const String&, const muse::draw::Font&, double x, double y, PointF offset, bool align);
    void setText(const String& t) { text = t; }
};

//---------------------------------------------------------
//   HarmonyRenderCtx
//---------------------------------------------------------

struct HarmonyRenderCtx {
    PointF pos = PointF();
    bool hAlign = true;
    std::vector<TextSegment*> textList;

    double x() const { return pos.x(); }
    double y() const { return pos.y(); }

    void setX(double v) { pos.setX(v); }
    void setY(double v) { pos.setY(v); }
};

struct RenderAction;
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

    int id() const;// DEPRECATED

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

    RealizedHarmony& realizedHarmony();
    const RealizedHarmony& getRealizedHarmony() const;

    bool isEditable() const override { return !isInFretBox(); }
    void startEditTextual(EditData&) override;
    bool isTextualEditAllowed(EditData&) const override;
    bool editTextual(EditData&) override;
    void endEditTextual(EditData&) override;

    bool isRealizable() const;
    bool isInFretBox() const;

    int bassTpc() const;                           // DEPRECATED
    int rootTpc() const;                           // DEPRECATED
    void addDegree(const HDegree& d);
    const std::vector<HDegree>& degreeList() const;
    HarmonyType harmonyType() const { return m_harmonyType; }
    void setHarmonyType(HarmonyType val);

    const ParsedChord* parsedForm() const;                                             // DEPRECATED

    const std::vector<TextSegment*>& textList() const { return m_textList; }

    void afterRead();
    void render();

    bool isPolychord() const { return m_chords.size() > 1; }
    const std::vector<HarmonyInfo*> chords() const { return m_chords; }
    void addChord(HarmonyInfo* info) { m_chords.push_back(info); }

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

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue propertyDefault(Pid id) const override;

    double mag() const override;
    void setUserMag(double m) { m_userMag = m; }

    void undoMoveSegment(Segment* newSeg, Fraction tickDiff) override;

    Color curColor() const override;

    struct LayoutData : public TextBase::LayoutData {
        ld_field<double> harmonyHeight = { "[Harmony] harmonyHeight", 0.0 };           // used for calculating the height is frame while editing.
        ld_field<std::vector<LineF> > polychordDividerLines = { "[Harmony] polychordDividerLine", std::vector<LineF>() };
    };
    DECLARE_LAYOUTDATA_METHODS(Harmony)

private:
    std::vector<HarmonyInfo*> m_chords;

    const std::vector<const ChordDescription*> parseHarmony(const String& s, bool syntaxOnly = false);
    const ChordDescription* parseSingleHarmony(const String& s, HarmonyInfo* info, bool syntaxOnly = false);

    NoteCaseType rootRenderCase(HarmonyInfo* info) const;
    NoteCaseType bassRenderCase() const;

    void renderSingleHarmony(HarmonyInfo* info, HarmonyRenderCtx& ctx);
    void renderRomanNumeral();
    void render(const String&, HarmonyRenderCtx& ctx);
    void render(const std::list<RenderAction>& renderList, HarmonyRenderCtx& ctx, int tpc,
                NoteSpellingType noteSpelling = NoteSpellingType::STANDARD, NoteCaseType noteCase = NoteCaseType::AUTO,
                double noteMag = 1.0);

    Sid getPropertyStyle(Pid) const override;

    Harmony* findInSeg(Segment* seg) const;

    bool m_isMisspelled = false;                         // show spell check warning
    HarmonyType m_harmonyType = HarmonyType::STANDARD;   // used to control rendering, transposition, export, etc.

    mutable RealizedHarmony m_realizedHarmony;           // the realized harmony used for playback

    std::vector<HDegree> m_degreeList;
    std::vector<muse::draw::Font> m_fontList;            // temp values used in render()
    std::vector<TextSegment*> m_textList;                // rendered chord

    bool m_leftParen = false;
    bool m_rightParen = false;                           // include opening and/or closing parenthesis
    bool m_play = true;                                  // whether or not to play back the harmony

    NoteCaseType m_rootCase = NoteCaseType::AUTO;
    NoteCaseType m_bassCase = NoteCaseType::AUTO;        // case as typed

    std::optional<double> m_userMag;
};
} // namespace mu::engraving
