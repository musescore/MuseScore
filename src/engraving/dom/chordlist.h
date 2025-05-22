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

#ifndef MU_ENGRAVING_CHORDLIST_H
#define MU_ENGRAVING_CHORDLIST_H

#include <map>

#include "global/allocator.h"
#include "global/types/string.h"
#include "global/containers.h"
#include "global/io/iodevice.h"

#include "modularity/ioc.h"
#include "../iengravingconfiguration.h"

namespace mu::engraving::compat {
class ReadChordListHook;
}

namespace mu::engraving {
class ChordList;
class MStyle;
class XmlWriter;
class XmlReader;

//---------------------------------------------------------
//   class HDegree
//---------------------------------------------------------

enum class HDegreeType : char {
    UNDEF, ADD, ALTER, SUBTRACT
};

class HDegree
{
public:
    HDegree() = default;
    HDegree(int v, int a, HDegreeType t) { m_value = v; m_alter = a; m_type = t; }
    int value() const { return m_value; }
    int alter() const { return m_alter; }
    HDegreeType type() const { return m_type; }
    String text() const;

private:
    int m_value = 0;
    int m_alter = 0;         // -1, 0, 1  (b - - #)
    HDegreeType m_type = HDegreeType::UNDEF;
};

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

class HChord
{
public:
    HChord() = default;
    HChord(int k) { m_keys = k; }
    HChord(int a, int b, int c=-1, int d=-1, int e=-1, int f=-1, int g=-1, int h=-1, int i=-1, int k=-1, int l=-1);
    HChord(const String&);

    void rotate(int semiTones);

    bool contains(int key) const           // key in chord?
    {
        return (1 << (key % 12)) & m_keys;
    }

    HChord& operator+=(int key)
    {
        m_keys |= (1 << (key % 12));
        return *this;
    }

    HChord& operator-=(int key)
    {
        m_keys &= ~(1 << (key % 12));
        return *this;
    }

    bool operator==(const HChord& o) const { return m_keys == o.m_keys; }
    bool operator!=(const HChord& o) const { return m_keys != o.m_keys; }

    int getKeys() const { return m_keys; }
    void print() const;

    String name(int tpc) const;
    String voicing() const;
    void add(const std::vector<HDegree>& degreeList);

protected:
    int m_keys = 0;

private:
    String m_str;
};

//---------------------------------------------------------
//   RenderAction
//---------------------------------------------------------

struct RenderAction
{
    enum class RenderActionType : char {
        SET, MOVE, PUSH, POP,
        NOTE, ACCIDENTAL, STOPHALIGN, SCALE
    };

    virtual RenderActionType actionType() const = 0;

    virtual void print() const { print(actionType(), u""); }

    virtual ~RenderAction() {}

protected:
    void print(RenderActionType type, const String& info) const;
};

struct RenderActionMove : RenderAction
{
    RenderActionMove() {}
    RenderActionMove(double movex, double movey)
        : m_movex(movex), m_movey(movey) {}

    RenderActionType actionType() const override { return RenderActionType::MOVE; }
    void print() const override;

    void setx(double x) { m_movex = x; }
    void sety(double y) { m_movex = y; }
    double x() const { return m_movex; }
    double y() const { return m_movey; }
private:
    double m_movex = 0.0, m_movey = 0.0;
};

struct RenderActionSet : RenderAction
{
    RenderActionSet() {}
    RenderActionSet(String s)
        : m_text(s) {}

    RenderActionType actionType() const override { return RenderActionType::SET; }
    void print() const override;

    const String& text() const { return m_text; }
private:
    String m_text;
};

struct RenderActionPush : RenderAction
{
    RenderActionPush() {}
    RenderActionType actionType() const override { return RenderActionType::PUSH; }
};

struct RenderActionPop : RenderAction
{
    RenderActionPop() {}
    RenderActionPop(bool popx, bool popy)
        : m_popx(popx), m_popy(popy) {}

