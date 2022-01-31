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

#include <QDir>
#include <QAction>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QMimeData>

#include "keyedit.h"

#include "commonscene/commonscenetypes.h"
#include "translation.h"

#include "engraving/rw/xml.h"
#include "engraving/libmscore/accidental.h"
#include "engraving/libmscore/clef.h"
#include "engraving/libmscore/keysig.h"
#include "engraving/libmscore/masterscore.h"
#include "engraving/libmscore/mscore.h"
#include "engraving/libmscore/factory.h"
#include "engraving/style/defaultstyle.h"
#include "engraving/compat/dummyelement.h"

#include "keycanvas.h"
#include "palettewidget.h"
#include "internal/palettecreator.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::palette;
using namespace Ms;

//---------------------------------------------------------
//   KeyCanvas
//---------------------------------------------------------

KeyCanvas::KeyCanvas(QWidget* parent)
    : QFrame(parent)
{
    setAcceptDrops(true);
    qreal mag = configuration()->paletteSpatium() * configuration()->paletteScaling() / gpaletteScore->spatium();
    _matrix = QTransform(mag, 0.0, 0.0, mag, 0.0, 0.0);
    imatrix = _matrix.inverted();
    dragElement = 0;
    setFocusPolicy(Qt::StrongFocus);
    QAction* a = new QAction("delete", this);
    a->setShortcut(Qt::Key_Delete);
    addAction(a);
    clef = Factory::createClef(gpaletteScore->dummy()->segment());
    clef->setClefType(ClefType::G);
    connect(a, &QAction::triggered, this, &KeyCanvas::deleteElement);
}

//---------------------------------------------------------
//   delete
//---------------------------------------------------------

void KeyCanvas::deleteElement()
{
    foreach (Accidental* a, accidentals) {
        if (a->selected()) {
            accidentals.removeOne(a);
            delete a;
            update();
            break;
        }
    }
}

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void KeyCanvas::clear()
{
    foreach (Accidental* a, accidentals) {
        delete a;
    }
    accidentals.clear();
    update();
}

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void KeyCanvas::paintEvent(QPaintEvent*)
{
    mu::draw::Painter painter(this, "keycanvas");
    painter.setAntialiasing(true);
    qreal wh = double(height());
    qreal ww = double(width());
    double y = wh * .5 - 2 * configuration()->paletteSpatium() * extraMag;

    qreal mag  = configuration()->paletteSpatium() * extraMag / gpaletteScore->spatium();
    _matrix    = QTransform(mag, 0.0, 0.0, mag, 0.0, y);
    imatrix    = _matrix.inverted();

    qreal x = 3;
    qreal w = ww - 6;

    painter.setWorldTransform(mu::Transform::fromQTransform(_matrix));

    QRectF r = imatrix.mapRect(QRectF(x, y, w, wh));

    RectF background = RectF::fromQRectF(imatrix.mapRect(QRectF(0, 0, ww, wh)));
    painter.fillRect(background, mu::draw::Color::white);

    draw::Pen pen(engravingConfiguration()->defaultColor());
    pen.setWidthF(engraving::DefaultStyle::defaultStyle().styleS(Sid::staffLineWidth).val() * gpaletteScore->spatium());
    painter.setPen(pen);

    for (int i = 0; i < 5; ++i) {
        qreal yy = r.y() + i * gpaletteScore->spatium();
        painter.drawLine(LineF(r.x(), yy, r.x() + r.width(), yy));
    }
    if (dragElement) {
        painter.save();
        painter.translate(dragElement->pagePos());
        dragElement->draw(&painter);
        painter.restore();
    }
    foreach (Accidental* a, accidentals) {
        painter.save();
        painter.translate(a->pagePos());
        a->draw(&painter);
        painter.restore();
    }
    clef->setPos(0.0, 0.0);
    clef->layout();
    painter.translate(clef->pagePos());
    clef->draw(&painter);
}

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void KeyCanvas::mousePressEvent(QMouseEvent* event)
{
    startMove = imatrix.map(QPointF(event->pos() - base));
    moveElement = 0;
    foreach (Accidental* a, accidentals) {
        QRectF r = a->abbox().toQRectF();
        if (r.contains(startMove)) {
            a->setSelected(true);
            moveElement = a;
        } else {
            a->setSelected(false);
        }
    }
    update();
}

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void KeyCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if (moveElement == 0) {
        return;
    }
    QPointF p = imatrix.map(QPointF(event->pos()));
    QPointF delta = p - startMove;
    moveElement->move(PointF::fromQPointF(delta));
    startMove = p;
    update();
}

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void KeyCanvas::mouseReleaseEvent(QMouseEvent*)
{
    if (moveElement == 0) {
        return;
    }
    snap(moveElement);
    update();
}

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void KeyCanvas::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* dta = event->mimeData();
    if (dta->hasFormat(mu::commonscene::MIME_SYMBOL_FORMAT)) {
        QByteArray a = dta->data(mu::commonscene::MIME_SYMBOL_FORMAT);

        XmlReader e(a);

        PointF dragOffset;
        Fraction duration;
        ElementType type = EngravingItem::readType(e, &dragOffset, &duration);
        if (type != ElementType::ACCIDENTAL) {
            return;
        }

        event->acceptProposedAction();
        dragElement = static_cast<Accidental*>(Factory::createItem(type, gpaletteScore->dummy()));
        dragElement->resetExplicitParent();
        dragElement->read(e);
        dragElement->layout();
    } else {
        if (MScore::debugMode) {
            qDebug("KeyCanvas::dragEnterEvent: formats:");
            foreach (const QString& s, event->mimeData()->formats()) {
                qDebug("   %s", qPrintable(s));
            }
        }
    }
}

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void KeyCanvas::dragMoveEvent(QDragMoveEvent* event)
{
    if (dragElement) {
        event->acceptProposedAction();
        PointF pos = PointF::fromQPointF(imatrix.map(QPointF(event->pos())));
        dragElement->setPos(pos);
        update();
    }
}

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void KeyCanvas::dropEvent(QDropEvent*)
{
    for (Accidental* a : accidentals) {
        a->setSelected(false);
    }
    dragElement->setSelected(true);
    accidentals.append(dragElement);
    snap(dragElement);
    dragElement = 0;
    update();
}

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

