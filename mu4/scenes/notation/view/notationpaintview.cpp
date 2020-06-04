//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "notationpaintview.h"

#include <QPainter>
#include <cmath>

#include "log.h"
#include "notationviewinputcontroller.h"
#include "actions/action.h"

using namespace mu::scene::notation;

static constexpr int PREF_UI_CANVAS_MISC_SELECTIONPROXIMITY = 6;

NotationPaintView::NotationPaintView()
    : QQuickPaintedItem()
{
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    //! TODO
    double mag  = 0.267;//preferences.getDouble(PREF_SCORE_MAGNIFICATION) * (mscore->physicalDotsPerInch() / DPI);
    m_matrix = QTransform::fromScale(mag, mag);

    m_inputController = new NotationViewInputController(this);

    // actions
    dispatcher()->reg("file-open", [this](const actions::ActionName&) { open(); });
    dispatcher()->reg("note-input", [this](const actions::ActionName&) { toggleNoteInput(); });
    dispatcher()->reg("pad-note-8", [this](const actions::ActionName& name) { padNote(name); });
}

//! NOTE Temporary method for tests
void NotationPaintView::open()
{
    QString filePath = interactive()->selectOpeningFile("Score", "", "");
    if (filePath.isEmpty()) {
        return;
    }

    m_notation = notationCreator()->newNotation();
    IF_ASSERT_FAILED(m_notation) {
        return;
    }

    domain::notation::INotation::Params params;
    params.pageSize.width = width();
    params.pageSize.height = height();
    bool ok = m_notation->load(filePath.toStdString(), params);
    if (!ok) {
        LOGE() << "failed load: " << filePath;
    }

    update();
}

void NotationPaintView::changeState(State st)
{
    if (st == m_state) {
        return;
    }

    // old state
    switch (m_state) {
    case State::NORMAL: break;
    case State::NOTE_ENTRY:
        endNoteEntry();
        break;
    }

    // new state
    m_state = st;
    switch (m_state) {
    case State::NORMAL: break;
    case State::NOTE_ENTRY:
        startNoteEntry();
        break;
    }
}

void NotationPaintView::toggleNoteInput()
{
    LOGI() << "toggleNoteInput";
    IF_ASSERT_FAILED(m_notation) {
        return;
    }

    if (state() == State::NOTE_ENTRY) {
        changeState(State::NORMAL);
    } else {
        changeState(State::NOTE_ENTRY);
    }
}

void NotationPaintView::startNoteEntry()
{
    LOGI() << "startNoteEntry";
    m_notation->startNoteEntry();
}

void NotationPaintView::endNoteEntry()
{
    LOGI() << "endNoteEntry";
}

void NotationPaintView::padNote(const actions::ActionName& name)
{
    LOGI() << "padNote: " << name;
    IF_ASSERT_FAILED(m_notation) {
        return;
    }
    m_notation->action(name);
}

void NotationPaintView::paint(QPainter* p)
{
    QRect rect(0, 0, width(), height());
    p->fillRect(rect, QColor("#D6E0E9"));

    p->setTransform(m_matrix);

    if (m_notation) {
        m_notation->paint(p, rect);
    } else {
        p->drawText(10, 10, "no notation");
    }
}

void NotationPaintView::moveScene(int dx, int dy)
{
    m_matrix.translate(dx, dy);
    update();
}

void NotationPaintView::scrollVertical(int dy)
{
    m_matrix.translate(0, dy);
    update();
}

void NotationPaintView::scrollHorizontal(int dx)
{
    m_matrix.translate(dx, 0);
    update();
}

void NotationPaintView::zoomStep(qreal step, const QPoint& pos)
{
    qreal mag = m_matrix.m11();
    mag *= qPow(1.1, step);
    zoom(mag, pos);
}

void NotationPaintView::zoom(qreal mag, const QPoint& pos)
{
    //! TODO Zoom to point not completed
    mag = qBound(0.05, mag, 16.0);

    qreal cmag = m_matrix.m11();
    if (qFuzzyCompare(mag, cmag)) {
        return;
    }

    qreal deltamag = mag / mag;

    QPointF p1 = m_matrix.inverted().map(pos);

    m_matrix.setMatrix(mag, m_matrix.m12(), m_matrix.m13(), m_matrix.m21(),
                       mag, m_matrix.m23(), m_matrix.dx() * deltamag, m_matrix.dy() * deltamag, m_matrix.m33());

    QPointF p2 = m_matrix.inverted().map(pos);
    QPointF p3 = p2 - p1;
    int dx = std::lrint(p3.x() * cmag);
    int dy = std::lrint(p3.y() * cmag);

    m_matrix.translate(dx, dy);

    update();
}

void NotationPaintView::wheelEvent(QWheelEvent* ev)
{
    m_inputController->wheelEvent(ev);
}

void NotationPaintView::mousePressEvent(QMouseEvent* ev)
{
    m_inputController->mousePressEvent(ev);
}

void NotationPaintView::mouseMoveEvent(QMouseEvent* ev)
{
    m_inputController->mouseMoveEvent(ev);
}

void NotationPaintView::mouseReleaseEvent(QMouseEvent* ev)
{
    m_inputController->mouseReleaseEvent(ev);
}

QPoint NotationPaintView::toLogical(const QPoint& p) const
{
    return m_matrix.inverted().map(p);
}

QPoint NotationPaintView::toPhysical(const QPoint& p) const
{
    return m_matrix.map(p);
}

NotationPaintView::State NotationPaintView::state() const
{
    return m_state;
}
