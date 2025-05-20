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

#include "palettewidget.h"

#include <cmath>

#include <QAccessible>
#include <QAccessibleEvent>
#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDrag>
#include <QFileInfo>
#include <QMenu>
#include <QMimeData>
#include <QResizeEvent>
#include <QToolTip>

#include "translation.h"
#include "types/bytearray.h"

#include "actions/actiontypes.h"
#include "commonscene/commonscenetypes.h"

#include "draw/types/color.h"
#include "draw/types/pen.h"

#include "engraving/rw/rwregister.h"

#include "engraving/dom/actionicon.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/image.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/symbol.h"
#include "engraving/dom/factory.h"
#include "engraving/style/defaultstyle.h"
#include "engraving/style/style.h"

#include "notation/utilities/engravingitempreviewpainter.h"

#include "internal/palettecelliconengine.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace muse::io;
using namespace mu::palette;
using namespace mu::engraving;
using namespace muse::draw;
using namespace muse::actions;

PaletteWidget::PaletteWidget(QWidget* parent)
    : QWidget(parent)
{
    m_palette = std::make_shared<Palette>();

    //! NOTE: need for accessibility
    m_palette->setParent(this);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);

    setReadOnly(false);
    setMouseTracking(true);
}

void PaletteWidget::setPalette(PalettePtr palette)
{
    if (!palette) {
        return;
    }

    m_palette = palette;
    setObjectName("PaletteWidget_" + name());

    //! NOTE: need for accessibility
    m_palette->setParent(this);

    update();
}

QAccessibleInterface* PaletteWidget::accessibleInterface(QObject* object)
{
    PaletteWidget* widget = qobject_cast<PaletteWidget*>(object);
    IF_ASSERT_FAILED(widget) {
        return nullptr;
    }

    return static_cast<QAccessibleInterface*>(new AccessiblePaletteWidget(widget));
}

QString PaletteWidget::name() const
{
    return m_palette->name();
}

void PaletteWidget::setName(const QString& name)
{
    m_palette->setName(name);
}

// ====================================================
// Cells & Elements
// ====================================================

const std::vector<PaletteCellPtr>& PaletteWidget::cells() const
{
    return m_palette->cells();
}

const std::vector<PaletteCellPtr>& PaletteWidget::actualCellsList() const
{
    return m_isFilterActive ? m_filteredCells : cells();
}

int PaletteWidget::actualCellCount() const
{
    return static_cast<int>(m_isFilterActive ? m_filteredCells.size() : cells().size());
}

PaletteCellPtr PaletteWidget::cellAt(size_t index) const
{
    if (index >= actualCellsList().size()) {
        return nullptr;
    }

    return actualCellsList().at(index);
}

ElementPtr PaletteWidget::elementForCellAt(int idx) const
{
    PaletteCellPtr cell = cellAt(idx);
    return cell ? cell->element : nullptr;
}

PaletteCellPtr PaletteWidget::insertElement(int idx, ElementPtr element, const QString& name, qreal mag, const QPointF offset,
                                            const QString& tag)
{
    PaletteCellPtr cell = m_palette->insertElement(idx, element, name, mag, offset, tag);

    update();
    updateGeometry();

    return cell;
}

PaletteCellPtr PaletteWidget::appendElement(ElementPtr element, const QString& name, qreal mag, const QPointF offset, const QString& tag)
{
    PaletteCellPtr cell = m_palette->appendElement(element, name, mag, offset, tag);

    setFixedHeight(heightForWidth(width()));
    updateGeometry();
    update();

    return cell;
}

PaletteCellPtr PaletteWidget::appendActionIcon(mu::engraving::ActionIconType type, ActionCode code)
{
    PaletteCellPtr cell = m_palette->appendActionIcon(type, code);

    setFixedHeight(heightForWidth(width()));
    updateGeometry();
    update();

    return cell;
}

void PaletteWidget::clear()
{
    m_palette->clear();
}

// ====================================================
// Drawing
// ====================================================

qreal PaletteWidget::mag() const
{
    return m_palette->mag();
}

void PaletteWidget::setMag(qreal val)
{
    m_palette->setMag(val);
}

qreal PaletteWidget::yOffset() const
{
    return m_palette->yOffset();
}

void PaletteWidget::setYOffset(qreal val)
{
    m_palette->setYOffset(val);
}

// ====================================================
// Grid
// ====================================================

bool PaletteWidget::drawGrid() const
{
    return m_palette->drawGrid();
}

void PaletteWidget::setDrawGrid(bool val)
{
    m_palette->setDrawGrid(val);
}

