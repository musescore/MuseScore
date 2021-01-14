//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "palette.h"

#include <cmath>
#include <QMenu>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QAction>
#include <QDrag>
#include <QMimeData>
#include <QAccessible>
#include <QAccessibleEvent>
#include <QToolTip>
#include <QBuffer>

#include "libmscore/element.h"
#include "libmscore/style.h"
#include "libmscore/sym.h"
#include "libmscore/symbol.h"
#include "libmscore/score.h"
#include "libmscore/image.h"
#include "libmscore/xml.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/clef.h"
#include "libmscore/segment.h"
#include "libmscore/measure.h"
#include "libmscore/staff.h"
#include "libmscore/system.h"
#include "libmscore/page.h"
#include "libmscore/keysig.h"
#include "libmscore/timesig.h"
#include "libmscore/part.h"
#include "libmscore/textline.h"
#include "libmscore/measure.h"
#include "libmscore/icon.h"
#include "libmscore/mscore.h"
#include "libmscore/imageStore.h"
#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"
#include "libmscore/slur.h"
#include "libmscore/fret.h"

#include "translation.h"

#include "widgetstatestore.h"
#include "commonscene/commonscenetypes.h"

#include "../palette_config.h"

using namespace mu::framework;
using namespace mu::palette;

namespace Ms {
//---------------------------------------------------------
//   paletteNeedsStaff
//    should a staff been drawn if e is used as icon in
//    a palette
//---------------------------------------------------------

static bool paletteNeedsStaff(Element* e)
{
    if (e == 0) {
        return false;
    }
    switch (e->type()) {
    case ElementType::CHORD:
    case ElementType::BAR_LINE:
    case ElementType::CLEF:
    case ElementType::KEYSIG:
    case ElementType::TIMESIG:
    case ElementType::REST:
    case ElementType::BAGPIPE_EMBELLISHMENT:
        return true;
    default:
        return false;
    }
}

//---------------------------------------------------------
//   Palette
//---------------------------------------------------------

Palette::Palette(QWidget* parent)
    : QWidget(parent)
{
    extraMag      = 1.0;
    currentIdx    = -1;
    dragIdx       = -1;
    selectedIdx   = -1;
    _yOffset      = 0.0;
    setGrid(50, 60);
    _drawGrid     = false;
    _selectable   = false;
    setMouseTracking(true);
    setReadOnly(false);
    setSystemPalette(false);
    _moreElements = false;
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Ignored);
    setObjectName("palette-cells");
}

Palette::Palette(PalettePanelPtr pp, QWidget* parent)
    : Palette(parent)
{
    setName(pp->name());
    const QSize gridSize = pp->gridSize();
    setGrid(gridSize.width(), gridSize.height());
    setMag(pp->mag());
    setDrawGrid(pp->drawGrid());
    setMoreElements(pp->moreElements());

    const auto allCells = pp->takeCells(0, pp->ncells());
    for (const PaletteCellPtr& cell : allCells) {
        Element* e = (cell.use_count() == 1) ? cell->element.get() : (cell->element ? cell->element->clone() : nullptr);
        if (e) {
            PaletteCell* newCell = append(e, cell->name, cell->tag, cell->mag);
            newCell->drawStaff = cell->drawStaff;
            newCell->xoffset = cell->xoffset;
            newCell->yoffset = cell->yoffset;
            newCell->readOnly = cell->readOnly;
        }
    }

    if (moreElements()) {
        connect(this, &Palette::displayMore, [](const QString& arg) {
            adapter()->showMasterPalette(arg);
        });
    }
}

Palette::~Palette()
{
    for (PaletteCell* cell : cells) {
        delete cell;
    }
}

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void Palette::resizeEvent(QResizeEvent* e)
{
    setFixedHeight(heightForWidth(e->size().width()));
}

//---------------------------------------------------------
//   filter
///  return true if all filtered
//---------------------------------------------------------

bool Palette::filter(const QString& text)
{
    filterActive = false;
    setMouseTracking(true);
    QString t = text.toLower();
    bool res = true;
    dragCells.clear();
    // if palette name is searched for, display all elements in the palette
    if (_name.startsWith(t, Qt::CaseInsensitive)) {
        PaletteCell* c  = cells.first();
        for (PaletteCell* cell : cells) {
            dragCells.append(cell);
        }

        bool contains = t.isEmpty() || c;
        if (!contains) {
            filterActive = true;
        }
        if (contains && res) {
            res = false;
        }
    }

    for (PaletteCell* cell : cells) {
        QStringList h = cell->name.toLower().split(" ");
        bool c        = false;
        QStringList n = t.split(" ");
        for (QString hs : h) {
            for (QString ns : n) {
                if (!ns.trimmed().isEmpty()) {
                    c = hs.trimmed().startsWith(ns.trimmed());
                }
            }
            if (c) {
                break;
            }
        }
        if (t.isEmpty() || c) {
            dragCells.append(cell);
        }
        bool contains = t.isEmpty() || c;
        if (!contains) {
            filterActive = true;
        }
        if (contains && res) {
            res = false;
        }
    }
    setFixedHeight(heightForWidth(width()));
    updateGeometry();
    return res;
}

//---------------------------------------------------------
//   setMoreElements
//---------------------------------------------------------

