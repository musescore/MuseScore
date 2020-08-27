//=============================================================================
//  PaletteListView
//
//  Copyright (C) 2020 Peter Jonas
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PALETTELISTVIEW_H__
#define __PALETTELISTVIEW_H__

#include "modularity/ioc.h"
#include "mu4/palette/ipaletteconfiguration.h"

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
    virtual void changeEvent(QEvent* event) override;

private:
    void setupStyle();
};
}

#endif // __PALETTELISTVIEW_H__
