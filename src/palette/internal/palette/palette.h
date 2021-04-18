//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#ifndef __PALETTE_H__
#define __PALETTE_H__

#include <QScrollArea>

#include "palettetree.h"
#include "libmscore/sym.h"

#include "modularity/ioc.h"
#include "ipaletteadapter.h"
#include "ipaletteconfiguration.h"

#include "iinteractive.h"

namespace Ms {
class Element;
class Sym;
class XmlWriter;
class XmlReader;
class Palette;

//---------------------------------------------------------
//    PaletteScrollArea
//---------------------------------------------------------

class PaletteScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    PaletteScrollArea(Palette* w, QWidget* parent = nullptr);

    bool restrictHeight() const;
    void setRestrictHeight(bool val);

private:
    void keyPressEvent(QKeyEvent*) override;
    void resizeEvent(QResizeEvent*) override;

    bool m_restrictHeight = false;
};

//---------------------------------------------------------
//   Palette
//---------------------------------------------------------

class Palette : public QWidget
{
    Q_OBJECT

    INJECT_STATIC(palette, mu::palette::IPaletteAdapter, adapter)
    INJECT_STATIC(palette, mu::palette::IPaletteConfiguration, configuration)
    INJECT(palette, mu::framework::IInteractive, interactive)

signals:
    void boxClicked(int);
    void changed();
    void displayMore(const QString& paletteName);

public:
    Palette(QWidget* parent = nullptr);
    Palette(PalettePanelPtr palettePanel, QWidget* parent = nullptr);

    void nextPaletteElement();
    void prevPaletteElement();
    void applyPaletteElement();
    static bool applyPaletteElement(ElementPtr element, Qt::KeyboardModifiers modifiers = {});
    PaletteCellPtr append(ElementPtr element, const QString& name, QString tag = QString(), qreal mag = 1.0);
    PaletteCellPtr add(int idx, ElementPtr element, const QString& name, const QString tag = QString(), qreal mag = 1.0);

    void emitChanged() { emit changed(); }
    void setGrid(int, int);
    ElementPtr element(int idx) const;
    void setDrawGrid(bool val);
    bool drawGrid() const;
    bool read(const QString& path);   // TODO: remove/reuse PalettePanel code
    void write(const QString& path);   // TODO: remove/reuse PalettePanel code
    void read(XmlReader&);
    void write(XmlWriter&) const;
    void clear();
    void setSelectable(bool val);
    bool selectable() const;
    int getSelectedIdx() const;
    void setSelected(int idx);
    bool readOnly() const;
    void setReadOnly(bool val);
    bool disableElementsApply() const;
    void setDisableElementsApply(bool val);

    bool useDoubleClickToActivate() const;
    void setUseDoubleClickToActivate(bool val);

    bool systemPalette() const { return m_systemPalette; }
    void setSystemPalette(bool val);

    void setMag(qreal val);
    qreal mag() const;
    void setYOffset(qreal val);
    qreal yOffset() const;
    int columns() const;
    int rows() const;
    int size() const;
    PaletteCellPtr cellAt(int index) const;
    void setCellReadOnly(int cellIndex, bool readonly);
    QString name() const;
    void setName(const QString& name);
    int gridWidth() const;
    int gridHeight() const;
    bool moreElements() const;
    void setMoreElements(bool val);
    bool filter(const QString& text);
    void setShowContextMenu(bool val);

    static qreal paletteScaling();
    static void paintPaletteElement(void* data, Element* element);

    int gridWidthM() const;
    int gridHeightM() const;

    int getCurrentIdx() const;
    void setCurrentIdx(int i);
    bool isFilterActive();
    QList<PaletteCellPtr> getDragCells();
    int heightForWidth(int) const override;
    QSize sizeHint() const override;
    int idx(const QPoint&) const;

private:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;
    bool event(QEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void dragEnterEvent(QDragEnterEvent*) override;
    void dragMoveEvent(QDragMoveEvent*) override;
    void dropEvent(QDropEvent*) override;
    void contextMenuEvent(QContextMenuEvent*) override;

    void showWritingFailedError(const QString& path) const;

    int idx2(const QPoint&) const;
    QRect idxRect(int) const;

    const QList<PaletteCellPtr>& ccp() const;
    QPixmap pixmap(int cellIdx) const;

    void applyElementAtPosition(QPoint pos, Qt::KeyboardModifiers modifiers);

    QString m_name;
    QList<PaletteCellPtr> m_cells;
    QList<PaletteCellPtr> m_dragCells; // used for filter & backup

    int m_hgrid = -1;
    int m_vgrid = -1;
    int m_currentIdx = -1;
    int m_pressedIndex = -1;
    int m_dragIdx = -1;
    int m_selectedIdx = -1;
    QPoint m_dragStartPosition;

    qreal m_extraMag = 0;
    bool m_drawGrid = false;
    bool m_selectable = false;
    bool m_disableElementsApply = false;
    bool m_useDoubleClickToActivate = false;
    bool m_readOnly = false;
    bool m_systemPalette = false;
    qreal m_yOffsetSpatium = 0;
    bool m_filterActive = false; // bool if filter is active

    bool m_moreElements = false;
    bool m_showContextMenu = true;
};
} // namespace Ms

#endif