void Palette::setMoreElements(bool val)
{
    _moreElements = val;
    if (val && (cells.isEmpty() || cells.back()->tag != "ShowMore")) {
        PaletteCell* cell = new PaletteCell;
        cell->name = mu::qtrc("palette", "Show More");
        cell->tag = "ShowMore";
        cells.append(cell);
    } else if (!val && !cells.isEmpty() && (cells.last()->tag == "ShowMore")) {
        PaletteCell* cell = cells.takeLast();
        delete cell;
    }
}

//---------------------------------------------------------
//   setSystemPalette
//---------------------------------------------------------

void Palette::setSystemPalette(bool val)
{
    _systemPalette = val;
    if (val) {
        setReadOnly(true);
    }
}

//---------------------------------------------------------
//   setReadOnly
//---------------------------------------------------------

void Palette::setReadOnly(bool val)
{
    _readOnly = val;
    setAcceptDrops(!val);
}

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void Palette::setMag(qreal val)
{
    extraMag = val;
}

//---------------------------------------------------------
//   guiMag
//---------------------------------------------------------

qreal Palette::paletteScaling()
{
    return configuration()->paletteScaling();
}

//---------------------------------------------------------
//   contextMenuEvent
//---------------------------------------------------------

void Palette::contextMenuEvent(QContextMenuEvent* event)
{
    if (!_showContextMenu) {
        return;
    }
    int i = idx(event->pos());
    if (i == -1) {
        // palette context menu
        if (!_moreElements) {
            return;
        }
        QMenu menu;
        QAction* moreAction = menu.addAction(mu::qtrc("palette", "More Elements…"));
        moreAction->setEnabled(_moreElements);
        QAction* action = menu.exec(mapToGlobal(event->pos()));
        if (action == moreAction) {
            emit displayMore(_name);
        }
        return;
    }

    QMenu menu;
    QAction* deleteCellAction   = menu.addAction(mu::qtrc("palette", "Delete"));
    QAction* contextAction = menu.addAction(mu::qtrc("palette", "Properties…"));
    deleteCellAction->setEnabled(!_readOnly);
    contextAction->setEnabled(!_readOnly);
    QAction* moreAction    = menu.addAction(mu::qtrc("palette", "More Elements…"));
    moreAction->setEnabled(_moreElements);

    if (filterActive || (cellAt(i) && cellAt(i)->readOnly)) {
        deleteCellAction->setEnabled(false);
    }

    if (!deleteCellAction->isEnabled() && !contextAction->isEnabled() && !moreAction->isEnabled()) {
        return;
    }

    const QAction* action = menu.exec(mapToGlobal(event->pos()));

    if (action == deleteCellAction) {
        PaletteCell* cell = cellAt(i);
        if (cell) {
            std::string title = mu::trc("palette", "Delete palette cell");
            std::string question
                = mu::qtrc("palette", "Are you sure you want to delete palette cell \"%1\"?").arg(cell->name).toStdString();

            IInteractive::Button button = interactive()->question(title, question, {
                    IInteractive::Button::Yes,
                    IInteractive::Button::No
                }, IInteractive::Button::Yes);

            if (button != IInteractive::Button::Yes) {
                return;
            }
            if (cell->tag == "ShowMore") {
                _moreElements = false;
            }
            delete cell;
        }
        cells[i] = nullptr;
        emit changed();
    } else if (action == contextAction) {
        //disable due to the new implementation
        /*PaletteCell* c = cellAt(i);
        if (c == 0)
              return;
        PaletteCellProperties props(c);
        if (props.exec())
              emit changed();*/
    } else if (moreAction && (action == moreAction)) {
        emit displayMore(_name);
    }

    bool sizeChanged = false;
    for (int j = 0; j < cells.size(); ++j) {
        if (!cellAt(j)) {
            cells.removeAt(j);
            sizeChanged = true;
        }
    }
    if (sizeChanged) {
        setFixedHeight(heightForWidth(width()));
        updateGeometry();
    }
    update();
}

//---------------------------------------------------------
//   setGrid
//---------------------------------------------------------

void Palette::setGrid(int hh, int vv)
{
    hgrid = hh;
    vgrid = vv;
    QSize s(hgrid, vgrid);
    s *= paletteScaling();
    setSizeIncrement(s);
    setBaseSize(s);
    setMinimumSize(s);
    updateGeometry();
}

//---------------------------------------------------------
//   element
//---------------------------------------------------------

