/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "abstractelementpopupmodel.h"
#include "log.h"

using namespace mu::notation;

static const QMap<mu::engraving::ElementType, PopupModelType> ELEMENT_POPUP_TYPES = {
    { mu::engraving::ElementType::HARP_DIAGRAM, PopupModelType::TYPE_HARP_DIAGRAM },
    { mu::engraving::ElementType::CAPO, PopupModelType::TYPE_CAPO },
    { mu::engraving::ElementType::STRING_TUNINGS, PopupModelType::TYPE_STRING_TUNINGS },
};

AbstractElementPopupModel::AbstractElementPopupModel(PopupModelType modelType, QObject* parent)
    : QObject(parent), m_modelType(modelType)
{
}

PopupModelType AbstractElementPopupModel::modelType() const
{
    return m_modelType;
}

QRect AbstractElementPopupModel::itemRect() const
{
    return m_itemRect;
}

bool AbstractElementPopupModel::supportsPopup(const engraving::ElementType& elementType)
{
    return modelTypeFromElement(elementType) != PopupModelType::TYPE_UNDEFINED;
}

PopupModelType AbstractElementPopupModel::modelTypeFromElement(const engraving::ElementType& elementType)
{
    return ELEMENT_POPUP_TYPES.value(elementType, PopupModelType::TYPE_UNDEFINED);
}

mu::PointF AbstractElementPopupModel::fromLogical(PointF point) const
{
    return currentNotation()->viewState() ? currentNotation()->viewState()->matrix().map(point) : PointF();
}

mu::RectF AbstractElementPopupModel::fromLogical(RectF rect) const
{
    return currentNotation()->viewState() ? currentNotation()->viewState()->matrix().map(rect) : RectF();
}

INotationUndoStackPtr AbstractElementPopupModel::undoStack() const
{
    return currentNotation() ? currentNotation()->undoStack() : nullptr;
}

void AbstractElementPopupModel::beginCommand()
{
    if (undoStack()) {
        undoStack()->prepareChanges();
    }
}

void AbstractElementPopupModel::beginMultiCommands()
{
    beginCommand();

    if (undoStack()) {
        undoStack()->lock();
    }
}

void AbstractElementPopupModel::endCommand()
{
    if (undoStack()) {
        undoStack()->commitChanges();
    }
}

void AbstractElementPopupModel::endMultiCommands()
{
    if (undoStack()) {
        undoStack()->unlock();
    }

    endCommand();
}

void AbstractElementPopupModel::updateNotation()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->notationChanged().notify();
}

INotationPtr AbstractElementPopupModel::currentNotation() const
{
    return globalContext()->currentNotation();
}

void AbstractElementPopupModel::changeItemProperty(mu::engraving::Pid id, const PropertyValue& value)
{
    IF_ASSERT_FAILED(m_item) {
        return;
    }

    mu::engraving::PropertyFlags flags = m_item->propertyFlags(id);
    if (flags == mu::engraving::PropertyFlags::STYLED) {
        flags = mu::engraving::PropertyFlags::UNSTYLED;
    }

    beginCommand();
    m_item->undoChangeProperty(id, value, flags);
    endCommand();
    updateNotation();
}

void AbstractElementPopupModel::changeItemProperty(mu::engraving::Pid id, const PropertyValue& value, mu::engraving::PropertyFlags flags)
{
    IF_ASSERT_FAILED(m_item) {
        return;
    }

    beginCommand();
    m_item->undoChangeProperty(id, value, flags);
    endCommand();
    updateNotation();
}

INotationInteractionPtr AbstractElementPopupModel::interaction() const
{
    INotationPtr notation = globalContext()->currentNotation();
    return notation ? notation->interaction() : nullptr;
}

INotationSelectionPtr AbstractElementPopupModel::selection() const
{
    INotationInteractionPtr interaction = this->interaction();
    return interaction ? interaction->selection() : nullptr;
}

void AbstractElementPopupModel::init()
{
    m_item = nullptr;

    INotationSelectionPtr selection = this->selection();
    if (!selection) {
        return;
    }

    INotationUndoStackPtr undoStack = this->undoStack();
    if (!undoStack) {
        return;
    }

    m_item = selection->element();

    undoStack->changesChannel().onReceive(this, [this] (const ChangesRange& range) {
        if (contains(range.changedTypes, elementType())) {
            emit dataChanged();
            updateItemRect();
        }
    });

    updateItemRect();
}

mu::engraving::ElementType AbstractElementPopupModel::elementType() const
{
    return ELEMENT_POPUP_TYPES.key(m_modelType, ElementType::INVALID);
}

void AbstractElementPopupModel::updateItemRect()
{
    QRect rect = m_item ? fromLogical(m_item->canvasBoundingRect()).toQRect() : QRect();

    if (m_itemRect != rect) {
        m_itemRect = rect;
        emit itemRectChanged(rect);
    }
}
