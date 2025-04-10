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

#include <QDir>
#include <QAction>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QMimeData>

#include "keyedit.h"

#include "commonscene/commonscenetypes.h"
#include "translation.h"

#include "engraving/rw/rwregister.h"
#include "engraving/dom/accidental.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/factory.h"
#include "engraving/style/defaultstyle.h"
#include "engraving/compat/dummyelement.h"

#include "types/symnames.h"

#include "keycanvas.h"
#include "palettewidget.h"
#include "internal/palettecreator.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;
using namespace mu::palette;

//---------------------------------------------------------
//   KeyCanvas
//---------------------------------------------------------

KeyCanvas::KeyCanvas(QWidget* parent)
    : QFrame(parent)
{
    setAcceptDrops(true);
    qreal mag = configuration()->paletteSpatium() * configuration()->paletteScaling() / gpaletteScore->style().spatium();
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
    muse::draw::Painter painter(this, "keycanvas");
    painter.setAntialiasing(true);
    qreal wh = double(height());
    qreal ww = double(width());
    double y = wh * .5 - 2 * configuration()->paletteSpatium() * extraMag;

    qreal mag  = configuration()->paletteSpatium() * extraMag / gpaletteScore->style().spatium();
    _matrix    = QTransform(mag, 0.0, 0.0, mag, 0.0, y);
    imatrix    = _matrix.inverted();

    qreal x = 3;
    qreal w = ww - 6;

    painter.setWorldTransform(Transform::fromQTransform(_matrix));

    QRectF r = imatrix.mapRect(QRectF(x, y, w, wh));

    RectF background = RectF::fromQRectF(imatrix.mapRect(QRectF(0, 0, ww, wh)));
    painter.fillRect(background, notationConfiguration()->foregroundColor());

    muse::draw::Pen pen(engravingConfiguration()->scoreInversionEnabled() ? engravingConfiguration()->scoreInversionColor()
                        : engravingConfiguration()->defaultColor());
    pen.setWidthF(engraving::DefaultStyle::defaultStyle().styleS(Sid::staffLineWidth).val() * gpaletteScore->style().spatium());
    painter.setPen(pen);

    for (int i = 0; i < 5; ++i) {
        qreal yy = r.y() + i * gpaletteScore->style().spatium();
        painter.drawLine(LineF(r.x(), yy, r.x() + r.width(), yy));
    }
    if (dragElement) {
        painter.save();
        painter.translate(dragElement->pagePos());
        gpaletteScore->renderer()->drawItem(dragElement, &painter);
        painter.restore();
    }
    foreach (Accidental* a, accidentals) {
        painter.save();
        painter.translate(a->pagePos());
        gpaletteScore->renderer()->drawItem(a, &painter);
        painter.restore();
    }
    clef->setPos(0.0, 0.0);

    engravingRender()->layoutItem(clef);

    painter.translate(clef->pagePos());
    gpaletteScore->renderer()->drawItem(clef, &painter);
}

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void KeyCanvas::mousePressEvent(QMouseEvent* event)
{
    startMove = imatrix.map(QPointF(event->pos() - base));
    moveElement = 0;
    foreach (Accidental* a, accidentals) {
        QRectF r = a->pageBoundingRect().toQRectF();
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

        rw::RWRegister::reader()->readItem(dragElement, e);
        engravingRender()->layoutItem(dragElement);
    } else {
        if (MScore::debugMode) {
            LOGD("KeyCanvas::dragEnterEvent: formats:");
            foreach (const QString& s, event->mimeData()->formats()) {
                LOGD("   %s", qPrintable(s));
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
        PointF pos = PointF::fromQPointF(imatrix.map(event->position()));
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
    double _spatium = gpaletteScore->style().spatium();
    double spatium2 = _spatium * .5;
    double y = a->ldata()->pos().y();
    double x = KEYEDIT_ACC_ZERO_POINT * _spatium;
    int line = round(y / spatium2);
    y = line * spatium2;
    a->mutldata()->setPosY(y);
    // take default xposition unless Control is pressed
    int i = accidentals.indexOf(a);
    if (i > 0) {
        qreal accidentalGap = DefaultStyle::baseStyle().styleS(Sid::keysigAccidentalDistance).val() * _spatium;
        Accidental* prev = accidentals[i - 1];
        double prevX = prev->ldata()->pos().x();
        qreal prevWidth = prev->symWidth(prev->symId());
        x = prevX + prevWidth + accidentalGap;
    }
    if (!QGuiApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
        a->mutldata()->setPosX(x);
    }
}

//---------------------------------------------------------
//   KeyEditor
//---------------------------------------------------------

KeyEditor::KeyEditor(QWidget* parent)
    : QWidget(parent, Qt::WindowFlags(Qt::Dialog | Qt::Window))
{
    setupUi(this);
    setWindowTitle(muse::qtrc("palette", "Key signatures"));

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

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
    // set all "builtin" key signatures to read only
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

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void KeyEditor::addClicked()
{
    const QList<Accidental*> al = canvas->getAccidentals();
    double spatium = gpaletteScore->style().spatium();
    double xoff = KEYEDIT_ACC_ZERO_POINT * spatium;

    KeySigEvent e;
    e.setCustom(true);
    qreal accidentalGap = DefaultStyle::baseStyle().styleS(Sid::keysigAccidentalDistance).val();
    for (int i = 0; i < al.size(); ++i) {
        Accidental* a = al[i];
        CustDef c;
        c.sym = a->symId();
        PointF pos = a->ldata()->pos();
        c.xAlt = (pos.x() - xoff) / spatium;
        if (i > 0) {
            Accidental* prev = al[i - 1];
            PointF prevPos = prev->ldata()->pos();
            qreal prevWidth = prev->symWidth(prev->symId());
            c.xAlt -= (prevPos.x() - xoff + prevWidth) / spatium + accidentalGap;
        }
        int line = static_cast<int>(round((pos.y() / spatium) * 2));
        bool flat = std::string(SymNames::nameForSymId(c.sym).ascii()).find("Flat") != std::string::npos;
        c.degree = (3 - line) % 7;
        c.degree += (c.degree < 0) ? 7 : 0;
        line += flat ? -1 : 1; // top accidentals in treble clef are gis (#), or es (b)
        c.octAlt = static_cast<int>((line - (line >= 0 ? 0 : 6)) / 7);
        e.customKeyDefs().push_back(c);
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
