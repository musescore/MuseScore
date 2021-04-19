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

#ifndef __PALETTELISTVIEW_H__
#define __PALETTELISTVIEW_H__

#include <QListView>

#include "modularity/ioc.h"
#include "palette/ipaletteconfiguration.h"

namespace Ms {
class Element;
class PalettePanel;

struct PaletteCell;

//---------------------------------------------------------
//   PaletteListView
/// Display a simple icon list of elements from a single
/// palette category (e.g. key signatures).
//---------------------------------------------------------

class PaletteListView : public QListView // see also QListWidget
{
    Q_OBJECT

    INJECT(palette, mu::palette::IPaletteConfiguration, configuration)

public:
    PaletteListView(PalettePanel* panel, QWidget* parent = nullptr);
    const PaletteCell* currentCell() const;
    Element* currentElement() const;
    void focusNextMatchingCell(const QString& str);

    int count() { return model()->rowCount(rootIndex()); }
    int currentRow() { return currentIndex().row(); }
    void setCurrentRow(int row) { setCurrentIndex(model()->index(row, 0, rootIndex())); }

    void incrementCurrentRow()
    {
        if (currentRow() < (count() - 1)) {
            setCurrentRow(currentRow() + 1);
        }
    }

    void decrementCurrentRow()
    {
        if (currentRow() > 0) {
            setCurrentRow(currentRow() - 1);
        }
    }

protected:
    virtual void keyPressEvent(QKeyEvent* event) override;
};
}

#endif // __PALETTELISTVIEW_H__