int PaletteWidget::gridWidth() const
{
    return m_palette->gridSize().width();
}

int PaletteWidget::gridHeight() const
{
    return m_palette->gridSize().height();
}

void PaletteWidget::setGridSize(int width, int height)
{
    m_palette->setGridSize(width, height);
    QSize s = QSize(width, height) * paletteScaling();
    setSizeIncrement(s);
    setBaseSize(s);
    setMinimumSize(s);
    updateGeometry();
}

int PaletteWidget::gridWidthScaled() const
{
    return gridWidth() * paletteScaling();
}

int PaletteWidget::gridHeightScaled() const
{
    return gridHeight() * paletteScaling();
}

qreal PaletteWidget::paletteScaling() const
{
    return configuration()->paletteScaling();
}

// ====================================================
// Filter
// ====================================================

bool PaletteWidget::isFilterActive()
{
    return m_isFilterActive;
}

/// Returns `true` if all cells are filtered.
bool PaletteWidget::setFilterText(const QString& text)
{
    m_isFilterActive = false;
    setMouseTracking(true);
    QString t = text.toLower();
    bool res = true;
    m_filteredCells.clear();
    // if palette name is searched for, display all elements in the palette
    if (name().startsWith(t, Qt::CaseInsensitive)) {
        const PaletteCellPtr& c = cells().front();
        for (const PaletteCellPtr& cell : cells()) {
            m_filteredCells.push_back(cell);
        }

        const bool contains = t.isEmpty() || c;
        if (!contains) {
            m_isFilterActive = true;
        }
        if (contains && res) {
            res = false;
        }
    }

    for (const PaletteCellPtr& cell : cells()) {
        const QStringList h = cell->name.toLower().split(" ");
        const QStringList n = t.split(" ");
        bool c = false;
        for (const QString& hs : h) {
            for (const QString& ns : n) {
                if (!ns.trimmed().isEmpty()) {
                    c = hs.trimmed().startsWith(ns.trimmed());
                }
            }
            if (c) {
                break;
            }
        }
        if (t.isEmpty() || c) {
            m_filteredCells.push_back(cell);
        }
        const bool contains = t.isEmpty() || c;
        if (!contains) {
            m_isFilterActive = true;
        }
        if (contains && res) {
            res = false;
        }
    }
    setFixedHeight(heightForWidth(width()));
    updateGeometry();
    return res;
}

// ====================================================
// Selection
// ====================================================

bool PaletteWidget::isSelectable() const
{
    return m_selectable;
}

void PaletteWidget::setSelectable(bool val)
{
    m_selectable = val;
}

int PaletteWidget::selectedIdx() const
{
    return m_selectedIdx;
}

void PaletteWidget::setSelected(int idx)
{
    if (m_selectedIdx == idx) {
        return;
    }

    int previous = m_selectedIdx;
    m_selectedIdx = idx;
    emit selectedChanged(idx, previous);
}

int PaletteWidget::currentIdx() const
{
    return m_currentIdx;
}

void PaletteWidget::setCurrentIdx(int idx)
{
    m_currentIdx = idx;
}

void PaletteWidget::nextPaletteElement()
{
    int i = m_currentIdx;
    if (i == -1) {
        return;
    }
    i++;
    if (i < actualCellCount() && cellAt(i)) {
        m_currentIdx = i;
    }

    // TODO: screenreader support
    //QString name = qApp->translate("palette", cellAt(i)->name.toUtf8());
    //ScoreAccessibility::makeReadable(name);

    update();
}

void PaletteWidget::prevPaletteElement()
{
    int i = m_currentIdx;
    if (i == -1) {
        return;
    }
    i--;
    if (i >= 0 && cellAt(i)) {
        m_currentIdx = i;
    }

    // TODO: screenreader support
    //QString name = qApp->translate("palette", cellAt(i)->name.toUtf8());
    //ScoreAccessibility::makeReadable(name);

    update();
}

// ====================================================
// Applying elements
// ====================================================

bool PaletteWidget::isApplyingElementsDisabled() const
{
    return m_isApplyingElementsDisabled;
}

void PaletteWidget::setApplyingElementsDisabled(bool val)
{
    m_isApplyingElementsDisabled = val;
}

bool PaletteWidget::useDoubleClickForApplyingElements() const
{
    return m_useDoubleClickForApplyingElements;
}

void PaletteWidget::setUseDoubleClickForApplyingElements(bool val)
{
    m_useDoubleClickForApplyingElements = val;
}