void KeyCanvas::snap(Accidental* a)
{
    double y        = a->ipos().y();
    double spatium2 = gpaletteScore->spatium() * .5;
    int line        = int((y + spatium2 * .5) / spatium2);
    y               = line * spatium2;
    a->rypos()      = y;
}

//---------------------------------------------------------
//   KeyEditor
//---------------------------------------------------------

KeyEditor::KeyEditor(QWidget* parent)
    : QWidget(parent, Qt::WindowFlags(Qt::Dialog | Qt::Window))
{
    setupUi(this);
    setWindowTitle(mu::qtrc("palette", "Key signatures"));

    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // create key signature palette

    QLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    keySigframe->setLayout(layout);

    m_keySigPaletteWidget = new PaletteWidget(this);
    m_keySigPaletteWidget->setPalette(PaletteCreator::newKeySigPalette());
    m_keySigPaletteWidget->setReadOnly(false);

    m_keySigArea = new PaletteScrollArea(m_keySigPaletteWidget);
    m_keySigArea->setSizePolicy(policy);
    m_keySigArea->setRestrictHeight(false);
    m_keySigArea->setFocusProxy(m_keySigPaletteWidget);
    m_keySigArea->setFocusPolicy(Qt::TabFocus);

    layout->addWidget(m_keySigArea);

    // create accidental palette

    layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    accidentalsFrame->setLayout(layout);

    m_accidentalsPaletteWidget = new PaletteWidget(this);
    m_accidentalsPaletteWidget->setPalette(PaletteCreator::newAccidentalsPalette());
    qreal adj = m_accidentalsPaletteWidget->mag();
    m_accidentalsPaletteWidget->setGridSize(m_accidentalsPaletteWidget->gridWidth() / adj, m_accidentalsPaletteWidget->gridHeight() / adj);
    m_accidentalsPaletteWidget->setMag(1.0);

    PaletteScrollArea* accidentalsPaletteArea = new PaletteScrollArea(m_accidentalsPaletteWidget);
    accidentalsPaletteArea->setSizePolicy(policy);
    accidentalsPaletteArea->setRestrictHeight(false);
    accidentalsPaletteArea->setFocusProxy(m_accidentalsPaletteWidget);
    accidentalsPaletteArea->setFocusPolicy(Qt::TabFocus);

    layout->addWidget(accidentalsPaletteArea);

    connect(addButton, &QPushButton::clicked, this, &KeyEditor::addClicked);
    connect(clearButton, &QPushButton::clicked, this, &KeyEditor::clearClicked);
    connect(m_keySigPaletteWidget, &PaletteWidget::changed, this, &KeyEditor::setDirty);

    //
    // set all "buildin" key signatures to read only
    //
    int n = m_keySigPaletteWidget->actualCellCount();
    for (int i = 0; i < n; ++i) {
        m_keySigPaletteWidget->setCellReadOnly(i, true);
    }

    if (!configuration()->useFactorySettings()) {
        m_keySigPaletteWidget->readFromFile(configuration()->keySignaturesDirPath().toQString());
    }

    //! NOTE: It is necessary for the correct start of navigation in the dialog
    setFocus();
}

KeyEditor::KeyEditor(const KeyEditor& widget)
    : KeyEditor(widget.parentWidget())
{
}

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void KeyEditor::addClicked()
{
    const QList<Accidental*> al = canvas->getAccidentals();
    double spatium = gpaletteScore->spatium();
    double xoff = 10000000.0;

    for (Accidental* a : al) {
        PointF pos = a->ipos();
        if (pos.x() < xoff) {
            xoff = pos.x();
        }
    }

    KeySigEvent e;
    e.setCustom(true);
    for (Accidental* a : al) {
        KeySym s;
        s.sym       = a->symbol();
        PointF pos = a->ipos();
        pos.rx()   -= xoff;
        s.spos      = pos / spatium;
        e.keySymbols().append(s);
    }
    auto ks = Factory::makeKeySig(gpaletteScore->dummy()->segment());
    ks->setKeySigEvent(e);
    m_keySigPaletteWidget->appendElement(ks, "custom");
    m_dirty = true;

    paletteProvider()->addCustomItemRequested().send(ks);
}

//---------------------------------------------------------
//   clearClicked
//---------------------------------------------------------

void KeyEditor::clearClicked()
{
    canvas->clear();
}

//---------------------------------------------------------
//   showKeyPalette
//---------------------------------------------------------

void KeyEditor::setShowKeyPalette(bool showKeyPalette)
{
    m_keySigArea->setVisible(showKeyPalette);
}

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void KeyEditor::save()
{
    QDir dir;
    dir.mkpath(configuration()->keySignaturesDirPath().toQString());
    m_keySigPaletteWidget->writeToFile(configuration()->keySignaturesDirPath().toQString());
}

bool KeyEditor::showKeyPalette() const
{
    return m_keySigArea->isVisible();
}
