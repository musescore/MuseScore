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
#ifndef MU_PALETTE_PALETTECELL_H
#define MU_PALETTE_PALETTECELL_H

#include "libmscore/engravingitem.h"

#include "modularity/ioc.h"
#include "ui/iuiactionsregister.h"

namespace Ms {
class XmlReader;
class XmlWriter;
}

namespace mu::palette {
struct PaletteCell;
using PaletteCellPtr = std::shared_ptr<PaletteCell>;
using PaletteCellConstPtr = std::shared_ptr<const PaletteCell>;

struct PaletteCell
{
    INJECT_STATIC(palette, mu::ui::IUiActionsRegister, actionsRegister)

    explicit PaletteCell();
    PaletteCell(Ms::ElementPtr e, const QString& _name, qreal _mag = 1.0);

    Ms::ElementPtr element;
    Ms::ElementPtr untranslatedElement;
    QString id;
    QString name; // used for tool tip

    bool drawStaff { false };
    double xoffset { 0.0 }; // in spatium units of "gscore"
    double yoffset { 0.0 };
    qreal mag      { 1.0 };
    bool readOnly { false };

    bool visible { true };
    bool custom { false };
    bool active { false };

    static constexpr const char* mimeDataFormat = "application/musescore/palette/cell";

    const char* translationContext() const;
    QString translatedName() const;

    void retranslate();
    void setElementTranslated(bool translate);

    void write(Ms::XmlWriter& xml) const;
    bool read(Ms::XmlReader&);
    QByteArray toMimeData() const;

    static PaletteCellPtr fromMimeData(const QByteArray& data);
    static PaletteCellPtr fromElementMimeData(const QByteArray& data);

private:
    static QString makeId();
};
}

Q_DECLARE_METATYPE(mu::palette::PaletteCell*)
Q_DECLARE_METATYPE(const mu::palette::PaletteCell*)

#endif // MU_PALETTE_PALETTECELL_H