void PaletteWidget::applyCurrentElementToScore()
{
    applyElementAtIndex(m_currentIdx);
}

void PaletteWidget::applyElementAtPosition(const QPointF& pos, Qt::KeyboardModifiers modifiers)
{
    applyElementAtIndex(cellIndexForPoint(pos), modifiers);
}

void PaletteWidget::applyElementAtIndex(int index, Qt::KeyboardModifiers modifiers)
{
    if (m_isApplyingElementsDisabled) {
        return;
    }

    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    const PaletteCellPtr cell = cellAt(index);
    if (!cell || !cell->element) {
        return;
    }

    notation->interaction()->applyPaletteElement(cell->element.get(), modifiers);
}

// ====================================================
// Settings
// ====================================================

bool PaletteWidget::readOnly() const
{
    return m_isReadOnly;
}

void PaletteWidget::setReadOnly(bool val)
{
    m_isReadOnly = val;
    setAcceptDrops(!val);
}

void PaletteWidget::setCellReadOnly(int cellIndex, bool readonly)
{
    cells()[cellIndex]->readOnly = readonly;
}

void PaletteWidget::setShowContextMenu(bool val)
{
    m_showContextMenu = val;
}

// ====================================================
// Helpers
// ====================================================

int PaletteWidget::columns() const
{
    return width() / gridWidthScaled();
}

int PaletteWidget::rows() const
{
    int c = columns();
    if (c == 0) {
        return 0;
    }
    return (actualCellCount() + c - 1) / c;
}

int PaletteWidget::cellIndexForPoint(const QPointF& p) const
{
    int idx = theoreticalCellIndexForPoint(p);
    if (idx < 0 || idx >= actualCellCount()) {
        return -1;
    }
    return idx;
}

int PaletteWidget::theoreticalCellIndexForPoint(const QPointF& p) const
{
    int hgridM = gridWidthScaled();
    int vgridM = gridHeightScaled();
    if (columns() == 0) {
        return -1;
    }
    int rightBorder = width() % hgridM;
    int hhgrid      = hgridM + (rightBorder / columns());

    int col = p.x() / hhgrid;
    int row = p.y() / vgridM;

    int nc = columns();
    if (col > nc) {
        return -1;
    }

    int idx = row * nc + col;
    if (idx < 0 || idx > columns() * rows()) {
        return -1;
    }
    return idx;
}

QRect PaletteWidget::rectForCellAt(int idx) const
{
    int hgridM = gridWidthScaled();
    int vgridM = gridHeightScaled();
    if (idx == -1) {
        return QRect();
    }
    if (columns() == 0) {
        return QRect();
    }

    int rightBorder = width() % hgridM;
    int hhgrid = hgridM + (rightBorder / columns());

    int cc = idx % columns();
    int cr = idx / columns();
    return QRect(cc * hhgrid, cr * vgridM, hhgrid, vgridM);
}

QPixmap PaletteWidget::pixmapForCellAt(int paletteIdx) const
{
    qreal _spatium = gpaletteScore->style().spatium();
    qreal magS     = configuration()->paletteSpatium() * mag() * paletteScaling();
    qreal mag      = magS / _spatium;

    PaletteCellPtr cell = cellAt(paletteIdx);
    if (!cell || !cell->element) {
        return QPixmap();
    }

    qreal cellMag = cell->mag * mag;
    ElementPtr element = cell->element;

    if (element->isActionIcon()) {
        toActionIcon(element.get())->setFontSize(ActionIcon::DEFAULT_FONT_SIZE * cell->mag);
        cellMag = 1.0;
    }

    engravingRender()->layoutItem(element.get());

    RectF r = element->ldata()->bbox();
    int w = lrint(r.width() * cellMag);
    int h = lrint(r.height() * cellMag);

    if (w * h == 0) {
        LOGD("zero pixmap %d %d %s", w, h, element->typeName());
        return QPixmap();
    }

    QPixmap pm(w, h);
    pm.fill(configuration()->elementsBackgroundColor());

    muse::draw::Painter painter(&pm, "palette");
    painter.setAntialiasing(true);

    painter.scale(cellMag, cellMag);

    painter.translate(-r.topLeft());
    PointF pos = element->ldata()->pos();
    element->setPos(0, 0);

    QColor color;
    // show voice colors for notes
    if (element->isChord()) {
        const Chord* chord = toChord(element.get());
        for (Note* note : chord->notes()) {
            note->setSelected(true);
        }
        color = element->curColor().toQColor();
    } else {
        color = palette().color(QPalette::Normal, QPalette::Text);
    }

    painter.setPen(Pen(color));

    notation::EngravingItemPreviewPainter::PaintParams params;
    params.painter = &painter;
    params.color = configuration()->elementsColor();

    notation::EngravingItemPreviewPainter::paintItem(element.get(), params);

    element->setPos(pos);
    return pm;
}