    RenderActionType actionType() const override { return RenderActionType::POP; }
    void print() const override;

    void setPopX(bool popx) { m_popx = popx; }
    void setPopY(bool popy) { m_popx = popy; }
    double popX() const { return m_popx; }
    double popY() const { return m_popy; }
private:
    bool m_popx = true, m_popy = true;
};

struct RenderActionNote : RenderAction
{
    RenderActionNote() {}
    RenderActionType actionType() const override { return RenderActionType::NOTE; }
};

struct RenderActionAccidental : RenderAction
{
    RenderActionAccidental() {}
    RenderActionType actionType() const override { return RenderActionType::ACCIDENTAL; }
};

struct RenderActionStopHAlign : RenderAction
{
    RenderActionStopHAlign() {}
    RenderActionType actionType() const override { return RenderActionType::STOPHALIGN; }
};

struct RenderActionScale : RenderAction
{
    RenderActionScale() {}
    RenderActionScale(double scale)
        : m_scale(scale) {}
    RenderActionType actionType() const override { return RenderActionType::SCALE; }

    double scale() const { return m_scale; }

    void print() const override;

private:
    double m_scale = 1.0;
};

//---------------------------------------------------------
//   ChordToken
//---------------------------------------------------------

enum class ChordTokenClass : char {
    ALL, QUALITY, EXTENSION, MODIFIER, TYPE
};

class ChordToken
{
public:
    ChordTokenClass tokenClass;
    StringList names;
    std::list<RenderAction*> renderList;
    void read(XmlReader&, int mscVersion);
    void write(XmlWriter&) const;
};

//---------------------------------------------------------
//   ParsedChord
//---------------------------------------------------------

class ParsedChord
{
public:
    ParsedChord() = default;

    bool parse(const String&, const ChordList*, bool syntaxOnly = false, bool preferMinor = false);
    String fromXml(const String&, const String&, const String&, const String&, const std::list<HDegree>&, const ChordList*);
    const std::list<RenderAction*>& renderList(const ChordList*);
    bool parseable() const { return m_parseable; }
    bool understandable() const { return m_understandable; }
    const String& name() const { return m_name; }
    const String& quality() const { return m_quality; }
    const String& extension() const { return m_extension; }
    const String& modifiers() const { return m_modifiers; }
    const StringList& modifierList() const { return m_modifierList; }
    const String& xmlKind() const { return m_xmlKind; }
    const String& xmlText() const { return m_xmlText; }
    const String& xmlSymbols() const { return m_xmlSymbols; }
    const String& xmlParens() const { return m_xmlParens; }
    const StringList& xmlDegrees() const { return m_xmlDegrees; }
    int keys() const { return m_chord.getKeys(); }
    const String& handle() const { return m_handle; }
    operator String() const {
        return m_handle;
    }
    bool operator==(const ParsedChord& c) const { return this->m_handle == c.m_handle; }
    bool operator!=(const ParsedChord& c) const { return !(*this == c); }

private:
    void configure(const ChordList*);
    void correctXmlText(const String& s = String());
    void addToken(String, ChordTokenClass);

    String m_name;
    String m_handle;
    String m_quality;
    String m_extension;
    String m_modifiers;
    StringList m_modifierList;
    std::list<ChordToken> m_tokenList;
    std::list<RenderAction*> m_renderList;
    String m_xmlKind;
    String m_xmlText;
    String m_xmlSymbols;
    String m_xmlParens;
    StringList m_xmlDegrees;
    StringList m_major, m_minor, m_diminished, m_augmented, m_lower, m_raise, m_mod, m_symbols;
    HChord m_chord;
    bool m_parseable = false;
    bool m_understandable = false;
};

//---------------------------------------------------------
//   ChordDescription
//---------------------------------------------------------

struct ChordDescription {
    int id = 0;               // Chord id number (Band In A Box Chord Number)
    StringList names;        // list of alternative chord names
                             // that will by recognized from keyboard entry (without root/base)
    std::list<ParsedChord> parsedChords;
    // parsed forms of primary name (optionally also include parsed forms of other names)
    String xmlKind;          // MusicXML: kind
    String xmlText;          // MusicXML: kind text=
    String xmlSymbols;       // MusicXML: kind use-symbols=
    String xmlParens;        // MusicXML: kind parentheses-degrees=
    StringList xmlDegrees;   // MusicXML: list of degrees (if any)
    HChord chord;             // C based chord
    std::list<RenderAction*> renderList;
    bool generated = false;
    bool renderListGenerated = false;
    bool exportOk = false;

