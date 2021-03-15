//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2009-2011 Werner Schweer and others
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

#include <QDir>
#include <QAction>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QMimeData>

#include "palette/palette.h"
#include "palette/palettecreator.h"
#include "keyedit.h"
#include "libmscore/keysig.h"
#include "libmscore/score.h"
#include "libmscore/accidental.h"
#include "keycanvas.h"
#include "libmscore/clef.h"
#include "libmscore/mscore.h"
#include "libmscore/xml.h"

#include "libmscore/draw/qpainterprovider.h"

#include "commonscene/commonscenetypes.h"
#include "translation.h"

#include "../palette_config.h"

namespace Ms {
extern bool useFactorySettings;
extern Palette* newAccidentalsPalette();
extern Palette* newKeySigPalette();

static const qreal editScale = 1.0;

//---------------------------------------------------------
//   KeyCanvas
//---------------------------------------------------------

KeyCanvas::KeyCanvas(QWidget* parent)
    : QFrame(parent)
{
    setAcceptDrops(true);
    extraMag   = editScale * configuration()->paletteScaling();
    qreal mag  = mu::palette::PALETTE_SPATIUM * extraMag / gscore->spatium();
    _matrix    = QTransform(mag, 0.0, 0.0, mag, 0.0, 0.0);
    imatrix    = _matrix.inverted();
    dragElement = 0;
    setFocusPolicy(Qt::StrongFocus);
    QAction* a = new QAction("delete", this);
    a->setShortcut(Qt::Key_Delete);
    addAction(a);
    clef = new Clef(gscore);
    clef->setClefType(ClefType::G);
    connect(a, SIGNAL(triggered()), SLOT(deleteElement()));
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
    double y = wh * .5 - 2 * mu::palette::PALETTE_SPATIUM * extraMag;

    qreal mag  = mu::palette::PALETTE_SPATIUM * extraMag / gscore->spatium();
    _matrix    = QTransform(mag, 0.0, 0.0, mag, 0.0, y);
    imatrix    = _matrix.inverted();

    qreal x = 3;
    qreal w = ww - 6;

    painter.setWorldTransform(_matrix);

    QRectF r = imatrix.mapRect(QRectF(x, y, w, wh));

    QRectF background = imatrix.mapRect(QRectF(0, 0, ww, wh));
    painter.fillRect(background, Qt::white);

    QPen pen(Qt::black);
    pen.setWidthF(MScore::defaultStyle().value(Sid::staffLineWidth).toDouble() * gscore->spatium());
    painter.setPen(pen);

    for (int i = 0; i < 5; ++i) {
        qreal yy = r.y() + i * gscore->spatium();
        painter.drawLine(QLineF(r.x(), yy, r.x() + r.width(), yy));
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
        QRectF r = a->abbox();
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
    moveElement->move(delta);
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

        QPointF dragOffset;
        Fraction duration;
        ElementType type = Element::readType(e, &dragOffset, &duration);
        if (type != ElementType::ACCIDENTAL) {
            return;
        }

        event->acceptProposedAction();
        dragElement = static_cast<Accidental*>(Element::create(type, gscore));
        dragElement->setParent(0);
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
        QPointF pos(imatrix.map(QPointF(event->pos())));
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
    double spatium2 = gscore->spatium() * .5;
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
    setWindowTitle(mu::qtrc("palette", "Key Signatures"));

    // create key signature palette

    QLayout* l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    frame->setLayout(l);

    sp = PaletteCreator::newKeySigPalette();
    sp->setReadOnly(false);

    _keyPalette = new PaletteScrollArea(sp);
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _keyPalette->setSizePolicy(policy);
    _keyPalette->setRestrictHeight(false);

    l->addWidget(_keyPalette);

    // create accidental palette

    l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    frame_3->setLayout(l);
    sp1 = PaletteCreator::newAccidentalsPalette();
    qreal adj = sp1->mag();
    sp1->setGrid(sp1->gridWidth() * editScale / adj, sp1->gridHeight() * editScale / adj);
    sp1->setMag(editScale);
    PaletteScrollArea* accPalette = new PaletteScrollArea(sp1);
    QSizePolicy policy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
    accPalette->setSizePolicy(policy1);
    accPalette->setRestrictHeight(false);

    l->addWidget(accPalette);

    connect(addButton,   SIGNAL(clicked()), SLOT(addClicked()));
    connect(clearButton, SIGNAL(clicked()), SLOT(clearClicked()));
    connect(sp,          SIGNAL(changed()), SLOT(setDirty()));

    //
    // set all "buildin" key signatures to read only
    //
    int n = sp->size();
    for (int i = 0; i < n; ++i) {
        sp->setCellReadOnly(i, true);
    }

    if (!configuration()->useFactorySettings()) {
        sp->read(configuration()->keySignaturesDirPath().toQString());
    }
}

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void KeyEditor::addClicked()
{
    // double extraMag = 2.0;
    const QList<Accidental*> al = canvas->getAccidentals();
    // qreal mag  = PALETTE_SPATIUM * extraMag / gscore->spatium();
    // double spatium = 2.0 * PALETTE_SPATIUM / extraMag;
    double spatium = gscore->spatium();
    double xoff = 10000000.0;

    for (Accidental* a : al) {
        QPointF pos = a->ipos();
        if (pos.x() < xoff) {
            xoff = pos.x();
        }
    }

    KeySigEvent e;
    e.setCustom(true);
    for (Accidental* a : al) {
        KeySym s;
        s.sym       = a->symbol();
        QPointF pos = a->ipos();
        pos.rx()   -= xoff;
        s.spos      = pos / spatium;
        e.keySymbols().append(s);
    }
    auto ks = makeElement<KeySig>(gscore);
    ks->setKeySigEvent(e);
    sp->append(ks, "custom");
    _dirty = true;
    emit keySigAdded(ks);
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

void KeyEditor::showKeyPalette(bool val)
{
    _keyPalette->setVisible(val);
}

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void KeyEditor::save()
{
    QDir dir;
    dir.mkpath(configuration()->keySignaturesDirPath().toQString());
    sp->write(configuration()->keySignaturesDirPath().toQString());
}
}
