//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __PALETTETREE_H__
#define __PALETTETREE_H__

#include <functional>
#include <QIconEngine>

#include "libmscore/element.h"
#include "libmscore/xml.h"

#include "modularity/ioc.h"
#include "../../ipaletteadapter.h"
#include "../../ipaletteconfiguration.h"
#include "async/asyncable.h"
#include "iinteractive.h"

namespace Ms {
struct PaletteCell;
using PaletteCellPtr = std::shared_ptr<PaletteCell>;
using PaletteCellConstPtr = std::shared_ptr<const PaletteCell>;

//---------------------------------------------------------
//   PaletteCell
//---------------------------------------------------------

struct PaletteCell
{
    INJECT_STATIC(palette, mu::palette::IPaletteAdapter, adapter)

    ElementPtr element;
    ElementPtr untranslatedElement;
    QString id;
    QString name;             // used for tool tip
    QString tag;

    bool drawStaff { false };
    double xoffset { 0.0 };
    double yoffset { 0.0 };          // in spatium units of "gscore"
    qreal mag      { 1.0 };
    bool readOnly  { false };

    bool visible   { true };
    bool custom    { false };
    bool active    { false };

    explicit PaletteCell() = default;
    PaletteCell(ElementPtr e, const QString& _name, qreal _mag = 1.0);

    static constexpr const char* mimeDataFormat = "application/musescore/palette/cell";

    const char* translationContext() const;
    QString translatedName() const;

    void retranslate();
    void setElementTranslated(bool translate);

    void write(XmlWriter& xml) const;
    bool read(XmlReader&);
    QByteArray mimeData() const;
    static PaletteCellPtr readMimeData(const QByteArray& data);
    static PaletteCellPtr readElementMimeData(const QByteArray& data);
    static QString makeId();
};

//---------------------------------------------------------
//   PaletteCellIconEngine
//---------------------------------------------------------

class PaletteCellIconEngine : public QIconEngine
{
    PaletteCellConstPtr _cell;
    qreal _extraMag = 1.0;

    PaletteCellConstPtr cell() const { return _cell; }

    INJECT_STATIC(palette, mu::palette::IPaletteConfiguration, configuration)

private:
    void paintCell(mu::draw::Painter& painter, const QRect& rect, bool selected, bool current) const;
    void paintScoreElement(mu::draw::Painter& painter, Element* element, qreal spatium, bool alignToStaff) const;

    static qreal paintStaff(mu::draw::Painter& painter, const QRect& rect, qreal spatium);
    static void paintTag(mu::draw::Painter& painter, const QRect& rect, QString tag);
    static void paintBackground(mu::draw::Painter& painter, const QRect& rect, bool selected, bool current);

public:
    PaletteCellIconEngine(PaletteCellConstPtr cell, qreal extraMag = 1.0)
        : _cell(cell), _extraMag(extraMag) {}

    QIconEngine* clone() const override { return new PaletteCellIconEngine(cell(), _extraMag); }

    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override;
};

//---------------------------------------------------------
//   PalettePanel
//---------------------------------------------------------

class PalettePanel;
using PalettePanelPtr = std::shared_ptr<PalettePanel>;

class PalettePanel
{
    Q_GADGET

    INJECT(palette, mu::framework::IInteractive, interactive)

public:
    enum class Type {
        Unknown = 0,
        Clef,
        KeySig,
        TimeSig,
        Bracket,
        Accidental,
        Articulation,
        Ornament,
        Breath,
        GraceNote,
        NoteHead,
        Line,
        BarLine,
        Arpeggio,
        Tremolo,
        Text,
        Tempo,
        Dynamic,
        Fingering,
        Repeat,
        FretboardDiagram,
        Accordion,
        BagpipeEmbellishment,
        Layout,
        Beam,
        Custom
    };
    Q_ENUM(Type)

private:
    QString _id;
    QString _name;
    Type _type;

    std::vector<PaletteCellPtr> cells;

    QSize _gridSize = QSize(64, 64);

    qreal _mag = 1.0;
    bool _drawGrid = false;
    bool _editable = true;
    qreal _yOffset = 0.0; // in spatium units of "gscore"

    bool _moreElements = false; // not used by QML palettes, default is false for compatibility with Palette class. TODO: remove?

    bool _visible = true;
    bool _expanded = false;

    Type guessType() const;
    std::function<void(PaletteCellPtr)> cellHandlerByPaletteType(const Type& type) const;

    void showWritingPaletteError(const QString& path) const;

public:
    PalettePanel(Type t = Type::Custom);

    PaletteCellPtr insert(int idx, ElementPtr element, const QString& name, qreal mag = 1.0);
    PaletteCellPtr append(ElementPtr element, const QString& name, qreal mag = 1.0);

    QString id() const;

    const QString& name() const { return _name; }
    void setName(const QString& str) { _name = str; }

    QString translatedName() const;

    QSize gridSize() const { return _gridSize; }
    void setGrid(QSize s) { _gridSize = s; }
    void setGrid(int w, int h) { _gridSize = QSize(w, h); }

    QSize scaledGridSize() const;

    qreal mag() const { return _mag; }
    void setMag(qreal val) { _mag = val; }

    bool drawGrid() const { return _drawGrid; }
    void setDrawGrid(bool val) { _drawGrid = val; }

    qreal yOffset() const { return _yOffset; }
    void setYOffset(qreal val) { _yOffset = val; }

    bool moreElements() const { return _moreElements; }
    void setMoreElements(bool val) { _moreElements = val; }

    bool visible() const { return _visible; }
    void setVisible(bool val) { _visible = val; }
    bool custom() const { return _type == Type::Custom; }

    static constexpr const char* mimeDataFormat = "application/musescore/palette/panel";

    void write(XmlWriter&) const;
    bool read(XmlReader&);
    QByteArray mimeData() const;
    static PalettePanelPtr readMimeData(const QByteArray& data);

    bool readFromFile(const QString& path);
    bool writeToFile(const QString& path) const;

    int ncells() const { return int(cells.size()); }
    bool empty() const { return cells.empty(); }
    PaletteCellPtr cell(int idx) { return cells[idx]; }
    PaletteCellConstPtr cell(int idx) const { return cells[idx]; }

    std::vector<PaletteCellPtr> takeCells(int idx, int count);
    bool insertCells(int idx, std::vector<PaletteCellPtr> cells);
    bool insertCell(int idx, PaletteCellPtr cell);

    int findPaletteCell(const PaletteCell& cell, bool matchName = true) const;

    bool expanded() const { return _expanded; }
    void setExpanded(bool val) { _expanded = val; }

    bool editable() const { return _editable; }
    void setEditable(bool val) { _editable = val; }

    Type type() const { return _type; }
    void setType(Type t) { _type = t; }

    Type contentType() const;

    void retranslate();
};

//---------------------------------------------------------
//   PaletteTree
//---------------------------------------------------------

struct PaletteTree
{
    std::vector<PalettePanelPtr> palettes;

    void write(XmlWriter&) const;
    bool read(XmlReader&);

    void insert(int idx, PalettePanel*);
    void append(PalettePanel*);

    void retranslate();
};

using PaletteTreePtr = std::shared_ptr<PaletteTree>;
} // namespace Ms

Q_DECLARE_METATYPE(const Ms::PaletteCell*)
Q_DECLARE_METATYPE(Ms::PaletteCell*)

#endif
