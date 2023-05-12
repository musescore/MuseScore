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
};

AbstractElementPopupModel::AbstractElementPopupModel(QObject* parent, mu::engraving::ElementType elementType)
    : QObject(parent), m_elementType(elementType)
{
}

QString AbstractElementPopupModel::title() const
{
    return m_title;
}

PopupModelType AbstractElementPopupModel::modelType() const
{
    return m_modelType;
}

EngravingItem* AbstractElementPopupModel::getElement()
{
    EngravingItem* hitElement = hitElementContext().element;

    return hitElement ? hitElement : selection()->element();
}

PopupModelType AbstractElementPopupModel::modelTypeFromElement(const engraving::ElementType& elementType)
{
    return ELEMENT_POPUP_TYPES.value(elementType, PopupModelType::TYPE_UNDEFINED);
}

void AbstractElementPopupModel::setModelType(PopupModelType modelType)
{
    m_modelType = modelType;
}

void AbstractElementPopupModel::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged();
}

void AbstractElementPopupModel::setElementType(engraving::ElementType type)
{
    m_elementType = type;
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

void AbstractElementPopupModel::endCommand()
{
    if (undoStack()) {
        undoStack()->commitChanges();
    }
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

const INotationInteraction::HitElementContext& AbstractElementPopupModel::hitElementContext() const
{
    if (INotationInteractionPtr interaction = this->interaction()) {
        return interaction->hitElementContext();
    }

    static INotationInteraction::HitElementContext dummy;
    return dummy;
}