// ====================================================
// Sizing
// ====================================================

int PaletteWidget::heightForWidth(int w) const
{
    int hgridM = gridWidthScaled();
    int vgridM = gridHeightScaled();
    int c = w / hgridM;
    if (c <= 0) {
        c = 1;
    }
    int rows = (actualCellCount() + c - 1) / c;
    if (rows <= 0) {
        rows = 1;
    }
    qreal magS = configuration()->paletteSpatium() * mag() * paletteScaling();
    int h = lrint(yOffset() * 2 * magS);
    return rows * vgridM + h;
}

QSize PaletteWidget::sizeHint() const
{
    int h = heightForWidth(width());
    int hgridM = gridWidthScaled();
    return QSize((width() / hgridM) * hgridM, h);
}

// ====================================================
// Event handling
// ====================================================

bool PaletteWidget::event(QEvent* ev)
{
    if (!m_palette) {
        return false;
    }

    int hgridM = gridWidthScaled();
    int vgridM = gridHeightScaled();
    // disable mouse hover when keyboard navigation is enabled
    if (m_isFilterActive && (ev->type() == QEvent::MouseMove || ev->type() == QEvent::ToolTip
                             || ev->type() == QEvent::WindowDeactivate)) {
        return true;
    } else if (columns() && (ev->type() == QEvent::ToolTip)) {
        int rightBorder = width() % hgridM;
        int hhgrid = hgridM + (rightBorder / columns());
        QHelpEvent* he = (QHelpEvent*)ev;
        int x = he->pos().x();
        int y = he->pos().y();

        int row = y / vgridM;
        int col = x / hhgrid;

        if (row < 0 || row >= rows()) {
            return false;
        }
        if (col < 0 || col >= columns()) {
            return false;
        }
        int idx = row * columns() + col;
        if (idx >= int(cells().size())) {
            return false;
        }
        if (cellAt(idx) == 0) {
            return false;
        }
        QToolTip::showText(he->globalPos(), cellAt(idx)->translatedName(), this);
        return false;
    }
    return QWidget::event(ev);
}

void PaletteWidget::mousePressEvent(QMouseEvent* ev)
{
    m_dragStartPosition = ev->pos();
    m_dragIdx           = cellIndexForPoint(m_dragStartPosition);

    m_pressedIndex = m_dragIdx;

    if (m_dragIdx == -1) {
        return;
    }

    if (m_selectable) {
        if (m_dragIdx != m_selectedIdx) {
            update(rectForCellAt(m_dragIdx) | rectForCellAt(m_selectedIdx));
        }

        emit boxClicked(m_dragIdx);
        m_selectedIdx = m_dragIdx;
    }

    update();
}

void PaletteWidget::mouseDoubleClickEvent(QMouseEvent* ev)
{
    if (m_useDoubleClickForApplyingElements) {
        applyElementAtPosition(ev->pos(), ev->modifiers());
    }
}

