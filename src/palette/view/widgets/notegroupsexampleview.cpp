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

#include "notegroupsexampleview.h"

#include <cmath>
#include <QMimeData>

#include "engraving/rw/xml.h"

#include "libmscore/masterscore.h"
#include "libmscore/engravingitem.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/actionicon.h"
#include "libmscore/chord.h"
#include "libmscore/factory.h"

#include "commonscene/commonscenetypes.h"

#include "log.h"

using namespace mu;
using namespace mu::palette;
using namespace mu::notation;
using namespace mu::engraving;

NoteGroupsExampleView::NoteGroupsExampleView(QWidget* parent)
    : notation::ExampleView(parent)
{
    setAcceptDrops(true);
}

void NoteGroupsExampleView::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* d = event->mimeData();
    if (d->hasFormat(mu::commonscene::MIME_SYMBOL_FORMAT)) {
        event->acceptProposedAction();

        QByteArray a = d->data(mu::commonscene::MIME_SYMBOL_FORMAT);

// LOGD("NoteGroupsExampleView::dragEnterEvent Symbol: <%s>", a.data());

        XmlReader e(ByteArray::fromQByteArrayNoCopy(a));
        PointF dragOffset;
        Fraction duration;      // dummy
        ElementType type = EngravingItem::readType(e, &dragOffset, &duration);

        m_dragElement = Factory::createItem(type, m_score->dummy());
        if (m_dragElement) {
            m_dragElement->resetExplicitParent();
            m_dragElement->read(e);
            m_dragElement->layout();
        }
        return;
    }
}

void NoteGroupsExampleView::dragLeaveEvent(QDragLeaveEvent*)
{
    if (m_dragElement) {
        delete m_dragElement;
        m_dragElement = 0;
    }
    setDropTarget(0);
}

struct MoveContext
{
    PointF pos;
    mu::engraving::Score* score = nullptr;
};

static void moveElement(void* data, EngravingItem* e)
{
    MoveContext* ctx = (MoveContext*)data;
    ctx->score->addRefresh(e->canvasBoundingRect());
    e->setPos(ctx->pos);
    ctx->score->addRefresh(e->canvasBoundingRect());
}

void NoteGroupsExampleView::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();

    if (!m_dragElement || !m_dragElement->isActionIcon()) {
        return;
    }

    const EngravingItem* newDropTarget = nullptr;

    PointF position = toLogical(event->posF());
    std::vector<EngravingItem*> el = elementsAt(position);

    for (const EngravingItem* e : el) {
        if (e->type() == ElementType::NOTE) {
            newDropTarget = e;
            break;
        }
    }

    setDropTarget(newDropTarget);

    MoveContext ctx{ position, m_score };
    m_dragElement->scanElements(&ctx, moveElement, false);
    m_score->update();
    return;
}

void NoteGroupsExampleView::setDropTarget(const EngravingItem* el)
{
    if (m_dropTarget != el) {
        if (m_dropTarget) {
            m_dropTarget->setDropTarget(false);
            m_dropTarget = 0;
        }
        m_dropTarget = el;
        if (m_dropTarget) {
            m_dropTarget->setDropTarget(true);
        }
    }
    if (!m_dropAnchor.isNull()) {
        QRectF r;
        r.setTopLeft(m_dropAnchor.p1());
        r.setBottomRight(m_dropAnchor.p2());
        m_dropAnchor = QLineF();
    }

    update();
}

void NoteGroupsExampleView::dropEvent(QDropEvent* event)
{
    PointF position = toLogical(event->posF());

    if (!m_dragElement) {
        return;
    }

    if (!m_dragElement->isActionIcon()) {
        delete m_dragElement;
        m_dragElement = nullptr;
        return;
    }

    foreach (EngravingItem* e, elementsAt(position)) {
        if (e->type() == ElementType::NOTE) {
            ActionIcon* icon = toActionIcon(m_dragElement);
            Chord* chord = toNote(e)->chord();
            emit beamPropertyDropped(chord, icon);
            break;
        }
    }

    event->acceptProposedAction();
    delete m_dragElement;
    m_dragElement = nullptr;
    setDropTarget(nullptr);
}

void NoteGroupsExampleView::mousePressEvent(QMouseEvent* event)
{
    ExampleView::mousePressEvent(event);

    PointF position = toLogical(event->pos());

    foreach (EngravingItem* e, elementsAt(position)) {
        if (e->type() == ElementType::NOTE) {
            emit noteClicked(toNote(e));
            break;
        }
    }
}