Element* Palette::element(int idx)
{
    if (idx < size() && cellAt(idx)) {
        return cellAt(idx)->element.get();
    } else {
        return 0;
    }
}

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void Palette::mousePressEvent(QMouseEvent* ev)
{
    dragStartPosition = ev->pos();
    dragIdx           = idx(dragStartPosition);

    pressedIndex = dragIdx;

/*
      // Take out of edit mode to prevent crashes when adding
      // elements from palette

      ScoreView* cv = mscore->currentScoreView();
      if (cv && cv->editMode())
            cv->changeState(ViewState::NORMAL);
*/
    if (dragIdx == -1) {
        return;
    }
    if (_selectable) {
        if (dragIdx != selectedIdx) {
            update(idxRect(dragIdx) | idxRect(selectedIdx));
            selectedIdx = dragIdx;
        }
        emit boxClicked(dragIdx);
    }
    PaletteCell* cell = cellAt(dragIdx);
    if (cell && (cell->tag == "ShowMore")) {
        emit displayMore(_name);
    }

    update();
}

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Palette::mouseMoveEvent(QMouseEvent* ev)
{
    if ((currentIdx != -1) && (dragIdx == currentIdx) && (ev->buttons() & Qt::LeftButton)
        && (ev->pos() - dragStartPosition).manhattanLength() > QApplication::startDragDistance()) {
        PaletteCell* cell = cellAt(currentIdx);
        if (cell && cell->element) {
            QDrag* drag         = new QDrag(this);
            QMimeData* mimeData = new QMimeData;
            const Element* el   = cell->element.get();

            mimeData->setData(mu::commonscene::MIME_SYMBOL_FORMAT, el->mimeData(QPointF()));
            drag->setMimeData(mimeData);

            drag->setPixmap(pixmap(currentIdx));

            QPoint hotsp(drag->pixmap().rect().bottomRight());
            drag->setHotSpot(hotsp);

            Qt::DropActions da;
            if (!(_readOnly || filterActive) && (ev->modifiers() & Qt::ShiftModifier)) {
                dragCells = cells;              // backup
                da = Qt::MoveAction;
            } else {
                da = Qt::CopyAction;
            }
            Qt::DropAction a = drag->exec(da);
            if (da == Qt::MoveAction && a != da) {
                cells = dragCells;              // restore on a failed move action
            }
            update();
        }
    } else {
        currentIdx = idx(ev->pos());
        if (currentIdx != -1 && cellAt(currentIdx) == 0) {
            currentIdx = -1;
        }
        update();
    }
}

//---------------------------------------------------------
//   applyPaletteElement
//---------------------------------------------------------

bool Palette::applyPaletteElement(Element* element, Qt::KeyboardModifiers modifiers)
{
    return adapter()->applyPaletteElement(element, modifiers);
}

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void PaletteScrollArea::keyPressEvent(QKeyEvent* event)
{
    QWidget* w = this->widget();
    Palette* p = static_cast<Palette*>(w);
    int pressedKey = event->key();
    switch (pressedKey) {
    case Qt::Key_Right:
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_Down:
    {
        int idx = p->getSelectedIdx();
        if (pressedKey == Qt::Key_Left || pressedKey == Qt::Key_Up) {
            idx--;
        } else {
            idx++;
        }
        if (idx < 0) {
            idx = p->size() - 1;
        } else if (idx >= p->size()) {
            idx = 0;
        }
        p->setSelected(idx);
        p->setCurrentIdx(idx);
        // set widget name to name of selected element
        // we could set the description, but some screen readers ignore it
        QString name = p->cellAt(idx)->translatedName();
        //ScoreAccessibility::makeReadable(name); //! TODO
        setAccessibleName(name);
        QAccessibleEvent aev(this, QAccessible::NameChanged);
        QAccessible::updateAccessibility(&aev);
        p->update();
        break;
    }
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (!p->disableElementsApply()) {
            p->applyPaletteElement();
        }
        break;
    default:
        break;
    }
    QScrollArea::keyPressEvent(event);
}

//---------------------------------------------------------
//   mouseDoubleClickEvent
//---------------------------------------------------------

void Palette::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (_useDoubleClickToActivate) {
        applyElementAtPosition(event->pos(), event->modifiers());
    }
}

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void Palette::mouseReleaseEvent(QMouseEvent* event)
{
    pressedIndex = -1;

    update();

    if (!_useDoubleClickToActivate) {
        applyElementAtPosition(event->pos(), event->modifiers());
    }
}

void Palette::applyElementAtPosition(QPoint pos, Qt::KeyboardModifiers modifiers)
{
    if (_disableElementsApply) {
        return;
    }

    const int index = idx(pos);

    if (index == -1) {
        return;
    }

    if (!adapter()->isSelected()) {
        return;
    }

    PaletteCell* cell = cellAt(index);

    if (!cell) {
        return;
    }

    applyPaletteElement(cell->element.get(), modifiers);
}

//---------------------------------------------------------
//   idx
//---------------------------------------------------------

int Palette::idx(const QPoint& p) const
{
    int hgridM = gridWidthM();
    int vgridM = gridHeightM();
    if (columns() == 0) {
        return -1;
    }
    int rightBorder = width() % hgridM;
    int hhgrid      = hgridM + (rightBorder / columns());

    int x = p.x();
    int y = p.y();

    int row = y / vgridM;
    int col = x / hhgrid;

    int nc = columns();
    if (col > nc) {
        return -1;
    }

    int idx = row * nc + col;
    if (idx < 0 || idx >= size()) {
        return -1;
    }
    return idx;
}

//---------------------------------------------------------
//   idx2
//   returns indexes outside of cells.size()
//---------------------------------------------------------