    ChordDescription() {}
    ChordDescription(int);
    ChordDescription(const String&);
    String quality() const { return m_quality; }
    void complete(ParsedChord* pc, const ChordList*);
    void read(XmlReader&, int mscVersion);
    void write(XmlWriter&) const;

private:
    String m_quality;
};

//---------------------------------------------------------
//   ChordSymbol
//---------------------------------------------------------

struct ChordSymbol {
    int fontIdx = -1;
    String name;
    String value;
    Char code;

    bool isValid() const { return fontIdx != -1; }
};

//---------------------------------------------------------
//   ChordFont
//---------------------------------------------------------

struct ChordFont {
    String family;
    String fontClass;
    double mag = 1.0;
    double stackedMag = 1.0;
};

//---------------------------------------------------------
//   ChordList
//---------------------------------------------------------

class ChordList : public std::map<int, ChordDescription>
{
    OBJECT_ALLOCATOR(engraving, ChordList)

public:
    std::list<ChordFont> fonts;
    std::list<RenderAction*> renderListRoot;
    std::list<RenderAction*> renderListFunction;
    std::list<RenderAction*> renderListBass;
    std::list<RenderAction*> renderListBassOffset;
    std::list<ChordToken> chordTokenList;
    static int privateID;

    bool autoAdjust() const { return m_autoAdjust; }
    double nominalMag() const { return m_nmag; }
    double nominalAdjust() const { return m_nadjust; }
    bool stackModifiers() const { return m_stackModifiers; }
    bool excludeModsHAlign() const { return m_excludeModsHAlign; }
    double stackedModifierMag() const { return m_stackedmmag; }
    void configureAutoAdjust(double emag = 1.0, double eadjust = 0.0, double mmag = 1.0, double madjust = 0.0, double stackedmmag = 0.0,
                             bool stackModifiers = false, bool excludeModsHAlign = false);
    double position(const StringList& names, ChordTokenClass ctc, size_t modifierIdx, size_t nmodifiers) const;

    void checkChordList(const muse::io::path_t& appDataPath, const MStyle& style);
    bool read(const muse::io::path_t& appDataPath, const String& name);
    bool read(muse::io::IODevice* device);
    bool write(const String&) const;
    bool write(muse::io::IODevice* device) const;
    bool loaded() const;
    void unload();

    const ChordDescription* description(int id) const;
    ChordSymbol symbol(const String& s) const { return muse::value(m_symbols, s); }

    void setCustomChordList(bool t) { m_customChordList = t; }
    bool customChordList() const { return m_customChordList; }

private:

    friend class compat::ReadChordListHook;

    void read(XmlReader& xml, int mscVersion);
    void write(XmlWriter& xml) const;

    std::map<String, ChordSymbol> m_symbols;
    bool m_autoAdjust = false;
    bool m_stackModifiers = false;
    bool m_excludeModsHAlign = false;
    double m_nmag = 1.0, m_nadjust = 0.0;   // adjust values are measured in percentage
    double m_emag = 1.0, m_eadjust = 0.0;   // (which is then applied to the height of the font)
    double m_mmag = 1.0, m_madjust = 0.0, m_stackedmmag = 0.0;

    bool m_customChordList = false;         // if true, chordlist will be saved as part of score
};
} // namespace mu::engraving
#endif
