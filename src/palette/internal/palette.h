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
#ifndef MU_PALETTE_PALETTE_H
#define MU_PALETTE_PALETTE_H

#include <functional>

#include "palettecell.h"

#include "engraving/rendering/isinglerenderer.h"
#include "engraving/dom/engravingitem.h"

#include "types/translatablestring.h"
#include "actions/actiontypes.h"

#include "modularity/ioc.h"
#include "../ipaletteconfiguration.h"
#include "iinteractive.h"

namespace mu::engraving {
enum class ActionIconType : signed char;
class XmlWriter;
class XMLReader;
}

namespace mu::palette {
class Palette;
using PalettePtr = std::shared_ptr<Palette>;

class Palette : public QObject
{
    Q_GADGET

    INJECT_STATIC(IPaletteConfiguration, configuration)
    INJECT_STATIC(muse::ui::IUiActionsRegister, actionsRegister)
    INJECT_STATIC(engraving::rendering::ISingleRenderer, engravingRender)
    INJECT(muse::IInteractive, interactive)

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
        Guitar,
        Keyboard,
        Pitch,
        Harp,
        StringTunings,
        Playback,
        Custom
    };
    Q_ENUM(Type)

    explicit Palette(Type t = Type::Custom, QObject* parent = nullptr);
    ~Palette();

    QString id() const;

    const QString& name() const { return m_name; }
    void setName(const QString& str) { m_name = str; }

    QString translatedName() const;
    void retranslate();

    Type type() const { return m_type; }
    void setType(Type t) { m_type = t; }

    Type contentType() const;

    // TODO: Remove QString overload
    PaletteCellPtr insertElement(size_t idx, engraving::ElementPtr element, const QString& name, qreal mag = 1.0,
                                 const QPointF& offset = QPointF(), const QString& tag = "");
    PaletteCellPtr insertElement(size_t idx, engraving::ElementPtr element, const muse::TranslatableString& name, qreal mag = 1.0,
                                 const QPointF& offset = QPointF(), const QString& tag = "");
    PaletteCellPtr insertActionIcon(size_t idx, engraving::ActionIconType type, muse::actions::ActionCode code, double mag = 1.0);
    // TODO: Remove QString overload
    PaletteCellPtr appendElement(engraving::ElementPtr element, const QString& name, qreal mag = 1.0,
                                 const QPointF& offset = QPointF(), const QString& tag = "");
    PaletteCellPtr appendElement(engraving::ElementPtr element, const muse::TranslatableString& name, qreal mag = 1.0,
                                 const QPointF& offset = QPointF(), const QString& tag = "");
    PaletteCellPtr appendActionIcon(engraving::ActionIconType type, muse::actions::ActionCode code, double mag = 1.0);

    bool insertCell(size_t idx, PaletteCellPtr cell);
    bool insertCells(size_t idx, std::vector<PaletteCellPtr> cells);
    bool removeCell(PaletteCellPtr cell);
    bool removeCells(std::vector<PaletteCellPtr> cells);

    const std::vector<PaletteCellPtr>& cells() const { return m_cells; }
    int cellsCount() const { return int(m_cells.size()); }
    bool empty() const { return m_cells.empty(); }

    PaletteCellPtr cellAt(int idx) { return m_cells.at(idx); }
    PaletteCellConstPtr cellAt(int idx) const { return m_cells.at(idx); }
    PaletteCellPtr cellAt(size_t idx) { return m_cells.at(idx); }
    PaletteCellConstPtr cellAt(size_t idx) const { return m_cells.at(idx); }

    int indexOfCell(const PaletteCell& cell, bool matchName = true) const;

    PaletteCellPtr takeCell(size_t idx);
    std::vector<PaletteCellPtr> takeCells(size_t idx, size_t count);
    void clear() { m_cells.clear(); }

    qreal yOffset() const { return m_yOffset; }
    void setYOffset(qreal val) { m_yOffset = val; }

    qreal mag() const { return m_mag; }
    void setMag(qreal val) { m_mag = val; }

    bool drawGrid() const { return m_drawGrid; }
    void setDrawGrid(bool val) { m_drawGrid = val; }

    QSize gridSize() const { return m_gridSize; }
    void setGridSize(QSize s) { m_gridSize = s; }
    void setGridSize(int w, int h) { m_gridSize = QSize(w, h); }

    QSize scaledGridSize() const;

    bool isVisible() const { return m_isVisible; }
    void setVisible(bool val) { m_isVisible = val; }

    bool isCustom() const { return m_type == Type::Custom; }

    bool isEditable() const { return m_isEditable; }
    void setEditable(bool val) { m_isEditable = val; }

    bool isExpanded() const { return m_isExpanded; }
    void setExpanded(bool val) { m_isExpanded = val; }

    bool read(engraving::XmlReader&, bool pasteMode);
    void write(engraving::XmlWriter&, bool pasteMode) const;
    static PalettePtr fromMimeData(const QByteArray& data);
    QByteArray toMimeData() const;

    bool readFromFile(const QString& path);
    bool writeToFile(const QString& path) const;

    static constexpr const char* mimeDataFormat = "application/musescore/palette";

private:
    void showWritingPaletteError(const QString& path) const;

    Type guessType() const;

    std::function<void(PaletteCellPtr)> cellHandlerByPaletteType(const Type& type) const;

    QString m_id;
    QString m_name;
    Type m_type;

    std::vector<PaletteCellPtr> m_cells;

    qreal m_yOffset = 0.0; // in spatium units of "gscore"
    qreal m_mag = 1.0;
    QSize m_gridSize = QSize(64, 64);
    bool m_drawGrid = false;

    bool m_isVisible = true;
    bool m_isEditable = true;
    bool m_isExpanded = false;
};
}

#endif // MU_PALETTE_PALETTE_H