void PaletteWidget::mouseMoveEvent(QMouseEvent* ev)
{
    if ((m_currentIdx != -1) && (m_dragIdx == m_currentIdx) && (ev->buttons() & Qt::LeftButton)
        && (ev->pos() - m_dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
        PaletteCellPtr cell = cellAt(m_currentIdx);
        if (cell && cell->element) {
            QDrag* drag         = new QDrag(this);
            QMimeData* mimeData = new QMimeData;
            const ElementPtr el   = cell->element;

            mimeData->setData(mu::commonscene::MIME_SYMBOL_FORMAT, el->mimeData().toQByteArray());
            drag->setMimeData(mimeData);

            drag->setPixmap(pixmapForCellAt(m_currentIdx));

            QPoint hotsp(drag->pixmap().rect().bottomRight());
            drag->setHotSpot(hotsp);

            Qt::DropActions da;
            if (!(m_isReadOnly || m_isFilterActive) && (ev->modifiers() & Qt::ShiftModifier)) {
                m_filteredCells = cells(); // backup
                da = Qt::MoveAction;
            } else {
                da = Qt::CopyAction;
            }
            Qt::DropAction a = drag->exec(da);
            if (da == Qt::MoveAction && a != da) {
                // restore on a failed move action
                m_palette->clear();
                m_palette->insertCells(0, m_filteredCells);
            }
            update();
        }
    } else {
        m_currentIdx = cellIndexForPoint(ev->pos());
        if (m_currentIdx != -1 && cellAt(m_currentIdx) == 0) {
            m_currentIdx = -1;
        }
        update();
    }
}

void PaletteWidget::mouseReleaseEvent(QMouseEvent* event)
{
    m_pressedIndex = -1;

    update();

    if (!m_useDoubleClickForApplyingElements) {
        applyElementAtPosition(event->position(), event->modifiers());
    }
}

void PaletteWidget::leaveEvent(QEvent*)
{
    if (m_currentIdx != -1) {
        QRect r = rectForCellAt(m_currentIdx);
        if (!(QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)) {
            m_currentIdx = -1;
        }
        update(r);
    }
}

void PaletteWidget::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* dta = event->mimeData();
    if (dta->hasUrls()) {
        QList<QUrl> ul = event->mimeData()->urls();
        QUrl u = ul.front();

        if (u.scheme() == "file") {
            QFileInfo fi(u.path());
            QString suffix(fi.suffix().toLower());
            if (suffix == "svg"
                || suffix == "svgz"
                || suffix == "jpg"
                || suffix == "jpeg"
                || suffix == "png"
                || suffix == "bmp"
                || suffix == "tif"
                || suffix == "tiff"
                ) {
                event->acceptProposedAction();
            }
        }
    } else if (dta->hasFormat(mu::commonscene::MIME_SYMBOL_FORMAT)) {
        event->accept();
        update();
    } else {
        event->ignore();
#ifndef NDEBUG
        LOGD("dragEnterEvent: formats:");
        for (const QString& s : event->mimeData()->formats()) {
            LOGD("   %s", qPrintable(s));
        }
#endif
    }
}

void PaletteWidget::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->source() == this) {
        if (m_currentIdx != -1 && event->proposedAction() == Qt::MoveAction) {
            int targetIdx = cellIndexForPoint(event->position());
            if (targetIdx != -1 && targetIdx != m_currentIdx) {
                PaletteCellPtr cell = m_palette->takeCell(m_currentIdx);
                m_palette->insertCell(targetIdx, cell);
                m_currentIdx = targetIdx;
                update();
            }
            event->accept();
            return;
        }
        event->ignore();
    } else {
        event->accept();
    }
}

void PaletteWidget::dropEvent(QDropEvent* event)
{
    ElementPtr element = nullptr;
    QString name;

    const QMimeData* datap = event->mimeData();
    if (datap->hasUrls()) {
        QList<QUrl> ul = event->mimeData()->urls();
        QUrl u = ul.front();
        if (u.scheme() == "file") {
            auto image = std::make_shared<Image>(gpaletteScore->dummy());
            QString filePath(u.toLocalFile());
            image->load(filePath);
            element = image;
            QFileInfo file(filePath);
            name = file.completeBaseName();
        }
    } else if (datap->hasFormat(mu::commonscene::MIME_SYMBOL_FORMAT)) {
        QByteArray dta(event->mimeData()->data(mu::commonscene::MIME_SYMBOL_FORMAT));
        muse::ByteArray ba = muse::ByteArray::fromQByteArrayNoCopy(dta);
        XmlReader xml(ba);
        PointF dragOffset;
        Fraction duration;
        ElementType type = EngravingItem::readType(xml, &dragOffset, &duration);

        if (type == ElementType::SYMBOL) {
            auto symbol = std::make_shared<Symbol>(gpaletteScore->dummy());
            rw::RWRegister::reader()->readItem(symbol.get(), xml);
            element = symbol;
        } else {
            element = std::shared_ptr<EngravingItem>(Factory::createItem(type, gpaletteScore->dummy()));
            if (element) {
                rw::RWRegister::reader()->readItem(element.get(), xml);
                element->setTrack(0);

                if (element->isActionIcon()) {
                    ActionIcon* icon = toActionIcon(element.get());
                    const muse::ui::UiAction& actionItem = actionsRegister()->action(icon->actionCode());
                    if (actionItem.isValid()) {
                        icon->setAction(icon->actionCode(), static_cast<char16_t>(actionItem.iconCode));
                    }
                }
            }
        }
    }

    if (!element) {
        event->ignore();
        return;
    }

    if (event->source() == this) {
        if (event->proposedAction() == Qt::MoveAction) {
            event->accept();
            emit changed();
            return;
        }
        event->ignore();
        return;
    }

    if (element->isFretDiagram()) {
        name = toFretDiagram(element.get())->harmonyText();
    }

    element->setSelected(false);

    int i = cellIndexForPoint(event->position());
    if (i == -1 || cells()[i]) {
        appendElement(element, name);
    } else {
        insertElement(i, element, name);
    }
    event->accept();

    setFixedHeight(heightForWidth(width()));
    updateGeometry();
    update();
    emit changed();
}

