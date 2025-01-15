/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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
#include "internal/partialtiepopupmodel.h"
#include "internal/shadownotepopupmodel.h"
#include "log.h"

using namespace mu::notation;

static const QMap<mu::engraving::ElementType, PopupModelType> ELEMENT_POPUP_TYPES = {
    { mu::engraving::ElementType::HARP_DIAGRAM, PopupModelType::TYPE_HARP_DIAGRAM },
    { mu::engraving::ElementType::CAPO, PopupModelType::TYPE_CAPO },
    { mu::engraving::ElementType::STRING_TUNINGS, PopupModelType::TYPE_STRING_TUNINGS },
    { mu::engraving::ElementType::SOUND_FLAG, PopupModelType::TYPE_SOUND_FLAG },
    { mu::engraving::ElementType::DYNAMIC, PopupModelType::TYPE_DYNAMIC },
    { mu::engraving::ElementType::TIE_SEGMENT, PopupModelType::TYPE_PARTIAL_TIE },
    { mu::engraving::ElementType::PARTIAL_TIE_SEGMENT, PopupModelType::TYPE_PARTIAL_TIE },
    { mu::engraving::ElementType::SHADOW_NOTE, PopupModelType::TYPE_SHADOW_NOTE }
};

static const QHash<PopupModelType, mu::engraving::ElementTypeSet> POPUP_DEPENDENT_ELEMENT_TYPES = {
    { PopupModelType::TYPE_HARP_DIAGRAM, { mu::engraving::ElementType::HARP_DIAGRAM } },
    { PopupModelType::TYPE_CAPO, { mu::engraving::ElementType::CAPO } },
    { PopupModelType::TYPE_STRING_TUNINGS, { mu::engraving::ElementType::STRING_TUNINGS } },
    { PopupModelType::TYPE_SOUND_FLAG, { mu::engraving::ElementType::SOUND_FLAG, mu::engraving::ElementType::STAFF_TEXT } },
    { PopupModelType::TYPE_DYNAMIC, { mu::engraving::ElementType::DYNAMIC } },
    { PopupModelType::TYPE_PARTIAL_TIE, { mu::engraving::ElementType::PARTIAL_TIE_SEGMENT, mu::engraving::ElementType::TIE_SEGMENT } },
};

AbstractElementPopupModel::AbstractElementPopupModel(PopupModelType modelType, QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this)), m_modelType(modelType)
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

bool AbstractElementPopupModel::supportsPopup(const EngravingItem* element)
{
    if (!element) {
        return false;
    }

    const PopupModelType modelType = modelTypeFromElement(element->type());
    if (modelType == PopupModelType::TYPE_UNDEFINED) {
        return false;
    }

    switch (modelType) {
    case PopupModelType::TYPE_PARTIAL_TIE:
        return PartialTiePopupModel::canOpen(element);
    case PopupModelType::TYPE_SHADOW_NOTE:
        return ShadowNotePopupModel::canOpen(element);
    default:
        return true;
    }
}

PopupModelType AbstractElementPopupModel::modelTypeFromElement(const engraving::ElementType& elementType)
{
    return ELEMENT_POPUP_TYPES.value(elementType, PopupModelType::TYPE_UNDEFINED);
}

muse::PointF AbstractElementPopupModel::fromLogical(muse::PointF point) const
{
    return currentNotation()->viewState() ? currentNotation()->viewState()->matrix().map(point) : muse::PointF();
}

muse::RectF AbstractElementPopupModel::fromLogical(muse::RectF rect) const
{
    return currentNotation()->viewState() ? currentNotation()->viewState()->matrix().map(rect) : muse::RectF();
}

INotationUndoStackPtr AbstractElementPopupModel::undoStack() const
{
    return currentNotation() ? currentNotation()->undoStack() : nullptr;
}

void AbstractElementPopupModel::beginCommand(const muse::TranslatableString& actionName)
{
    if (undoStack()) {
        undoStack()->prepareChanges(actionName);
    }
}

void AbstractElementPopupModel::beginMultiCommands(const muse::TranslatableString& actionName)
{
    beginCommand(actionName);

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

    beginCommand(muse::TranslatableString("undoableAction", "Edit element property"));
    m_item->undoChangeProperty(id, value, flags);
    endCommand();
    updateNotation();
}

void AbstractElementPopupModel::changeItemProperty(mu::engraving::Pid id, const PropertyValue& value, mu::engraving::PropertyFlags flags)
{
    IF_ASSERT_FAILED(m_item) {
        return;
    }

    beginCommand(muse::TranslatableString("undoableAction", "Edit element property"));
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
        for (ElementType type : dependentElementTypes()) {
            if (muse::contains(range.changedTypes, type)) {
                emit dataChanged();
                updateItemRect();
            }
        }
    });

    updateItemRect();
}

mu::engraving::ElementType AbstractElementPopupModel::elementType() const
{
    return ELEMENT_POPUP_TYPES.key(m_modelType, ElementType::INVALID);
}

const mu::engraving::ElementTypeSet& AbstractElementPopupModel::dependentElementTypes() const
{
    auto it = POPUP_DEPENDENT_ELEMENT_TYPES.find(m_modelType);
    if (it != POPUP_DEPENDENT_ELEMENT_TYPES.end()) {
        return it.value();
    }

    static const engraving::ElementTypeSet dummy;
    return dummy;
}

void AbstractElementPopupModel::updateItemRect()
{
    const QRect rect = m_item ? fromLogical(m_item->canvasBoundingRect()).toQRect() : QRect();

    if (m_itemRect != rect) {
        m_itemRect = rect;
        emit itemRectChanged(rect);
    }
}
