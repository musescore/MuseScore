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
#ifndef MU_PALETTE_PALETTECELL_H
#define MU_PALETTE_PALETTECELL_H

#include "engraving/dom/engravingitem.h"

#include <QAccessibleInterface>

#include "modularity/ioc.h"
#include "ui/iuiactionsregister.h"

namespace mu::engraving {
class XmlReader;
class XmlWriter;
}

namespace mu::palette {
class PaletteCell;
using PaletteCellPtr = std::shared_ptr<PaletteCell>;
using PaletteCellConstPtr = std::shared_ptr<const PaletteCell>;

class AccessiblePaletteCellInterface : public QAccessibleInterface
{
public:
    AccessiblePaletteCellInterface(PaletteCell* cell);

    bool isValid() const override;
    QObject* object() const override;
    QAccessibleInterface* childAt(int x, int y) const override;
    QAccessibleInterface* parent() const override;
    QAccessibleInterface* child(int index) const override;
    int childCount() const override;
    int indexOfChild(const QAccessibleInterface*) const override;
    QString text(QAccessible::Text t) const override;
    void setText(QAccessible::Text t, const QString& text) override;
    QRect rect() const override;
    QAccessible::Role role() const override;
    QAccessible::State state() const override;

private:
    QObject* parentWidget() const;

    PaletteCell* m_cell = nullptr;
};

class PaletteCell : public QObject
{
    Q_OBJECT
    INJECT_STATIC(muse::ui::IUiActionsRegister, actionsRegister)

public:
    explicit PaletteCell(QObject* parent = nullptr);
    PaletteCell(mu::engraving::ElementPtr e, const QString& _name, qreal _mag = 1.0,
                const QPointF& offset = QPointF(), const QString& tag = "", QObject* parent = nullptr);

    static QAccessibleInterface* accessibleInterface(QObject* object);

    static constexpr const char* mimeDataFormat = "application/musescore/palette/cell";

    const char* translationContext() const;
    QString translatedName() const;

    void retranslate();
    void setElementTranslated(bool translate);

    void write(mu::engraving::XmlWriter& xml, bool pasteMode) const;
    bool read(mu::engraving::XmlReader&, bool pasteMode);
    QByteArray toMimeData() const;

    static PaletteCellPtr fromMimeData(const QByteArray& data);
    static PaletteCellPtr fromElementMimeData(const QByteArray& data);

    mu::engraving::ElementPtr element;
    mu::engraving::ElementPtr untranslatedElement;
    QString id;

    QString name; // used for tool tip
    qreal mag { 1.0 };
    double xoffset { 0.0 }; // in spatium units of "gscore"
    double yoffset { 0.0 };
    QString tag;

    bool drawStaff { false };
    bool readOnly { false };
    bool visible { true };
    bool custom { false };
    bool active { false };
    bool focused { false };

private:
    static QString makeId();
};
}

#endif // MU_PALETTE_PALETTECELL_H