void PaletteWidget::resizeEvent(QResizeEvent* e)
{
    setFixedHeight(heightForWidth(e->size().width()));
}

void PaletteWidget::paintEvent(QPaintEvent* /*event*/)
{
    qreal _spatium = gpaletteScore->style().spatium();
    qreal magS     = configuration()->paletteSpatium() * mag() * paletteScaling();
    qreal mag      = magS / _spatium;
    gpaletteScore->style().setSpatium(SPATIUM20);

    muse::draw::Painter painter(this, "palette");
    painter.setAntialiasing(true);

    if (m_paintOptions.backgroundColor.isValid()) {
        painter.setBrush(m_paintOptions.backgroundColor);
    }

    painter.setPen(configuration()->gridColor());
    painter.drawRoundedRect(RectF(0, 0, width(), height()), 2, 2);

    //
    // draw grid
    //
    int hgridM = gridWidthScaled();
    int vgridM = gridHeightScaled();

    if (columns() == 0) {
        return;
    }
    int rightBorder = width() % hgridM;
    int hhgrid = hgridM + (rightBorder / columns());

    if (drawGrid()) {
        for (int row = 1; row < rows(); ++row) {
            int x2 = row < rows() - 1 ? columns() * hhgrid : width();
            int y  = row * vgridM;
            painter.drawLine(0, y, x2, y);
        }
        for (int column = 1; column < columns(); ++column) {
            int x = hhgrid * column;
            painter.drawLine(x, 0, x, rows() * vgridM);
        }
    }

    qreal dy = lrint(2 * magS);

    Color linesColor = m_paintOptions.linesColor.isValid() ? m_paintOptions.linesColor
                       : configuration()->elementsColor();

    Color selectionColor = m_paintOptions.selectionColor.isValid() ? m_paintOptions.selectionColor
                           : configuration()->accentColor();

    //
    // draw symbols
    //
    for (int idx = 0; idx < int(actualCellsList().size()); ++idx) {
        int yoffset = _spatium * yOffset();
        RectF r = RectF::fromQRectF(rectForCellAt(idx));
        RectF rShift = r.translated(0, yoffset);
        QColor c = selectionColor.toQColor();

        PaletteCellPtr currentCell = actualCellsList().at(idx);
        if (!currentCell) {
            continue;
        }

        if (currentCell->focused) {
            painter.setPen(QColor(uiConfiguration()->currentTheme().values[muse::ui::FONT_PRIMARY_COLOR].toString()));
            painter.setBrush(QColor(Qt::transparent));

            int borderWidth = uiConfiguration()->currentTheme().values[muse::ui::NAVIGATION_CONTROL_BORDER_WIDTH].toInt();
            qreal border = borderWidth / 2;

            painter.drawRoundedRect(r.adjusted(border, border, -border, -border), borderWidth, borderWidth);
            r.adjust(borderWidth, borderWidth, -borderWidth, -borderWidth);
        }

        if (idx == m_selectedIdx) {
            c.setAlphaF(0.5f);
            painter.fillRect(r, c);
        } else if (idx == m_pressedIndex) {
            c.setAlphaF(0.75f);
            painter.fillRect(r, c);
        } else if (idx == m_currentIdx) {
            c.setAlphaF(0.2f);
            painter.fillRect(r, c);
        }

        QString tag = currentCell->tag;
        if (!tag.isEmpty()) {
            painter.setPen(QColor(Qt::darkGray));
            Font font(painter.font());
            font.setPixelSize(uiConfiguration()->fontSize(muse::ui::FontSizeType::BODY));
            painter.setFont(font);
            painter.drawText(rShift, Qt::AlignLeft | Qt::AlignTop, tag);
        }

        muse::draw::Pen pen(linesColor);
        pen.setWidthF(engraving::DefaultStyle::defaultStyle().styleS(Sid::staffLineWidth).val() * magS);
        painter.setPen(pen);

        ElementPtr el = currentCell->element;
        if (!el) {
            continue;
        }

        bool drawStaff = currentCell->drawStaff;
        int row    = idx / columns();
        int column = idx % columns();

        qreal cellMag = currentCell->mag * mag;
        if (el->isActionIcon()) {
            toActionIcon(el.get())->setFontSize(ActionIcon::DEFAULT_FONT_SIZE * currentCell->mag);
            cellMag = 1.0;
        }

        engravingRender()->layoutItem(el.get());

        if (drawStaff) {
            qreal y = r.y() + vgridM * .5 - dy + yOffset() * _spatium * cellMag;
            qreal x = r.x() + 3;
            qreal w = hhgrid - 6;
            for (int i = 0; i < 5; ++i) {
                qreal yy = y + i * magS;
                painter.setPen(linesColor);
                painter.drawLine(LineF(x, yy, x + w, yy));
            }
        }
        painter.save();
        painter.scale(cellMag, cellMag);

        double gw = hhgrid / cellMag;
        double gh = vgridM / cellMag;
        double gx = column * gw + currentCell->xoffset * _spatium;
        double gy = row * gh + currentCell->yoffset * _spatium;

        double sw = el->width();
        double sh = el->height();
        double sy = 0;

        if (drawStaff) {
            sy = gy + gh * .5 - 2.0 * _spatium;
        } else {
            sy  = gy + (gh - sh) * .5 - el->ldata()->bbox().y();
        }
        double sx  = gx + (gw - sw) * .5 - el->ldata()->bbox().x();

        sy += yOffset() * _spatium;

        painter.translate(sx, sy);

        QColor color;
        if (idx != m_selectedIdx) {
            // show voice colors for notes
            if (el->isChord()) {
                color = el->curColor().toQColor();
            } else {
                color = palette().color(QPalette::Normal, QPalette::Text);
            }
        } else {
            color = palette().color(QPalette::Normal, QPalette::HighlightedText);
        }

        painter.setPen(Pen(color));

        notation::EngravingItemPreviewPainter::PaintParams params;
        params.painter = &painter;
        params.color = configuration()->elementsColor();

        params.useElementColors = m_paintOptions.useElementColors;
        params.colorsInversionEnabled = m_paintOptions.colorsInverionsEnabled;

        notation::EngravingItemPreviewPainter::paintItem(el.get(), params);

        painter.restore();
    }
}

void PaletteWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (!m_showContextMenu) {
        return;
    }
    int i = cellIndexForPoint(event->pos());
    if (i == -1) {
        // palette context menu
        return;
    }

    QMenu menu;
    QAction* deleteCellAction = menu.addAction(muse::qtrc("palette", "Delete"));
    QAction* contextAction = menu.addAction(muse::qtrc("palette", "Properties…"));
    deleteCellAction->setEnabled(!m_isReadOnly);
    contextAction->setEnabled(!m_isReadOnly);

    if (m_isFilterActive || (cellAt(i) && cellAt(i)->readOnly)) {
        deleteCellAction->setEnabled(false);
    }

    if (!deleteCellAction->isEnabled() && !contextAction->isEnabled()) {
        return;
    }

    const QAction* action = menu.exec(mapToGlobal(event->pos()));

    if (action == deleteCellAction) {
        PaletteCellPtr cell = cellAt(i);
        if (cell) {
            std::string title = muse::trc("palette", "Delete palette cell");
            std::string question = muse::qtrc("palette", "Are you sure you want to delete palette cell “%1”?")
                                   .arg(cell->name).toStdString();

            auto promise = interactive()->questionAsync(title, question, {
                muse::IInteractive::Button::Yes,
                muse::IInteractive::Button::No
            }, muse::IInteractive::Button::Yes);

            promise.onResolve(this, [this, i](const IInteractive::Result& res) {
                if (res.isButton(IInteractive::Button::Yes)) {
                    m_palette->takeCell(i);
                    emit changed();
                }
            });
        }
    } else {
        bool sizeChanged = false;
        for (size_t j = 0; j < cells().size(); ++j) {
            if (!cellAt(j)) {
                m_palette->takeCells(i, 1);
                sizeChanged = true;
            }
        }
        if (sizeChanged) {
            setFixedHeight(heightForWidth(width()));
            updateGeometry();
        }
        update();
    }
}

// ====================================================
// Read/write
// ====================================================

void PaletteWidget::read(XmlReader& e, bool pasteMode)
{
    m_palette->read(e, pasteMode);
}

void PaletteWidget::write(XmlWriter& xml, bool pasteMode) const
{
    m_palette->write(xml, pasteMode);
}

bool PaletteWidget::readFromFile(const QString& path)
{
    return m_palette->readFromFile(path);
}

/// Write as compressed zip file and include images as needed
void PaletteWidget::writeToFile(const QString& path) const
{
    m_palette->writeToFile(path);
}

