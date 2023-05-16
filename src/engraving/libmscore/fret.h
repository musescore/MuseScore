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

#ifndef __FRET_H__
#define __FRET_H__

#include "engravingitem.h"
#include "harmony.h"

#include "draw/types/font.h"

namespace mu::engraving {
class Factory;
class StringData;
class Chord;

// Keep this in order - not used directly for comparisons, but the dots will appear in
// this order in fret multidot mode. See fretproperties.cpp.
enum class FretDotType : signed char {
    NORMAL = 0,
    CROSS,
    SQUARE,
    TRIANGLE = 3
};

enum class FretMarkerType : signed char {
    NONE,
    CIRCLE,
    CROSS
};

class FretItem
{
public:
    struct Barre {
        int startString;
        int endString;

        Barre() { startString = endString = -1; }
        Barre(int s, int e)
            : startString(s), endString(e) {}
        bool exists() const { return startString > -1; }
    };

    struct Dot {
        int fret                { 0 };
        FretDotType dtype       { FretDotType::NORMAL };
        int fingering           { 0 };     // NOTE:JT - possible future feature?

        Dot() {}
        Dot(int f, FretDotType t = FretDotType::NORMAL)
            : fret(f), dtype(t) {}
        bool exists() const { return fret > 0; }
    };

    struct Marker {
        FretMarkerType mtype;

        Marker() { mtype = FretMarkerType::NONE; }
        Marker(FretMarkerType t)
            : mtype(t) {}
        bool exists() const { return mtype != FretMarkerType::NONE; }
    };

    struct MarkerTypeNameItem {
        FretMarkerType mtype;
        const char* name;
    };

    struct DotTypeNameItem {
        FretDotType dtype;
        const char* name;
    };

    static const std::vector<FretItem::MarkerTypeNameItem> markerTypeNameMap;
    static const std::vector<FretItem::DotTypeNameItem> dotTypeNameMap;

    static Char markerToChar(FretMarkerType t);
    static String markerTypeToName(FretMarkerType t);
    static FretMarkerType nameToMarkerType(String n);
    static String dotTypeToName(FretDotType t);
    static FretDotType nameToDotType(String n);
};

// The three main storage containers used by fret diagrams
typedef std::map<int, FretItem::Barre> BarreMap;
typedef std::map<int, FretItem::Marker> MarkerMap;
typedef std::map<int, std::vector<FretItem::Dot> > DotMap;

class FretUndoData
{
    FretDiagram* _diagram         { nullptr };
    BarreMap _barres;
    MarkerMap _markers;
    DotMap _dots;

public:
    FretUndoData() {}
    FretUndoData(FretDiagram* fd);

    void updateDiagram();
};

//---------------------------------------------------------
//   @@ FretDiagram
///    Fretboard diagram
//
//   @P userMag    double
//   @P strings    int  number of strings
//   @P frets      int  number of frets
//   @P fretOffset int
//
//   Note that, while strings are zero-indexed, frets are one-indexed
//---------------------------------------------------------

class FretDiagram final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, FretDiagram)
    DECLARE_CLASSOF(ElementType::FRET_DIAGRAM)

public:

    ~FretDiagram();

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    void draw(mu::draw::Painter*) const override;
    EngravingItem* linkedClone() override;
    FretDiagram* clone() const override { return new FretDiagram(*this); }

    Segment* segment() const { return toSegment(explicitParent()); }

    static std::shared_ptr<FretDiagram> createFromString(Score* score, const String& s);

    std::vector<mu::LineF> dragAnchorLines() const override;
    mu::PointF pagePos() const override;
    double centerX() const;
    double rightX() const;

    int  strings() const { return _strings; }
    int  frets()   const { return _frets; }
    void setStrings(int n);
    void setFrets(int n) { _frets = n; }

    void setDot(int string, int fret, bool add = false, FretDotType dtype = FretDotType::NORMAL);
    void setBarre(int startString, int endString, int fret);
    void setBarre(int string, int fret, bool add = false);
    void setMarker(int string, FretMarkerType marker);
    /*void setFingering(int string, int finger);*/
    void clear();
    void undoSetFretDot(int _string, int _fret, bool _add = false, FretDotType _dtype = FretDotType::NORMAL);
    void undoSetFretMarker(int _string, FretMarkerType _mtype);
    void undoSetFretBarre(int _string, int _fret, bool _add = false);
    void undoFretClear();
    int  fretOffset() const { return _fretOffset; }
    void setFretOffset(int val) { _fretOffset = val; }
    int  maxFrets() const { return _maxFrets; }
    void setMaxFrets(int val) { _maxFrets = val; }
    bool showNut() const { return _showNut; }
    void setShowNut(bool val) { _showNut = val; }

    String harmonyText() const { return _harmony ? _harmony->plainText() : String(); }
    void setHarmony(String harmonyText);

    std::vector<FretItem::Dot> dot(int s, int f = 0) const;
    FretItem::Marker marker(int s) const;
    FretItem::Barre barre(int fret) const;

    BarreMap barres() const { return _barres; }
    DotMap dots() const { return _dots; }
    MarkerMap markers() const { return _markers; }

    Harmony* harmony() const { return _harmony; }

    void init(StringData*, Chord*);
    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    void endEditDrag(EditData& editData) override;
    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    double userMag() const { return _userMag; }
    void setUserMag(double m) { _userMag = m; }

    String accessibleInfo() const override;
    String screenReaderInfo() const override;

    friend class FretUndoData;

private:
    friend class layout::v0::TLayout;
    friend class Factory;

    FretDiagram(Segment* parent = nullptr);
    FretDiagram(const FretDiagram&);

    void removeDot(int s, int f = 0);
    void removeBarre(int f);
    void removeBarres(int string, int fret = 0);
    void removeMarker(int s);
    void removeDotsMarkers(int ss, int es, int fret);

    int _strings       { 6 };
    int _frets         { 4 };
    int _fretOffset    { 0 };
    int _maxFrets      { 24 };
    bool _showNut      { true };
    Orientation _orientation      { Orientation::VERTICAL };

    // Barres are stored in the format: K: fret, V: barre struct
    BarreMap _barres;

    // Dots stored as K: string, V: dot struct
    DotMap _dots;

    // Markers stored as K: string, V: marker struct
    MarkerMap _markers;

    Harmony* _harmony  { nullptr };

    double m_stringLw;
    double m_nutLw;
    double m_stringDist;
    double m_fretDist;
    mu::draw::Font m_font;
    double _userMag     { 1.0 };                 // allowed 0.1 - 10.0
    double m_markerSize;
    int _numPos;
};
} // namespace mu::engraving

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::engraving::FretDiagram*)
#endif

#endif
