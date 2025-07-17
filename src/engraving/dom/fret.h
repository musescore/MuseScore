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

#include "engravingitem.h"

#include "draw/types/font.h"

namespace mu::engraving {
class Factory;
class StringData;
class Chord;
class Harmony;

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
        int startString = -1;
        int endString = -1;

        Barre() = default;
        Barre(int s, int e)
            : startString(s), endString(e) {}

        bool exists() const { return startString > -1; }
    };

    struct Dot {
        int fret = 0;
        FretDotType dtype = FretDotType::NORMAL;
        int fingering = 0;     // NOTE:JT - possible future feature?

        Dot() = default;
        Dot(int f, FretDotType t = FretDotType::NORMAL)
            : fret(f), dtype(t) {}

        bool exists() const { return fret > 0; }
    };

    struct Marker {
        FretMarkerType mtype = FretMarkerType::NONE;

        Marker() = default;
        Marker(FretMarkerType t)
            : mtype(t) {}

        bool exists() const { return mtype != FretMarkerType::NONE; }
    };

    struct MarkerTypeNameItem {
        FretMarkerType mtype = FretMarkerType::NONE;
        const char* name = nullptr;
    };

    struct DotTypeNameItem {
        FretDotType dtype = FretDotType::NORMAL;
        const char* name = nullptr;
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
public:
    FretUndoData() {}
    FretUndoData(FretDiagram* fd);

    void updateDiagram();

private:

    FretDiagram* m_diagram = nullptr;
    BarreMap m_barres;
    MarkerMap m_markers;
    DotMap m_dots;

    int m_strings = 0;
    int m_frets = 0;
    int m_fretOffset = 0;
    int m_maxFrets = 0;
    bool m_showNut = true;
    bool m_showFingering = false;
    Orientation m_orientation = Orientation::VERTICAL;
    double m_userMag = 1.0;
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

    EngravingItem* linkedClone() override;
    FretDiagram* clone() const override { return new FretDiagram(*this); }

    Segment* segment() const;

    static std::shared_ptr<FretDiagram> createFromPattern(Score* score, const String& s);
    static String patternFromDiagram(const FretDiagram* diagram);

    void updateDiagram(const String& harmonyName);

    std::vector<LineF> dragAnchorLines() const override;
    PointF pagePos() const override;
    double mainWidth() const;

    int  strings() const { return m_strings; }
    int  frets()   const { return m_frets; }
    void setStrings(int n);
    void setFrets(int n) { m_frets = n; }

    void setDot(int string, int fret, bool add = false, FretDotType dtype = FretDotType::NORMAL);
    void addDotForDotStyleBarre(int string, int fret);
    void removeDotForDotStyleBarre(int string, int fret);
    void setBarre(int startString, int endString, int fret);
    void setBarre(int string, int fret, bool add = false);
    void setMarker(int string, FretMarkerType marker);
    /*void setFingering(int string, int finger);*/
    void clear();
    void undoSetFretDot(int _string, int _fret, bool _add = false, FretDotType _dtype = FretDotType::NORMAL);
    void undoSetFretMarker(int _string, FretMarkerType _mtype);
    void undoSetFretBarre(int _string, int _fret, bool _add = false);
    void undoFretClear();
    int  fretOffset() const { return m_fretOffset; }
    void setFretOffset(int val) { m_fretOffset = val; }
    int  maxFrets() const { return m_maxFrets; }
    void setMaxFrets(int val) { m_maxFrets = val; }
    bool showNut() const { return m_showNut; }
    void setShowNut(bool val) { m_showNut = val; }
    double userMag() const { return m_userMag; }
    void setUserMag(double m) { m_userMag = m; }
    int numPos() const;

    Orientation orientation() const { return m_orientation; }

    String harmonyText() const;
    Harmony* harmony() const { return m_harmony; }
    void setHarmony(String harmonyText);
    void linkHarmony(Harmony* harmony);
    void unlinkHarmony();

    std::vector<FretItem::Dot> dot(int s, int f = 0) const;
    FretItem::Marker marker(int s) const;
    FretItem::Barre barre(int fret) const;

    const BarreMap& barres() const { return m_barres; }
    const DotMap& dots() const { return m_dots; }
    const MarkerMap& markers() const { return m_markers; }

    muse::draw::Font fretNumFont() const;
    muse::draw::Font fingeringFont() const;

    void init(StringData*, Chord*);
    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;

    RectF drag(EditData&) override;
    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    void setTrack(track_idx_t val) override;

    String accessibleInfo() const override;
    String screenReaderInfo() const override;

    bool showFingering() const { return m_showFingering; }
    void setShowFingering(bool v) { m_showFingering = v; }
    const std::vector<int>& fingering() const { return m_fingering; }
    void setFingering(std::vector<int> v);

    static FretDiagram* makeFromHarmonyOrFretDiagram(const EngravingItem* harmonyOrFretDiagram);

    bool isInFretBox() const;
    bool isCustom(const String& harmonyNameForCompare) const;

    bool allowTimeAnchor() const { return explicitParent() && parent()->isSegment(); }

    friend class FretUndoData;

    struct FingeringItem {
        String fingerNumber;
        PointF pos;
        FingeringItem(String s, PointF p)
            : fingerNumber(s), pos(p) {}
    };

    struct LayoutData : public EngravingItem::LayoutData {
        double stringLineWidth = 0.0;
        double nutLineWidth = 0.0;
        double nutY = 0.0;
        double stringDist = 0.0;
        double fretDist = 0.0;
        double markerSize = 0.0;
        double markerY = 0.0;
        double stringExtendTop = 0.0;
        double stringExtendBottom = 0.0;
        double dotDiameter = 0.0;
        double fretNumPadding = 0.0;
        double gridHeight = 0.0;
        std::vector<FingeringItem> fingeringItems;
        PainterPath slurPath = PainterPath();
        String fretText = String();
    };
    DECLARE_LAYOUTDATA_METHODS(FretDiagram)

private:
    friend class Factory;

    FretDiagram(Segment* parent = nullptr);
    FretDiagram(const FretDiagram&);

    void readHarmonyToDiagramFile(const muse::io::path_t& filePath) const;

    void initDefaultValues();

    void removeDot(int s, int f = 0);
    void removeBarre(int f);
    void removeBarres(int string, int fret = 0);
    void removeMarker(int s);
    void removeDotsMarkers(int ss, int es, int fret);

    static void applyDiagramPattern(FretDiagram* diagram, const String& pattern);

    void applyAlignmentToHarmony();

    int m_strings = 0;
    int m_frets = 0;
    int m_fretOffset = 0;
    int m_maxFrets = 0;
    bool m_showNut = false;
    Orientation m_orientation = Orientation::VERTICAL;

    // Barres are stored in the format: K: fret, V: barre struct
    BarreMap m_barres;

    // Dots stored as K: string, V: dot struct
    DotMap m_dots;

    // Markers stored as K: string, V: marker struct
    MarkerMap m_markers;

    Harmony* m_harmony = nullptr;

    double m_userMag = 1.0;                 // allowed 0.1 - 10.0

    bool m_showFingering = false;
    std::vector<int> m_fingering = std::vector<int>(m_strings, 0);
};
} // namespace mu::engraving

#ifndef NO_QT_SUPPORT
Q_DECLARE_METATYPE(mu::engraving::FretDiagram*)
#endif