bool PaletteWidget::handleEvent(QEvent* event)
{
    return QWidget::event(event);
}

const PaletteWidget::PaintOptions& PaletteWidget::paintOptions() const
{
    return m_paintOptions;
}

void PaletteWidget::setPaintOptions(const PaintOptions& options)
{
    m_paintOptions = options;
}

// ====================================================
// PaletteScrollArea
// ====================================================

PaletteScrollArea::PaletteScrollArea(PaletteWidget* w, QWidget* parent)
    : QScrollArea(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidget(w);
    setWidgetResizable(false);
    m_restrictHeight = true;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

bool PaletteScrollArea::restrictHeight() const
{
    return m_restrictHeight;
}

void PaletteScrollArea::setRestrictHeight(bool val)
{
    m_restrictHeight = val;
}

void PaletteScrollArea::keyPressEvent(QKeyEvent* event)
{
    PaletteWidget* p = qobject_cast<PaletteWidget*>(widget());
    int pressedKey = event->key();
    switch (pressedKey) {
    case Qt::Key_Right:
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_Down:
    {
        int idx = p->selectedIdx();
        if (pressedKey == Qt::Key_Left || pressedKey == Qt::Key_Up) {
            idx--;
        } else {
            idx++;
        }
        if (idx < 0) {
            idx = p->actualCellCount() - 1;
        } else if (idx >= p->actualCellCount()) {
            idx = 0;
        }

        p->setSelected(idx);
        p->setCurrentIdx(idx);

        p->update();
        return;
    }
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (!p->isApplyingElementsDisabled()) {
            p->applyCurrentElementToScore();
        }
        return;
    default:
        break;
    }
    QScrollArea::keyPressEvent(event);
}

void PaletteScrollArea::resizeEvent(QResizeEvent* re)
{
    QScrollArea::resizeEvent(re);

    PaletteWidget* palette = qobject_cast<PaletteWidget*>(widget());
    int h = palette->heightForWidth(width());
    palette->resize(QSize(width() - 6, h));
    if (m_restrictHeight) {
        setMaximumHeight(h + 6);
    }
}

AccessiblePaletteWidget::AccessiblePaletteWidget(PaletteWidget* palette)
    : QAccessibleWidget(palette)
{
    m_palette = palette;

    connect(m_palette, &PaletteWidget::selectedChanged, this, [this](int index, int previous) {
        PaletteCellPtr curCell = m_palette->cellAt(index);
        PaletteCellPtr previousCell = m_palette->cellAt(previous);

        if (previousCell) {
            previousCell->focused = false;

            QAccessible::State qstate;
            qstate.active = false;
            qstate.focused = false;
            qstate.selected = false;

            QAccessibleEvent ev(previousCell.get(), QAccessible::Focus);
            QAccessible::updateAccessibility(&ev);
        }

        if (curCell) {
            curCell->focused = true;

            QAccessible::State qstate;
            qstate.active = true;
            qstate.focused = true;
            qstate.selected = true;

            QAccessibleEvent ev(curCell.get(), QAccessible::Focus);
            QAccessible::updateAccessibility(&ev);
        }
    });
}

QObject* AccessiblePaletteWidget::object() const
{
    return m_palette;
}

QAccessibleInterface* AccessiblePaletteWidget::child(int index) const
{
    PaletteCellPtr cell = m_palette->cellAt(index);
    if (!cell) {
        return nullptr;
    }

    return QAccessible::queryAccessibleInterface(cell.get());
}

int AccessiblePaletteWidget::childCount() const
{
    return m_palette->actualCellCount();
}

int AccessiblePaletteWidget::indexOfChild(const QAccessibleInterface* child) const
{
    for (int i = 0; i < childCount(); ++i) {
        auto childAccessible = QAccessible::queryAccessibleInterface(m_palette->cellAt(i).get());
        if (childAccessible == child) {
            return i;
        }
    }

    return -1;
}

QAccessible::Role AccessiblePaletteWidget::role() const
{
    return QAccessible::StaticText;
}

QAccessible::State AccessiblePaletteWidget::state() const
{
    QAccessible::State state = QAccessibleWidget::state();
    state.selectable = true;
    state.active = true;
    state.focusable = true;
    state.focused = m_palette->hasFocus();

    return state;
}

QAccessibleInterface* AccessiblePaletteWidget::focusChild() const
{
    int selectIndex = m_palette->selectedIdx();
    return QAccessible::queryAccessibleInterface(m_palette->cellAt(selectIndex).get());
}