int Palette::idx2(const QPoint& p) const
{
    int hgridM = gridWidthM();
    int vgridM = gridHeightM();
    if (columns() == 0) {
        return -1;
    }
    int rightBorder = width() % hgridM;
    int hhgrid      = hgridM + (rightBorder / columns());

    int x = p.x();
    int y = p.y();

    int row = y / vgridM;
    int col = x / hhgrid;

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

//---------------------------------------------------------
//   idxRect
//---------------------------------------------------------

QRect Palette::idxRect(int i) const
{
    int hgridM = gridWidthM();
    int vgridM = gridHeightM();
    if (i == -1) {
        return QRect();
    }
    if (columns() == 0) {
        return QRect();
    }

    int rightBorder = width() % hgridM;
    int hhgrid = hgridM + (rightBorder / columns());

    int cc = i % columns();
    int cr = i / columns();
    return QRect(cc * hhgrid, cr * vgridM, hhgrid, vgridM);
}

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void Palette::leaveEvent(QEvent*)
{
    if (currentIdx != -1) {
        QRect r = idxRect(currentIdx);
        if (!(QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)) {
            currentIdx = -1;
        }
        update(r);
    }
}

//---------------------------------------------------------
//   nextPaletteElement
//---------------------------------------------------------

void Palette::nextPaletteElement()
{
    int i = currentIdx;
    if (i == -1) {
        return;
    }
    i++;
    if (i < size() && cellAt(i)) {
        currentIdx = i;
    }

    // TODO: screenreader support
    //QString name = qApp->translate("Palette", cellAt(i)->name.toUtf8());
    //ScoreAccessibility::makeReadable(name);

    update();
}

//---------------------------------------------------------
//   prevPaletteElement
//---------------------------------------------------------

void Palette::prevPaletteElement()
{
    int i = currentIdx;
    if (i == -1) {
        return;
    }
    i--;
    if (i >= 0 && cellAt(i)) {
        currentIdx = i;
    }

    // TODO: screenreader support
    //QString name = qApp->translate("Palette", cellAt(i)->name.toUtf8());
    //ScoreAccessibility::makeReadable(name);

    update();
}

//---------------------------------------------------------
//   applyPaletteElement
//---------------------------------------------------------

void Palette::applyPaletteElement()
{
    if (!adapter()->isSelected()) {
        return;
    }

    // apply currently selected palette symbol to selected score elements
    int i = currentIdx;
    if (i < size() && cellAt(i)) {
        applyPaletteElement(cellAt(i)->element.get());
    } else {
        return;
    }
}

//---------------------------------------------------------
//   append
//    append element to palette
//---------------------------------------------------------

PaletteCell* Palette::append(Element* s, const QString& name, QString tag, qreal mag)
{
    if (s == 0) {
        cells.append(0);
        return 0;
    }
    PaletteCell* cell = new PaletteCell;
    cell->id = PaletteCell::makeId();

    int idx;
    if (_moreElements) {
        cells.insert(cells.size() - 1, cell);
        idx = cells.size() - 2;
    } else {
        cells.append(cell);
        idx = cells.size() - 1;
    }
    PaletteCell* pc = add(idx, s, name, tag, mag);
    setFixedHeight(heightForWidth(width()));
    updateGeometry();
    return pc;
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

PaletteCell* Palette::add(int idx, Element* s, const QString& name, QString tag, qreal mag)
{
    if (s) {
        s->setPos(0.0, 0.0);
        s->setOffset(QPointF());
    }

    PaletteCell* cell = new PaletteCell;
    cell->id = PaletteCell::makeId();

    if (idx < cells.size()) {
        delete cells[idx];
    } else {
        for (int i = cells.size(); i <= idx; ++i) {
            cells.append(0);
        }
    }
    cells[idx]      = cell;
    cell->element.reset(s);
    cell->name      = name;
    cell->tag       = tag;
    cell->drawStaff = paletteNeedsStaff(s);
    cell->xoffset   = 0;
    cell->yoffset   = 0;
    cell->mag       = mag;
    cell->readOnly  = false;
    update();
    if (s && s->isIcon()) {
        Icon* icon = toIcon(s);
        connect(adapter()->getAction(icon->action()), SIGNAL(toggled(bool)), SLOT(actionToggled(bool)));
    }
    updateGeometry();
    return cell;
}

//---------------------------------------------------------
//   paintPaletteElement
//---------------------------------------------------------

void Palette::paintPaletteElement(void* data, Element* element)
{
    QPainter* painter = static_cast<QPainter*>(data);
    painter->save();
    painter->translate(element->pos()); // necessary for drawing child elements

    QColor color = configuration()->elementsColor();

    QColor colorBackup = element->color();
    element->undoSetColor(color);

    QColor frameColorBackup = element->getProperty(Pid::FRAME_FG_COLOR).value<QColor>();
    element->undoChangeProperty(Pid::FRAME_FG_COLOR, color);

    element->draw(painter);

    element->undoSetColor(colorBackup);
    element->undoChangeProperty(Pid::FRAME_FG_COLOR, frameColorBackup);
    painter->restore();
}

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void Palette::paintEvent(QPaintEvent* /*event*/)
{
    qreal _spatium = gscore->spatium();
    qreal magS     = PALETTE_SPATIUM * extraMag * paletteScaling();
    qreal mag      = magS / _spatium;
    gscore->setSpatium(SPATIUM20);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    p.setPen(configuration()->gridColor());
    p.drawRoundedRect(0, 0, width(), height(), 2, 2);

    //
    // draw grid
    //
    int hgridM = gridWidthM();
    int vgridM = gridHeightM();

    if (columns() == 0) {
        return;
    }
    int rightBorder = width() % hgridM;
    int hhgrid = hgridM + (rightBorder / columns());

    if (_drawGrid) {
        for (int row = 1; row < rows(); ++row) {
            int x2 = row < rows() - 1 ? columns() * hhgrid : width();
            int y  = row * vgridM;
            p.drawLine(0, y, x2, y);
        }
        for (int column = 1; column < columns(); ++column) {
            int x = hhgrid * column;
            p.drawLine(x, 0, x, rows() * vgridM);
        }
    }

    qreal dy = lrint(2 * magS);

    //
    // draw symbols
    //
    QPen pen(configuration()->elementsColor());
    pen.setWidthF(MScore::defaultStyle().value(Sid::staffLineWidth).toDouble() * magS);

    for (int idx = 0; idx < ccp()->size(); ++idx) {
        int yoffset  = gscore->spatium() * _yOffset;
        QRect r      = idxRect(idx);
        QRect rShift = r.translated(0, yoffset);
        p.setPen(pen);
        QColor c(configuration()->accentColor());

        if (idx == selectedIdx) {
            c.setAlphaF(0.5);
            p.fillRect(r, c);
        } else if (idx == pressedIndex) {
            c.setAlphaF(0.75);
            p.fillRect(r, c);
        } else if (idx == currentIdx) {
            c.setAlphaF(0.2);
            p.fillRect(r, c);
        }

        if (ccp()->at(idx) == 0) {
            continue;
        }
        PaletteCell* cc = ccp()->at(idx);          // current cell

        QString tag = cc->tag;
        if (!tag.isEmpty()) {
            p.setPen(configuration()->gridColor());
            QFont f(p.font());
            f.setPointSize(12);
            p.setFont(f);
            if (tag == "ShowMore") {
                p.drawText(idxRect(idx), Qt::AlignCenter, "???");
            } else {
                p.drawText(rShift, Qt::AlignLeft | Qt::AlignTop, tag);
            }
        }

        p.setPen(pen);

        Element* el = cc->element.get();
        if (el == 0) {
            continue;
        }
        bool drawStaff = cc->drawStaff;
        int row    = idx / columns();
        int column = idx % columns();

        qreal cellMag = cc->mag * mag;
        if (el->isIcon()) {
            toIcon(el)->setExtent((hhgrid < vgridM ? hhgrid : vgridM) - 4);
            cellMag = 1.0;
        }
        el->layout();

        if (drawStaff) {
            qreal y = r.y() + vgridM * .5 - dy + _yOffset * _spatium * cellMag;
            qreal x = r.x() + 3;
            qreal w = hhgrid - 6;
            for (int i = 0; i < 5; ++i) {
                qreal yy = y + i * magS;
                p.setPen(configuration()->elementsColor());
                p.drawLine(QLineF(x, yy, x + w, yy));
            }
        }
        p.save();
        p.scale(cellMag, cellMag);

        double gw = hhgrid / cellMag;
        double gh = vgridM / cellMag;
        double gx = column * gw + cc->xoffset * _spatium;
        double gy = row * gh + cc->yoffset * _spatium;

        double sw = el->width();
        double sh = el->height();
        double sy;

        if (drawStaff) {
            sy = gy + gh * .5 - 2.0 * _spatium;
        } else {
            sy  = gy + (gh - sh) * .5 - el->bbox().y();
        }
        double sx  = gx + (gw - sw) * .5 - el->bbox().x();

        sy += _yOffset * _spatium;

        p.translate(sx, sy);

        QColor color;
        if (idx != selectedIdx) {
            // show voice colors for notes
            if (el->isChord()) {
                color = el->curColor();
            } else {
                color = palette().color(QPalette::Normal, QPalette::Text);
            }
        } else {
            color = palette().color(QPalette::Normal, QPalette::HighlightedText);
        }

        p.setPen(QPen(color));
        el->scanElements(&p, paintPaletteElement);
        p.restore();
    }
}

//---------------------------------------------------------
//   pixmap
//---------------------------------------------------------

QPixmap Palette::pixmap(int paletteIdx) const
{
    qreal _spatium = gscore->spatium();
    qreal magS     = PALETTE_SPATIUM * extraMag * paletteScaling();
    qreal mag      = magS / _spatium;
    PaletteCell* c = cellAt(paletteIdx);
    if (!c || !c->element) {
        return QPixmap();
    }
    qreal cellMag = c->mag * mag;
    Element* e = c->element.get();
    e->layout();
    QRectF r = e->bbox();
    int w    = lrint(r.width() * cellMag);
    int h    = lrint(r.height() * cellMag);

    if (w * h == 0) {
        qDebug("zero pixmap %d %d %s", w, h, e->name());
        return QPixmap();
    }

    QPixmap pm(w, h);
    pm.fill(configuration()->elementsBackgroundColor());
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);

    if (e->isIcon()) {
        toIcon(e)->setExtent(w < h ? w : h);
    }
    p.scale(cellMag, cellMag);

    p.translate(-r.topLeft());
    QPointF pos = e->ipos();
    e->setPos(0, 0);

    QColor color;
    // show voice colors for notes
    if (e->isChord()) {
        Chord* chord = toChord(e);
        for (Note* n : chord->notes()) {
            n->setSelected(true);
        }
        color = e->curColor();
    } else {
        color = palette().color(QPalette::Normal, QPalette::Text);
    }

    p.setPen(QPen(color));
    e->scanElements(&p, paintPaletteElement);

    e->setPos(pos);
    return pm;
}

//---------------------------------------------------------
//   event
//---------------------------------------------------------

bool Palette::event(QEvent* ev)
{
    int hgridM = gridWidthM();
    int vgridM = gridHeightM();
    // disable mouse hover when keyboard navigation is enabled
    if (filterActive && (ev->type() == QEvent::MouseMove || ev->type() == QEvent::ToolTip
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
        if (idx >= cells.size()) {
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

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Palette::write(XmlWriter& xml) const
{
    xml.stag(QString("Palette name=\"%1\"").arg(XmlWriter::xmlString(_name)));
    xml.tag("gridWidth", hgrid);
    xml.tag("gridHeight", vgrid);
    xml.tag("mag", extraMag);
    if (_drawGrid) {
        xml.tag("grid", _drawGrid);
    }

    xml.tag("moreElements", _moreElements);
    if (_yOffset != 0.0) {
        xml.tag("yoffset", _yOffset);
    }

    int n = cells.size();
    for (int i = 0; i < n; ++i) {
        if (cells[i] && cells[i]->tag == "ShowMore") {
            continue;
        }
        if (cells[i] == 0 || cells[i]->element == 0) {
            xml.tagE("Cell");
            continue;
        }
        if (!cells[i]->name.isEmpty()) {
            xml.stag(QString("Cell name=\"%1\"").arg(XmlWriter::xmlString(cells[i]->name)));
        } else {
            xml.stag("Cell");
        }
        if (cells[i]->drawStaff) {
            xml.tag("staff", cells[i]->drawStaff);
        }
        if (cells[i]->xoffset) {
            xml.tag("xoffset", cells[i]->xoffset);
        }
        if (cells[i]->yoffset) {
            xml.tag("yoffset", cells[i]->yoffset);
        }
        if (!cells[i]->tag.isEmpty()) {
            xml.tag("tag", cells[i]->tag);
        }
        if (cells[i]->mag != 1.0) {
            xml.tag("mag", cells[i]->mag);
        }
        cells[i]->element->write(xml);
        xml.etag();
    }
    xml.etag();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool Palette::read(const QString& p)
{
    QString path(p);
    if (!path.endsWith(".mpal")) {
        path += ".mpal";
    }

    MQZipReader f(path);
    if (!f.exists()) {
        qDebug("palette <%s> not found", qPrintable(path));
        return false;
    }
    clear();

    QByteArray ba = f.fileData("META-INF/container.xml");

    XmlReader e(ba);
    // extract first rootfile
    QString rootfile = "";
    QList<QString> images;
    while (e.readNextStartElement()) {
        if (e.name() != "container") {
            e.unknown();
            break;
        }
        while (e.readNextStartElement()) {
            if (e.name() != "rootfiles") {
                e.unknown();
                break;
            }
            while (e.readNextStartElement()) {
                const QStringRef& tag(e.name());

                if (tag == "rootfile") {
                    if (rootfile.isEmpty()) {
                        rootfile = e.attribute("full-path");
                    }
                    e.readNext();
                } else if (tag == "file") {
                    images.append(e.readElementText());
                } else {
                    e.unknown();
                }
            }
        }
    }
    //
    // load images
    //
    foreach (const QString& s, images) {
        imageStore.add(s, f.fileData(s));
    }

    if (rootfile.isEmpty()) {
        qDebug("can't find rootfile in: %s", qPrintable(path));
        return false;
    }

    ba = f.fileData(rootfile);
    e.clear();
    e.addData(ba);
    while (e.readNextStartElement()) {
        if (e.name() == "museScore") {
            QString version = e.attribute("version");
            QStringList sl = version.split('.');
            int versionId = sl[0].toInt() * 100 + sl[1].toInt();
            gscore->setMscVersion(versionId);

            while (e.readNextStartElement()) {
                if (e.name() == "Palette") {
                    QString name = e.attribute("name");
                    setName(name);
                    read(e);
                } else {
                    e.unknown();
                }
            }
        } else {
            e.unknown();
        }
    }
    return true;
}

//---------------------------------------------------------
//   writeFailed
//---------------------------------------------------------

void Palette::showWritingFailedError(const QString& path) const
{
    std::string title = mu::trc("palette", "Writing Palette File");
    std::string message = mu::qtrc("palette", "Writing Palette File\n%1\nfailed: ").arg(path).toStdString();
    interactive()->message(IInteractive::Type::Critical, title, message);
}

//---------------------------------------------------------
//   write
//    write as compressed zip file and include
//    images as needed
//---------------------------------------------------------

void Palette::write(const QString& p)
{
    QSet<ImageStoreItem*> images;
    int n = cells.size();
    for (int i = 0; i < n; ++i) {
        if (cells[i] == 0 || cells[i]->element == 0 || cells[i]->element->type() != ElementType::IMAGE) {
            continue;
        }
        images.insert(static_cast<Image*>(cells[i]->element.get())->storeItem());
    }

    QString path(p);
    if (!path.endsWith(".mpal")) {
        path += ".mpal";
    }

    MQZipWriter f(path);
    // f.setCompressionPolicy(QZipWriter::NeverCompress);
    f.setCreationPermissions(
        QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner
        | QFile::ReadUser | QFile::WriteUser | QFile::ExeUser
        | QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup
        | QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);

    if (f.status() != MQZipWriter::NoError) {
        showWritingFailedError(path);
        return;
    }
    QBuffer cbuf;
    cbuf.open(QIODevice::ReadWrite);
    XmlWriter xml(gscore, &cbuf);
    xml.header();
    xml.stag("container");
    xml.stag("rootfiles");
    xml.stag(QString("rootfile full-path=\"%1\"").arg(XmlWriter::xmlString("palette.xml")));
    xml.etag();
    foreach (ImageStoreItem* ip, images) {
        QString ipath = QString("Pictures/") + ip->hashName();
        xml.tag("file", ipath);
    }
    xml.etag();
    xml.etag();
    cbuf.seek(0);
    //f.addDirectory("META-INF");
    //f.addDirectory("Pictures");
    f.addFile("META-INF/container.xml", cbuf.data());

    // save images
    foreach (ImageStoreItem* ip, images) {
        QString ipath = QString("Pictures/") + ip->hashName();
        f.addFile(ipath, ip->buffer());
    }
    {
        QBuffer cbuf1;
        cbuf1.open(QIODevice::ReadWrite);
        XmlWriter xml1(gscore, &cbuf1);
        xml1.header();
        xml1.stag("museScore version=\"" MSC_VERSION "\"");
        write(xml1);
        xml1.etag();
        cbuf1.close();
        f.addFile("palette.xml", cbuf1.data());
    }
    f.close();
    if (f.status() != MQZipWriter::NoError) {
        showWritingFailedError(path);
    }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Palette::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& t(e.name());
        if (t == "gridWidth") {
            hgrid = e.readDouble();
        } else if (t == "gridHeight") {
            vgrid = e.readDouble();
        } else if (t == "mag") {
            extraMag = e.readDouble();
        } else if (t == "grid") {
            _drawGrid = e.readInt();
        } else if (t == "moreElements") {
            setMoreElements(e.readInt());
        } else if (t == "yoffset") {
            _yOffset = e.readDouble();
        } else if (t == "drumPalette") {      // obsolete
            e.skipCurrentElement();
        } else if (t == "Cell") {
            PaletteCell* cell = new PaletteCell;
            cell->id = PaletteCell::makeId();
            cell->name = e.attribute("name");
            bool add = true;
            while (e.readNextStartElement()) {
                const QStringRef& t1(e.name());
                if (t1 == "staff") {
                    cell->drawStaff = e.readInt();
                } else if (t1 == "xoffset") {
                    cell->xoffset = e.readDouble();
                } else if (t1 == "yoffset") {
                    cell->yoffset = e.readDouble();
                } else if (t1 == "mag") {
                    cell->mag = e.readDouble();
                } else if (t1 == "tag") {
                    cell->tag = e.readElementText();
                } else {
                    cell->element.reset(Element::name2Element(t1, gscore));
                    if (cell->element == 0) {
                        e.unknown();
                        delete cell;
                        return;
                    } else {
                        cell->element->read(e);
                        cell->element->styleChanged();
                        if (cell->element->type() == ElementType::ICON) {
                            Icon* icon = static_cast<Icon*>(cell->element.get());
                            QAction* ac = adapter()->getAction(icon->action());
                            if (ac) {
                                QIcon qicon(ac->icon());
                                icon->setAction(icon->action(), qicon);
                            } else {
                                add = false;                 // action is not valid, don't add it to the palette.
                            }
                        }
                    }
                }
            }
            if (add) {
                int idx = _moreElements ? cells.size() - 1 : cells.size();
                cells.insert(idx, cell);
            }
        } else {
            e.unknown();
        }
    }
    // make sure hgrid and vgrid are not 0, we divide by them later
    if (hgrid <= 0) {
        hgrid = 28;
    }
    if (vgrid <= 0) {
        vgrid = 28;
    }
}

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Palette::clear()
{
    qDeleteAll(cells);
    cells.clear();
}

//---------------------------------------------------------
//   columns
//---------------------------------------------------------

int Palette::columns() const
{
    return width() / gridWidthM();
}

//---------------------------------------------------------
//   rows
//---------------------------------------------------------

int Palette::rows() const
{
    int c = columns();
    if (c == 0) {
        return 0;
    }
    return (size() + c - 1) / c;
}

//---------------------------------------------------------
//   heightForWidth
//---------------------------------------------------------

int Palette::heightForWidth(int w) const
{
    int hgridM = gridWidthM();
    int vgridM = gridHeightM();
    int c = w / hgridM;
    if (c <= 0) {
        c = 1;
    }
    int s = size();
    if (_moreElements) {
        s += 1;
    }
    int rows = (s + c - 1) / c;
    if (rows <= 0) {
        rows = 1;
    }
    qreal magS = PALETTE_SPATIUM * extraMag * paletteScaling();
    int h = lrint(_yOffset * 2 * magS);
    return rows * vgridM + h;
}

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize Palette::sizeHint() const
{
    int h = heightForWidth(width());
    int hgridM = gridWidthM();
    return QSize((width() / hgridM) * hgridM, h);
}

//---------------------------------------------------------
//   actionToggled
//---------------------------------------------------------

void Palette::actionToggled(bool /*val*/)
{
    selectedIdx = -1;
    int nn = ccp()->size();
    for (int n = 0; n < nn; ++n) {
        const Element* e = cellAt(n)->element.get();
        if (e && e->type() == ElementType::ICON) {
            QAction* a = adapter()->getAction(static_cast<const Icon*>(e)->action());
            if (a->isChecked()) {
                selectedIdx = n;
                break;
            }
        }
    }
    update();
}

//---------------------------------------------------------
// PaletteScrollArea
//---------------------------------------------------------

PaletteScrollArea::PaletteScrollArea(Palette* w, QWidget* parent)
    : QScrollArea(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidget(w);
    setWidgetResizable(false);
    _restrictHeight = true;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

//---------------------------------------------------------
// resizeEvent
//---------------------------------------------------------

void PaletteScrollArea::resizeEvent(QResizeEvent* re)
{
    QScrollArea::resizeEvent(re);   // necessary?

    Palette* palette = static_cast<Palette*>(widget());
    int h = palette->heightForWidth(width());
    palette->resize(QSize(width() - 6, h));
    if (_restrictHeight) {
        setMaximumHeight(h + 6);
    }
}

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void Palette::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* dta = event->mimeData();
    if (dta->hasUrls()) {
        QList<QUrl> ul = event->mimeData()->urls();
        QUrl u = ul.front();
        if (MScore::debugMode) {
            qDebug("dragEnterEvent: Url: %s", qPrintable(u.toString()));
            qDebug("   scheme <%s> path <%s>", qPrintable(u.scheme()), qPrintable(u.path()));
        }
        if (u.scheme() == "file") {
            QFileInfo fi(u.path());
            QString suffix(fi.suffix().toLower());
            if (suffix == "svg"
                || suffix == "jpg"
                || suffix == "jpeg"
                || suffix == "png"
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
        qDebug("dragEnterEvent: formats:");
        for (const QString& s : event->mimeData()->formats()) {
            qDebug("   %s", qPrintable(s));
        }
#endif
    }
}

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void Palette::dragMoveEvent(QDragMoveEvent* event)
{
    int i = idx(event->pos());
    if (event->source() == this) {
        if (i != -1) {
            if (currentIdx != -1 && event->proposedAction() == Qt::MoveAction) {
                if (i != currentIdx) {
                    PaletteCell* c = cells.takeAt(currentIdx);
                    cells.insert(i, c);
                    currentIdx = i;
                    update();
                }
                event->accept();
                return;
            }
        }
        event->ignore();
    } else {
        event->accept();
    }
}

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void Palette::dropEvent(QDropEvent* event)
{
    Element* e = 0;
    QString name;

    const QMimeData* datap = event->mimeData();
    if (datap->hasUrls()) {
        QList<QUrl> ul = event->mimeData()->urls();
        QUrl u = ul.front();
        if (u.scheme() == "file") {
            QFileInfo fi(u.path());
            Image* s = new Image(gscore);
            QString filePath(u.toLocalFile());
            s->load(filePath);
            e = s;
            QFileInfo f(filePath);
            name = f.completeBaseName();
        }
    } else if (datap->hasFormat(mu::commonscene::MIME_SYMBOL_FORMAT)) {
        QByteArray dta(event->mimeData()->data(mu::commonscene::MIME_SYMBOL_FORMAT));
        XmlReader xml(dta);
        QPointF dragOffset;
        Fraction duration;
        ElementType type = Element::readType(xml, &dragOffset, &duration);

        if (type == ElementType::SYMBOL) {
            Symbol* symbol = new Symbol(gscore);
            symbol->read(xml);
            e = symbol;
        } else {
            e = Element::create(type, gscore);
            if (e) {
                e->read(xml);
                e->setTrack(0);
                if (e->isIcon()) {
                    Icon* i = toIcon(e);
                    const QByteArray& action = i->action();
                    if (!action.isEmpty()) {
                        QAction* a = adapter()->getAction(action);
                        if (a) {
                            QIcon icon(a->icon());
                            i->setAction(action, icon);
                        }
                    }
                }
            }
        }
    }
    if (e == 0) {
        event->ignore();
        return;
    }
    if (event->source() == this) {
        delete e;
        if (event->proposedAction() == Qt::MoveAction) {
            event->accept();
            emit changed();
            return;
        }
        event->ignore();
        return;
    }

    if (e->isFretDiagram()) {
        name = toFretDiagram(e)->harmonyText();
    }

    e->setSelected(false);
    int i = idx(event->pos());
    if (i == -1 || cells[i]) {
        append(e, name);
    } else {
        add(i, e, name);
    }
    event->accept();
    while (!cells.isEmpty() && cells.back() == 0) {
        cells.removeLast();
    }
    setFixedHeight(heightForWidth(width()));
    updateGeometry();
    update();
    emit changed();
}
}
